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

static bool _GSC_Handle_BlueLED(uint8_t *data);

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
    switch (cmd)
    {
        case SYS_LED_BLUE:
            return _GSC_Handle_BlueLED(data);
            break;

        case SYS_CMD_SYS_NAME:
            break;
            default:
            break;
    }

}

/**
 * @brief 
 * 
 * @param data 
 */
static bool _GSC_Handle_BlueLED(uint8_t *data)
{
    if(strcmp(data, "ON") == 0)
    {
       HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET);
    }
    else if(strcmp(data, "OFF") == 0)
    {
       HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET);
    }
    else
    {
        return false;
    }

    return true;
    
}