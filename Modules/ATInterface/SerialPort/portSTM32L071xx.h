/**
 * @file portSTM32L071xx.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-11-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef PORTSTM32L071XX_H
#define PORTSTM32L071XX_H

#include "main.h"

#define MAX_UART_RX_BUFFER 550
#define MAX_UART_TX_BUFFER 550

typedef struct 
{
    uint8_t *raw_data;
    uint16_t size;

}__attribute__((packed)) RAW_DATA_Storage_t;


typedef struct
{
	UART_HandleTypeDef *phuart;			//!< Pointer to UART handle.
	RAW_DATA_Storage_t rxStorage;				//!< Rx storage instance.
	RAW_DATA_Storage_t txStorage;				//!< Tx storage instance.

}__attribute__((packed)) SP_Context_t;	//Serial Port ctx


bool SP_PlatformInit(SP_Context_t *sp_ctx);
HAL_StatusTypeDef SP_HandleUARTError(SP_Context_t *sp_ctx);
bool SP_RxComplete(SP_Context_t *sp_ctx, uint16_t size);


#endif // PORTSTM32L071XX_H