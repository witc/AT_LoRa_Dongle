#ifndef PORTSTM32L071XX_H
#define PORTSTM32L071XX_H

#include "main.h"

#define MAX_UART_RX_BUFFER 255
#define MAX_UART_TX_BUFFER 255

typedef struct 
{
    uint8_t *raw_data;
    uint8_t size;

}RAW_DATA_Storage_t;


typedef struct
{
	UART_HandleTypeDef *phuart;			//!< Pointer to UART handle.
	RAW_DATA_Storage_t rxStorage;				//!< Rx storage instance.
	RAW_DATA_Storage_t txStorage;				//!< Tx storage instance.

} SP_Context_t;	//Serial Port ctx


bool SP_PlatformInit(SP_Context_t *sp_ctx);
HAL_StatusTypeDef SP_HandleUARTError(SP_Context_t *sp_ctx);


#endif // PORTSTM32L071XX_H