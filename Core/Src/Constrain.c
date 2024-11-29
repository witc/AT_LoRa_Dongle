/**
 * @file Constrain.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-11-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#include "Constrain.h"
#include "stdbool.h"

/**
 * @brief 
 * 
 * @param value 
 * @param min 
 * @param max 
 * @return uint8_t 
 */

uint8_t Constrain_u8(uint8_t value, uint8_t min, uint8_t max, bool *wasConstrained)
{
    if (value < min)
    {
        if (wasConstrained) *wasConstrained = true;
        return min;
    }
    else if (value > max)
    {
        if (wasConstrained) *wasConstrained = true;
        return max;
    }
    else
    {
        if (wasConstrained) *wasConstrained = false;
        return value;
    }
}


/**
 * @brief 
 * 
 * @param value 
 * @param min 
 * @param max 
 * @return uint16_t 
 */
uint16_t Constrain_u16(uint16_t value, uint16_t min, uint16_t max, bool *wasConstrained)
{
    if (value < min)
    {
        if (wasConstrained) *wasConstrained = true;
        return min;
    }
    else if (value > max)
    {
        if (wasConstrained) *wasConstrained = true;
        return max;
    }
    else
    {
        if (wasConstrained) *wasConstrained = false;
        return value;
    }
}

/**
 * @brief 
 * 
 * @param value 
 * @param min 
 * @param max 
 * @return int32_t 
 */
int32_t Constrain_s32(int32_t value, int32_t min, int32_t max,bool *wasConstrained)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

/**
 * @brief 
 * 
 * @param value 
 * @param min 
 * @param max 
 * @return uint32_t 
 */
uint32_t Constrain_u32(uint32_t value, uint32_t min, uint32_t max,bool *wasConstrained)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

/**
 * @brief 
 * 
 * @param value 
 * @param min 
 * @param max 
 * @return float 
 */
float Constrain_f(float value, float min, float max,bool *wasConstrained)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}
