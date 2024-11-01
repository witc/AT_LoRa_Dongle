/*
 * radioUser.h
 *
 *  Created on: Nov 15, 2023
 *      Author: Uzivatel
 */

#ifndef SEMTECHRADIO_RADIOUSER_H_
#define SEMTECHRADIO_RADIOUSER_H_

#include "main.h"
#include "RF_Task.h"

#define RF_MINIMAL_RX_SIZE				5
#define RF_HEART_BEAT_TIMEOUT_MS			(3*CONST_1_MIN)

/*
 *
 */
typedef struct
{
	uint8_t 	*packet;
	uint8_t		size;
	uint8_t		rx_rssi;
	uint8_t		rad;		//TODOJR zatim nepotrebujem urcuje ktere radio patri k paketu
}packet_info_t;


/**
 * @brief 
 * 
 */
typedef enum
{
	RADIO_CMD_NONE=0,
	RADIO_CMD_INIT,
	RADIO_CMD_SLEEP,
	RADIO_CMD_STANDBY,
	RADIO_CMD_STOP_RX,
	RADIO_CMD_START_RX,
	RADIO_CMD_SEND_UNIVERSAL_PAYLOAD_LBT,
	RADIO_CMD_SEND_UNIVERSAL_PAYLOAD_NOW,
	RADIO_CMD_TX_CW,
	RADIO_CMD_START_CAD,
	RADIO_CMD_STOP_TX_AND_DISCARD,

}RFCommands_e;


void ru_radio_process_commands( RFCommands_e cmd,radio_context_t *ctx, const dataQueue_t *rxm);
void ru_radio_process_IRQ( radio_context_t *ctx);
radio_modes_e	ru_get_radio_last_status( radio_context_t	*ctx);
bool ru_radio_send_packet(uint8_t *data, uint8_t size, radio_context_t	*ctx);
bool ru_radioCleanAndStandby(ral_standby_cfg_t standbyMode, radio_context_t *ctx);
bool ru_radioCleanAndSleep( bool onlySleep, radio_context_t *ctx);
bool ru_radioInit(radio_context_t	*ctx);
void ru_sx1262_assign( radio_context_t	*ctx);
void ru_radio_rfSwitch(bool tx,radio_context_t	*ctx);
void ru_radio_start_CAD(radio_context_t	*ctx);
void ru_radio_start_rx(radio_context_t	*ctx);


#endif /* SEMTECHRADIO_RADIOUSER_H_ */
