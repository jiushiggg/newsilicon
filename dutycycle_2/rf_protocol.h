#ifndef _RF_FSM_H_
#define _RF_FSM_H_

#include <stdint.h>
#define GROUP_WOR_MAX_VALUE  (UINT8)30
#define HB_PERIOD				90
#define SET_WOR_PERIOD			4
#define GROUP_WOR_PERIOD		2

void rf_fsm(void);
void HeartBeat(void);
void st_remote_init(void);

extern uint8_t received_data;

#endif
