#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <stdbool.h>

#define TIME_SUPPORT
//#undef TIME_SUPPORT

/** Tamaño maximo de la cola de eventos, ajustar segun RAM disponible */
#define MAX_QUEUE_LENGTH 20U
/** Tamaño maximo del mensaje a almacenar, ajustar segun necesidad */
#define MAX_MESSAGE_LENGTH 64U

/** Tamaño maximo de cada linea impresa. Salvo que el nombre del subsistema sea muy largo deberia alcanzar.*/
#define MAX_PRINT_OUTPUT (MAX_MESSAGE_LENGTH + 64U)

/** Enumerador de subsistemas. 
* Este enumerador contiene todos los subsistemas que va a manejar el logger. Se pueden agregar/quitar subsitemas, 
* pero hay que tener en cuenta de agregar el nombre en el arreglo namesSubSystem. Ademas, MAX_NUMBER_SUBSYSTEMS siempre
* tiene que ser el ultimo elemento.
*/
typedef enum
{
	eCORE = 0,
	eSPI,
	eI2C,
	eUART,
	eADC,
	ePWM,
	eTIMER1,
	eTIMER2,
	eTIMER3,
	MAX_NUMBER_SUBSYSTEMS /**< Solo utilizado para conocer la cantidad maxima de subsistemas (no usar en logger_log) */
} eLogSubSystem;

static const char *namesSubSystem[] = {"CORE", "SPI", "I2C", "UART", "ADC", "PWM", "TIMER1", "TIMER2", "TIMER3"};

/** Enumerador de niveles. 
* Usado para indicar la criticidad del mensaje. No agregar ni quitar niveles asi usamos niveles comunes. 
*/
typedef enum
{
	eDEBUG = 0,
	eINFO,  	
	eWARNING,	
	eERROR,		
	eCRITICAL,	
	eNONE		/**< Solo utilizado para deshabilitar el subsistema (no usar en logger_log) */
} eLogLevel;

static const char *namesLogLevel[] = {"DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"};

/** Iniciar la libreria de log.
 *
 * Esta funcion permite inicializar la libreria de log, recibe tres punteros a funcion para poder "imprimir"
 * la informacion, para poder recuperar la hora actual y para poder persistir la informacion. (Si no se cuenta con RTC se 
 * puede utilizar las funciones estandar de time.h time(NULL), clock() o cualquier otra funcion que retorne la cantidad de segundos desde
 * que inicio el sistema.)
 *
 * @param [in] ptrPrint Puntero a la funcion que muestra la informacion (ej: HAL_UART_WRITE()). Recibe como parametro
 * 						un puntero a char donde retorna un string con hora, subsystema, nivel de error y mensaje. 
 * @param [in] ptrTime Puntero a la funcion que devuelve la cantidad de segundos desde 01-01-1970 00:00:00
 * @param [in] ptrWriteFlash Puntero a la funcion que persiste la informacion en flash. Recibe como parametro
 * 							 un puntero a char donde retorna un string con hora, subsystema, nivel de error y mensaje. 
 */
#if defined(TIME_SUPPORT)
void logger_init(void (*ptrPrint)(char *), time_t (*ptrTime)(), void (*ptrWriteFlash)(char *));
#else
void logger_init(void (*ptrPrint)(char *));
#endif

/** Registrar evento en el log
 *
 * Esta funcion registra un evento en el log. Solo agrega un elemento al buffer circular de tamaño maximo MAX_QUEUE_LENGTH.
 * El mensaje a almacenar tiene tamaño maximo de MAX_MESSAGE_LENGTH.
 *
 * @param [in] subSystem Subsitema que origina el mensaje. Ver el enum eLogSubSystem para conocer los subsistemas disponibles.
 * @param [in] level Nivel de importancia del error o mensaje. Ver el enum eLogLevel para conocer los distintos niveles.
 * @param [in] message Mensaje a registrar. Maxima longitud MAX_MESSAGE_LENGTH.
 * @param [in] flash_write Indico si quiero grabar el mensaje en flash (flash_write = true) o solo imprimirlo.
 */
void logger_log(const eLogSubSystem subSystem, const eLogLevel level, const char *message, bool flash_write);

/** Nivel de error a registrar
 *
 * Esta funcion permite establecer a cada subsistema el nivel a partir del cual el mensaje o error 
 * se va a almacenar en el logger. Se van a registrar todos los eventos iguales o mayores que el nivel que se fije.
 *
 * @param [in] subSystem A que subsistema se le va a fijar el nivel. Ver el enum eLogSubSystem para conocer los subsistemas disponibles.
 * @param [in] level A partir de que nivel se va a guardar el mensaje. Ver el enum eLogLevel para conocer los distintos niveles.
 */
void logger_set_output_level(const eLogSubSystem subSystem, const eLogLevel level);

/** Activar todos los subsitemas y todos los niveles.
 *
 */
void logger_global_on();

/** Activo solo un subsistema
*
 * Solo activo el subsistema pasado por parametro. Los otros quedan como estan.
 * 
 * @param [in] subSystem Que subsistema voy a activar.
 */
void logger_enable_subsystem(const eLogSubSystem subSystem);

/** Desactivar todos los subsitemas y todos los niveles.
 *
 */
void logger_global_off();

/** Desactivo solo un subsistema
 *
 * Solo desactivo el subsistema pasado por parametro. Los otros quedan como estan.
 * 
 * @param [in] subSystem Que subsistema voy a desactivar.
 */
void logger_disable_subsystem(const eLogSubSystem subSystem);

/** Persistir el log.
 *
 * Esta funcion persiste los mensajes del log llamando al puntero a funcion ptrPrint que se paso en logger_init.
 * La idea es llamar a esta funcion cuando el sistema tenga tiempo, asi el logger no perjudica la performance del sistema.
 *
 */
void logger_flush();

#endif

