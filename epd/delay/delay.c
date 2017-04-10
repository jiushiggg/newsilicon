//#include "msp430g2755.h"
#include "..\sys_init\datatype.h"
#include "udelay.h"

//#pragma optimize=none
//MONITOR void Delay_us(INT16 us) UPDATA_SEGMENT 
void Delay_us(INT16 us)  
{
	UDELAY_Delay(us);
}
//#pragma optimize=none
//MONITOR void Delay_us_20(void) UPDATA_SEGMENT
void Delay_us_20(void) 
{
	UDELAY_Delay(20);
/*
  volatile UINT16 my_1us_delay_const =  0;
  volatile UINT16 us_delay_const = 0;
  
  
  for(us_delay_const = 0 ;us_delay_const < 1 ;us_delay_const++)
  {
    for(my_1us_delay_const = 0 ; my_1us_delay_const < 8 ; my_1us_delay_const++);
  }
*/
}
/** 毫秒级延时
* @param n delay的毫秒数
*/
//#pragma optimize=none
//MONITOR void Delay_ms(UINT16 ms) UPDATA_SEGMENT
void Delay_ms(UINT16 ms) 
{
/*
  volatile UINT16 my_1ms_delay_const =  1400;
  volatile UINT16 ms_delay_const = 0;
  for(ms_delay_const = 0 ;ms_delay_const < ms ;ms_delay_const++)
  {
    for(my_1ms_delay_const = 0 ; my_1ms_delay_const < 606 ; my_1ms_delay_const++);
  }
 */
	volatile UINT16 ms_delay_const = 0;
	for(ms_delay_const = 0 ;ms_delay_const < ms ;ms_delay_const++)
	{
		UDELAY_Delay(1000);
	}
}
