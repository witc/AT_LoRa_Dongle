

#ifndef MODULES_RF_INC_RF_CONFIG_H_
#define MODULES_RF_INC_RF_CONFIG_H_


//TODO: not working
#define RF_DIO1_NVIC		EXTI2_3_IRQn	
#define RF_DIO1_EXTI_LINE	EXTI_IMR_IM5	

// RADIO
#define SX1262
#define TCXO_SETUP_TIME_MS                   5 // [ms]
#define RADIO_TCXO_SETUP_TIME				(TCXO_SETUP_TIME_MS << 6) //Delay duration = RADIO_TCXO_SETUP_TIME *15.625 Âµs
#define RF_USE_TCXO							true


#define DEBUG_FREQ_OFFSET					0

#define MAX_SIZE_RADIO_BUFFER	253

/* RX config */
#define RF_RX_FREQUENCY							869525000+DEBUG_FREQ_OFFSET
#define RF_RX_SF								RAL_LORA_SF9
#define RF_RX_BW								RAL_LORA_BW_125_KHZ
#define RF_RX_CODERATE							RAL_LORA_CR_4_5
#define RF_RX_PREAMBLE_LEN						15	//musi byt >= nez je na vysilaci
#define RF_RX_HEADER_TYPE						RAL_LORA_PKT_EXPLICIT
#define RF_RX_PACKET_LEN						MAX_SIZE_RADIO_BUFFER	//MAX dle calculatoru
#define RF_RX_CRC_ON							true
#define RF_RX_IQ_INVERT							true


/* TX config */
#define RF_TX_FREQUENCY							869525000+DEBUG_FREQ_OFFSET
#define RF_TX_POWER								22
#define RF_TX_SF								RAL_LORA_SF9
#define RF_TX_BW								RAL_LORA_BW_125_KHZ
#define RF_TX_CODERATE							RAL_LORA_CR_4_5
#define RF_TX_PREAMBLE_LEN						10
#define RF_TX_HEADER_TYPE						RAL_LORA_PKT_EXPLICIT
#define RF_TX_CRC_ON							true
#define RF_TX_IQ_INVERT							false



#define RF_MODULATION_LORA					1
#define RF_MODULATION						RF_MODULATION_LORA



#endif /* MODULES_RF_INC_RF_CONFIG_H_ */
