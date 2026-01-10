

#ifndef AUX_PIN_LOGIC_H
#define AUX_PIN_LOGIC_H

#include "main.h"
#include "FreeRTOS.h"

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t      pinMask;

    uint32_t      period_ms;
    uint8_t       duty_pct;    // 0–100

    TimerHandle_t timer;
    bool          isHigh;
} AUX_PinControl_t;

#define AUX_PINS_COUNT 8


void AUX_InitTimers(void);
void AUX_StartPWM(uint8_t index, uint16_t period, uint8_t duty);
void AUX_StopPWM(uint8_t index);
#endif