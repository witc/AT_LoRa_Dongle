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

/**
 * @brief 
 * 
 * @param freq 
 */
void NVMA_Set_LR_Freq_TX(uint32_t freq)
{
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_FREQ_TX, freq);
    HAL_FLASHEx_DATAEEPROM_Lock();
}

/**
 * @brief 
 * 
 * @param freq 
 */
void NVMA_Get_LR_Freq_TX(uint32_t *freq)
{
    *freq = *((uint32_t *)EE_ADDR_LR_FREQ_TX);
}


/**
 * @brief 
 * 
 * @param freq 
 */
void NVMA_Set_LR_Freq_RX(uint32_t freq)
{
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_FREQ_RX, freq);
    HAL_FLASHEx_DATAEEPROM_Lock();
}

/**
 * @brief 
 * 
 * @param freq 
 */
void NVMA_Get_LR_Freq_RX(uint32_t *freq)
{
    *freq = *((uint32_t *)EE_ADDR_LR_FREQ_RX);
}

/**
 * @brief 
 * 
 * @param power 
 */
void NVMA_Set_LR_TX_Power(uint8_t power)
{
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_TX_POWER, power);
    HAL_FLASHEx_DATAEEPROM_Lock();
}

/**
 * @brief 
 * 
 * @param power 
 */
void NVMA_Get_LR_TX_Power(uint8_t *power)
{
    *power = *((uint8_t *)EE_ADDR_LR_TX_POWER);
}

/**
 * @brief 
 * 
 * @param sf 
 */
void NVMA_Set_LR_TX_SF(uint8_t sf)
{
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_TX_SF, sf);
    HAL_FLASHEx_DATAEEPROM_Lock();
}

/**
 * @brief 
 * 
 * @param sf 
 */
void NVMA_Get_LR_TX_SF(uint8_t *sf)
{
    *sf = *((uint8_t *)EE_ADDR_LR_TX_SF);
}

/**
 * @brief 
 * 
 * @param sf 
 */
void NVMA_Set_LR_RX_SF(uint8_t sf)
{
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_RX_SF, sf);
    HAL_FLASHEx_DATAEEPROM_Lock();
}

/**
 * @brief 
 * 
 * @param sf 
 */
void NVMA_Get_LR_RX_SF(uint8_t *sf)
{
    *sf = *((uint8_t *)EE_ADDR_LR_RX_SF);
}

/**
 * @brief 
 * 
 * @param bw 
 */
void NVMA_Set_LR_TX_BW(uint8_t bw)
{
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_TX_BW, bw);
    HAL_FLASHEx_DATAEEPROM_Lock();
}

/**
 * @brief 
 * 
 * @param bw 
 */
void NVMA_Get_LR_TX_BW(uint8_t *bw)
{
    *bw = *((uint8_t *)EE_ADDR_LR_TX_BW);
}

/**
 * @brief 
 * 
 * @param bw 
 */
void NVMA_Set_LR_RX_BW(uint8_t bw)
{
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_RX_BW, bw);
    HAL_FLASHEx_DATAEEPROM_Lock();
}

/**
 * @brief 
 * 
 * @param bw 
 */
void NVMA_Get_LR_RX_BW(uint8_t *bw)
{
    *bw = *((uint8_t *)EE_ADDR_LR_RX_BW);
}

/**
 * @brief 
 * 
 * @param iq 
 */
void NVMA_Set_LR_TX_IQ(uint8_t iq)
{
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_TX_IQ, iq);
    HAL_FLASHEx_DATAEEPROM_Lock();
}

/**
 * @brief 
 * 
 * @param iq 
 */
void NVMA_Get_LR_TX_IQ(uint8_t *iq)
{
    *iq = *((uint8_t *)EE_ADDR_LR_TX_IQ);
}

/**
 * @brief 
 * 
 * @param iq 
 */
void NVMA_Set_LR_RX_IQ(uint8_t iq)
{
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_RX_IQ, iq);
    HAL_FLASHEx_DATAEEPROM_Lock();
}

/**
 * @brief 
 * 
 * @param iq 
 */
void NVMA_Get_LR_RX_IQ(uint8_t *iq)
{
    *iq = *((uint8_t *)EE_ADDR_LR_RX_IQ);
}

/**
 * @brief 
 * 
 * @param cr 
 */
void NVMA_Set_LR_TX_CR(uint8_t cr)
{
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_TX_CR, cr);
    HAL_FLASHEx_DATAEEPROM_Lock();
}

/**
 * @brief 
 * 
 * @param cr 
 */
void NVMA_Get_LR_TX_CR(uint8_t *cr)
{
    *cr = *((uint8_t *)EE_ADDR_LR_TX_CR);
}

/**
 * @brief 
 * 
 * @param cr 
 */
void NVMA_Set_LR_RX_CR(uint8_t cr)
{
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_RX_CR, cr);
    HAL_FLASHEx_DATAEEPROM_Lock();
}

/**
 * @brief 
 * 
 * @param cr 
 */
void NVMA_Get_LR_RX_CR(uint8_t *cr)
{
    *cr = *((uint8_t *)EE_ADDR_LR_RX_CR);
}

/**
 * @brief 
 * 
 * @param mode 
 */
void NVMA_Set_LR_HeaderMode_TX(uint8_t mode)
{
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_HEADERMODE_TX, mode);
    HAL_FLASHEx_DATAEEPROM_Lock();
}

/**
 * @brief 
 * 
 * @param mode 
 */
void NVMA_Get_LR_HeaderMode_TX(uint8_t *mode)
{
    *mode = *((uint8_t *)EE_ADDR_LR_HEADERMODE_TX);
}

/**
 * @brief 
 * 
 * @param mode 
 */
void NVMA_Set_LR_HeaderMode_RX(uint8_t mode)
{
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_HEADERMODE_RX, mode);
    HAL_FLASHEx_DATAEEPROM_Lock();
}

/**
 * @brief 
 * 
 * @param mode 
 */
void NVMA_Get_LR_HeaderMode_RX(uint8_t *mode)
{
    *mode = *((uint8_t *)EE_ADDR_LR_HEADERMODE_RX);
}

/**
 * @brief 
 * 
 * @param crc 
 */
void NVMA_Set_LR_CRC_TX(uint8_t crc)
{
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_CRC_TX, crc);
    HAL_FLASHEx_DATAEEPROM_Lock();
}

/**
 * @brief 
 * 
 * @param crc 
 */
void NVMA_Get_LR_CRC_TX(uint8_t *crc)
{
    *crc = *((uint8_t *)EE_ADDR_LR_CRC_TX);
}

/**
 * @brief 
 * 
 * @param crc 
 */
void NVMA_Set_LR_CRC_RX(uint8_t crc)
{
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_CRC_RX, crc);
    HAL_FLASHEx_DATAEEPROM_Lock();
}

/**
 * @brief 
 * 
 * @param crc 
 */
void NVMA_Get_LR_CRC_RX(uint8_t *crc)
{
    *crc = *((uint8_t *)EE_ADDR_LR_CRC_RX);
}

/**
 * @brief 
 * 
 * @param size 
 */
void NVMA_Set_LR_PreamSize_TX(uint16_t size)
{
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_PREAM_SIZE_TX, size);
    HAL_FLASHEx_DATAEEPROM_Lock();
}

/**
 * @brief 
 * 
 * @param size 
 */
void NVMA_Get_LR_PreamSize_TX(uint16_t *size)
{
    *size = *((uint16_t *)EE_ADDR_LR_PREAM_SIZE_TX);
}

/**
 * @brief 
 * 
 * @param size 
 */
void NVMA_Set_LR_PreamSize_RX(uint16_t size)
{
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EE_ADDR_LR_PREAM_SIZE_RX, size);
    HAL_FLASHEx_DATAEEPROM_Lock();
}

/**
 * @brief 
 * 
 * @param size 
 */
void NVMA_Get_LR_PreamSize_RX(uint16_t *size)
{
    *size = *((uint16_t *)EE_ADDR_LR_PREAM_SIZE_RX);
}
