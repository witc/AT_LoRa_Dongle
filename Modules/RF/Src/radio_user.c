/*
 * radioUser.c
 *
 *  Created on: Nov 15, 2023
 *      Author: Uzivatel
 */

#include "main.h"
#include "RF_Config.h"
#include "radio_user.h"
#include "ral.h"
#include "ralf.h"
#include "ralf_sx126x.h"
#include "NVMA.h"


extern osMessageQId queueMainHandle;

#define LOG_TAG "[RADIO-USER]"
#define LOG_LEVEL LOG_LEVEL_NONE
#include "Log.h"

extern SPI_HandleTypeDef hspi1;


// Mapování hodnot 0-9 na šířky pásma v ral_lora_bw_t
static const ral_lora_bw_t BW_MAP[] = {
    RAL_LORA_BW_007_KHZ,   // BW 0 -> 7.81 kHz
    RAL_LORA_BW_010_KHZ,   // BW 1 -> 10.42 kHz
    RAL_LORA_BW_015_KHZ,   // BW 2 -> 15.63 kHz
    RAL_LORA_BW_020_KHZ,   // BW 3 -> 20.83 kHz
    RAL_LORA_BW_031_KHZ,   // BW 4 -> 31.25 kHz
    RAL_LORA_BW_041_KHZ,   // BW 5 -> 41.67 kHz
    RAL_LORA_BW_062_KHZ,   // BW 6 -> 62.5 kHz
    RAL_LORA_BW_125_KHZ,   // BW 7 -> 125 kHz (default for SX1262)
    RAL_LORA_BW_250_KHZ,   // BW 8 -> 250 kHz
    RAL_LORA_BW_500_KHZ    // BW 9 -> 500 kHz
};


/**
 * @brief Get the lora bw from user value object
 * 
 * @param user_value 
 * @return ral_lora_bw_t 
 */
ral_lora_bw_t get_lora_bw_from_user_value(uint8_t user_value)
{
    if (user_value >= sizeof(BW_MAP) / sizeof(BW_MAP[0]))
    {
        return RAL_LORA_BW_125_KHZ;
    }

    return BW_MAP[user_value];
}


ral_lora_cr_t get_lora_cr_from_user_value(uint8_t user_value)
{
	switch (user_value)
	{
		case 45:
			return RAL_LORA_CR_4_5;
		case 46:
			return RAL_LORA_CR_4_6;
		case 47:
			return RAL_LORA_CR_4_7;
		case 48:
			return RAL_LORA_CR_4_8;
		default:
			return RAL_LORA_CR_4_5; // Default to 4/5 if invalid value
	}
}


/**
 * @brief 
 * 
 * @param ctx 
 */
void ru_sx1262_assign( radio_context_t	*ctx)
{
	ctx->rfConfig.radioHal.TCXO_is_used = RF_USE_TCXO;

	ctx->rfConfig.radioHal.AtomicActionEnter=vPortEnterCritical;
	ctx->rfConfig.radioHal.AtomicActionExit=vPortExitCritical;
	ctx->rfConfig.radioHal.pin_BUSY.port=SX1262_BUSY_GPIO_Port;
	ctx->rfConfig.radioHal.pin_BUSY.pin=SX1262_BUSY_Pin;
	ctx->rfConfig.radioHal.pin_RESET.port=SX1262_RESET_GPIO_Port;
	ctx->rfConfig.radioHal.pin_RESET.pin=SX1262_RESET_Pin;
	ctx->rfConfig.radioHal.pin_DIO1.port=SX1262_DIO1_GPIO_Port;
	ctx->rfConfig.radioHal.pin_DIO1.pin=SX1262_DIO1_Pin;
	ctx->rfConfig.radioHal.pin_NSS.port=SX1262_NSS_GPIO_Port;
	ctx->rfConfig.radioHal.pin_NSS.pin=SX1262_NSS_Pin;
	ctx->rfConfig.radioHal.pin_RF_SWITCH_1.port=SX1262_RF_SW_GPIO_Port;
	ctx->rfConfig.radioHal.pin_RF_SWITCH_1.pin=SX1262_RF_SW_Pin;
	ctx->rfConfig.radioHal.pin_RF_SWITCH_2.port=0;
	ctx->rfConfig.radioHal.pin_RF_SWITCH_2.pin=0;
	ctx->rfConfig.radioHal.TCXO_is_used=true;
	ctx->rfConfig.radioHal.DIO2_AS_RF_SWITCH=false;
	ctx->rfConfig.radioHal.target=&hspi1;
	//ctx->rfConfig.radioHal.AtomicActionEnter=vTaskSuspendAll;
	//ctx->rfConfig.radioHal.AtomicActionExit=xTaskResumeAll;

	/* config rx */
	ctx->rfConfig.loraParam_rx.mod_params.bw=RF_RX_BW;
	ctx->rfConfig.loraParam_rx.mod_params.cr=RF_RX_CODERATE;
	ctx->rfConfig.loraParam_rx.mod_params.ldro=0;
	ctx->rfConfig.loraParam_rx.mod_params.sf=RF_RX_SF;
	ctx->rfConfig.loraParam_rx.output_pwr_in_dbm = 0;
	ctx->rfConfig.loraParam_rx.rf_freq_in_hz=RF_RX_FREQUENCY;
	ctx->rfConfig.loraParam_rx.pkt_params.crc_is_on=RF_RX_CRC_ON;
	ctx->rfConfig.loraParam_rx.pkt_params.header_type=RAL_LORA_PKT_EXPLICIT;
	ctx->rfConfig.loraParam_rx.pkt_params.invert_iq_is_on=RF_RX_IQ_INVERT;
	ctx->rfConfig.loraParam_rx.pkt_params.preamble_len_in_symb=8;
	ctx->rfConfig.loraParam_rx.pkt_params.pld_len_in_bytes = RF_RX_PACKET_LEN;
	ctx->rfConfig.loraParam_rx.sync_word = 0x12;	//TODO kompatibilita, 0x12 pro 0x1424 - private, nebo 0x34 pro 0x3444 - public

	/* config tx */
	ctx->rfConfig.loraParam_tx.mod_params.bw=RF_TX_BW;
	ctx->rfConfig.loraParam_tx.mod_params.cr=RF_TX_CODERATE;
	ctx->rfConfig.loraParam_tx.mod_params.ldro=0;
	ctx->rfConfig.loraParam_tx.mod_params.sf=RF_TX_SF;
	ctx->rfConfig.loraParam_tx.output_pwr_in_dbm = RF_TX_POWER;
	ctx->rfConfig.loraParam_tx.rf_freq_in_hz=RF_TX_FREQUENCY;
	ctx->rfConfig.loraParam_tx.pkt_params.crc_is_on=RF_TX_CRC_ON;
	ctx->rfConfig.loraParam_tx.pkt_params.header_type=RAL_LORA_PKT_EXPLICIT;
	ctx->rfConfig.loraParam_tx.pkt_params.invert_iq_is_on=RF_TX_IQ_INVERT;
	ctx->rfConfig.loraParam_tx.pkt_params.preamble_len_in_symb=8;
	ctx->rfConfig.loraParam_tx.pkt_params.pld_len_in_bytes = 1;
	ctx->rfConfig.loraParam_tx.sync_word = 0x12;	//TODO kompatibilita, 0x12 pro 0x1424 - private, nebo 0x34 pro 0x3444 - public


	ctx->rfConfig.ralf = ( ralf_t ) RALF_SX126X_INSTANTIATE( &ctx->rfConfig.radioHal );

}

/**
 * @brief 
 * 
 * @param ctx 
 * @return true 
 * @return false 
 */
bool ru_radioInit(radio_context_t *ctx)
{
	ral_status_t ret=RAL_STATUS_OK;
	ral_t* ral;
	ralf_t* ralf;
	ral = &ctx->rfConfig.ralf.ral;
	ralf = &ctx->rfConfig.ralf;	//TODO neni to stejne jako to nad tim?

	ret+=ral_reset(ral);
	ret+=ral_wakeup(ral);
	ret+=ral_init(ral);
	ret+=ralf_setup_lora(ralf, &ctx->rfConfig.loraParam_rx);
	ret+=ral_set_standby(ral, RAL_STANDBY_CFG_RC);

	ret+=ral_set_dio_irq_params(ral, RAL_IRQ_ALL); //RAL_IRQ_TX_DONE | RAL_IRQ_RX_DONE | RAL_IRQ_RX_CRC_ERROR);
	ret+=ral_clear_irq_status(ral, RAL_IRQ_ALL);
	ret+=ral_set_rx_tx_fallback_mode(ral, RAL_FALLBACK_STDBY_RC);

	if(ret!=RAL_STATUS_OK) return false;

	/* zkouska pripojeni */
	uint8_t read;
	ral_read_reg( ral, 0x08AC, (uint8_t*)&read,1);

	if(read!=0x94)
	{
		return false;
	}

	if(ral_set_standby(ral,RAL_STANDBY_CFG_RC)!=RAL_STATUS_OK) return false;

	//ru_read_all_regs(ctx);

	HAL_NVIC_DisableIRQ(RF_DIO1_NVIC);
	HAL_NVIC_ClearPendingIRQ(RF_DIO1_NVIC);

	// disable irq
	CLEAR_BIT(EXTI->IMR, RF_DIO1_EXTI_LINE);
	// clear irq flag
	WRITE_REG(EXTI->PR, RF_DIO1_EXTI_LINE);
	// enable irq
	SET_BIT(EXTI->IMR, RF_DIO1_EXTI_LINE);

	HAL_NVIC_EnableIRQ(RF_DIO1_NVIC);

	ctx->rfConfig.lastMode = RF_MODE_IDLE;
	ctx->rx_to_uart = true;
	return true;

}



/**
 * @brief 
 * 
 * @param loraParam 
 * @return true 
 * @return false 
 */
bool ru_load_radio_config_tx(ralf_params_lora_t *loraParam)
{	
	uint8_t bw;
	uint8_t cr;
	NVMA_Get_LR_CRC_TX((uint8_t*)&loraParam->pkt_params.crc_is_on);
	NVMA_Get_LR_HeaderMode_TX(&loraParam->pkt_params.header_type);
	NVMA_Get_LR_TX_IQ((uint8_t*)&loraParam->pkt_params.invert_iq_is_on);
	NVMA_Get_LR_PreamSize_TX(&loraParam->pkt_params.preamble_len_in_symb);
	NVMA_Get_LR_TX_SF(&loraParam->mod_params.sf);

	NVMA_Get_LR_TX_CR(&cr);
	cr = get_lora_cr_from_user_value(cr);
	loraParam->mod_params.cr = cr;

	NVMA_Get_LR_Freq_TX(&loraParam->rf_freq_in_hz);
	NVMA_Get_LR_TX_Power((uint8_t *)&loraParam->output_pwr_in_dbm);

	NVMA_Get_LR_TX_BW(&bw);
	bw = get_lora_bw_from_user_value(bw);
	loraParam->mod_params.bw = bw;

	//NVMA_Get_LR_(&loraParam->sync_word);		//TODO
	
	return true;
}

bool ru_load_radio_config_rx(ralf_params_lora_t *loraParam)
{	
	uint8_t bw;
	uint8_t cr;

	NVMA_Get_LR_CRC_RX((uint8_t*)&loraParam->pkt_params.crc_is_on);
	NVMA_Get_LR_HeaderMode_RX(&loraParam->pkt_params.header_type);
	NVMA_Get_LR_RX_IQ((uint8_t*)&loraParam->pkt_params.invert_iq_is_on);
	NVMA_Get_LR_PreamSize_RX(&loraParam->pkt_params.preamble_len_in_symb);
	NVMA_Get_LR_RX_SF(&loraParam->mod_params.sf);
	
	NVMA_Get_LR_RX_CR(&cr);
	cr = get_lora_cr_from_user_value(cr);
	loraParam->mod_params.cr = cr;

	NVMA_Get_LR_Freq_RX(&loraParam->rf_freq_in_hz);

	NVMA_Get_LR_RX_BW(&bw);
	get_lora_bw_from_user_value(bw);
	loraParam->mod_params.bw = bw;
	//NVMA_Get_LR_(&loraParam->sync_word);		//TODO
	
	return true;
}

/**
 * @brief 
 * 
 * @param standbyMode 
 * @param ctx 
 * @return true 
 * @return false 
 */
bool ru_radioCleanAndStandby(ral_standby_cfg_t standbyMode, radio_context_t *ctx)
{
	ral_t* ral;
	ral = &ctx->rfConfig.ralf.ral;

	ctx->rfConfig.lastMode = RF_MODE_IDLE;
	ral_set_standby(ral, standbyMode);

	ru_radio_rfSwitch(false,ctx);

	ral_clear_irq_status(ral, RAL_IRQ_ALL);

	/* restart HB timer */
	osTimerStart(ctx->timers.rfHBTimer.timer,(RF_HEART_BEAT_TIMEOUT_MS));

	return true;
}


/*
 *
 */
bool ru_radioCleanAndSleep( bool onlySleep, radio_context_t *ctx)
{
	ral_t* ral;
	ral = &ctx->rfConfig.ralf.ral;

	ctx->rfConfig.lastMode = RF_MODE_IDLE;
	ral_set_standby(ral, RAL_STANDBY_CFG_RC);

	ru_radio_rfSwitch(false,ctx);
	if(onlySleep)	return true;

	ral_clear_irq_status(ral, RAL_IRQ_ALL);
	ral_set_sleep(ral, RAL_STANDBY_CFG_RC);

	return true;
}


/**
 * @brief 
 * 
 * @param data 
 * @param size 
 * @param ctx 
 * @return true 
 * @return false 
 */
bool ru_radio_send_packet(uint8_t *data, uint8_t size, radio_context_t	*ctx)
{
	ral_t* ral;
	ralf_t* ralf;

	ralf = &ctx->rfConfig.ralf;	//TODO neni to stejne jako to nad tim?
	ral = &ctx->rfConfig.ralf.ral;

	ral_set_dio_irq_params(ral, RAL_IRQ_TX_DONE );

	ctx->rfConfig.loraParam_tx.pkt_params.pld_len_in_bytes = size;
	ru_load_radio_config_tx(&ctx->rfConfig.loraParam_tx);

	//ral_set_lora_pkt_params(ral,&ctx->rfConfig.loraParam_tx.pkt_params); //TODO asi je obsazeno v tom nize
	ralf_setup_lora(ralf, &ctx->rfConfig.loraParam_tx);

	LOG_INFO("TX params: Freq: %lu Hz, SF: %d, BW: %d, CR: %d/%d, Preamble: %d symb, CRC: %s, IQ Inv: %s",
		ctx->rfConfig.loraParam_tx.rf_freq_in_hz,
		ctx->rfConfig.loraParam_tx.mod_params.sf,
		ctx->rfConfig.loraParam_tx.mod_params.bw,
		(ctx->rfConfig.loraParam_tx.mod_params.cr + 4),
		4,
		ctx->rfConfig.loraParam_tx.pkt_params.preamble_len_in_symb,
		ctx->rfConfig.loraParam_tx.pkt_params.crc_is_on ? "ON" : "OFF",
		ctx->rfConfig.loraParam_tx.pkt_params.invert_iq_is_on ? "ON" : "OFF"
	);

	ral_set_pkt_payload(ral, data, size);
	ru_radio_rfSwitch(true,ctx);
	ral_set_tx(ral);

	ctx->rfConfig.lastMode = RF_MODE_TX;

	return true;
}

/**
 * @brief 
 * 
 * @param tx 
 * @param ctx 
 */
void ru_radio_rfSwitch(bool tx,radio_context_t	*ctx)
{
	if(tx == true)
	{	
		if(ctx->rfConfig.radioHal.pin_RF_SWITCH_1.port != NULL)		HAL_GPIO_WritePin(ctx->rfConfig.radioHal.pin_RF_SWITCH_1.port, ctx->rfConfig.radioHal.pin_RF_SWITCH_1.pin, GPIO_PIN_RESET);
		if(ctx->rfConfig.radioHal.pin_RF_SWITCH_2.port != NULL)		HAL_GPIO_WritePin(ctx->rfConfig.radioHal.pin_RF_SWITCH_2.port, ctx->rfConfig.radioHal.pin_RF_SWITCH_2.pin, GPIO_PIN_RESET);
	}
	else
	{
		if(ctx->rfConfig.radioHal.pin_RF_SWITCH_1.port != NULL)	 HAL_GPIO_WritePin(ctx->rfConfig.radioHal.pin_RF_SWITCH_1.port, ctx->rfConfig.radioHal.pin_RF_SWITCH_1.pin, GPIO_PIN_SET);
		if(ctx->rfConfig.radioHal.pin_RF_SWITCH_2.port != NULL)   HAL_GPIO_WritePin(ctx->rfConfig.radioHal.pin_RF_SWITCH_2.port, ctx->rfConfig.radioHal.pin_RF_SWITCH_2.pin, GPIO_PIN_RESET);
	}

}


/**
 * @brief 
 * 
 * @param ctx 
 */
void ru_radio_start_rx(radio_context_t	*ctx)
{
	ral_t* ral;
	ralf_t* ralf;
	ral_status_t ret=RAL_STATUS_OK;
	ral = &ctx->rfConfig.ralf.ral;
	ralf = &ctx->rfConfig.ralf;

	ru_radioCleanAndStandby(RAL_STANDBY_CFG_RC, ctx);

	ru_load_radio_config_rx(&ctx->rfConfig.loraParam_rx);

	// log all rx params
	LOG_INFO("RX params: Freq: %lu Hz, SF: %d, BW: %d, CR: %d/%d, Preamble: %d symb, CRC: %s, IQ Inv: %s",
		ctx->rfConfig.loraParam_rx.rf_freq_in_hz,
		ctx->rfConfig.loraParam_rx.mod_params.sf,
		ctx->rfConfig.loraParam_rx.mod_params.bw,
		(ctx->rfConfig.loraParam_rx.mod_params.cr + 4),
		4,
		ctx->rfConfig.loraParam_rx.pkt_params.preamble_len_in_symb,
		ctx->rfConfig.loraParam_rx.pkt_params.crc_is_on ? "ON" : "OFF",
		ctx->rfConfig.loraParam_rx.pkt_params.invert_iq_is_on ? "ON" : "OFF"
	);

	ret += ralf_setup_lora(ralf, &ctx->rfConfig.loraParam_rx);
	ret += ral_set_dio_irq_params(ral,RAL_IRQ_RX_DONE  | RAL_IRQ_RX_TIMEOUT| RAL_IRQ_RX_CRC_ERROR);
	ret += ral_cfg_rx_boosted(ral,true);	
	ret += ral_set_rx(ral,RAL_RX_TIMEOUT_CONTINUOUS_MODE);

	if(ret != RAL_STATUS_OK) _exit(5315321);

	ctx->rfConfig.lastMode = RF_MODE_RX;
}


/**
 * @brief 
 * 
 * @param ctx 
 */
void ru_radio_start_CAD(radio_context_t	*ctx)
{
	ral_t* ral;
	ralf_t* ralf;
	ral_lora_cad_params_t cadPar;

	ral_status_t ret=RAL_STATUS_OK;
	ral = &ctx->rfConfig.ralf.ral;
	ralf = &ctx->rfConfig.ralf;
	ru_radioCleanAndStandby(RAL_STANDBY_CFG_RC, ctx);

	ret += ralf_setup_lora(ralf, &ctx->rfConfig.loraParam_rx);
	ret += ral_set_dio_irq_params(ral, RAL_IRQ_ALL);

	cadPar.cad_det_min_in_symb = 10;
	cadPar.cad_det_peak_in_symb = 23;
	cadPar.cad_symb_nb = RAL_LORA_CAD_04_SYMB;
	cadPar.cad_timeout_in_ms = 0;
	cadPar.cad_exit_mode = RAL_LORA_CAD_ONLY;

	ret += ral_set_lora_cad_params(ral, &cadPar);
	ret += ral_set_lora_cad(ral);

	if(ret != RAL_STATUS_OK) _exit(68431);

	ctx->rfConfig.lastMode = RF_MODE_CAD;

}


/**
 * @brief 
 * 
 * @param ctx 
 * @return radio_modes_e 
 */
radio_modes_e	ru_get_radio_last_status( radio_context_t	*ctx)
{
	return ctx->rfConfig.lastMode;
}

/**
 *
 * @param cmd
 * @param GlobalData
 * @param ReceiveData
 */
void ru_radio_process_commands(RFCommands_e cmd,radio_context_t *ctx, const dataQueue_t *rxm)
{
	ral_t* ral;
	ral = &ctx->rfConfig.ralf.ral;
	packet_info_t *pkt;

	switch (cmd)
	{
		case RADIO_CMD_INIT:
			ru_radioInit(ctx);
			ru_radioCleanAndStandby(RAL_STANDBY_CFG_RC,ctx);
			break;

		case RADIO_CMD_SLEEP:
			ru_radioCleanAndSleep(false,ctx);
			//ztracime konfiguraci v sx1262!!
			break;

		case RADIO_CMD_STANDBY:
			ru_radioCleanAndStandby(RAL_STANDBY_CFG_RC,ctx);

			break;

		case RADIO_CMD_TX_CW:
			ru_radioCleanAndStandby(RAL_STANDBY_CFG_RC,ctx);
			ru_radio_rfSwitch(true, ctx);
			ral_set_tx_cw(ral);
			ctx->rfConfig.lastMode = RF_MODE_TX;
			break;

		case RADIO_CMD_SEND_UNIVERSAL_PAYLOAD_NOW:
			pkt = (packet_info_t*)rxm->ptr;
			ru_radioCleanAndStandby(RAL_STANDBY_CFG_XOSC,ctx);
			ru_radio_send_packet(pkt->packet,pkt->size,ctx);
			vPortFree(pkt->packet);
			uint32_t toa_ms =  ral_get_lora_time_on_air_in_ms(ral, &ctx->rfConfig.loraParam_tx.pkt_params, &ctx->rfConfig.loraParam_tx.mod_params);
			LOG_INFO("RF data sent: %d B, TOA: %d ms", pkt->size,toa_ms);

			break;

		case RADIO_CMD_SEND_UNIVERSAL_PAYLOAD_LBT:

			break;

		case RADIO_CMD_START_RX:
			ru_radioCleanAndStandby(RAL_STANDBY_CFG_RC,ctx);
			ru_radio_start_rx(ctx);

			break;

		case RADIO_CMD_START_CAD:


			break;

		default:
			break;
	}

}

/**
 * @brief 
 * 
 * @param ctx 
 */
void ru_radio_process_IRQ(radio_context_t *ctx)
{
	radio_modes_e	mode;
	ral_t* 			ral = &ctx->rfConfig.ralf.ral;
	uint8_t			rxPayload[MAX_SIZE_RADIO_BUFFER];
	uint16_t		rxSize;
	ral_irq_t		irqSet;
	int16_t			RSSI;
	dataQueue_t		txm;	//tx message
	packet_info_t	*rx_pkt;
	uint8_t			*rx_raw_data;

	mode = ru_get_radio_last_status(ctx);
	ral_get_irq_status(ral, &irqSet);

	switch (mode)
	{
		case RF_MODE_RX:
	
		    if (((irqSet & RAL_IRQ_RX_DONE) == RAL_IRQ_RX_DONE) && ((irqSet & RAL_IRQ_RX_CRC_ERROR) != RAL_IRQ_RX_CRC_ERROR))
		    {
		    	if(ral_get_pkt_payload(ral,MAX_SIZE_RADIO_BUFFER,rxPayload,&rxSize) == RAL_STATUS_OK)
		    	{
					ral_get_rssi_inst(ral, &RSSI);	
					LOG_INFO("RX: %d B, RSSI: %d dBm", rxSize, (int16_t)RSSI);

					if(ctx->rx_to_uart == true && rxSize > 0)
					{
						rx_raw_data =  pvPortMalloc(rxSize);
						if (rx_raw_data == NULL)
						{
							_exit(313513);
						}

						memcpy(rx_raw_data,&rxPayload,rxSize);

						rx_pkt = pvPortMalloc(sizeof(packet_info_t));
						rx_pkt->packet = rx_raw_data;
						rx_pkt->size = rxSize;
						rx_pkt->rx_rssi = RSSI;

						txm.cmd = CMD_MAIN_RF_RX_PACKET;
						txm.ptr = rx_pkt;

						xQueueSend(queueMainHandle,&txm,portMAX_DELAY);
					}
					
		    	}
		    }
			else if(irqSet & RAL_IRQ_RX_CRC_ERROR)
			{
				LOG_DEBUG("Semtech CRC Error");
			}

		//	ru_radioCleanAndStandby(RAL_STANDBY_CFG_XOSC, ctx);
		    ru_radio_start_rx(ctx);

			break;

		case RF_MODE_TX:
			ru_radio_start_rx(ctx);
			break;

		case RF_MODE_CAD:
			if(irqSet & RAL_IRQ_CAD_DONE )
			{
				ru_radioCleanAndStandby(RAL_STANDBY_CFG_XOSC, ctx);
				if(irqSet & RAL_IRQ_CAD_OK )
				{
					ru_radio_start_rx(ctx);
					break;
				}
			}
			ru_radioCleanAndStandby(RAL_STANDBY_CFG_XOSC, ctx);
			break;

		case RF_MODE_IDLE:
			ru_radioCleanAndStandby(RAL_STANDBY_CFG_RC, ctx);
			break;
		default:
			break;
	}
}


