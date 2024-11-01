/*
 * RF1_Task.c
 *
 *  Created on: Nov 14, 2023
 *      Author: Uzivatel
 */

#include "main.h"
#include "RF_Task.h"
//#include "radio_user.h"



// extern osMessageQueueId_t queueRadioHandle;
// extern osMessageQueueId_t  queueCoreHandle;

// void radio_task_off(radio_context_t *glData,dataQueue_t *rxd);
// void radio_task_on(radio_context_t *glData,dataQueue_t *rxd);


// void (*radio_states[2])(radio_context_t *glData, dataQueue_t *rxd) = {radio_task_off, radio_task_on};


// /*
//  *
//  */
// void radio_task_off(radio_context_t *glData,dataQueue_t *rxd)
// {
// 	dataQueue_t	sd;
// 	sd.ptr = NULL;

// 	switch (rxd->cmd)
// 	{
// 		case CMD_RF_TURN_ON:
// 			ru_radio_process_commands(0,glData,RF_CMD_INIT_ON);
// 			ru_radio_process_commands(1,glData,RF_CMD_INIT_ON);

// 			ru_radio_process_commands(0,glData,RF_CMD_START_RX);
// 			ru_radio_process_commands(1,glData,RF_CMD_START_RX);

// 			sd.cmd = CMD_CORE_RF_IS_ON;
// 			xQueueSend(queueCoreHandle,&sd,portMAX_DELAY);

// 			glData->rfTaskState.previousState = glData->rfTaskState.currentState;
// 			glData->rfTaskState.currentState = RF_TASK_ON;
// 			break;

// 		case CMD_RF_TURN_OFF:
// 			ru_radio_process_commands(0,glData,RF_CMD_INIT_OFF);
// 			ru_radio_process_commands(1,glData,RF_CMD_INIT_OFF);

// 			sd.cmd = CMD_CORE_RF_IS_OFF;
// 			xQueueSend(queueCoreHandle,&sd,portMAX_DELAY);

// 			glData->rfTaskState.previousState = glData->rfTaskState.currentState;
// 			glData->rfTaskState.currentState = RF_TASK_OFF;
// 			break;

// 		default:
// 			break;
// 	}
// }


// /*
//  *
//  */
// void radio_task_on(radio_context_t *glData, dataQueue_t *rxd)
// {
// 	dataQueue_t	sd;
// 	sd.ptr = NULL;
// 	uint8_t rad;

// 	switch (rxd->cmd)
// 	{
// 		case CMD_RF_TURN_ON:
// 			ru_radio_process_commands(0,glData,RF_CMD_INIT_ON);
// 			ru_radio_process_commands(1,glData,RF_CMD_INIT_ON);

// 			ru_radio_process_commands(0,glData,RF_CMD_START_RX);
// 			ru_radio_process_commands(1,glData,RF_CMD_START_RX);

// 			sd.cmd = CMD_CORE_RF_IS_ON;
// 			xQueueSend(queueCoreHandle,&sd,portMAX_DELAY);

// 			glData->rfTaskState.previousState = glData->rfTaskState.currentState;
// 			glData->rfTaskState.currentState = RF_TASK_ON;
// 			break;

// 		case CMD_RF_TURN_OFF:
// 			ru_radio_process_commands(0,glData,RF_CMD_INIT_OFF);
// 			ru_radio_process_commands(1,glData,RF_CMD_INIT_OFF);

// 			sd.cmd = CMD_CORE_RF_IS_OFF;
// 			xQueueSend(queueCoreHandle,&sd,portMAX_DELAY);

// 			glData->rfTaskState.previousState = glData->rfTaskState.currentState;
// 			glData->rfTaskState.currentState = RF_TASK_OFF;
// 			break;

// 		case CMD_RF_IRQ_FIRED:

// 			ru_radio_process_IRQ(glData);


// 			break;

// 		case CMD_RF_SEND_DATA_NOW:

// 			ru_radio_send_packet(rxd->ptr,rxd->data, glData);


// 			break;

// 		default:
// 			break;
// 	}

// }



/*
 *
 */
void radio_task(void)
{

	while (1)
	{
		/* code */
	}
	
	// dataQueue_t rxd;
	// BaseType_t ret;
	// radio_context_t glData;
	// glData.rfTaskState.currentState = RF_TASK_ON;
	// glData.rfTaskState.previousState = RF_TASK_ON;

	// ru_sx1262_assign(&glData);

	// if(ru_radioInit(&glData) == false)	log_error(48351);

	// ru_radio_process_commands(0,&glData,RF_CMD_START_RX);

	// for(;;)
	// {

	// 	ret = xQueueReceive(queueRadioHandle, &rxd, portMAX_DELAY);
	// 	if (ret == pdPASS)
	// 	{
	// 		radio_states[glData.rfTaskState.currentState](&glData, &rxd);

	// 		/* Clear malloc */
	// 		vPortFree(rxd.ptr);
	// 		rxd.ptr=NULL;
	// 	}


	// }
}




