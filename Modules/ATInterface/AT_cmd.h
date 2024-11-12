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
    SYS_LED_BLUE            = 9,

    /* LoRa SX1262-specific commands */
    SYS_CMD_TX_FREQ         = 20,
    SYS_CMD_RX_FREQ         = 21,
    SYS_CMD_TX_POWER        = 22,
    SYS_CMD_TX_SF           = 23,
    SYS_CMD_RX_SF           = 24,
    SYS_CMD_TX_BW           = 25,
    SYS_CMD_RX_BW           = 26,
    SYS_CMD_TX_IQ           = 27,
    SYS_CMD_RX_IQ           = 28,
    SYS_CMD_TX_CR           = 29,
    SYS_CMD_RX_CR           = 30,
    SYS_CMD_HEADERMODE_TX   = 31,
    SYS_CMD_HEADERMODE_RX   = 32,
    SYS_CMD_CRC_TX          = 33,
    SYS_CMD_CRC_RX          = 34,
    SYS_CMD_PREAM_SIZE_TX   = 35,
    SYS_CMD_PREAM_SIZE_RX   = 36,

} eATCommands;


/**
 * @brief 
 * 
 */
typedef struct AT_cmd
{
    SP_Context_t sp_ctx;
    bool (*onDataReceivedFromISR)(char *params, eATCommands cmdToCore, uint16_t size); 

} __attribute__((packed)) AT_cmd_t;

void UART_SendResponse(char *response);
void AT_HandleATCommand(uint16_t size);
void AT_Init(AT_cmd_t *atCmd);

#endif // AT_CMD_H

