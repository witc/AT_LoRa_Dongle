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


const AT_CommandLimit_t AT_CommandLimits[] = {
    {SYS_CMD_TX_FREQ, 150000000, 960000000},        // TX frequency in Hz (100 MHz to 960 MHz)
    {SYS_CMD_RX_FREQ, 150000000, 960000000},        // RX frequency in Hz (100 MHz to 960 MHz)
    {SYS_CMD_TX_POWER, 0, 22},                     // TX power in dBm (0 to 22 dBm)
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

extern osMessageQueueId_t queueRadioHandle;

const uint32_t AllowedBandwidths[] = {7810, 10420, 15630, 20830, 31250, 41670, 62500, 125000, 250000, 500000};
const size_t AllowedBandwidthCount = sizeof(AllowedBandwidths) / sizeof(AllowedBandwidths[0]);

static bool _GSC_Handle_BlueLED(uint8_t *data);
static void _GSC_Handle_TX(uint8_t *data, uint8_t size);
static bool GetCommandLimits(eATCommands cmd, int32_t *minValue, int32_t *maxValue);
static bool IsValidBandwidth(uint32_t bandwidth);
 
/**
 * @brief Parse data to uint32_t
 * 
 * @param data 
 * @return uint32_t 
 */
static uint32_t AT_ParseUint32(uint8_t *data)
{
    return (uint32_t)strtoul((char *)data, NULL, 10);
}

/**
 * @brief Parse data to uint8_t
 * 
 * @param data 
 * @return uint8_t 
 */
static uint8_t AT_ParseUint8(uint8_t *data)
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
static void AT_FormatUint32Response(uint32_t value, uint8_t *response, uint16_t *response_size)
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
static void AT_FormatUint8Response(uint8_t value, uint8_t *response, uint16_t *response_size)
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
            uint32_t freq;
            if (isQuery)
            {
                NVMA_Get_LR_Freq_TX(&freq);
                AT_FormatUint32Response(freq, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                freq = AT_ParseUint32(data);
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
            uint32_t freq;
            if (isQuery)
            {
                NVMA_Get_LR_Freq_RX(&freq);
                AT_FormatUint32Response(freq, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                freq = AT_ParseUint32(data);
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
            uint8_t power;
            if (isQuery)
            {
                NVMA_Get_LR_TX_Power(&power);
                AT_FormatUint8Response(power, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                power = AT_ParseUint8(data);
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
            uint8_t sf;
            if (isQuery)
            {
                NVMA_Get_LR_TX_SF(&sf);
                AT_FormatUint8Response(sf, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                sf = AT_ParseUint8(data);
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
            uint8_t sf;
            if (isQuery)
            {
                NVMA_Get_LR_RX_SF(&sf);
                AT_FormatUint8Response(sf, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                sf = AT_ParseUint8(data);
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
            uint32_t bw;
            if (isQuery)
            {
                NVMA_Get_LR_TX_BW(&bw);
                AT_FormatUint32Response(bw, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                bw = AT_ParseUint32(data);
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
            uint32_t bw;
            if (isQuery)
            {
                NVMA_Get_LR_RX_BW(&bw);
                AT_FormatUint32Response(bw, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                bw = AT_ParseUint32(data);
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
            uint8_t iq;
            if (isQuery)
            {
                NVMA_Get_LR_TX_IQ(&iq);
                AT_FormatUint8Response(iq, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                iq = AT_ParseUint8(data);
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
            uint8_t iq;
            if (isQuery)
            {
                NVMA_Get_LR_RX_IQ(&iq);
                AT_FormatUint8Response(iq, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                iq = AT_ParseUint8(data);
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
            uint8_t cr;

            if (isQuery)
            {
                NVMA_Get_LR_TX_CR(&cr);
                AT_FormatUint8Response(cr, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                cr = AT_ParseUint8(data);
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
            uint8_t cr;
            if (isQuery)
            {
                NVMA_Get_LR_RX_CR(&cr);
                AT_FormatUint8Response(cr, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                cr = AT_ParseUint8(data);
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
            uint8_t mode;
            if (isQuery)
            {
                NVMA_Get_LR_HeaderMode_TX(&mode);
                AT_FormatUint8Response(mode, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                mode = AT_ParseUint8(data);
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
            uint8_t mode;
            if (isQuery)
            {
                NVMA_Get_LR_HeaderMode_RX(&mode);
                AT_FormatUint8Response(mode, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                mode = AT_ParseUint8(data);
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
            uint8_t crc;
            if (isQuery)
            {
                NVMA_Get_LR_CRC_TX(&crc);
                AT_FormatUint8Response(crc, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                crc = AT_ParseUint8(data);
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
            uint8_t crc;
            if (isQuery)
            {
                NVMA_Get_LR_CRC_RX(&crc);
                AT_FormatUint8Response(crc, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                crc = AT_ParseUint8(data);
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
            uint16_t size;
            if (isQuery)
            {
                NVMA_Get_LR_PreamSize_TX(&size);
                AT_FormatUint32Response(size, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                size = (uint16_t)AT_ParseUint32(data);
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
            uint16_t size;
            if (isQuery)
            {
                NVMA_Get_LR_PreamSize_RX(&size);
                AT_FormatUint32Response(size, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                size = (uint16_t)AT_ParseUint32(data);
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    size = Constrain_u16(size, minValue, maxValue);
                }
                NVMA_Set_LR_PreamSize_RX(size);
            }
            break;
        }

        case SYS_CMD_RF_TX_HEX:
        {   uint8_t packet[256];
            uint8_t packetSize;
            packetSize = HexStringToByteArray((char *)data, packet, sizeof(packet));
            if(packetSize == 0)
            {
                AT_SendResponse("ERROR: Invalid HEX data\r\n");
                commandHandled = false;
                break;
            }
            _GSC_Handle_TX(packet,packetSize);
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
                period = (uint32_t)AT_ParseUint32(data);
                if (GetCommandLimits(cmd, &minValue, &maxValue))
                {
                    period = Constrain_u16(period, minValue, maxValue);
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