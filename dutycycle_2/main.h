#ifndef _MAIN_H_
#define _MAIN_H_

#define SLAVE_ON_TIME         15    // time to stay on (ms)
#define SLAVE_OFF_TIME        2500  // time to stay off (ms)
#define SLAVE_BLAST_RX_DELAY  1000    // keep waiting for blast to end (ms)
#define SLEEP_TIME  1000    // keep waiting for blast to end (ms)

void slaveTimerExpiredCallback( RTCDRV_TimerID_t id, void *incomingPacket);
void rail_status(char * data);
#endif
