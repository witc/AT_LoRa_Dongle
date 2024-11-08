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


/**
 * @brief 
 * 
 * @param params 
 */
static void AT_HandleAT(char *params);
static void AT_HandleFactorMode(char *params);
static void AT_HandleHelp(char *params);
static void AT_HandleRestartSys(char *params);


extern UART_HandleTypeDef huart1;


/**
 * @brief 
 * 
 */
typedef struct
{
    char *command;
    void (*handler)(char *params, uint8_t cmdtoCore);  // Handler s pevnym parametrem
    void (*simpleHandler)(char *params);            // Handler bez pevneho parametru
    uint8_t cmdtoCore;
    bool isFixedParamUsed;
    const char *usage;   
    const char *parameters;
} AT_Command_Struct;

/* Table of AT commands */
AT_Command_Struct AT_Commands[] = {
    {"AT",                      NULL,                  AT_HandleHelp,          0,                          false, "AT - Basic test command",                  ""},
    {"AT+HELP",                 NULL,                  AT_HandleHelp,          0,                          false, "AT+HELP - List all supported commands",      ""},
    {"AT+FACTORY_MODE",         NULL,                  AT_HandleFactorMode,    0,                          false, "AT+FACTORY_MODE - Enable factory mode",    "=ON, =OFF"},
    {"AT+SYS_RESTART",          NULL,                  AT_HandleRestartSys,    0,                          false, "AT+SYS_RESTART - Restart the system",        ""},                       
    // {"AT+FACTORY_MODE",         NULL,                  AT_HandleFactorMode,0,                          false, "AT+FACTORY_MODE - Enable factory mode",    "=ON, =OFF"},
   
    // {"AT+SYS_RESTART",          NULL,                  AT_HandleRestartSys,0,                          false, "AT+SYS_RESTART - Restart the system",        ""},
    // {"AT+SYS_STATE",            AT_RouteToCoreTask,   NULL,                SYS_CMD_SYS_STATE,          true,  "AT+SYS_STATE - Get or set system state",     "?, =ON, =OFF"},
    // {"AT+SYS_NAME",             AT_RouteToCoreTask,   NULL,                SYS_CMD_SYS_NAME,           true,  "AT+SYS_NAME=<name> - Set system name",       "?"},
    // {"AT+RF_MAC",               AT_RouteToCoreTask,   NULL,                SYS_CMD_RF_MAC,             true,  "AT+RF_MAC=<MAC> ",                           "?"},
    // {"AT+FACTORY_RST",          AT_RouteToCoreTask,   NULL,                SYS_CMD_FACTORY_RST,        true,  "AT+FACTORY_RST - Factory reset the device",  ""},
    // {"AT+LORA_TX_CW",           AT_RouteToCoreTask,   NULL,                SYS_CMD_TX_CW,              true,  "AT+LORA_TX_CW - Transmit continuous wave",   "=ON, =OFF"},
    // {"AT+LORA_RX_TO_UART",      AT_RouteToCoreTask,   NULL,                SYS_CMD_LORA_RX_TO_UART,    true,  "AT+LORA_RX_TO_UART - Forward LoRa data",     "=ON, =OFF"},
    // {"AT+LORA_SEND",            AT_RouteToCoreTask,   NULL,                SYS_CMD_LORA_SEND,          true,  "AT+LORA_SEND=<length>,<data> - Send data",   ""},
    // {"AT+RF_PAIR",              AT_RouteToCoreTask,   NULL,                SYS_CMD_RF_PAIR,            true,  "AT+RF_PAIR - Pair RF device",                "=ON, =OFF"},
};


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
void AT_HandleATCommand(SP_Context_t *sp_ctx, uint8_t size)
{   
    bool dataUsed = false;
    char *data=(char*) sp_ctx->rxStorage.raw_data;

    AT_ToUpperCase(data,size);

    for (uint16_t i = 0; i < sizeof(AT_Commands) / sizeof(AT_Command_Struct); i++)
    {   
        size_t commandLen = strlen(AT_Commands[i].command);

        if (strncmp(data, AT_Commands[i].command, commandLen) == 0 &&
            (data[commandLen] == '\0' ||
             data[commandLen] == '\r' ||
             data[commandLen] == '\n' ||
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

            
            if(AT_Commands[i].handler != NULL)
            {
                AT_Commands[i].handler(params,AT_Commands[i].cmdtoCore);
            }
            else if(AT_Commands[i].simpleHandler != NULL)
            {
                AT_Commands[i].simpleHandler(params);
            }
            
            dataUsed = true;
        }
    }

    if(dataUsed == false)    UART_SendResponse("ERROR - Check your EOL sequence\r\n");  // Neznámý příkaz

    memset(data,0,size);

    SP_RxComplete(sp_ctx, size);  
   
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
        UART_SendResponse("Factory mode UI enabled\r\n");
    }
    else if(memcmp(params,"OFF",3) == 0)
    {
        UART_SendResponse("Factory mode UI disabled\r\n");
    }
    else
    {
        UART_SendResponse("ERROR\r\n");
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

    UART_SendResponse("\r\n");
    UART_SendResponse("Supported AT commands:\r\n");

    for (uint16_t i = 0; i < sizeof(AT_Commands) / sizeof(AT_Command_Struct); i++)
    {
       // UART_SendResponse(AT_Commands[i].command);
       // UART_SendResponse((char *)" - ");
        UART_SendResponse((char*)AT_Commands[i].usage);

        if (strlen(AT_Commands[i].parameters) > 0)
        {
            UART_SendResponse(" (Par: ");
            UART_SendResponse((char*)AT_Commands[i].parameters);
            UART_SendResponse(")");
        }

        UART_SendResponse("\r\n");
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
void UART_SendResponse(char *response)
{   
    //TODOJR DMA
    HAL_UART_Transmit(&huart1, (uint8_t *)response, strlen(response),0xFFFF);
}
