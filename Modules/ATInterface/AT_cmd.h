/**
 * @file AT_cmd.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-11-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef AT_CMD_H
#define AT_CMD_H

#include "portSTM32L071xx.h"

/**
 * @brief 
 * 
 */
typedef enum
{   
    SYS_CMD_NONE            = 0,
    SYS_CMD_SYS_STATE       = 1,
    SYS_CMD_SYS_NAME        = 2,
    SYS_CMD_RF_MAC          = 3,
    SYS_CMD_FACTORY_RST     = 4,
    SYS_CMD_TX_CW           = 5,    
    SYS_CMD_LORA_RX_TO_UART = 6,
    SYS_CMD_LORA_SEND       = 7,
    SYS_CMD_RF_PAIR         = 8,

}eSystemCommands;

/**
 * @brief 
 * 
 */
typedef struct AT_cmd
{
    eSystemCommands cmd;
    uint8_t         dataBuff[300];

}AT_cmd_t;

void UART_SendResponse(char *response);
void AT_HandleATCommand(SP_Context_t *sp_ctx, uint16_t size);

#endif // AT_CMD_H

