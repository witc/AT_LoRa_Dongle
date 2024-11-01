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


extern osMessageQId queueMainHandle;

//#define //LOG_TAG "[RADIO-USER]"
//#define //LOG_LEVEL //LOG_LEVEL_DEBUG
//#include "Log.h"

extern SPI_HandleTypeDef hspi1;

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
	//ctx->rfConfig.radioHal.pin_RF_SWITCH_1.port=RF_SW_1_GPIO_Port;
	//ctx->rfConfig.radioHal.pin_RF_SWITCH_1.pin=RF_SW_1_Pin;
	//ctx->rfConfig.radioHal.pin_RF_SWITCH_2.port=RF_SW_2_GPIO_Port;
	//ctx->rfConfig.radioHal.pin_RF_SWITCH_2.pin=RF_SW_2_Pin;
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

	ctx->rfConfig.lastMode = RF_MODE_IDLE;
	return true;

}


/*
 *
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


/*
 *
 */
bool ru_radio_send_packet(uint8_t *data, uint8_t size, radio_context_t	*ctx)
{
	ral_t* ral;
	ralf_t* ralf;

	ralf = &ctx->rfConfig.ralf;	//TODO neni to stejne jako to nad tim?
	ral = &ctx->rfConfig.ralf.ral;

	ral_set_dio_irq_params(ral, RAL_IRQ_TX_DONE );

	ctx->rfConfig.loraParam_tx.pkt_params.pld_len_in_bytes = size;
//	ral_set_lora_pkt_params(ral,&ctx->rfConfig.loraParam_tx.pkt_params); //TODO asi je obsazeno v tom nize

	ralf_setup_lora(ralf, &ctx->rfConfig.loraParam_tx);

	ral_set_pkt_payload(ral, data, size);
	ru_radio_rfSwitch(true,ctx);
	ral_set_tx(ral);

	ctx->rfConfig.lastMode = RF_MODE_TX;

	return true;
}

/*
 *
 */
void ru_radio_rfSwitch(bool tx,radio_context_t	*ctx)
{
	if(tx == true)
	{
		HAL_GPIO_WritePin(ctx->rfConfig.radioHal.pin_RF_SWITCH_1.port, ctx->rfConfig.radioHal.pin_RF_SWITCH_1.pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(ctx->rfConfig.radioHal.pin_RF_SWITCH_2.port, ctx->rfConfig.radioHal.pin_RF_SWITCH_2.pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(ctx->rfConfig.radioHal.pin_RF_SWITCH_1.port, ctx->rfConfig.radioHal.pin_RF_SWITCH_1.pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ctx->rfConfig.radioHal.pin_RF_SWITCH_2.port, ctx->rfConfig.radioHal.pin_RF_SWITCH_2.pin, GPIO_PIN_RESET);
	}

}


/*
 *
 */
void ru_radio_start_rx(radio_context_t	*ctx)
{
	ral_t* ral;
	ralf_t* ralf;
	ral_status_t ret=RAL_STATUS_OK;
	ral = &ctx->rfConfig.ralf.ral;
	ralf = &ctx->rfConfig.ralf;

	ru_radioCleanAndStandby(RAL_STANDBY_CFG_RC, ctx);

//	ctx->rfConfig.loraParam_rx.symb_nb_timeout =
	ret += ralf_setup_lora(ralf, &ctx->rfConfig.loraParam_rx);
	ret += ral_set_dio_irq_params(ral,RAL_IRQ_RX_DONE  | RAL_IRQ_RX_TIMEOUT| RAL_IRQ_RX_CRC_ERROR);
	ret += ral_cfg_rx_boosted(ral,true);	
	ret += ral_set_rx(ral,RAL_RX_TIMEOUT_CONTINUOUS_MODE);

	if(ret != RAL_STATUS_OK) log_error(5315321);

	ctx->rfConfig.lastMode = RF_MODE_RX;
}


/*
 *
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

	if(ret != RAL_STATUS_OK) log_error(68431);

	ctx->rfConfig.lastMode = RF_MODE_CAD;

}


/*
 *
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
			ru_radioCleanAndStandby(RAL_STANDBY_CFG_RC,ctx);
			ru_radio_send_packet(rxm->ptr,rxm->data,ctx);

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

/*
 *
 */
void ru_radio_process_IRQ(radio_context_t *ctx)
{
	radio_modes_e	mode;
	ral_t* 			ral = &ctx->rfConfig.ralf.ral;
	uint8_t			rxPayload[MAX_SIZE_RADIO_BUFFER];
	uint16_t		rxSize;
	ral_irq_t		irqSet;
	int16_t			RSSI;
	uint16_t		crc;
	dataQueue_t		txm;	//tx message
	packet_info_t	*rx_pkt;
	uint8_t			*rx_raw_data;

	mode = ru_get_radio_last_status(ctx);
	ral_get_irq_status(ral, &irqSet);

	switch (mode)
	{
		case RF_MODE_RX:
			LOG_INFO("SX1262: IRQ during RX mode");
		    if (((irqSet & RAL_IRQ_RX_DONE) == RAL_IRQ_RX_DONE) && ((irqSet & RAL_IRQ_RX_CRC_ERROR) != RAL_IRQ_RX_CRC_ERROR))
		    {
		    	if(ral_get_pkt_payload(ral,255,rxPayload,&rxSize) == RAL_STATUS_OK)
		    	{
					ral_get_rssi_inst(ral, &RSSI);	//TODO nefunguje RSSI cteni
					LOG_INFO("RX: %d B, RSSI: %d dBm", rxSize, RSSI);

					rx_raw_data =  pvPortMalloc(rxSize);
					if (rx_raw_data == NULL)
					{
						log_error(313513);
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
			else if(irqSet & RAL_IRQ_RX_CRC_ERROR)
			{
				LOG_DEBUG("Semtech CRC Error");
			}

			ru_radioCleanAndStandby(RAL_STANDBY_CFG_XOSC, ctx);
		    ru_radio_start_rx(ctx);

			break;

		case RF_MODE_TX:
		//	vTaskSuspendAll();
			ru_radio_start_rx(ctx);
		//	xTaskResumeAll();

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


