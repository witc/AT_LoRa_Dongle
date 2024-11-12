/**
 * @file general_sys_cmd.c
 * @author your name (you@domain.com)
 * @brief 
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

static bool _GSC_Handle_BlueLED(uint8_t *data);


/**
 * @brief 
 * 
 * @param data 
 * @return uint32_t 
 */
static uint32_t ParseToUint32(uint8_t *data)
{
    return (uint32_t)strtoul((char *)data, NULL, 10);
}
/**
 * @brief 
 * 
 * @param data 
 * @return uint8_t 
 */
static uint8_t ParseToUint8(uint8_t *data)
{
    return (uint8_t)atoi((char *)data);
}

/**
 * @brief 
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
 * @brief 
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
 * @brief 
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
    char response[32];         // Buffer pro odpověď
    uint16_t response_size = 0;
    bool isQuery = (data[0] == '?');  // Kontrola, zda jde o dotaz
    bool hasResponse = false;
    bool commandHandled = true;

    switch (cmd)
    {   
        case SYS_LED_BLUE:
            commandHandled = _GSC_Handle_BlueLED(data);
            break;

        case SYS_CMD_TX_FREQ:
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
                NVMA_Set_LR_Freq_TX(freq);
            }
            break;

        case SYS_CMD_RX_FREQ:
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
                NVMA_Set_LR_Freq_RX(freq);
            }
            break;

        case SYS_CMD_TX_POWER:
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
                NVMA_Set_LR_TX_Power(power);
            }
            break;

        case SYS_CMD_TX_SF:
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
                NVMA_Set_LR_TX_SF(sf);
            }
            break;

        case SYS_CMD_RX_SF:
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
                NVMA_Set_LR_RX_SF(sf);
            }
            break;

        case SYS_CMD_TX_BW:
            if (isQuery)
            {
                uint8_t bw;
                NVMA_Get_LR_TX_BW(&bw);
                ReturnUint8(bw, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                uint8_t bw = ParseToUint8(data);
                NVMA_Set_LR_TX_BW(bw);
            }
            break;

        case SYS_CMD_RX_BW:
            if (isQuery)
            {
                uint8_t bw;
                NVMA_Get_LR_RX_BW(&bw);
                ReturnUint8(bw, (uint8_t *)response, &response_size);
                hasResponse = true;
            }
            else
            {
                uint8_t bw = ParseToUint8(data);
                NVMA_Set_LR_RX_BW(bw);
            }
            break;

        case SYS_CMD_TX_IQ:
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
                NVMA_Set_LR_TX_IQ(iq);
            }
            break;

        case SYS_CMD_RX_IQ:
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
                NVMA_Set_LR_RX_IQ(iq);
            }
            break;

        case SYS_CMD_TX_CR:
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
                NVMA_Set_LR_TX_CR(cr);
            }
            break;

        case SYS_CMD_RX_CR:
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
                NVMA_Set_LR_RX_CR(cr);
            }
            break;

        case SYS_CMD_HEADERMODE_TX:
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
                NVMA_Set_LR_HeaderMode_TX(mode);
            }
            break;

        case SYS_CMD_HEADERMODE_RX:
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
                NVMA_Set_LR_HeaderMode_RX(mode);
            }
            break;

        case SYS_CMD_CRC_TX:
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
                NVMA_Set_LR_CRC_TX(crc);
            }
            break;

        case SYS_CMD_CRC_RX:
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
                NVMA_Set_LR_CRC_RX(crc);
            }
            break;

        case SYS_CMD_PREAM_SIZE_TX:
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
                NVMA_Set_LR_PreamSize_TX(size);
            }
            break;

        case SYS_CMD_PREAM_SIZE_RX:
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
                NVMA_Set_LR_PreamSize_RX(size);
            }
            break;

        default:
            commandHandled = false;
            break;
    }

    if (hasResponse)
    {
        AT_SendResponse(response);
    }
    else
    {
        if(commandHandled == false)
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
 * @brief 
 * 
 * @param data 
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