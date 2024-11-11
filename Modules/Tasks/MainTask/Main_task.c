/*
 * main_task.c
 *
 *  Created on: Dec 13, 2023
 *      Author: Uzivatel
 */

#include "main.h"
#include "main_task.h"
#include "radio_user.h"
#include "semphr.h"
#include "AT_cmd.h"
#include "general_sys_cmd.h"

#define LOG_LEVEL	LOG_LEVEL_VERBOSE
#include "Log.h"
//#include "process_main_task.h"

// Struktura pro uložení statického semaforu
StaticSemaphore_t xSemaphoreBuffer_USART;
SemaphoreHandle_t xBinarySemaphore_USART;

extern UART_HandleTypeDef huart1;
extern osMessageQId queueRadioHandle;
extern osMessageQId queueMainHandle;

void main_task_off(main_ctx_t *ctx,dataQueue_t *rxd);
void main_task_on(main_ctx_t *ctx,dataQueue_t *rxd);

volatile uint32_t GL_debug=0;

void (*main_task_states[2])(main_ctx_t *ctx, dataQueue_t *rxd) = {main_task_off, main_task_on};


static uint8_t rxBuffer_USART[MAX_UART_RX_BUFFER];
static uint8_t txBuffer_USART[MAX_UART_RX_BUFFER];

static uint8_t rxShadowBuffer_USART[MAX_UART_RX_BUFFER];

static SemaphoreHandle_t _Main_QueueSemaphore;
static StaticSemaphore_t _Main_QueueSemaphoreBuffer;


/**
 * @brief 
 * 
 * @param xTimer 
 */
static void _Main_Alive_Callback(TimerHandle_t xTimer)
{
    static uint8_t pattern_cnt = 0;

    // Definice patternu (čas v milisekundách) - odpovídá patternu X_X______
    const uint32_t pattern[] = {100,150,100,1000};
    const uint8_t pattern_length = sizeof(pattern) / sizeof(pattern[0]);

    xTimerChangePeriod(xTimer, pdMS_TO_TICKS(pattern[pattern_cnt]), portMAX_DELAY);

    HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);

    //Next step
    pattern_cnt = (pattern_cnt + 1) % pattern_length;
}


/**
 * @brief 
 * 
 * @param data 
 * @param atCmd 
 * @param size 
 * @return true 
 * @return false 
 */
bool AT_CustomCommandHandler(char *data,eATCommands atCmd, uint16_t size)
{
	dataQueue_t txm;
	txm.ptr = NULL;

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if(xSemaphoreTakeFromISR(xBinarySemaphore_USART, &xHigherPriorityTaskWoken) == pdTRUE)
	{
		memcpy(rxShadowBuffer_USART, data, size);
		txm.cmd = CMD_MAIN_AT_RX_PACKET;
		txm.tmp_8 = (uint8_t) atCmd;
		txm.tmp_16 = size;
		MT_SendDataToMainTask(&txm);

		//xSemaphoreGiveFromISR(xBinarySemaphore_USART, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	else
	{
		return false;
	}

	return true;
}

/**
 * @brief 
 * 
 * @param ctx 
 * @param rxd 
 */
void main_task_off(main_ctx_t *ctx,dataQueue_t *rxd)
{


}



/**
 * @brief 
 * 
 * @param ctx 
 * @param rxd 
 */
void main_task_on(main_ctx_t *ctx, dataQueue_t *rxd)
{
	packet_info_t	*rx_pkt;
	bool 	AtCmdProcessed;

	switch (rxd->cmd)
	{
		case CMD_MAIN_RF_RX_PACKET:
			rx_pkt = rxd->ptr;
			//prt_decode_rx_data(rx_pkt->packet, rx_pkt->size, ctx);

			vPortFree(rx_pkt->packet);
			rx_pkt->packet=NULL;

			break;

		case CMD_MAIN_AT_RX_PACKET:
			AtCmdProcessed = GSC_ProcessCommand((eATCommands) rxd->tmp_8, rxShadowBuffer_USART, rxd->tmp_16);
			xSemaphoreGive(xBinarySemaphore_USART);
			if(AtCmdProcessed == true)	
			{
				UART_SendResponse("OK\r\n");
			}
			else
			{
				UART_SendResponse("ERROR\r\n");
			}
			
			break;

		default:
			break;
	}

}

/**
 * @brief 
 * 
 * @param data 
 * @return true 
 * @return false 
 */
bool MT_SendDataToMainTask(dataQueue_t *data)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t result;

    // Zkontrolujeme, zda je voláno z ISR
    if (__get_IPSR())
    {
        // Pokud jsme v ISR, pokusíme se získat semafor bez blokování
        if (xSemaphoreTakeFromISR(_Main_QueueSemaphore, &xHigherPriorityTaskWoken) == pdTRUE)
        {
            // Semafor je volný, můžeme poslat data do fronty
            result = xQueueSendFromISR(queueMainHandle, data, &xHigherPriorityTaskWoken);

            // Uvolníme semafor pro další přístup
            xSemaphoreGiveFromISR(_Main_QueueSemaphore, &xHigherPriorityTaskWoken);

            // Přepneme kontext, pokud je to nutné
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        else
        {
            // Semafor nebyl volný - fronta je momentálně obsazena
            return false;
        }
    }
    else
    {
        // Mimo ISR - čekáme na získání semaforu
        if (xSemaphoreTake(_Main_QueueSemaphore, portMAX_DELAY) == pdTRUE)
        {
            // Semafor je získán, posíláme data do fronty
            result = xQueueSend(queueMainHandle, data, portMAX_DELAY);

            // Uvolníme semafor
            xSemaphoreGive(_Main_QueueSemaphore);
        }
        else
        {
            // Semafor nebyl získán, vracíme chybu
            return false;
        }
    }

    return (result == pdTRUE);
}


/**
 * @brief 
 * 
 */
void main_task(void)
{
	dataQueue_t	rxd;
	BaseType_t ret;
	main_ctx_t ctx;
	ctx.task_state = MAIN_TASK_ON;

	HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, false);
	HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, false);

	_Main_QueueSemaphore = xSemaphoreCreateMutexStatic(&_Main_QueueSemaphoreBuffer);

	ctx.timers.LED_alive.timer = xTimerCreateStatic("LED alive timer", pdMS_TO_TICKS(100), pdFALSE, NULL, 
														 _Main_Alive_Callback,  &ctx.timers.LED_alive.timerPlace);

	osTimerStart(ctx.timers.LED_alive.timer, pdMS_TO_TICKS(100));

	/* init and start recieve*/

	 if (xBinarySemaphore_USART == NULL)
    {
        // Vytvoření semaforu
        xBinarySemaphore_USART = xSemaphoreCreateBinaryStatic(&xSemaphoreBuffer_USART);

        if (xBinarySemaphore_USART != NULL)
        {
            xSemaphoreGive(xBinarySemaphore_USART);
        }
    }

	AT_cmd_t at_ctx;
	at_ctx.sp_ctx.rxStorage.raw_data = rxBuffer_USART;
	at_ctx.sp_ctx.rxStorage.size = MAX_UART_RX_BUFFER;
	at_ctx.sp_ctx.txStorage.raw_data = txBuffer_USART;
	at_ctx.sp_ctx.txStorage.size = MAX_UART_TX_BUFFER;
	at_ctx.sp_ctx.phuart = &huart1;
	// Assign the custom command handler to be called when data is received from ISR
	at_ctx.onDataReceivedFromISR = AT_CustomCommandHandler;
    AT_Init(&at_ctx);


	LOG_DEBUG("Main task started");

	for(;;)
	{
		ret = xQueueReceive(queueMainHandle, &rxd, portMAX_DELAY);
		if (ret == pdPASS)
		{
			main_task_states[ctx.task_state](&ctx, &rxd);

			/* Clear malloc */
			vPortFree(rxd.ptr);
			rxd.ptr=NULL;
		}

	}
}

