#ifndef __HOLTEK_MSP430_REED_H__
#define __HOLTEK_MSP430_REED_H__
#include "..\sys_init\datatype.h"

#define REED_IO_INTERRUPT_MODE
#define REED_IO_INTERRUPT_OPEN
#define REED_IO_INTERRUPT_CLOSE
#define REED_IO_INTERRUPT_READY
#define REED_READ_INTERRUPT_IO		1 //todo


void isr_reed_init(void);
void isr_reed_enable(BOOL onoff);
void reed_fun_select( UINT16 magnet);
void magent4_clear_fun(BOOL led_open);
void quit_magnet_change_page_fun(void);
#endif
