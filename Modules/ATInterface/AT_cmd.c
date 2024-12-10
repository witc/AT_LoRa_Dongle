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
static void AT_HandleIdentify(char *params);
static void AT_HandleRF_TX_HEX(char *params);

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
    {"AT+IDENTIFY",              AT_HandleIdentify,      0,                                 "AT+IDENTIFY - Identify the device",               ""},
    {"AT+FACTORY_MODE",          AT_HandleFactorMode,    0,                                 "AT+FACTORY_MODE - Enable factory mode",           "=ON, =OFF"},
    {"AT+SYS_RESTART",           AT_HandleRestartSys,    0,                                 "AT+SYS_RESTART - Restart the system",             ""},
    {"AT+LED_BLUE",              NULL,                   SYS_LED_BLUE,                      "AT+LED_BLUE - Set LED blue state",                "=ON, =OFF"},
    /* multiple LoRa params*/
    {"AT+LR_TX_SET",                NULL,               SYS_CMD_TX_COMPLETE_SET,             "AT+LR_TX_SET - Set multiple TX parameters",      "=SF:<value>,BW:<value>,CR:<value>,Freq:<value>,IQ:<value>,Header:<value>,CRC:<value>,Power:<value>, ?"},
    {"AT+LR_RX_SET",                NULL,               SYS_CMD_RX_COMPLETE_SET,             "AT+LR_RX_SET - Set multiple RX parameters",      "=SF:<value>,BW:<value>,CR:<value>,Freq:<value>,IQ:<value>,Header:<value>,CRC:<value>, ?"},
    /* single LoRa params*/
    {"AT+LR_TX_FREQ",                NULL,               SYS_CMD_TX_FREQ,                     "AT+LR_TX_FREQ - Set TX frequency",                "=<frequency_in_Hz>, ?"},
    {"AT+LR_RX_FREQ",                NULL,               SYS_CMD_RX_FREQ,                     "AT+LR_RX_FREQ - Set RX frequency",                "=<frequency_in_Hz>, ?"},
    {"AT+LR_TX_POWER",               NULL,               SYS_CMD_TX_POWER,                    "AT+LR_TX_POWER - Set TX power",                   "=<power_in_dBm>, ?"},
    {"AT+LR_TX_SF",                  NULL,               SYS_CMD_TX_SF,                       "AT+LR_TX_SF - Set TX spreading factor",           "=5 to 12, ?"},
    {"AT+LR_RX_SF",                  NULL,               SYS_CMD_RX_SF,                       "AT+LR_RX_SF - Set RX spreading factor",           "=5 to 12, ?"},
    {"AT+LR_TX_BW",                  NULL,               SYS_CMD_TX_BW,                       "AT+LR_TX_BW - Set TX bandwidth",                  "=7810 (BW 0) to 500000 Hz (BW 9): {7.81, 10.42, 15.63, 20.83, 31.25, 41.67, 62.5, 125, 250, 500 kHz}"},
    {"AT+LR_RX_BW",                  NULL,               SYS_CMD_RX_BW,                       "AT+LR_RX_BW - Set RX bandwidth",                  "=7810 (BW 0) to 500000 Hz (BW 9): {7.81, 10.42, 15.63, 20.83, 31.25, 41.67, 62.5, 125, 250, 500 kHz}"},
    {"AT+LR_TX_IQ_INV",              NULL,               SYS_CMD_TX_IQ,                       "AT+LR_TX_IQ_INV - Set TX IQ inversion",           "=1, =0, ?"},
    {"AT+LR_RX_IQ_INV",              NULL,               SYS_CMD_RX_IQ,                       "AT+LR_RX_IQ_INV - Set RX IQ inversion",           "=1, =0, ?"},
    {"AT+LR_TX_CR",                  NULL,               SYS_CMD_TX_CR,                       "AT+LR_TX_CR - Set TX coding rate",                "=45, =46, =47, =48, ?"},
    {"AT+LR_RX_CR",                  NULL,               SYS_CMD_RX_CR,                       "AT+LR_RX_CR - Set RX coding rate",                "=45, =46, =47, =48, ?"},
    {"AT+LR_TX_HEADERMODE",         NULL,               SYS_CMD_HEADERMODE_TX,               "AT+LR_TX_HEADERMODE - Enable TX header mode, explicit = 0",    "=1, =0, ?"},
    {"AT+LR_RX_HEADERMODE",         NULL,               SYS_CMD_HEADERMODE_RX,               "AT+LR_RX_HEADERMODE - Enable RX header mode, explicit = 0",    "=1, =0, ?"},
    {"AT+LR_TX_CRC",                NULL,               SYS_CMD_CRC_TX,                      "AT+LR_TX_CRC - Set TX CRC check",                "=1, =0, ?"},
    {"AT+LR_RX_CRC",                NULL,               SYS_CMD_CRC_RX,                      "AT+LR_RX_CRC - Set RX CRC check",                "=1, =0, ?"},
    {"AT+LR_TX_PREAMBLE_SIZE",      NULL,               SYS_CMD_PREAM_SIZE_TX,               "AT+LR_TX_PREAMBLE_SIZE",                         "=<1 to 65535>, optimum >=8, ?"  },
    {"AT+LR_RX_PREAMBLE_SIZE",      NULL,               SYS_CMD_PREAM_SIZE_RX,               "AT+LR_RX_PREAMBLE_SIZE",                         "=<1 to 65535> should be >= TX side,?"},
 
    {"AT+RF_TX_HEX",                NULL,               SYS_CMD_RF_TX_HEX,                   "AT+RF_TX_HEX - Transmit data via RF in HEX format",  "=<HEX data>"},
    {"AT+RF_TX_TXT",                NULL,               SYS_CMD_RF_TX_TXT,                   "AT+RF_TX_TXT - Transmit data via RF in text format", "=<Text data>"},
    {"AT+RF_TX_PERIOD",             NULL,               SYS_CMD_RF_PERIOD_SET,               "AT+RF_PERIOD - Set TX period",                "=<period_ms>, ?"},
    {"AT+RF_TX_PERIOD_CTRL",        NULL,               SYS_CMD_RF_PERIOD_CTRL,              "AT+RF_PERIOD_CTRL - Start/Stop periodic TX",      "=<ON|OFF>, ?"},
    {"AT+RF_TX_SAVE_PCKT",          NULL,               SYS_CMD_RF_SAVE_PCKT_NVM,            "AT+RF_TX_SAVE_PCKT - Save packet to NVM",         "=<HEX data>, ?"},
    {"AT+RF_TX_FROM_NVM",           NULL,               SYS_CMD_RF_TX_FROM_NVM,              "AT+RF_TX_FROM_NVM - Transmit saved RF packet from NVM",                   ""},
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
    bool isCommand = false;

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
            isCommand = true;
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
                    AT_SendStringResponse("ERROR - Missing parameters\r\n");
                    break;
                }

                if(at_ctx.onDataReceivedFromISR == NULL)
                {
                    AT_SendStringResponse("ERROR - No handler for this command\r\n");
                    break;
                }
                
                if(at_ctx.onDataReceivedFromISR(params,(uint8_t) AT_Commands[i].cmdtoCore,size) == false)
                {
                    AT_SendStringResponse("ERROR - Previous data was not processed yet!\r\n");
                }
                
                //AT_Commands[i].handler(params,AT_Commands[i].cmdtoCore,size);
            }
            else
            {
                AT_Commands[i].simpleHandler(params);
            }
            
        }
    }
    if(isCommand == false)
    {
        AT_SendStringResponse("ERROR - Unknown command\r\n");
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
    AT_SendStringResponse("ERROR - UART error\r\n");
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
        AT_SendStringResponse("Factory mode UI enabled\r\n");
    }
    else if(memcmp(params,"OFF",3) == 0)
    {
        AT_SendStringResponse("Factory mode UI disabled\r\n");
    }
    else
    {
        AT_SendStringResponse("ERROR\r\n");
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

    AT_SendStringResponse("\r\n");
    AT_SendStringResponse("Supported AT commands:\r\n");

    for (uint16_t i = 0; i < sizeof(AT_Commands) / sizeof(AT_Command_Struct); i++)
    {
        AT_SendStringResponse((char*)AT_Commands[i].usage);

        if (strlen(AT_Commands[i].parameters) > 0)
        {
            AT_SendStringResponse(" (Par: ");
            AT_SendStringResponse((char*)AT_Commands[i].parameters);
            AT_SendStringResponse(")");
        }

        AT_SendStringResponse("\r\n");
    }

    // Add examples for complex commands
    AT_SendStringResponse("\r\nExamples for complex commands:\r\n");
    AT_SendStringResponse("  AT+LR_TX_SET=SF:9,BW:7,CR:45,Freq:869525000,IQ:0,Header:0,CRC:1,Power:22\r\n");
    AT_SendStringResponse("  AT+LR_RX_SET=SF:9,BW:7,CR:45,Freq:869525000,IQ:1,Header:0,CRC:1\r\n");
}

/**
 * @brief 
 * 
 * @param params 
 */
static void AT_HandleIdentify(char *params)
{
    UNUSED(params);
    AT_SendStringResponse("AT-LoRa_Dongle v1.0\r\n");
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
void AT_SendStringResponse(char *response)
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
