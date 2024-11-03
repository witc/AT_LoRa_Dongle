/*
 * main_task.c
 *
 *  Created on: Dec 13, 2023
 *      Author: Uzivatel
 */

#include "main.h"
#include "main_task.h"
#include "radio_user.h"
//#include "process_main_task.h"


extern osMessageQId queueRadioHandle;
extern osMessageQId queueMainHandle;

void main_task_off(main_ctx_t *ctx,dataQueue_t *rxd);
void main_task_on(main_ctx_t *ctx,dataQueue_t *rxd);

volatile uint32_t GL_debug=0;

void (*main_task_states[2])(main_ctx_t *ctx, dataQueue_t *rxd) = {main_task_off, main_task_on};


/**
 * @brief 
 * 
 * @param xTimer 
 */
static void _Main_Alive_Callback(TimerHandle_t xTimer)
{
    static uint8_t pattern_cnt = 0;

    // Definice patternu (čas v milisekundách) - odpovídá patternu X_X_______
    const uint32_t pattern[] = {100,100,100,1000};
    const uint8_t pattern_length = sizeof(pattern) / sizeof(pattern[0]);

    // Nastavení periody podle aktuálního stavu patternu
    xTimerChangePeriod(xTimer, pdMS_TO_TICKS(pattern[pattern_cnt]), portMAX_DELAY);

    // Přepnutí LED
    HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);

    // Posun na další stav patternu (cyklicky)
    pattern_cnt = (pattern_cnt + 1) % pattern_length;
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
void main_task_on(main_ctx_t *ctx,dataQueue_t *rxd)
{
	packet_info_t	*rx_pkt;

	switch (rxd->cmd)
	{
		case CMD_MAIN_RF_RX_PACKET:
			rx_pkt = rxd->ptr;
			//prt_decode_rx_data(rx_pkt->packet, rx_pkt->size, ctx);

			vPortFree(rx_pkt->packet);
			rx_pkt->packet=NULL;

			break;

		default:
			break;
	}

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

	HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, true);
	HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, true);

	ctx.timers.LED_alive.timer = xTimerCreateStatic("LED alive timer", pdMS_TO_TICKS(100), pdFALSE, NULL, 
														 _Main_Alive_Callback,  &ctx.timers.LED_alive.timerPlace);

	osTimerStart(ctx.timers.LED_alive.timer, pdMS_TO_TICKS(100));
	HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, false);

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

