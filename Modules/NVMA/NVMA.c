/**
 * @file NVMA.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-11-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "main.h"
#include "NVMA.h"
#include "semphr.h"


static SemaphoreHandle_t xEepromMutex;

/**
 * @brief Clear all FLASH error flags before programming
 *        This is necessary because HAL_FLASHEx_DATAEEPROM_Program 
 *        will fail if any error flags are set from previous operations
 */
static void NVMA_ClearFlashErrors(void)
{
    //return;
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | 
                           FLASH_FLAG_SIZERR | FLASH_FLAG_OPTVERR | 
                           FLASH_FLAG_RDERR | FLASH_FLAG_FWWERR | 
                           FLASH_FLAG_NOTZEROERR);
}

void NVMA_Init(void)
{
    
    if(xEepromMutex == NULL)
    {
        xEepromMutex = xSemaphoreCreateMutex();
    }
    
}

/**
 * @brief 
 * 
 * @param freq 
 */
void NVMA_Set_LR_Freq_TX(uint32_t freq)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_FREQ_TX, freq);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param freq 
 */
void NVMA_Get_LR_Freq_TX(uint32_t *freq)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *freq = *((uint32_t *)EE_ADDR_LR_FREQ_TX);
    xSemaphoreGive(xEepromMutex);
}


/**
 * @brief 
 * 
 * @param freq 
 */
void NVMA_Set_LR_Freq_RX(uint32_t freq)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_FREQ_RX, freq);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param freq 
 */
void NVMA_Get_LR_Freq_RX(uint32_t *freq)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *freq = *((uint32_t *)EE_ADDR_LR_FREQ_RX);
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param power 
 */
void NVMA_Set_LR_TX_Power(uint8_t power)
{
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EE_ADDR_LR_TX_POWER, power);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param power 
 */
void NVMA_Get_LR_TX_Power(uint8_t *power)
{
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *power = *((uint8_t *)EE_ADDR_LR_TX_POWER);
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param sf 
 */
void NVMA_Set_LR_TX_SF(uint8_t sf)
{
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EE_ADDR_LR_TX_SF, sf);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param sf 
 */
void NVMA_Get_LR_TX_SF(uint8_t *sf)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *sf = *((uint8_t *)EE_ADDR_LR_TX_SF);
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param sf 
 */
void NVMA_Set_LR_RX_SF(uint8_t sf)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EE_ADDR_LR_RX_SF, sf);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param sf 
 */
void NVMA_Get_LR_RX_SF(uint8_t *sf)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *sf = *((uint8_t *)EE_ADDR_LR_RX_SF);
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param bw 
 */
void NVMA_Set_LR_TX_BW(uint8_t bw)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EE_ADDR_LR_TX_BW, bw);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param bw 
 */
void NVMA_Get_LR_TX_BW(uint8_t *bw)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *bw = *((uint8_t *)EE_ADDR_LR_TX_BW);
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param bw 
 */
void NVMA_Set_LR_RX_BW(uint8_t bw)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EE_ADDR_LR_RX_BW, bw);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param bw 
 */
void NVMA_Get_LR_RX_BW(uint8_t *bw)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *bw = *((uint8_t *)EE_ADDR_LR_RX_BW);
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param iq 
 */
void NVMA_Set_LR_TX_IQ(uint8_t iq)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EE_ADDR_LR_TX_IQ, iq);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param iq 
 */
void NVMA_Get_LR_TX_IQ(uint8_t *iq)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *iq = *((uint8_t *)EE_ADDR_LR_TX_IQ);
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param iq 
 */
void NVMA_Set_LR_RX_IQ(uint8_t iq)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EE_ADDR_LR_RX_IQ, iq);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param iq 
 */
void NVMA_Get_LR_RX_IQ(uint8_t *iq)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *iq = *((uint8_t *)EE_ADDR_LR_RX_IQ);
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param cr 
 */
void NVMA_Set_LR_TX_CR(uint8_t cr)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EE_ADDR_LR_TX_CR, cr);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param cr 
 */
void NVMA_Get_LR_TX_CR(uint8_t *cr)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *cr = *((uint8_t *)EE_ADDR_LR_TX_CR);
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param cr 
 */
void NVMA_Set_LR_RX_CR(uint8_t cr)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EE_ADDR_LR_RX_CR, cr);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param cr 
 */
void NVMA_Get_LR_RX_CR(uint8_t *cr)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *cr = *((uint8_t *)EE_ADDR_LR_RX_CR);
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param mode 
 */
void NVMA_Set_LR_HeaderMode_TX(uint8_t mode)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EE_ADDR_LR_HEADERMODE_TX, mode);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param mode 
 */
void NVMA_Get_LR_HeaderMode_TX(uint8_t *mode)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *mode = *((uint8_t *)EE_ADDR_LR_HEADERMODE_TX);
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param mode 
 */
void NVMA_Set_LR_HeaderMode_RX(uint8_t mode)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EE_ADDR_LR_HEADERMODE_RX, mode);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param mode 
 */
void NVMA_Get_LR_HeaderMode_RX(uint8_t *mode)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *mode = *((uint8_t *)EE_ADDR_LR_HEADERMODE_RX);
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param crc 
 */
void NVMA_Set_LR_CRC_TX(uint8_t crc)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EE_ADDR_LR_CRC_TX, crc);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param crc 
 */
void NVMA_Get_LR_CRC_TX(uint8_t *crc)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *crc = *((uint8_t *)EE_ADDR_LR_CRC_TX);
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param crc 
 */
void NVMA_Set_LR_CRC_RX(uint8_t crc)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EE_ADDR_LR_CRC_RX, crc);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param crc 
 */
void NVMA_Get_LR_CRC_RX(uint8_t *crc)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *crc = *((uint8_t *)EE_ADDR_LR_CRC_RX);
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param size 
 */
void NVMA_Set_LR_PreamSize_TX(uint16_t size)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_PREAM_SIZE_TX, size);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param size 
 */
void NVMA_Get_LR_PreamSize_TX(uint16_t *size)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *size = *((uint16_t *)EE_ADDR_LR_PREAM_SIZE_TX);
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param size 
 */
void NVMA_Set_LR_PreamSize_RX(uint16_t size)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_PREAM_SIZE_RX, size);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param size 
 */
void NVMA_Get_LR_PreamSize_RX(uint16_t *size)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *size = *((uint16_t *)EE_ADDR_LR_PREAM_SIZE_RX);
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param active 
 */
void NVMA_Set_LR_Active_RX_To_UART(uint8_t active)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EE_ADDR_LR_ACTIVE_RX_TO_UART, active);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param active 
 */
void NVMA_Get_LR_Active_RX_To_UART(uint8_t *active)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *active = *((uint8_t *)EE_ADDR_LR_ACTIVE_RX_TO_UART);
    xSemaphoreGive(xEepromMutex);
}


/**
 * @brief 
 * 
 * @param size 
 */
void NVMA_Get_LR_Saved_Pckt_Size(uint16_t   *size)
{
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *size = *((uint16_t *)EE_ADDR_LR_SAVED_PCKT_SIZE);
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param size 
 */
void NVMA_Set_LR_Pckt_Size(uint16_t size)
{
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_SAVED_PCKT_SIZE, size);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}


/**
 * @brief 
 * 
 * @param pckt 
 */
void NVMA_Set_LR_TX_RF_PCKT(uint8_t *pckt, size_t size)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    for (size_t i = 0; i < size; i++)
    {
        HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EE_ADDR_LR_TX_RF_PCKT + (i * sizeof(uint8_t)), pckt[i]);
    }
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param pckt 
 */
void NVMA_Get_LR_TX_RF_PCKT(uint8_t *pckt, size_t size)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    for (size_t i = 0; i < size; i++)
    {
        pckt[i] = *((uint8_t *)(EE_ADDR_LR_TX_RF_PCKT + (i * sizeof(uint8_t))));
    }
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param period 
 */
void NVMA_Set_LR_TX_Period_TX(uint32_t period)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_TX_PERIOD_TX, period);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief 
 * 
 * @param period 
 */
void NVMA_Get_LR_TX_Period_TX(uint32_t *period)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *period = *((uint32_t *)EE_ADDR_LR_TX_PERIOD_TX);
    xSemaphoreGive(xEepromMutex);
}

void NVMA_Set_RX_To_UART(uint8_t active)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EE_ADDR_RX_TO_UART, active);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

void NVMA_Get_RX_TO_UART(uint8_t *active)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *active = *((uint8_t *)EE_ADDR_RX_TO_UART);
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief Set TX LDRO (Low Data Rate Optimization)
 * @param ldro 0=off, 1=on, 2=auto
 */
void NVMA_Set_LR_TX_LDRO(uint8_t ldro)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EE_ADDR_LR_TX_LDRO, ldro);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief Get TX LDRO (Low Data Rate Optimization)
 * @param ldro 0=off, 1=on, 2=auto
 */
void NVMA_Get_LR_TX_LDRO(uint8_t *ldro)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *ldro = *((uint8_t *)EE_ADDR_LR_TX_LDRO);
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief Set RX LDRO (Low Data Rate Optimization)
 * @param ldro 0=off, 1=on, 2=auto
 */
void NVMA_Set_LR_RX_LDRO(uint8_t ldro)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EE_ADDR_LR_RX_LDRO, ldro);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief Get RX LDRO (Low Data Rate Optimization)
 * @param ldro 0=off, 1=on, 2=auto
 */
void NVMA_Get_LR_RX_LDRO(uint8_t *ldro)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *ldro = *((uint8_t *)EE_ADDR_LR_RX_LDRO);
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief Set RX expected payload length (for implicit header mode)
 * @param len Payload length in bytes (1-255, 0 to use actual received size)
 */
void NVMA_Set_LR_RX_PldLen(uint8_t len)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EE_ADDR_LR_RX_PLDLEN, len);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief Get RX expected payload length (for implicit header mode)
 * @param len Pointer to store payload length
 */
void NVMA_Get_LR_RX_PldLen(uint8_t *len)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *len = *((uint8_t *)EE_ADDR_LR_RX_PLDLEN);
    xSemaphoreGive(xEepromMutex);
}
/**
 * @brief Check if baud rate is valid (standard values only)
 * @param baud Baud rate to check
 * @return true if valid, false otherwise
 */
bool NVMA_Is_Valid_Baud(uint32_t baud)
{
    const uint32_t valid_bauds[] = {9600, 19200, 38400, 57600, 115200, 230400};
    for (size_t i = 0; i < sizeof(valid_bauds) / sizeof(valid_bauds[0]); i++)
    {
        if (baud == valid_bauds[i])
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Set UART baud rate
 * @param baud Baud rate (9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600)
 */
void NVMA_Set_UART_Baud(uint32_t baud)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_UART_BAUD, baud);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief Get UART baud rate from NVMA
 * @param baud Pointer to store baud rate
 * @note Returns default 115200 if stored value is invalid
 */
void NVMA_Get_UART_Baud(uint32_t *baud)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *baud = *((uint32_t *)EE_ADDR_UART_BAUD);
    xSemaphoreGive(xEepromMutex);
    
    // Validate - return default if invalid (e.g. 0xFFFFFFFF on fresh EEPROM)
    if (!NVMA_Is_Valid_Baud(*baud))
    {
        *baud = NVMA_DEFAULT_UART_BAUD;
    }
}

/**
 * @brief Set RX output format (HEX or ASCII)
 * @param format 0=HEX, 1=ASCII
 */
void NVMA_Set_RX_Format(uint8_t format)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    NVMA_ClearFlashErrors();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EE_ADDR_RX_FORMAT, format);
    HAL_FLASHEx_DATAEEPROM_Lock();
    xSemaphoreGive(xEepromMutex);
}

/**
 * @brief Get RX output format
 * @param format Pointer to store format (0=HEX, 1=ASCII)
 * @note Returns HEX (0) as default if invalid value stored
 */
void NVMA_Get_RX_Format(uint8_t *format)
{   
    xSemaphoreTake(xEepromMutex, portMAX_DELAY);
    *format = *((uint8_t *)EE_ADDR_RX_FORMAT);
    xSemaphoreGive(xEepromMutex);
    
    // Validate - return HEX as default if invalid
    if (*format > RX_FORMAT_ASCII)
    {
        *format = RX_FORMAT_HEX;
    }
}