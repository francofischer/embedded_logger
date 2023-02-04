## Libreria de log para sistemas embebidos

Esta libreria permite registrar mensajes de diferentes subsitemas con diferentes niveles de importancia.

Se pueden agregar más subsistemas con tan solo modificar el enum eSubSystems.

El tamaño maximo del buffer circular esta definido en logger.h

Ejemplo de uso:

```
#include <stdio.h>
#include <time.h>

#include "logger.h"

time_t horaActual()
{
	//Si poseemos RTC, colocar la funcion que devuelve la cantidad de segundos desde UNIX EPOCH aca.
    
	//Retorna la hora en formato UNIX standard (time.h)
    return time(NULL);
    
    //Sino, podemos usar clock() / CLOCKS_PER_SEC que nos devuelve la cantidad de segundos desde que inicio el sistema.
    //return (clock() / CLOCKS_PER_SEC);
}

void imprimirLog(char *mensaje)
{
    printf("%s", mensaje);
}

void write_log(char *mensaje)
{
	//Funcion que persiste el parametro en FLASH o memoria no volatil
}

int main()
{

	//Debemos pasarle los punteros a la funcion donde vamos a imprimir o persistir los mensajes y la funcion que devuelve la hora.
    logger_init(imprimirLog, horaActual, write_log);

    logger_log(eSPI, eINFO, "Primer elemento de la cola", false);
    logger_log(eSPI, eCRITICAL, "Segundo elemento de la cola", false);
    logger_log(eSPI, eDEBUG, "Tercer elemento de la cola", false);
    logger_log(eSPI, eWARNING, "Cuarto elemento de la cola", false);
    logger_log(eSPI, eDEBUG, "Quinto elemento de la cola", false);
    logger_log(eSPI, eCRITICAL, "Sexto elemento de la cola", false);
    logger_log(eUART, eCRITICAL, "Septimo elemento de la cola", false);
    logger_log(eUART, eINFO, "Octavo elemento de la cola", false);
    
    logger_global_on();
    
    logger_flush();
    
    return 0;
}
```

La salida del mismo seria algo como:

```
20220806 04:15:03, SPI, [INFO]: Primer elemento de la cola
20220806 04:15:03, SPI, [CRITICAL]: Segundo elemento de la cola
20220806 04:15:03, SPI, [DEBUG]: Tercer elemento de la cola
20220806 04:15:03, SPI, [WARNING]: Cuarto elemento de la cola
20220806 04:15:03, SPI, [DEBUG]: Quinto elemento de la cola
20220806 04:15:03, SPI, [CRITICAL]: Sexto elemento de la cola
20220806 04:15:03, UART, [CRITICAL]: Septimo elemento de la cola
20220806 04:15:03, UART, [INFO]: Octavo elemento de la cola
```

En caso de no necesitar almacenar el tiempo o que no este implementada la libreria time.h en el SDK cambiar el #define TIME_SUPPORT en logger.h a #undef TIME_SUPPORT

En caso de no querer persistir el log en alguna memoria, se puede pasar un punturo nulo en log_init