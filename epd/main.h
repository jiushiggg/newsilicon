#ifndef _MAIN_H_
#define _MAIN_H_

#include "rtcdriver.h"

// Memory manager configuration
#define MAX_BUFFER_SIZE 26
extern uint8_t receiveBuffer[MAX_BUFFER_SIZE];

void myChangeRadioConfig(uint8_t channel, bps_enum bps, uint8_t* ID, uint8_t len);
void slaveTimerExpiredCallback( RTCDRV_TimerID_t id, void *incomingPacket);
void rail_status(char * data);

#endif
