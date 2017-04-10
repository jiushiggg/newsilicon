#ifndef __HOLTEK_MSP430_SYSISR_H__
#define __HOLTEK_MSP430_SYSISR_H__

#include <rtcdriver.h>

#define LED_30MS_TIME_CONT              ((UINT16)39)
#define SMCLK_8M_DIV8_5MS	        ((UINT16)5000)
#define RF_RP_WKUP_TIME_3MS_HALF        ((UINT16)1575)
#define RF_GET_WKUP_TIME_3MS            ((UINT16)3150)
#define TIMER_SMCLK_30MS_CONT           ((UINT16)30000)
#define TIMER_SMCLK_50MS_CONT           ((UINT16)50000)

VOID smclk_timer_isr(BOOL bOnOff, UINT16 times, UINT16 cycle);
void sys_wakeup_wait(UINT32 tm);
BOOL isr_event_response_fun(void);
VOID set_led_timer(BOOL bOnOff,UINT16 times, UINT16 n);
void time_wdg_fun(BOOL bOnOff, UINT16 n);
BOOL timer_calibration_fun(void);
void timer_cont_rst(void);
BOOL timer_cont_calibration_check(void);
extern void slaveTimerExpiredCallback( RTCDRV_TimerID_t id, void *incomingPacket);

#endif
