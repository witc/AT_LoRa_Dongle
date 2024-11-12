/**
 * @file Constrain.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-11-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#ifndef CONSTRAIN_H
#define CONSTRAIN_H

#include <stdint.h>

// Funkce pro omezení hodnot pro různé datové typy
uint8_t Constrain_u8(uint8_t value, uint8_t min, uint8_t max);
uint16_t Constrain_u16(uint16_t value, uint16_t min, uint16_t max);
int32_t Constrain_s32(int32_t value, int32_t min, int32_t max);
float Constrain_f(float value, float min, float max);
uint32_t Constrain_u32(uint32_t value, uint32_t min, uint32_t max);

#endif // CONSTRAIN_H
