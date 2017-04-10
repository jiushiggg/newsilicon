#ifndef __HOLTEK_MSP430_LED_H__
#define __HOLTEK_MSP430_LED_H__
//#include "msp430g2755.h"
#include "..\sys_init\datatype.h"

#define LED_RED_OUT
#define LED_RED_IO_MODE
#define LED_RED_REN_CLOSE
#define LED_RED_OFF
#define LED_RED_ON

#define LED_GREEN_OUT
#define LED_GREEN_IO_MODE
#define LED_GREEN_REN_CLOSE
#define LED_GREEN_OFF
#define LED_GREEN_ON


#define LED_BLUE_OUT
#define LED_BLUE_IO_MODE
#define LED_BLUE_REN_CLOSE
#define LED_BLUE_OFF
#define LED_BLUE_ON

#pragma pack(1)

typedef struct LED_STATE 
{
  BOOL led_off;
  BOOL timer_enable;
  BOOL  horselamp; 
  UINT32 all_times; 
} LED_STATE;

typedef struct EPD_ATTRIBUTE_PKG_ST  
{

  UINT8 t0_cont;
  UINT8 secur_code[2];
  UINT8 display_time[4];
  UINT8  mag_dis_time;
  UINT8 reseved[2];
  UINT8 led_num;
  UINT8 t0_time;
  UINT8 t1_time;
  UINT16 t2_time;
  UINT16 period;
  UINT16 magnet;
  UINT16 global_crc;
  
}EPD_ATTRIBUTE_PKG_ST;

typedef struct EPD_RC_ST
{
  UINT8  ctrl;
  UINT8  pkg_num[2];
  UINT8  reseved;
  UINT8  secur_code[2];
  UINT8  t0_cont;
  UINT8  led_num;
  UINT8  t0_time;
  UINT8  t1_time;
  UINT16 t2_time;
  UINT16 period;
  UINT16 display_time;
  UINT8 page_num;
  UINT8 led_map[4];
  UINT8 default_page;
  UINT8 reserved1[2];
  UINT16 crc; 
}EPD_RC_ST;

typedef struct LED_LOOP_ST
{
  UINT8  t0_cont;
  UINT8  led_num;
  UINT8  t0_time;
  UINT8  t1_time;
  UINT16 t2_time;
  UINT16 period;
  UINT8 led_map[4];
}LED_LOOP_ST;
#pragma pack()



/*-----------------º¯ÊýÉùÃ÷-------------------*/
void normal_led_color_define(UINT8 tp);
void led_init(void);
void led_all_off(void);
void attr_led_choose(void);
void led_control_fun(void);
//void led_test(void);
UINT16 led_str_init(void);
void rc_led_choose(void);
void load_led_attribute_fun(void);
void led_clear_fun(void);
void led_timer_set(UINT16 cont);
void rc_led_init(void);
#endif
