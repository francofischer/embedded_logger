#include <stdio.h>
#include <string.h>
#include <time.h>

#include "logger.h"

typedef struct
{
	char message[MAX_MESSAGE_LENGTH]; 	/**< Mensaje que el programador quiere registrar 	*/
	eLogSubSystem subSystem;  			/**< Subsistema que origina el mensaje 				*/
	eLogLevel logLevel;  				/**< Nivel de importancia del mensaje 				*/

#if defined(TIME_SUPPORT)
	time_t logTime;						/**< Hora a la que se produjo el evento 			*/
#endif

	bool flash_write;


} LogMessage_t;

//Almaceno que nivel de error voy a logear por cada subsistema.
static eLogLevel outputLevel[MAX_NUMBER_SUBSYSTEMS] = {0};

//Punteros a funcion que retorna el tiempo y "imprime" o persiste el log.
//timeNow retorna el numero de segundos desde 01-01-1970 00:00:00
static void (*printLog)(char *) = NULL;
#if defined(TIME_SUPPORT)
static time_t (*timeNow)() = NULL;
#endif
static void (*writeFlash)(char *) = NULL;

/* ----------------------------------- */
/* Funciones para manejar el buffer    */
/* ----------------------------------- */
//Buffer circular donde alamaceno los mensajes y sus respectivos indices.
static LogMessage_t ringBuffer[MAX_QUEUE_LENGTH]; 
static size_t ringBufferTip, ringBufferTail, ringBufferCount = 0;

//Tipos de errores que devuelve el buffer circular.
typedef enum
{
	eRB_NO_ERROR = 0,
	eRB_FULL,
	eRB_EMPTY,
	eRB_NULL,	
} eRB_Status;

//Funcion para agregar mensajes al buffer.
inline static eRB_Status ringBuffer_put(LogMessage_t *dataToPut)
{       
	//Copio el mensaje al buffer.
    memcpy(ringBuffer + ringBufferTip, dataToPut, sizeof(LogMessage_t)); 
    
	//Si llegue al final, vuelvo a arrancar del principio. Sino, aumento en uno el indice.
	ringBufferTip = (ringBufferTip + 1) % MAX_QUEUE_LENGTH;

	//Aviso que se lleno el buffer, pero el indice aumento igual asi que voy a empezar a pisar mensajes viejos.
	//Aumento el Tail para siempre recuperar en orden (mantener el FIFO).
	//Si no queres que pase esto, podes copiar esto antes de incrementar ringBufferTip.
    if (ringBufferCount == MAX_QUEUE_LENGTH)
	{
		ringBufferTail = (ringBufferTail + 1) % MAX_QUEUE_LENGTH;
        return eRB_FULL;
	}
    else
    	ringBufferCount++;

    return eRB_NO_ERROR;
}

//Funcion para obtener mensajes del buffer.
inline static eRB_Status ringBuffer_get(LogMessage_t *dataToGet)
{
    if (ringBufferCount == 0)
        return eRB_EMPTY;
    
	//Te copio el elemento del buffer.
    memcpy(dataToGet, ringBuffer + ringBufferTail, sizeof(LogMessage_t));
    
	//Si llegue al final, vuelvo a arrancar del principio. Sino, aumento en uno el indice.
    ringBufferTail = (ringBufferTail + 1) % MAX_QUEUE_LENGTH;
    
	ringBufferCount--;

    return eRB_NO_ERROR;
}

/* ----------------------------------- */


void logger_log(const eLogSubSystem subSystem, const eLogLevel level, const char *message, bool flash_write)
{
	//Solo voy a almacenar el mensaje si el nivel de importancia es igual o mas alto que el nivel que configuraste.
	if(level >= outputLevel[subSystem])
	{
		LogMessage_t temporal;

		//Copio el mensaje y me aseguro que al menos se copie un '\0'
		strncpy(temporal.message, message, MAX_MESSAGE_LENGTH);
		temporal.message[MAX_MESSAGE_LENGTH - 1] = '\0';

		temporal.logLevel = level;
		temporal.subSystem = subSystem;

		temporal.flash_write = flash_write;

		//Ojo con los punteros nulos...
#if defined(TIME_SUPPORT)
        if (timeNow != NULL)
		    temporal.logTime = timeNow();
		else
			temporal.logTime = 0;
#endif

		ringBuffer_put(&temporal);
	}
}

static void logger_get_message(const LogMessage_t element, char *logOutput)
{
	if(element.logLevel >= outputLevel[element.subSystem])
	{	
		//Formateo la cantidad de segundos desde 01-01-1970 00:00:00 a dia y hora
#if defined(TIME_SUPPORT)
		char formatedTime[20];
		strftime(formatedTime, 20,"%Y-%m-%d %H:%M:%S", gmtime(&element.logTime));
		
		//2022-08-06 03:53:07, SPI, [INFO]: Mensaje
		sprintf(logOutput, "%s, [%s]\t%s:\t%s\r\n", formatedTime, namesLogLevel[element.logLevel], namesSubSystem[element.subSystem], element.message);
#else
		sprintf(logOutput, "[%s]\t%s:\t%s\r\n", namesLogLevel[element.logLevel], namesSubSystem[element.subSystem], element.message);
#endif
	}
	else	
	    sprintf(logOutput, "\r");
}

void logger_set_output_level(const eLogSubSystem subSystem, const eLogLevel level)
{
	outputLevel[subSystem] = level;
}

void logger_global_on()
{
	for (size_t i = 0; i < MAX_NUMBER_SUBSYSTEMS; i++)
		outputLevel[i] = eDEBUG;
}

void logger_enable_subsystem(const eLogSubSystem subSystem)
{
	outputLevel[subSystem] = eDEBUG;
}

void logger_global_off()
{
	for (size_t i = 0; i < MAX_NUMBER_SUBSYSTEMS; i++)
		outputLevel[i] = eNONE;
}

void logger_disable_subsystem(const eLogSubSystem subSystem)
{
	outputLevel[subSystem] = eNONE;
}

void logger_flush()
{
    char salidaMensaje[MAX_PRINT_OUTPUT] = {0};
    LogMessage_t LogMessage = {0};

    while (ringBuffer_get(&LogMessage) != eRB_EMPTY)
    {
        logger_get_message(LogMessage, salidaMensaje);
        
		if (printLog != NULL)
            printLog(salidaMensaje);

		if (writeFlash != NULL && LogMessage.flash_write == true)
          writeFlash(salidaMensaje);
    }
}

#if defined(TIME_SUPPORT)
void logger_init(void (*ptrPrint)(char *), time_t (*ptrTime)(), void (*ptrWriteFlash)(char *))
{
	if (ptrTime != NULL)
	timeNow = ptrTime;

	if (ptrPrint != NULL)
	printLog = ptrPrint;

	if (ptrWriteFlash != NULL)
	writeFlash = ptrWriteFlash;

}
#else
void logger_init(void (*ptrPrint)(char *), void (*ptrWriteFlash)(char *))
{
	if (ptrPrint != NULL)
		printLog = ptrPrint;

	if (ptrWriteFlash != NULL)
		writeFlash = ptrWriteFlash;
}
#endif
