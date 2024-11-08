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

void UART_SendResponse(char *response);
void AT_HandleATCommand(SP_Context_t *sp_ctx, uint8_t size);

#endif // AT_CMD_H

