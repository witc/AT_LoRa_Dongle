/**
 * @file Log.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-11-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#define LOG_LEVEL LOG_LEVEL_VERBOSE
#include "Log.h"

#include <stdarg.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "cmsis_gcc.h"
#include "cmsis_os2.h"

static StaticSemaphore_t semaphore_buffer;
static SemaphoreHandle_t semaphore;
FILE* logOutputStream = NULL;

void LOG_Initialise(void)
{
	semaphore = xSemaphoreCreateRecursiveMutexStatic(&semaphore_buffer);
	configASSERT(semaphore);
	logOutputStream = stdout;
}

int LOG_StdoutLock(void)
{
	if(__get_IPSR())
	{
		#if ENABLE_IRQ_LOGGING
			// This suluotion is not thread safe, logs might be mixed and they are also prolonging ISR but it is better than nothing...
			printf("ISR: ");
			return 1;
		#endif
		// Log inside interrupt is not currently supported
		return 0;
	}

	if(semaphore == NULL)
	{
		// Semaphore is not ready
		return 0;
	}

	if(xTaskGetSchedulerState() != taskSCHEDULER_RUNNING)
	{
		// In suspended mode logging is not supported
		return 0;
	}

	if(xSemaphoreTakeRecursive(semaphore, pdMS_TO_TICKS(3000)) == pdFALSE)
	{
		// Timeout, skip log
		return 0;
	}

	// Logging ready
	return 1;
}

void LOG_StdoutUnlock(void)
{
	#if ENABLE_IRQ_LOGGING
	if(__get_IPSR())
	{
		return;
	}
	#endif
	xSemaphoreGiveRecursive(semaphore);
}

/**
 * @brief Enable log prints. Note that this affects only prints using global logOutputStream!
 * 
 */
void LOG_GeneralEnable(void)
{
	logOutputStream = stdout;
}

/**
 * @brief Disable log prints. Note that this affects only prints using global logOutputStream!
 * 
 */
void LOG_GeneralDisable(void)
{
	logOutputStream = NULL;
}

#if (LOG_WITH_TIMESTAMP == 1)
/**
 * @brief Get the system time object
 * 
 * @return SystemTime 
 */
SystemTime get_log_timeStamp()
{
    unsigned long total_milliseconds = osKernelGetTickCount();
    SystemTime time;
    
    time.hours = total_milliseconds / 3600000;
    total_milliseconds %= 3600000;
    
    time.minutes = total_milliseconds / 60000;
    total_milliseconds %= 60000;
    
    time.seconds = total_milliseconds / 1000;
    time.milliseconds = total_milliseconds % 1000;
    
    return time;
}

#endif
/**
 * @}
 *
 */
