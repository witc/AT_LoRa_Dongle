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


void UART_SendResponse(char *response);
void ProcessATCommand(char *data, uint8_t size);

#endif // AT_CMD_H

