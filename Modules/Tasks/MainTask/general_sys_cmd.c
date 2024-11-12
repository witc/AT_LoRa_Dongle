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


const AT_CommandLimit_t AT_CommandLimits[] = {
    {SYS_CMD_TX_FREQ, 100000000, 960000000},        // TX frequency in Hz (100 MHz to 960 MHz)
    {SYS_CMD_RX_FREQ, 100000000, 960000000},        // RX frequency in Hz (100 MHz to 960 MHz)
    {SYS_CMD_TX_POWER, 0, 22},                      // TX power in dBm (0 to 22 dBm)
    {SYS_CMD_TX_SF, 5, 12},                         // TX spreading factor (5 to 12)
    {SYS_CMD_RX_SF, 5, 12},                         // RX spreading factor (5 to 12)
    {SYS_CMD_TX_BW, 7810, 500000},                  // TX bandwidth in Hz (7.81 kHz to 500 kHz)
    {SYS_CMD_RX_BW, 7810, 500000},                  // RX bandwidth in Hz (7.81 kHz to 500 kHz)
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

const uint32_t AllowedBandwidths[] = {7810, 10420, 15630, 20830, 31250, 41670, 62500, 125000, 250000, 500000};
const size_t AllowedBandwidthCount = sizeof(AllowedBandwidths) / sizeof(AllowedBandwidths[0]);

static bool _GSC_Handle_BlueLED(uint8_t *data);
static bool GetCommandLimits(eATCommands cmd, int32_t *minValue, int32_t *maxValue);
static bool IsValidBandwidth(uint32_t bandwidth);
 
/**
 * @brief Parse data to uint32_t
 * 
 * @param data 
 * @return uint32_t 
 */
static uint32_t ParseToUint32(uint8_t *data)
{
    return (uint32_t)strtoul((char *)data, NULL, 10);
}

/**
 * @brief Parse data to uint8_t
 * 
 * @param data 
 * @return uint8_t 
 */
static uint8_t ParseToUint8(uint8_t *data)
{
    return (uint8_t)atoi((char *)data);
}

/**
 * @brief Return uint32_t value as a string
 * 
 * @param value 
 * @param response 
 * @param response_size 
 */
static void ReturnUint32(uint32_t value, uint8_t *response, uint16_t *response_size)
{
    *response_size = sprintf((char *)response, "%lu", (unsigned long)value);
}

/**
 * @brief Return uint8_t value as a string
 * 
 * @param value 
 * @param response 
 * @param response_size 
 */
static void ReturnUint8(uint8_t value, uint8_t *response, uint16_t *response_size)
{
    *response_size = sprintf((char *)response, "%u", value);
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
    char response[32];
    uint16_t response_size = 0;
    bool isQuery = (data[0] == '?');
    bool hasResponse = false;
    bool commandHandled = true;

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
            if (isQuery)
            {
                uint32_t freq;
                NVMA_Get_LR_Freq_TX(&freq);
                ReturnUint32(freq, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                uint32_t freq = ParseToUint32(data);
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    freq = Constrain_u32(freq, minValue, maxValue);
                }
                NVMA_Set_LR_Freq_TX(freq);
            }
            break;
        }

        case SYS_CMD_RX_FREQ:
        {
            if (isQuery)
            {
                uint32_t freq;
                NVMA_Get_LR_Freq_RX(&freq);
                ReturnUint32(freq, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                uint32_t freq = ParseToUint32(data);
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    freq = Constrain_u32(freq, minValue, maxValue);
                }
                NVMA_Set_LR_Freq_RX(freq);
            }
            break;
        }

        case SYS_CMD_TX_POWER:
        {
            if (isQuery)
            {
                uint8_t power;
                NVMA_Get_LR_TX_Power(&power);
                ReturnUint8(power, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                uint8_t power = ParseToUint8(data);
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    power = Constrain_u8(power, minValue, maxValue);
                }
                NVMA_Set_LR_TX_Power(power);
            }
            break;
        }

        case SYS_CMD_TX_SF:
        {
            if (isQuery)
            {
                uint8_t sf;
                NVMA_Get_LR_TX_SF(&sf);
                ReturnUint8(sf, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                uint8_t sf = ParseToUint8(data);
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    sf = Constrain_u8(sf, minValue, maxValue);
                }
                NVMA_Set_LR_TX_SF(sf);
            }
            break;
        }

        case SYS_CMD_RX_SF:
        {
            if (isQuery)
            {
                uint8_t sf;
                NVMA_Get_LR_RX_SF(&sf);
                ReturnUint8(sf, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                uint8_t sf = ParseToUint8(data);
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    sf = Constrain_u8(sf, minValue, maxValue);
                }
                NVMA_Set_LR_RX_SF(sf);
            }
            break;
        }

        case SYS_CMD_TX_BW:
        {
            if (isQuery)
            {
                uint32_t bw;
                NVMA_Get_LR_TX_BW(&bw);
                ReturnUint32(bw, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
               uint32_t bw = ParseToUint32(data);
                if (IsValidBandwidth(bw))
                {
                    NVMA_Set_LR_TX_BW(bw);
                }
                else
                {
                    AT_SendResponse("ERROR: Invalid Bandwidth\r\n");
                    commandHandled = false;
                }
            }
            break;
        }

        case SYS_CMD_RX_BW:
        {
            if (isQuery)
            {
                uint32_t bw;
                NVMA_Get_LR_RX_BW(&bw);
                ReturnUint32(bw, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                uint32_t bw = ParseToUint32(data);
                if (IsValidBandwidth(bw))
                {
                    NVMA_Set_LR_RX_BW(bw);
                }
                else
                {
                    AT_SendResponse("ERROR: Invalid Bandwidth\r\n");
                    commandHandled = false;
                }
            }
            break;
        }

        case SYS_CMD_TX_IQ:
        {
            if (isQuery)
            {
                uint8_t iq;
                NVMA_Get_LR_TX_IQ(&iq);
                ReturnUint8(iq, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                uint8_t iq = ParseToUint8(data);
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    iq = Constrain_u8(iq, minValue, maxValue);
                }
                NVMA_Set_LR_TX_IQ(iq);
            }
            break;
        }

        case SYS_CMD_RX_IQ:
        {
            if (isQuery)
            {
                uint8_t iq;
                NVMA_Get_LR_RX_IQ(&iq);
                ReturnUint8(iq, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                uint8_t iq = ParseToUint8(data);
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    iq = Constrain_u8(iq, minValue, maxValue);
                }
                NVMA_Set_LR_RX_IQ(iq);
            }
            break;
        }

        case SYS_CMD_TX_CR:
        {
            if (isQuery)
            {
                uint8_t cr;
                NVMA_Get_LR_TX_CR(&cr);
                ReturnUint8(cr, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                uint8_t cr = ParseToUint8(data);
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    cr = Constrain_u8(cr, minValue, maxValue);
                }
                NVMA_Set_LR_TX_CR(cr);
            }
            break;
        }

        case SYS_CMD_RX_CR:
        {
            if (isQuery)
            {
                uint8_t cr;
                NVMA_Get_LR_RX_CR(&cr);
                ReturnUint8(cr, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                uint8_t cr = ParseToUint8(data);
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    cr = Constrain_u8(cr, minValue, maxValue);
                }
                NVMA_Set_LR_RX_CR(cr);
            }
            break;
        }

        case SYS_CMD_HEADERMODE_TX:
        {
            if (isQuery)
            {
                uint8_t mode;
                NVMA_Get_LR_HeaderMode_TX(&mode);
                ReturnUint8(mode, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                uint8_t mode = ParseToUint8(data);
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    mode = Constrain_u8(mode, minValue, maxValue);
                }
                NVMA_Set_LR_HeaderMode_TX(mode);
            }
            break;
        }

        case SYS_CMD_HEADERMODE_RX:
        {
            if (isQuery)
            {
                uint8_t mode;
                NVMA_Get_LR_HeaderMode_RX(&mode);
                ReturnUint8(mode, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                uint8_t mode = ParseToUint8(data);
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    mode = Constrain_u8(mode, minValue, maxValue);
                }
                NVMA_Set_LR_HeaderMode_RX(mode);
            }
            break;
        }

        case SYS_CMD_CRC_TX:
        {
            if (isQuery)
            {
                uint8_t crc;
                NVMA_Get_LR_CRC_TX(&crc);
                ReturnUint8(crc, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                uint8_t crc = ParseToUint8(data);
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    crc = Constrain_u8(crc, minValue, maxValue);
                }
                NVMA_Set_LR_CRC_TX(crc);
            }
            break;
        }

        case SYS_CMD_CRC_RX:
        {
            if (isQuery)
            {
                uint8_t crc;
                NVMA_Get_LR_CRC_RX(&crc);
                ReturnUint8(crc, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                uint8_t crc = ParseToUint8(data);
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    crc = Constrain_u8(crc, minValue, maxValue);
                }
                NVMA_Set_LR_CRC_RX(crc);
            }
            break;
        }

        case SYS_CMD_PREAM_SIZE_TX:
        {
            if (isQuery)
            {
                uint16_t size;
                NVMA_Get_LR_PreamSize_TX(&size);
                ReturnUint32(size, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                uint16_t size = (uint16_t)ParseToUint32(data);
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    size = Constrain_u16(size, minValue, maxValue);
                }
                NVMA_Set_LR_PreamSize_TX(size);
            }
            break;
        }

        case SYS_CMD_PREAM_SIZE_RX:
        {
            if (isQuery)
            {
                uint16_t size;
                NVMA_Get_LR_PreamSize_RX(&size);
                ReturnUint32(size, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                uint16_t size = (uint16_t)ParseToUint32(data);
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    size = Constrain_u16(size, minValue, maxValue);
                }
                NVMA_Set_LR_PreamSize_RX(size);
            }
            break;
        }

        default:
        {
            commandHandled = false;
            break;
        }
    }

    if (hasResponse)
    {
        AT_SendResponse(response);
    }
    else
    {
        if (commandHandled == false)
        {
            AT_SendResponse("ERROR\r\n");
        }
        else
        {
            AT_SendResponse("OK\r\n");
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