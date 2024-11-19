/**
 * @file AT_cmd.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-11-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "main.h"
#include "AT_cmd.h"
#include <ctype.h>
#include "Main_task.h"
#include "semphr.h"


/**
 * @brief 
 * 
 * @param params 
 */
static void AT_HandleFactorMode(char *params);
static void AT_HandleHelp(char *params);
static void AT_HandleRestartSys(char *params);

extern UART_HandleTypeDef huart1;
extern osMessageQueueId_t queueMainHandle;

AT_cmd_t at_ctx;
SemaphoreHandle_t xUART_TXSemaphore;

/**
 * @brief 
 * 
 */
typedef struct
{
    char *command;
   // void (*handler)(char *params, uint8_t cmdtoCore,uint16_t size);  // Handler s pevnym parametrem
    void (*simpleHandler)(char *params);            // Handler bez pevneho parametru
    eATCommands cmdtoCore;
  //  bool isFixedParamUsed;
    const char *usage;   
    const char *parameters;
} AT_Command_Struct;


/* Table of AT commands */
const AT_Command_Struct AT_Commands[] = {
    {"AT",                       AT_HandleHelp,          0,                                 "AT - Basic test command",                         ""},
    {"AT+HELP",                  AT_HandleHelp,          0,                                 "AT+HELP - List all supported commands",           ""},
    {"AT+FACTORY_MODE",          AT_HandleFactorMode,    0,                                 "AT+FACTORY_MODE - Enable factory mode",           "=ON, =OFF"},
    {"AT+SYS_RESTART",           AT_HandleRestartSys,    0,                                 "AT+SYS_RESTART - Restart the system",             ""},
    {"AT+LED_BLUE",              NULL,                   SYS_LED_BLUE,                      "AT+LED_BLUE - Set LED blue state",                "=ON, =OFF"},

    /* LoRa RF settings for SX1262 */
    {"AT+LR_TXFREQ",                NULL,               SYS_CMD_TX_FREQ,                     "AT+LR_TXFREQ - Set TX frequency",                "=<frequency_in_Hz>, ?"},
    {"AT+LR_RXFREQ",                NULL,               SYS_CMD_RX_FREQ,                     "AT+LR_RXFREQ - Set RX frequency",                "=<frequency_in_Hz>, ?"},
    {"AT+LR_TXPOWER",               NULL,               SYS_CMD_TX_POWER,                    "AT+LR_TXPOWER - Set TX power",                   "=<power_in_dBm>, ?"},
    {"AT+LR_TXSF",                  NULL,               SYS_CMD_TX_SF,                       "AT+LR_TXSF - Set TX spreading factor",           "=5 to 12, ?"},
    {"AT+LR_RXSF",                  NULL,               SYS_CMD_RX_SF,                       "AT+LR_RXSF - Set RX spreading factor",           "=5 to 12, ?"},
    {"AT+LR_TXBW",                  NULL,               SYS_CMD_TX_BW,                       "AT+LR_TXBW - Set TX bandwidth",                  "=7810 to 500000 Hz, ?"},
    {"AT+LR_RXBW",                  NULL,               SYS_CMD_RX_BW,                       "AT+LR_RXBW - Set RX bandwidth",                  "=7810 to 500000 Hz, ?"},
    {"AT+LR_TXIQ_INV",              NULL,               SYS_CMD_TX_IQ,                       "AT+LR_TXIQ_INV - Set TX IQ inversion",           "=TRUE, =FALSE, ?"},
    {"AT+LR_RXIQ_INV",              NULL,               SYS_CMD_RX_IQ,                       "AT+LR_RXIQ_INV - Set RX IQ inversion",           "=TRUE, =FALSE, ?"},
    {"AT+LR_TXCR",                  NULL,               SYS_CMD_TX_CR,                       "AT+LR_TXCR - Set TX coding rate",                "=45, =46, =47, =48, ?"},
    {"AT+LR_RXCR",                  NULL,               SYS_CMD_RX_CR,                       "AT+LR_RXCR - Set RX coding rate",                "=45, =46, =47, =48, ?"},
    {"AT+LR_HEADERMODE_TX",         NULL,               SYS_CMD_HEADERMODE_TX,               "AT+LR_HEADERMODE_TX - Enable TX header mode",    "=TRUE, =FALSE, ?"},
    {"AT+LR_HEADERMODE_RX",         NULL,               SYS_CMD_HEADERMODE_RX,               "AT+LR_HEADERMODE_RX - Enable RX header mode",    "=TRUE, =FALSE, ?"},
    {"AT+LR_CRC_TX",                NULL,               SYS_CMD_CRC_TX,                      "AT+LR_CRC_TX - Set TX CRC check",                "=TRUE, =FALSE, ?"},
    {"AT+LR_CRC_RX",                NULL,               SYS_CMD_CRC_RX,                      "AT+LR_CRC_RX - Set RX CRC check",                "=TRUE, =FALSE, ?"},
    {"AT+LR_PREAMBLE_SIZE_TX",      NULL,               SYS_CMD_PREAM_SIZE_TX,               "AT+LR_PREAMBLE_SIZE_TX",                         "=<1 to 65535>, ?"  },
    {"AT+LR_PREAMBLE_SIZE_RX",      NULL,               SYS_CMD_PREAM_SIZE_RX,               "AT+LR_PREAMBLE_SIZE_RX",                         "=<1 to 65535> should be >= TX side,?"},
    {"AT+RF_TX_HEX",                NULL,               SYS_CMD_RF_TX_HEX,                   "AT+RF_TX_HEX - Transmit data via RF in HEX format",  "=<HEX data>"},
    {"AT+RF_TX_TXT",                NULL,               SYS_CMD_RF_TX_TXT,                   "AT+RF_TX_TXT - Transmit data via RF in text format", "=<Text data>"},
    {"AT+RF_TX_FROM_NVM",           NULL,               SYS_CMD_RF_TX_FROM_NVM,              "Transmit saved RF packet from NVM",                   ""},
    {"AT+RF_TX_PERIOD",             NULL,               SYS_CMD_RF_PERIOD_SET,               "AT+RF_PERIOD - Set TX period",                "=<period_ms>, ?"},
    {"AT+RF_TX_PERIOD_CTRL",        NULL,               SYS_CMD_RF_PERIOD_CTRL,              "AT+RF_PERIOD_CTRL - Start/Stop periodic TX",      "=<ON|OFF>, ?"},
    {"AT+RF_TX_SAVE_PCKT",          NULL,               SYS_CMD_RF_SAVE_PCKT_NVM,            "AT+RF_TX_SAVE_PCKT - Save packet to NVM",         "=<HEX data>, ?"},
    {"AT+RF_TX_PERIOD_STATUS",      NULL,               SYS_CMD_RF_PERIOD_STATUS,            "AT+RF_PERIOD_STATUS - Get periodic TX status",     "?"}
};



/**
 * @brief 
 * 
 * @param p_at_Ctx 
 */
void AT_Init(AT_cmd_t *p_at_Ctx)
{   
    at_ctx.onDataReceivedFromISR = NULL;

    if(xUART_TXSemaphore == NULL)
    {
        xUART_TXSemaphore = xSemaphoreCreateBinary();
        if (xUART_TXSemaphore != NULL)
        {
            xSemaphoreGive(xUART_TXSemaphore);
        }else
        {
            Error_Handler();
        }
    }
    
    at_ctx.sp_ctx.phuart = p_at_Ctx->sp_ctx.phuart;
    at_ctx.sp_ctx.rxStorage.raw_data = p_at_Ctx->sp_ctx.rxStorage.raw_data;
    at_ctx.sp_ctx.rxStorage.size = p_at_Ctx->sp_ctx.rxStorage.size;
    at_ctx.sp_ctx.txStorage.raw_data = p_at_Ctx->sp_ctx.txStorage.raw_data;
    at_ctx.sp_ctx.txStorage.size = p_at_Ctx->sp_ctx.txStorage.size;

   
    at_ctx.onDataReceivedFromISR = p_at_Ctx->onDataReceivedFromISR;

    SP_PlatformInit(&at_ctx.sp_ctx);

}


/**
 * @brief 
 * 
 * @param str 
 */
static void AT_ToUpperCase(char *str, size_t max_len)
{
    size_t i = 0;
    while (i < max_len && *str) {
        if (*str >= 'a' && *str <= 'z') {  // Kontrola, zda je znak malé písmeno
            *str = *str - ('a' - 'A');     // Převod na velké písmeno
        }
        str++;
        i++;
    }
}

/**
 * @brief 
 * 
 * @param data 
 */
static void AT_TrimEndings(char *data)
{
    size_t length = strlen(data);

    while (length > 0 && (data[length - 1] == '\n' || data[length - 1] == '\r'))
    {
        data[length - 1] = '\0';
        length--;
    }
}

/**
 * @brief 
 * 
 * @param data 
 */
void AT_HandleATCommand(uint16_t size)
{   
    //char *data=(char*) sp_ctx->rxStorage.raw_data;
    char *data = (char*) at_ctx.sp_ctx.rxStorage.raw_data;
    bool noParam = false;

    AT_ToUpperCase(data,size);
    AT_TrimEndings(data);

    for (uint16_t i = 0; i < sizeof(AT_Commands) / sizeof(AT_Command_Struct); i++)
    {   
        size_t commandLen = strlen(AT_Commands[i].command);

        if (strncmp(data, AT_Commands[i].command, commandLen) == 0 &&
            (data[commandLen] == '\0' ||
             data[commandLen] == '=' ||
             data[commandLen] == '?'))
        {
            char *params = data + commandLen;  // Nastavíme ukazatel za příkaz

            // Pokud je tam `=`, přeskočíme znak `=`, abychom získali parametry
            if (*params == '=')
            {
                params++;
            }
            // Pokud je tam `?`, předáme `?` jako parametr (pro `GET` příkaz)
            else if (*params == '?')
            {
                params = "?";  // Nastavíme `params` přímo na `?`
            }
            else
            {
                // zadny parametr
                noParam= true;
            }

            if(AT_Commands[i].simpleHandler == NULL)
            {      
                if (noParam == true)
                {
                    AT_SendResponse("ERROR - Missing parameters\r\n");
                    break;
                }

                if(at_ctx.onDataReceivedFromISR == NULL)
                {
                    AT_SendResponse("ERROR - No handler for this command\r\n");
                    break;
                }
                
                if(at_ctx.onDataReceivedFromISR(params,(uint8_t) AT_Commands[i].cmdtoCore,size) == false)
                {
                    AT_SendResponse("ERROR - Previous data was not processed yet!\r\n");
                }
                
                //AT_Commands[i].handler(params,AT_Commands[i].cmdtoCore,size);
            }
            else
            {
                AT_Commands[i].simpleHandler(params);
            }
            
        }
    }

    memset(data,0,size);
    SP_RxComplete(&at_ctx.sp_ctx, size);  
   
}

/**
 * @brief 
 * 
 */
void AT_HandleUartError(void)
{   
    SP_HandleUARTError(&at_ctx.sp_ctx);
    AT_SendResponse("ERROR - UART error\r\n");
}

/**
 * @brief 
 * 
 * @param params 
 */
static  void AT_HandleFactorMode(char *params)
{   
    if(memcmp(params,"ON",2) == 0)
    {
        AT_SendResponse("Factory mode UI enabled\r\n");
    }
    else if(memcmp(params,"OFF",3) == 0)
    {
        AT_SendResponse("Factory mode UI disabled\r\n");
    }
    else
    {
        AT_SendResponse("ERROR\r\n");
    }
    
}

/**
 * @brief 
 * 
 * @param params 
 */
static void AT_HandleHelp(char *params)
{   
    UNUSED(params);

    AT_SendResponse("\r\n");
    AT_SendResponse("Supported AT commands:\r\n");

    for (uint16_t i = 0; i < sizeof(AT_Commands) / sizeof(AT_Command_Struct); i++)
    {
       // AT_SendResponse(AT_Commands[i].command);
       // AT_SendResponse((char *)" - ");
        AT_SendResponse((char*)AT_Commands[i].usage);

        if (strlen(AT_Commands[i].parameters) > 0)
        {
            AT_SendResponse(" (Par: ");
            AT_SendResponse((char*)AT_Commands[i].parameters);
            AT_SendResponse(")");
        }

        AT_SendResponse("\r\n");
    }
}


/**
 * @brief 
 * 
 * @param params 
 */
static void AT_HandleRestartSys(char *params)
{
    UNUSED(params);
    NVIC_SystemReset();
}


/**
 * @brief 
 * 
 * @param response 
 */
void AT_SendResponse(char *response)
{   
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (__get_IPSR())  
    {
        // Použití semaforu z ISR
        if (xSemaphoreTakeFromISR(xUART_TXSemaphore, &xHigherPriorityTaskWoken) == pdTRUE)
        {
         //   HAL_UART_Transmit_DMA(&huart1, (uint8_t *)response, strlen(response));
            HAL_UART_Transmit(&huart1, (uint8_t *)response, strlen(response),HAL_MAX_DELAY);
            xSemaphoreGiveFromISR(xUART_TXSemaphore, &xHigherPriorityTaskWoken);
            // Uvolníme semafor při dokončení přenosu (v přerušení)
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
    else
    {
        if (xSemaphoreTake(xUART_TXSemaphore, portMAX_DELAY) == pdTRUE)
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)response, strlen(response),HAL_MAX_DELAY);
            xSemaphoreGive(xUART_TXSemaphore);
        }
    }

    //TODOJR DMA
    //HAL_UART_Transmit(&huart1, (uint8_t *)response, strlen(response),0xFFFF);
}
