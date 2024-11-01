/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "RF_Task.h"
#include "Main_task.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for TaskMain */
osThreadId_t TaskMainHandle;
uint32_t TaskCoreBuffer[ 256 ];
osStaticThreadDef_t TaskCoreControlBlock;
const osThreadAttr_t TaskMain_attributes = {
  .name = "TaskMain",
  .cb_mem = &TaskCoreControlBlock,
  .cb_size = sizeof(TaskCoreControlBlock),
  .stack_mem = &TaskCoreBuffer[0],
  .stack_size = sizeof(TaskCoreBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TaskRF */
osThreadId_t TaskRFHandle;
uint32_t TaskRFBuffer[ 256 ];
osStaticThreadDef_t TaskRFControlBlock;
const osThreadAttr_t TaskRF_attributes = {
  .name = "TaskRF",
  .cb_mem = &TaskRFControlBlock,
  .cb_size = sizeof(TaskRFControlBlock),
  .stack_mem = &TaskRFBuffer[0],
  .stack_size = sizeof(TaskRFBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for queueRadio */
osMessageQueueId_t queueRadioHandle;
uint8_t queueRadioBuffer[ 16 * sizeof( dataQueue_t ) ];
osStaticMessageQDef_t queueRadioControlBlock;
const osMessageQueueAttr_t queueRadio_attributes = {
  .name = "queueRadio",
  .cb_mem = &queueRadioControlBlock,
  .cb_size = sizeof(queueRadioControlBlock),
  .mq_mem = &queueRadioBuffer,
  .mq_size = sizeof(queueRadioBuffer)
};
/* Definitions for queueMain */
osMessageQueueId_t queueMainHandle;
uint8_t queueMainBuffer[ 16 * sizeof( dataQueue_t ) ];
osStaticMessageQDef_t queueMainControlBlock;
const osMessageQueueAttr_t queueMain_attributes = {
  .name = "queueMain",
  .cb_mem = &queueMainControlBlock,
  .cb_size = sizeof(queueMainControlBlock),
  .mq_mem = &queueMainBuffer,
  .mq_size = sizeof(queueMainBuffer)
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartTaskCore(void *argument);
void StartTaskRF(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
return 0;
}
/* USER CODE END 1 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of queueRadio */
  queueRadioHandle = osMessageQueueNew (16, sizeof(dataQueue_t), &queueRadio_attributes);

  /* creation of queueMain */
  queueMainHandle = osMessageQueueNew (16, sizeof(dataQueue_t), &queueMain_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of TaskMain */
  TaskMainHandle = osThreadNew(StartTaskCore, NULL, &TaskMain_attributes);

  /* creation of TaskRF */
  TaskRFHandle = osThreadNew(StartTaskRF, NULL, &TaskRF_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartTaskCore */
/**
  * @brief  Function implementing the TaskCore thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartTaskCore */
void StartTaskCore(void *argument)
{
  /* USER CODE BEGIN StartTaskCore */
  /* Infinite loop */
  main_task();
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTaskCore */
}

/* USER CODE BEGIN Header_StartTaskRF */
/**
* @brief Function implementing the TaskRF thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskRF */
void StartTaskRF(void *argument)
{
  /* USER CODE BEGIN StartTaskRF */
  //RF_TaskEntry();
  radio_task();
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTaskRF */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

