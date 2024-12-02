/**
 * @file general_sys_cmd.c
 * @brief General system command processing
 * @version 0.1
 * @date 2024-11-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "main.h"
#include "Main_task.h"
#include "general_sys_cmd.h"
#include "NVMA.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "AT_cmd.h"
#include "radio_user.h"
#include <errno.h>

#define RESPONSE_BUFF_SIZE  32

const AT_CommandLimit_t AT_CommandLimits[] = {
    {SYS_CMD_TX_FREQ, 150000000, 960000000},        // TX frequency in Hz (100 MHz to 960 MHz)
    {SYS_CMD_RX_FREQ, 150000000, 960000000},        // RX frequency in Hz (100 MHz to 960 MHz)
    {SYS_CMD_TX_POWER, 0, 22},                     // TX power in dBm (0 to 22 dBm)
    {SYS_CMD_TX_SF, 5, 12},                         // TX spreading factor (5 to 12)
    {SYS_CMD_RX_SF, 5, 12},                         // RX spreading factor (5 to 12)
    {SYS_CMD_TX_BW, 0, 9},                          // TX bandwidth 0-9
    {SYS_CMD_RX_BW, 0, 9},                          // RX bandwidth 0-9
    {SYS_CMD_TX_IQ, 0, 1},                          // TX IQ inversion (0 = FALSE, 1 = TRUE)
    {SYS_CMD_RX_IQ, 0, 1},                          // RX IQ inversion (0 = FALSE, 1 = TRUE)
    {SYS_CMD_TX_CR, 45, 48},                        // TX coding rate (45 to 48, corresponds to different CR values)
    {SYS_CMD_RX_CR, 45, 48},                        // RX coding rate (45 to 48)
    {SYS_CMD_HEADERMODE_TX, 0, 1},                  // TX header mode (0 = FALSE, 1 = TRUE)
    {SYS_CMD_HEADERMODE_RX, 0, 1},                  // RX header mode (0 = FALSE, 1 = TRUE)
    {SYS_CMD_CRC_TX, 0, 1},                         // TX CRC check (0 = FALSE, 1 = TRUE)
    {SYS_CMD_CRC_RX, 0, 1},                         // RX CRC check (0 = FALSE, 1 = TRUE)
    {SYS_CMD_PREAM_SIZE_TX, 1, 65535},              // TX preamble size (1 to 65535)
    {SYS_CMD_PREAM_SIZE_RX, 1, 65535},              // RX preamble size (1 to 65535, should be >= TX)
};

extern osMessageQueueId_t queueRadioHandle;

const uint32_t AllowedBandwidths[] = {7810, 10420, 15630, 20830, 31250, 41670, 62500, 125000, 250000, 500000};
const size_t AllowedBandwidthCount = sizeof(AllowedBandwidths) / sizeof(AllowedBandwidths[0]);

static bool _GSC_Handle_BlueLED(uint8_t *data);
static void _GSC_Handle_TX(uint8_t *data, uint8_t size);
static bool GetCommandLimits(eATCommands cmd, int32_t *minValue, int32_t *maxValue);
static bool IsValidBandwidth(uint32_t bandwidth);
static uint8_t HexStringToByteArray(const char *hexStr, uint8_t *byteArray, size_t byteArraySize);
 
/**
 * @brief Parse data to uint32_t
 * 
 * @param data 
 * @return uint32_t 
 */
bool AT_ParseUint32(const uint8_t *data, uint32_t *value)
{
    uint32_t result = 0;
    const uint8_t *ptr = data;

    if (*ptr == '\0')
    {
        // Prázdný řetězec
        return false;
    }

    while (*ptr != '\0')
    {
        if (*ptr < '0' || *ptr > '9')
        {
            // Nenarazili jsme na číslici
            return false;
        }

        uint8_t digit = *ptr - '0';

        // Kontrola přetečení
        if (result > (UINT32_MAX - digit) / 10)
        {
            // Došlo by k přetečení
            return false;
        }

        result = result * 10 + digit;
        ptr++;
    }

    *value = result;
    return true;
}

/**
 * @brief Parse data to uint8_t
 * 
 * @param data 
 * @return uint8_t 
 */
bool AT_ParseUint8(const uint8_t *data, uint8_t *value)
{
    uint32_t result = 0;
    const uint8_t *ptr = data;

    if (*ptr == '\0')
    {
        // Prázdný řetězec
        return false;
    }

    while (*ptr != '\0')
    {
        if (*ptr < '0' || *ptr > '9')
        {
            // Nenarazili jsme na číslici
            return false;
        }

        uint8_t digit = *ptr - '0';

        // Kontrola přetečení pro uint8_t
        if (result > ((uint32_t)UINT8_MAX - digit) / 10)
        {
            // Došlo by k přetečení
            return false;
        }

        result = result * 10 + digit;
        ptr++;
    }

    *value = (uint8_t)result;
    return true;
}

/**
 * @brief 
 * 
 * @param tx 
 * @param params 
 */
void ProcessRFMultiSetCommand(bool tx, char *params)
{      
   bool isQuery = (params[0] == '?' && params[1] == '\0');

    // Seznam příkazů k iteraci
    eATCommands commands[] = {
        tx ? SYS_CMD_TX_SF : SYS_CMD_RX_SF,
        tx ? SYS_CMD_TX_BW : SYS_CMD_RX_BW,
        tx ? SYS_CMD_TX_CR : SYS_CMD_RX_CR,
        tx ? SYS_CMD_TX_FREQ : SYS_CMD_RX_FREQ,
        tx ? SYS_CMD_TX_IQ : SYS_CMD_RX_IQ,
        tx ? SYS_CMD_HEADERMODE_TX : SYS_CMD_HEADERMODE_RX,
        tx ? SYS_CMD_CRC_TX : SYS_CMD_CRC_RX,
        tx ? SYS_CMD_TX_POWER : SYS_CMD_TX_POWER
    };

    const char *keys[] = {
        "SF", "BW", "CR", "Freq", "IQ", "Header", "CRC", "Power"
    };


    // Pokud jde o dotaz
    if (isQuery)
    {
        // Iteruj přes všechny příkazy a volej `GSC_ProcessCommand`
        for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
        {   
            AT_SendStringResponse(keys[i]);
            AT_SendStringResponse(":");

             // Získej hodnotu pomocí `GSC_ProcessCommand`
            if (GSC_ProcessCommand(commands[i], (uint8_t *)"?", 1))
            {
                // Hodnota už je vypsána přímo v `GSC_ProcessCommand`
            }
            else
            {
                AT_SendStringResponse("ERROR");
            }

            AT_SendStringResponse("\r\n"); // Přidání nového řádku pro čitelnost
        }
        return;
    }

    bool success = true;
    char *token = strtok(params, ",");

    bool errorOccurred = false;
    char errorMessages[256] = ""; // Pole pro chybové zprávy
    size_t errorMessagesLen = 0;

    while (token != NULL)
    {
        char *key = token;
        char *value_str = strchr(token, ':');
        if (value_str != NULL)
        {
            *value_str = '\0'; // Ukončíme řetězec klíče
            value_str++; // Posuneme ukazatel na hodnotu
        }
        else
        {
            success = false;
            break;
        }

        eATCommands cmd = SYS_CMD_NONE;

        if (strcmp(key, "SF") == 0)
        {
            if(tx == true)    cmd = SYS_CMD_TX_SF;
            else              cmd = SYS_CMD_RX_SF;
            // Případně ověření hodnoty
        }
        else if (strcmp(key, "BW") == 0)
        {
            if(tx == true) cmd = SYS_CMD_TX_BW;
            else           cmd = SYS_CMD_RX_BW;
            // Případně ověření hodnoty
        }
        else if (strcmp(key, "CR") == 0)
        {
            if(tx == true) cmd = SYS_CMD_TX_CR;
            else           cmd = SYS_CMD_RX_CR;
            // Případně ověření hodnoty
        }
        else if (strcmp(key, "POWER") == 0)
        {
            if(tx == true) cmd = SYS_CMD_TX_POWER;
            // Případně ověření hodnoty
        }
        else if (strcmp(key, "FREQ") == 0)
        {
            if(tx == true) cmd = SYS_CMD_TX_FREQ;
            else           cmd = SYS_CMD_RX_FREQ;
            // Případně ověření hodnoty
        }
        else if (strcmp(key, "IQ") == 0)
        {
            if(tx == true) cmd = SYS_CMD_TX_IQ;
            else           cmd = SYS_CMD_RX_IQ;
        
        }
        else if (strcmp(key, "HEADER") == 0)
        {
            if(tx == true) cmd = SYS_CMD_HEADERMODE_TX;
            else           cmd = SYS_CMD_HEADERMODE_RX;

        }
        else if (strcmp(key, "CRC") == 0)
        {
            if(tx == true) cmd = SYS_CMD_CRC_TX;
            else           cmd = SYS_CMD_CRC_RX;
           
        }
        else
        {
            success = false;
            AT_SendStringResponse("ERROR: Unknown parameter\r\n");
            break;
        }

        // Předáme value_str přímo do GSC_ProcessCommand
        if (!GSC_ProcessCommand(cmd, (uint8_t *)value_str, strlen(value_str)))
        {   
            errorOccurred = true;
            if (errorMessagesLen < sizeof(errorMessages) - 1)
            {
                strncat(errorMessages, "Failed to set parameter: ", sizeof(errorMessages) - errorMessagesLen - 1);
                strncat(errorMessages, key, sizeof(errorMessages) - errorMessagesLen - 1);
                strncat(errorMessages, "; ", sizeof(errorMessages) - errorMessagesLen - 1);
                errorMessagesLen = strlen(errorMessages);
            }

            success = false;
            //AT_SendStringResponse("ERROR: Failed to set parameter\r\n");
           // break;
        }

        // Další token
        token = strtok(NULL, ",");
    }

    // if (success == false)
    // {
    //    AT_SendStringResponse("ERROR: Invalid parameter format\r\n");
    // }
}




/**
 * @brief Return uint32_t value as a string
 * 
 * @param value 
 * @param response 
 * @param response_size 
 */
static void AT_FormatUint32Response(uint32_t value, uint8_t *response, uint16_t *response_size)
{
    *response_size = snprintf((char *)response, RESPONSE_BUFF_SIZE, "%lu", (unsigned long) value);
}

/**
 * @brief Return uint8_t value as a string
 * 
 * @param value 
 * @param response 
 * @param response_size 
 */
static void AT_FormatUint8Response(uint8_t value, uint8_t *response, uint16_t *response_size)
{   
    *response_size = snprintf((char *)response, RESPONSE_BUFF_SIZE, "%u", value);
    //*response_size = sprintf((char *)response, "%u", value);
}

/**
 * @brief Get command limits
 * 
 * @param cmd 
 * @param minValue 
 * @param maxValue 
 * @return true 
 * @return false 
 */
static bool GetCommandLimits(eATCommands cmd, int32_t *minValue, int32_t *maxValue)
{
    for (uint16_t i = 0; i < sizeof(AT_CommandLimits) / sizeof(AT_CommandLimits[0]); i++)
    {
        if (AT_CommandLimits[i].cmd == cmd)
        {
            *minValue = AT_CommandLimits[i].minValue;
            *maxValue = AT_CommandLimits[i].maxValue;
            return true;
        }
    }
    return false; // If the command was not found
}

/**
 * @brief 
 * 
 * @param bandwidth 
 * @return staic 
 */
static bool IsValidBandwidth(uint32_t bandwidth)
{
    for (size_t i = 0; i < AllowedBandwidthCount; i++)
    {
        if (AllowedBandwidths[i] == bandwidth)
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Convert hex string to uint8_t array
 * 
 * @param hexStr 
 * @param byteArray 
 * @param byteArraySize 
 * @return true 
 * @return false 
 */
static uint8_t HexStringToByteArray(const char *hexStr, uint8_t *byteArray, size_t byteArraySize)
{
    size_t hexStrLen = strlen(hexStr);
    if (hexStrLen % 2 != 0 || hexStrLen / 2 > byteArraySize)
    {
        return 0;
    }

    for (size_t i = 0; i < hexStrLen; i += 2)
    {
        char byteStr[3] = {hexStr[i], hexStr[i + 1], '\0'};
        byteArray[i / 2] = (uint8_t)strtoul(byteStr, NULL, 16);
    }

    return (uint8_t)(hexStrLen / 2);
}

/**
 * @brief Process system command
 * 
 * @param cmd 
 * @param data 
 * @param size 
 * @return true 
 * @return false 
 */
bool GSC_ProcessCommand(eATCommands cmd, uint8_t *data, uint16_t size)
{
    UNUSED(size);
    char response[RESPONSE_BUFF_SIZE];
    uint16_t response_size = 0;
    bool isQuery = (data[0] == '?');
    bool hasResponse = false;
    bool commandHandled = true;
    bool constrained = false;

    int32_t minValue, maxValue;

    switch (cmd)
    {   
        case SYS_LED_BLUE:
        {
            commandHandled = _GSC_Handle_BlueLED(data);
            break;
        }

        case SYS_CMD_TX_FREQ:
        {   
            uint32_t freq;
            if (isQuery)
            {
                NVMA_Get_LR_Freq_TX(&freq);
                AT_FormatUint32Response(freq, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                if (!AT_ParseUint32(data, &freq))
                {
                    AT_SendStringResponse("ERROR: Invalid TX_FREQ value\r\n");
                    commandHandled = false;
                    break;
                }
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    freq = Constrain_u32(freq, minValue, maxValue,&constrained);
                    if(constrained)
                    {
                        AT_SendStringResponse("ERROR: TX_FREQ value out of limit\r\n");
                        commandHandled = false;
                        break;
                    }
                }
                NVMA_Set_LR_Freq_TX(freq);
            }
            break;
        }

        case SYS_CMD_RX_FREQ:
        {
            uint32_t freq;
            if (isQuery)
            {
                NVMA_Get_LR_Freq_RX(&freq);
                AT_FormatUint32Response(freq, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                if (!AT_ParseUint32(data, &freq))
                {
                    AT_SendStringResponse("ERROR: Invalid RX_FREQ value\r\n");
                    commandHandled = false;
                    break;
                }
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    freq = Constrain_u32(freq, minValue, maxValue,&constrained);
                    if(constrained)
                    {
                        AT_SendStringResponse("ERROR: RX_FREQ value out of limit\r\n");
                        commandHandled = false;
                        break;
                    }
                }
                NVMA_Set_LR_Freq_RX(freq);
            }
            break;
        }

        case SYS_CMD_TX_POWER:
        {
            uint8_t power;
            if (isQuery)
            {
                NVMA_Get_LR_TX_Power(&power);
                AT_FormatUint8Response(power, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                if (!AT_ParseUint8(data, &power))
                {
                    AT_SendStringResponse("ERROR: Invalid TX_POWER value\r\n");
                    commandHandled = false;
                    break;
                }
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    power = Constrain_u8(power, minValue, maxValue, &constrained);
                    if(constrained)
                    {
                        AT_SendStringResponse("ERROR: TX_POWER value out of limit\r\n");
                        commandHandled = false;
                        break;
                    }
                }
                NVMA_Set_LR_TX_Power(power);
            }
            break;
        }

        case SYS_CMD_TX_SF:
        {   
            uint8_t sf;
            if (isQuery)
            {
                NVMA_Get_LR_TX_SF(&sf);
                AT_FormatUint8Response(sf, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                if (!AT_ParseUint8(data, &sf))
                {
                    AT_SendStringResponse("ERROR: Invalid TX_SF value\r\n");
                    commandHandled = false;
                    break;
                }
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    sf = Constrain_u8(sf, minValue, maxValue,&constrained);
                    if(constrained)
                    {
                        AT_SendStringResponse("ERROR: TX_SF value out of limit\r\n");
                        commandHandled = false;
                        break;
                    }
                }
                NVMA_Set_LR_TX_SF(sf);
            }
            break;
        }

        case SYS_CMD_RX_SF:
        {   
            uint8_t sf;
            if (isQuery)
            {
                NVMA_Get_LR_RX_SF(&sf);
                AT_FormatUint8Response(sf, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                if (!AT_ParseUint8(data, &sf))
                {
                    AT_SendStringResponse("ERROR: Invalid RX_SF value\r\n");
                    commandHandled = false;
                    break;
                }
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    sf = Constrain_u8(sf, minValue, maxValue,&constrained);
                    if(constrained)
                    {
                        AT_SendStringResponse("ERROR: RX_SF value out of limit\r\n");
                        commandHandled = false;
                        break;
                    }
                }
                NVMA_Set_LR_RX_SF(sf);
            }
            break;
        }

        case SYS_CMD_TX_BW:
        {   
            uint8_t bw;
            if (isQuery)
            {
                NVMA_Get_LR_TX_BW(&bw);
                AT_FormatUint32Response(bw, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                if (!AT_ParseUint8(data, &bw))
                {
                    AT_SendStringResponse("ERROR: Invalid TX_BW value\r\n");
                    commandHandled = false;
                    break;
                }
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    bw = Constrain_u8(bw, minValue, maxValue,&constrained);
                    if(constrained)
                    {
                        AT_SendStringResponse("ERROR: TX_BW value out of limit\r\n");
                        commandHandled = false;
                        break;
                    }
                }
                NVMA_Set_LR_TX_BW(bw);
            }
            break;
        }

        case SYS_CMD_RX_BW:
        {   
            uint8_t bw;
            if (isQuery)
            {
                NVMA_Get_LR_RX_BW(&bw);
                AT_FormatUint32Response(bw, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                if (!AT_ParseUint8(data, &bw))
                {
                    AT_SendStringResponse("ERROR: Invalid RX_BW value\r\n");
                    commandHandled = false;
                    break;
                }
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    bw = Constrain_u8(bw, minValue, maxValue,&constrained);
                    if(constrained)
                    {
                        AT_SendStringResponse("ERROR: RX_BW value out of limit\r\n");
                        commandHandled = false;
                        break;
                    }
                }
                NVMA_Set_LR_RX_BW(bw);
            }
            break;
        }

        case SYS_CMD_TX_IQ:
        {   
            uint8_t iq;
            if (isQuery)
            {
                NVMA_Get_LR_TX_IQ(&iq);
                AT_FormatUint8Response(iq, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                if (!AT_ParseUint8(data, &iq))
                {
                    AT_SendStringResponse("ERROR: Invalid TX_IQ value\r\n");
                    commandHandled = false;
                    break;
                }
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    iq = Constrain_u8(iq, minValue, maxValue,&constrained);
                    if(constrained)
                    {
                        AT_SendStringResponse("ERROR: TX_IQ value out of limit\r\n");
                        commandHandled = false;
                        break;
                    }
                }
                NVMA_Set_LR_TX_IQ(iq);
            }
            break;
        }

        case SYS_CMD_RX_IQ:
        {   
            uint8_t iq;
            if (isQuery)
            {
                NVMA_Get_LR_RX_IQ(&iq);
                AT_FormatUint8Response(iq, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                if (!AT_ParseUint8(data, &iq))
                {
                    AT_SendStringResponse("ERROR: Invalid RX_IQ value\r\n");
                    commandHandled = false;
                    break;
                }
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    iq = Constrain_u8(iq, minValue, maxValue,&constrained);
                    if(constrained)
                    {
                        AT_SendStringResponse("ERROR: RX_IQ value out of limit\r\n");
                        commandHandled = false;
                        break;
                    }
                }
                NVMA_Set_LR_RX_IQ(iq);
            }
            break;
        }

        case SYS_CMD_TX_CR:
        {   
            uint8_t cr;
            if (isQuery)
            {
                NVMA_Get_LR_TX_CR(&cr);
                AT_FormatUint8Response(cr, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                if (!AT_ParseUint8(data, &cr))
                {
                    AT_SendStringResponse("ERROR: Invalid TX_CR value\r\n");
                    commandHandled = false;
                    break;
                }
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    cr = Constrain_u8(cr, minValue, maxValue,&constrained);
                    if(constrained)
                    {
                        AT_SendStringResponse("ERROR: TX_CR value out of limit\r\n");
                        commandHandled = false;
                        break;
                    }
                }
                NVMA_Set_LR_TX_CR(cr);
            }
            break;
        }

        case SYS_CMD_RX_CR:
        {   
            uint8_t cr;
            if (isQuery)
            {
                NVMA_Get_LR_RX_CR(&cr);
                AT_FormatUint8Response(cr, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                if (!AT_ParseUint8(data, &cr))
                {
                    AT_SendStringResponse("ERROR: Invalid RX_CR value\r\n");
                    commandHandled = false;
                    break;
                }
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    cr = Constrain_u8(cr, minValue, maxValue,&constrained);
                    if(constrained)
                    {
                        AT_SendStringResponse("ERROR: RX_CR value out of limit\r\n");
                        commandHandled = false;
                        break;
                    }
                }
                NVMA_Set_LR_RX_CR(cr);
            }
            break;
        }

        case SYS_CMD_HEADERMODE_TX:
        {   
            uint8_t mode;
            if (isQuery)
            {
                NVMA_Get_LR_HeaderMode_TX(&mode);
                AT_FormatUint8Response(mode, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                if (!AT_ParseUint8(data, &mode))
                {
                    AT_SendStringResponse("ERROR: Invalid HEADERMODE_TX value\r\n");
                    commandHandled = false;
                    break;
                }
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    mode = Constrain_u8(mode, minValue, maxValue,&constrained);
                    if(constrained)
                    {
                        AT_SendStringResponse("ERROR: HEADERMODE_TX value out of limit\r\n");
                        commandHandled = false;
                        break;
                    }
                }
                NVMA_Set_LR_HeaderMode_TX(mode);
            }
            break;
        }

        case SYS_CMD_HEADERMODE_RX:
        {   
            uint8_t mode;
            if (isQuery)
            {
                NVMA_Get_LR_HeaderMode_RX(&mode);
                AT_FormatUint8Response(mode, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                if (!AT_ParseUint8(data, &mode))
                {
                    AT_SendStringResponse("ERROR: Invalid HEADERMODE_RX value\r\n");
                    commandHandled = false;
                    break;
                }
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    mode = Constrain_u8(mode, minValue, maxValue,&constrained);
                    if(constrained)
                    {
                        AT_SendStringResponse("ERROR: HEADERMODE_RX value out of limit\r\n");
                        commandHandled = false;
                        break;
                    }
                }
                NVMA_Set_LR_HeaderMode_RX(mode);
            }
            break;
        }

        case SYS_CMD_CRC_TX:
        {   
            uint8_t crc;
            if (isQuery)
            {
                NVMA_Get_LR_CRC_TX(&crc);
                AT_FormatUint8Response(crc, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                if (!AT_ParseUint8(data, &crc))
                {
                    AT_SendStringResponse("ERROR: Invalid CRC_TX value\r\n");
                    commandHandled = false;
                    break;
                }
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    crc = Constrain_u8(crc, minValue, maxValue,&constrained);
                    if(constrained)
                    {
                        AT_SendStringResponse("ERROR: CRC_TX value out of limit\r\n");
                        commandHandled = false;
                        break;
                    }
                }
                NVMA_Set_LR_CRC_TX(crc);
            }
            break;
        }

        case SYS_CMD_CRC_RX:
        {   
            uint8_t crc;
            if (isQuery)
            {
                NVMA_Get_LR_CRC_RX(&crc);
                AT_FormatUint8Response(crc, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                if (!AT_ParseUint8(data, &crc))
                {
                    AT_SendStringResponse("ERROR: Invalid CRC_RX value\r\n");
                    commandHandled = false;
                    break;
                }
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    crc = Constrain_u8(crc, minValue, maxValue,&constrained);
                    if(constrained)
                    {
                        AT_SendStringResponse("ERROR: CRC_RX value out of limit\r\n");
                        commandHandled = false;
                        break;
                    }
                }
                NVMA_Set_LR_CRC_RX(crc);
            }
            break;
        }

        case SYS_CMD_PREAM_SIZE_TX:
        {   
            uint16_t size;
            if (isQuery)
            {
                NVMA_Get_LR_PreamSize_TX(&size);
                AT_FormatUint32Response(size, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                if (!AT_ParseUint32(data, &size))
                {
                    AT_SendStringResponse("ERROR: Invalid PREAM_SIZE_TX value\r\n");
                    commandHandled = false;
                    break;
                }
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    size = Constrain_u16(size, minValue, maxValue,&constrained);
                    if(constrained)
                    {
                        AT_SendStringResponse("ERROR: PREAM_SIZE_TX value out of limit\r\n");
                        commandHandled = false;
                        break;
                    }
                }
                NVMA_Set_LR_PreamSize_TX(size);
            }
            break;
        }

        case SYS_CMD_PREAM_SIZE_RX:
        {   
            uint16_t size;
            if (isQuery)
            {
                NVMA_Get_LR_PreamSize_RX(&size);
                AT_FormatUint32Response(size, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                if (!AT_ParseUint32(data, &size))
                {
                    AT_SendStringResponse("ERROR: Invalid PREAM_SIZE_RX value\r\n");
                    commandHandled = false;
                    break;
                }
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    size = Constrain_u16(size, minValue, maxValue,&constrained);
                    if(constrained)
                    {
                        AT_SendStringResponse("ERROR: PREAM_SIZE_RX value out of limit\r\n");
                        commandHandled = false;
                        break;
                    }
                }
                NVMA_Set_LR_PreamSize_RX(size);
            }
            break;
        }

        case SYS_CMD_RF_TX_HEX:
        {   
            uint8_t packet[256];
            uint8_t packetSize;
            packetSize = HexStringToByteArray((char *)data, packet, sizeof(packet));
            if (packetSize == 0)
            {
                AT_SendStringResponse("ERROR: Invalid HEX data\r\n");
                commandHandled = false;
                break;
            }
            _GSC_Handle_TX(packet, packetSize);
            break;
        }

        case SYS_CMD_RF_TX_TXT:
        {
            // Transmit data via RF in text format
            break;
        }

        case SYS_CMD_RF_TX_FROM_NVM:
        {
            // Transmit saved RF packet from NVM
            break;
        }

        case SYS_CMD_RF_PERIOD_SET:
        {
            uint32_t period;
            if (isQuery)
            {
                NVMA_Get_LR_TX_Period_TX(&period);
                AT_FormatUint32Response(period, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                if (!AT_ParseUint32(data, &period))
                {
                    AT_SendStringResponse("ERROR: Invalid TX_PERIOD value\r\n");
                    commandHandled = false;
                    break;
                }
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    period = Constrain_u32(period, minValue, maxValue,&constrained);
                    if(constrained)
                    {
                        AT_SendStringResponse("ERROR: TX_PERIOD value out of limit\r\n");
                        commandHandled = false;
                        break;
                    }
                }
                NVMA_Set_LR_TX_Period_TX(period);
            }
            break;
        }

        case SYS_CMD_RF_PERIOD_CTRL:
        {
            // Start/Stop periodic TX
            break;
        }

        case SYS_CMD_RF_SAVE_PCKT_NVM:
        {   
            if (isQuery)
            {      
                uint16_t pcktSize;
                NVMA_Get_LR_Saved_Pckt_Size(&pcktSize);
                // NVMA_Get_LR_TX_RF_PCKT(packet,pcktSize);
                // AT_FormatUint32Response(period, (uint8_t *)response, &response_size);
                // hasResponse = true;
            }
            else
            {
                // Save packet to NVM
            }
            break;
        }

        case SYS_CMD_RF_PERIOD_STATUS:
        {
            // Get periodic TX status
            break;
        }

        case SYS_CMD_TX_COMPLETE_SET:
        {
            // Set TX complete callback
            ProcessRFMultiSetCommand(true, (char *)data);
            break;
        }

        case SYS_CMD_RX_COMPLETE_SET:
        {
            // Set RX complete callback
            ProcessRFMultiSetCommand(false, (char *)data);
            break;
        }

        default:
        {
            commandHandled = false;
            AT_SendStringResponse("ERROR\r\n");
            break;
        }
    }

    if (hasResponse)
    {
        AT_SendStringResponse(response);
    }
    else
    {
        if (commandHandled == false)
        {
          //
        }
        else
        {
            AT_SendStringResponse("OK\r\n");
        }
    }

    return commandHandled;
}
  

/**
 * @brief Handle Blue LED command
 * 
 * @param data 
 * @return true 
 * @return false 
 */
static bool _GSC_Handle_BlueLED(uint8_t *data)
{
    if(strcmp((char*) data, "ON") == 0)
    {
       HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET);
    }
    else if(strcmp((char*) data, "OFF") == 0)
    {
       HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET);
    }
    else
    {
        return false;
    }

    return true;
}

/**
 * @brief 
 * 
 * @param data 
 * @param size 
 * @return staic 
 */
static void _GSC_Handle_TX(uint8_t *data, uint8_t size)
{   
    dataQueue_t     txm;
    packet_info_t	*tx_pkt;
    uint8_t         *tx_raw_data;

    tx_raw_data =  pvPortMalloc(size);
    if (tx_raw_data == NULL)
    {
        _exit(313513);
    }

    tx_pkt = pvPortMalloc(sizeof(packet_info_t));
    if (tx_pkt == NULL)
    {
        _exit(314687);
    }
    
    tx_pkt->packet = tx_raw_data;
    tx_pkt->size = size;
    
    txm.ptr = tx_pkt;
    txm.cmd = CMD_RF_SEND_DATA_NOW;

    xQueueSend(queueRadioHandle,&txm,portMAX_DELAY);
}