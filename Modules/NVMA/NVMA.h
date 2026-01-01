/**
 * @file NVMA.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-11-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef NVMA_H
#define NVMA_H
		
#define EE_ADDR_LR_FREQ_TX                      (DATA_EEPROM_BASE)    
#define EE_ADDR_LR_FREQ_RX                      (EE_ADDR_LR_FREQ_TX + sizeof(uint32_t))
#define EE_ADDR_LR_TX_POWER                     (EE_ADDR_LR_FREQ_RX + sizeof(uint32_t))
#define EE_ADDR_LR_TX_SF                        (EE_ADDR_LR_TX_POWER + sizeof(uint32_t))
#define EE_ADDR_LR_RX_SF                        (EE_ADDR_LR_TX_SF + sizeof(uint32_t))
#define EE_ADDR_LR_TX_BW                        (EE_ADDR_LR_RX_SF + sizeof(uint32_t))
#define EE_ADDR_LR_RX_BW                        (EE_ADDR_LR_TX_BW + sizeof(uint32_t))
#define EE_ADDR_LR_TX_IQ                        (EE_ADDR_LR_RX_BW + sizeof(uint32_t))
#define EE_ADDR_LR_RX_IQ                        (EE_ADDR_LR_TX_IQ + sizeof(uint32_t))
#define EE_ADDR_LR_TX_CR                        (EE_ADDR_LR_RX_IQ + sizeof(uint32_t))
#define EE_ADDR_LR_RX_CR                        (EE_ADDR_LR_TX_CR + sizeof(uint32_t))
#define EE_ADDR_LR_HEADERMODE_TX                (EE_ADDR_LR_RX_CR + sizeof(uint32_t))
#define EE_ADDR_LR_HEADERMODE_RX                (EE_ADDR_LR_HEADERMODE_TX + sizeof(uint32_t))
#define EE_ADDR_LR_CRC_TX                       (EE_ADDR_LR_HEADERMODE_RX + sizeof(uint32_t))
#define EE_ADDR_LR_CRC_RX                       (EE_ADDR_LR_CRC_TX + sizeof(uint32_t))
#define EE_ADDR_LR_PREAM_SIZE_TX                (EE_ADDR_LR_CRC_RX + sizeof(uint32_t))
#define EE_ADDR_LR_PREAM_SIZE_RX                (EE_ADDR_LR_PREAM_SIZE_TX + sizeof(uint32_t))

#define EE_ADDR_LR_ACTIVE_RX_TO_UART            (EE_ADDR_LR_PREAM_SIZE_RX + sizeof(uint32_t))
#define EE_ADDR_LR_SAVED_PCKT_SIZE              (EE_ADDR_LR_ACTIVE_RX_TO_UART + sizeof(uint32_t))
#define EE_ADDR_LR_TX_RF_PCKT                   (EE_ADDR_LR_SAVED_PCKT_SIZE + sizeof(uint32_t))   // max is 256 B
#define EE_ADDR_LR_TX_PERIOD_TX                 (EE_ADDR_LR_TX_RF_PCKT + (64*sizeof(uint32_t)))
#define EE_ADDR_RX_TO_UART                      (EE_ADDR_LR_TX_PERIOD_TX + sizeof(uint32_t))    //uint8_t
#define EE_ADDR_LR_TX_LDRO                      (EE_ADDR_RX_TO_UART + sizeof(uint32_t))
#define EE_ADDR_LR_RX_LDRO                      (EE_ADDR_LR_TX_LDRO + sizeof(uint32_t))




void NVMA_Init(void);

void NVMA_Set_LR_Freq_TX(uint32_t freq);
void NVMA_Get_LR_Freq_TX(uint32_t *freq);

void NVMA_Set_LR_Freq_RX(uint32_t freq);
void NVMA_Get_LR_Freq_RX(uint32_t *freq);

void NVMA_Set_LR_TX_Power(uint8_t power);
void NVMA_Get_LR_TX_Power(uint8_t *power);

void NVMA_Set_LR_TX_SF(uint8_t sf);
void NVMA_Get_LR_TX_SF(uint8_t *sf);

void NVMA_Set_LR_RX_SF(uint8_t sf);
void NVMA_Get_LR_RX_SF(uint8_t *sf);

void NVMA_Set_LR_TX_BW(uint8_t bw);
void NVMA_Get_LR_TX_BW(uint8_t *bw);

void NVMA_Set_LR_RX_BW(uint8_t bw);
void NVMA_Get_LR_RX_BW(uint8_t *bw);

void NVMA_Set_LR_TX_IQ(uint8_t iq);
void NVMA_Get_LR_TX_IQ(uint8_t *iq);

void NVMA_Set_LR_RX_IQ(uint8_t iq);
void NVMA_Get_LR_RX_IQ(uint8_t *iq);

void NVMA_Set_LR_TX_CR(uint8_t cr);
void NVMA_Get_LR_TX_CR(uint8_t *cr);

void NVMA_Set_LR_RX_CR(uint8_t cr);
void NVMA_Get_LR_RX_CR(uint8_t *cr);

void NVMA_Set_LR_HeaderMode_TX(uint8_t mode);
void NVMA_Get_LR_HeaderMode_TX(uint8_t *mode);

void NVMA_Set_LR_HeaderMode_RX(uint8_t mode);
void NVMA_Get_LR_HeaderMode_RX(uint8_t *mode);

void NVMA_Set_LR_CRC_TX(uint8_t crc);
void NVMA_Get_LR_CRC_TX(uint8_t *crc);

void NVMA_Set_LR_CRC_RX(uint8_t crc);
void NVMA_Get_LR_CRC_RX(uint8_t *crc);

void NVMA_Set_LR_PreamSize_TX(uint16_t size);
void NVMA_Get_LR_PreamSize_TX(uint16_t *size);

void NVMA_Set_LR_PreamSize_RX(uint16_t size);
void NVMA_Get_LR_PreamSize_RX(uint16_t *size);

void NVMA_Set_LR_Active_RX_To_UART(uint8_t active);
void NVMA_Get_LR_Active_RX_To_UART(uint8_t *active);

void NVMA_Set_LR_Pckt_Size(uint16_t size);
void NVMA_Get_LR_Saved_Pckt_Size(uint16_t *size);

void NVMA_Set_LR_TX_RF_PCKT(uint8_t *pckt, size_t size);
void NVMA_Get_LR_TX_RF_PCKT(uint8_t *pckt, size_t size);

void NVMA_Set_LR_TX_Period_TX(uint32_t period);
void NVMA_Get_LR_TX_Period_TX(uint32_t *period);

void NVMA_Set_RX_To_UART(uint8_t active);
void NVMA_Get_RX_TO_UART(uint8_t *active);

void NVMA_Set_LR_TX_LDRO(uint8_t ldro);
void NVMA_Get_LR_TX_LDRO(uint8_t *ldro);

void NVMA_Set_LR_RX_LDRO(uint8_t ldro);
void NVMA_Get_LR_RX_LDRO(uint8_t *ldro);

#endif // NVMA_H