/**
 * @file general_sys_cmd.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-11-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef GENERAL_SYS_CMD_H
#define GENERAL_SYS_CMD_H

#include "main.h"
#include "AT_cmd.h"

typedef struct {
    eATCommands cmd;  // Typ příkazu
    int32_t minValue; // Minimální povolená hodnota
    int32_t maxValue; // Maximální povolená hodnota
    size_t maxLength;   // Maximální délka vstupu (počet znaků)
} AT_CommandLimit_t;



bool GSC_ProcessCommand(eATCommands cmd, uint8_t *data, uint16_t size);

#endif // GENERAL_SYS_CMD_H



