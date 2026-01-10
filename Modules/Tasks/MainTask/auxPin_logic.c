

#include "auxPin_logic.h"
#include "FreeRTOS.h"



AUX_PinControl_t auxPins[AUX_PINS_COUNT] = {
    {aux1_GPIO_Port, aux1_Pin, 0, 0, NULL, false}, // index 0
    {aux2_GPIO_Port, aux2_Pin, 0, 0, NULL, false}, // index 1
    {aux3_GPIO_Port, aux3_Pin, 0, 0, NULL, false}, // index 2
    {aux4_GPIO_Port, aux4_Pin, 0, 0, NULL, false}, // index 3
    {aux5_GPIO_Port, aux5_Pin, 0, 0, NULL, false}, // index 4
    {aux6_GPIO_Port, aux6_Pin, 0, 0, NULL, false}, // index 5
    {aux7_GPIO_Port, aux7_Pin, 0, 0, NULL, false}, // index 6
    {aux8_GPIO_Port, aux8_Pin, 0, 0, NULL, false}  // index 7
};


/**
 * @brief Common callback for all AUX pins
 * 
 * @param xTimer 
 */
#define AUX_MIN_TIME_MS 1  // Minimální čas pro fázi

static void AUX_TimerCallback(TimerHandle_t xTimer)
{
    AUX_PinControl_t *ctx = (AUX_PinControl_t *) pvTimerGetTimerID(xTimer);

    if (ctx == NULL)
        return;

    if (ctx->isHigh)
    {
        HAL_GPIO_WritePin(ctx->port, ctx->pinMask, GPIO_PIN_RESET);
        ctx->isHigh = false;

        uint32_t lowTime = ctx->period_ms * (100 - ctx->duty_pct) / 100;
        if (lowTime < AUX_MIN_TIME_MS)
            lowTime = AUX_MIN_TIME_MS;

        xTimerChangePeriod(ctx->timer, pdMS_TO_TICKS(lowTime), 0);
    }
    else
    {
        HAL_GPIO_WritePin(ctx->port, ctx->pinMask, GPIO_PIN_SET);
        ctx->isHigh = true;

        uint32_t highTime = ctx->period_ms * ctx->duty_pct / 100;
        if (highTime < AUX_MIN_TIME_MS)
            highTime = AUX_MIN_TIME_MS;

        xTimerChangePeriod(ctx->timer, pdMS_TO_TICKS(highTime), 0);
    }
}


#define AUX_MIN_TIME_MS 1  // minimálně 1 ms pro fázi

void AUX_StartPWM(uint8_t index, uint16_t period, uint8_t duty)
{
    AUX_PinControl_t *ctx = &auxPins[index];

    ctx->period_ms = period;
    ctx->duty_pct = duty;
    ctx->isHigh = false;

    if (ctx->timer == NULL)
        return;

    xTimerStop(ctx->timer, 0);

    if (duty == 0)
    {
        HAL_GPIO_WritePin(ctx->port, ctx->pinMask, GPIO_PIN_RESET);
        return;
    }

    if (duty == 100)
    {
        HAL_GPIO_WritePin(ctx->port, ctx->pinMask, GPIO_PIN_SET);
        return;
    }

    // Výpočet s minimem
    uint32_t highTime = (period * duty) / 100;
    uint32_t lowTime = period - highTime;

    if (highTime < AUX_MIN_TIME_MS) highTime = AUX_MIN_TIME_MS;
    if (lowTime < AUX_MIN_TIME_MS) lowTime = AUX_MIN_TIME_MS;

    // Nastartuj timer rovnou do HIGH fáze
    xTimerChangePeriod(ctx->timer, pdMS_TO_TICKS(highTime), 0);
    HAL_GPIO_WritePin(ctx->port, ctx->pinMask, GPIO_PIN_SET);
    ctx->isHigh = true;
    xTimerStart(ctx->timer, 0);
}



void AUX_InitTimers(void)
{
    for (uint8_t i = 0; i < AUX_PINS_COUNT; i++)
    {
        if (auxPins[i].timer == NULL)
        {
            auxPins[i].timer = xTimerCreate(
                "AUX_PWM",
                pdMS_TO_TICKS(1),     // Dummy hodnota, změní se později
                pdFALSE,
                &auxPins[i],          // TimerID = pointer na konkrétní AUX pin
                AUX_TimerCallback
            );
        }
    }
}


void AUX_StopPWM(uint8_t index)
{
    AUX_PinControl_t *ctx = &auxPins[index];

    if (ctx->timer != NULL)
    {
        xTimerStop(ctx->timer, 0);
        HAL_GPIO_WritePin(ctx->port, ctx->pinMask, GPIO_PIN_RESET); // pin do LOW
        ctx->isHigh = false;
    }
}
