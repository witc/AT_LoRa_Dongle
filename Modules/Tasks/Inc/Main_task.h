/*
 * main_task.h
 *
 *  Created on: Dec 13, 2023
 *      Author: Uzivatel
 */

#ifndef INC_MAIN_TASK_H_
#define INC_MAIN_TASK_H_

/**
 * @brief 
 * 
 */
typedef enum
{
	MAIN_TASK_OFF = 0,
	MAIN_TASK_ON = 1,

}task_state_e;

/**
 * @brief 
 * 
 */
typedef struct
{
	TimerResource_t	LED_alive;

}main_timers_t;


typedef struct
{
	task_state_e	task_state;
	main_timers_t	timers;

}main_ctx_t;


void main_task(void);
void irq_RELE_falling(void);


#endif /* INC_MAIN_TASK_H_ */
