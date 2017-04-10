#ifndef __HOLTEK_MSP430_DELAY_H__
#define __HOLTEK_MSP430_DELAY_H__

#include "..\sys_init\datatype.h"

/** @defgroup delay
*  系统延时函数
*  @{
*/
void Delay_us(INT16 us);
void Delay_us_20(void) ;
void Delay_ms(UINT16 ms) ;
#endif
