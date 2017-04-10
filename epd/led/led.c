#include "led.h"
#include "global_variable.h"
#include "sys_init.h"
#include "delay.h"
#include <string.h>
#include "three_event.h"
#include "crc16.h"
#include "reed.h"
#include "sys_isr.h"
#include "datatype.h"

void rc_led_init(void)
{
  
  if(((rc_attr_info.t0_time ==0)&&(rc_attr_info.t1_time ==0)&&(rc_attr_info.t2_time ==0)&&(rc_attr_info.period ==0)) ||(rc_attr_info.period>0))
  {
    led_tp_init_fun(0);
    rc_led_choose();
    led_tp_info.timer_enable = TRUE;
    led_tp_info.all_times  = 0;
    house_loop_times = 0;
    gEventFlag |= EVENT_FLAG_INTER_LED;
    led_all_off();
    magent4_clear_fun(TRUE);
  }
  
}
void led_all_off(void)
{
  LED_RED_OFF;
  LED_GREEN_OFF;
  LED_BLUE_OFF;
}
void led_clear_fun(void)
{
  led_tp_info.led_off = TRUE;
  led_tp_info.all_times = 0;
  led_tp_info.timer_enable = FALSE;
  house_loop_times = 0;
  hose_led_choose = 0;
}


void house_led_color_define(UINT8 tp)
{
  
  if(led_tp_info.led_off == TRUE)
  {
    led_all_off();
    return ;
  }
  switch(tp)
  {
  case 0x01:
    LED_RED_ON;
    //  LED_GREEN_OFF;
    //  LED_BLUE_OFF;
    break;
  case 0x02:
    LED_RED_ON;
    LED_BLUE_ON;
    //  LED_GREEN_OFF;
    break;
  case 0x03:
    LED_RED_ON;
    LED_GREEN_ON;
    // LED_BLUE_OFF;
    break;
  case 0x04:
    LED_GREEN_ON;
    // LED_RED_OFF;
    // LED_BLUE_OFF;
    break;
  case 0x05:
    LED_BLUE_ON;
    // LED_RED_OFF;
    // LED_GREEN_OFF;
    break;
  case 0x06:
    LED_GREEN_ON;
    LED_RED_ON;
    LED_BLUE_ON;
    break;
  case 0x07:
    LED_GREEN_ON;
    // LED_BLUE_ON;
    //LED_RED_OFF;
    break;
  default:
    led_all_off();
    break;
  }
}

UINT8 get_led_older_fun(void)
{
#define SINGKE_LED_FLAG ((UINT32)0x0000000f)
  
  UINT8 tp=0,ty= 0;
loop:
  tp =  hose_led_choose / 2;
  ty =  hose_led_choose % 2;
  
  tp = led_tp.led_map[tp] >> (4-ty*4);
  tp &=0x0f;
  
  if(tp == 0 && hose_led_choose == 0)
  { 
    house_loop_times = 0;
    led_tp_info.all_times = 0;
    led_tp_info.timer_enable = FALSE;
    led_tp_info.led_off = TRUE;
    return 0;
  }
  if(tp == 0)
  {
    house_loop_times++;
    hose_led_choose = 0;
    goto loop;
  }
  return (UINT8)tp;
  
}

void normal_led_color_define(UINT8 tp)
{
  
  if(led_tp_info.led_off == TRUE)
  {
    led_all_off();
    return ;
  }
  switch(tp)
  {
  case 0x01:
    LED_BLUE_ON; 
    break;
  case 0x02:
    LED_RED_ON;
    break;
  case 0x03:
    LED_RED_ON;
    LED_BLUE_ON; 
    break;
  case 0x04:
    LED_GREEN_ON;
    break;
  case 0x05:
    LED_GREEN_ON;
    LED_BLUE_ON; 
    break;
  case 0x06:
    LED_GREEN_ON;
    LED_RED_ON;
    break;
  case 0x07:
    LED_RED_ON;
    LED_GREEN_ON;
    LED_BLUE_ON;
    break;
  default:
    LED_GREEN_ON;
    break;
  }
}

void rc_led_choose(void)
{
  
  if(rc_attr_info.led_num == 0)
    led_tp_info.horselamp = TRUE;   
  else
    led_tp_info.horselamp = FALSE;
}
void attr_led_choose(void)
{
  /*
  if(epd_attr_info.led_num == 0)
  led_tp_info.horselamp = TRUE;   
  else
  */
  led_tp_info.horselamp = FALSE;
}
void def_rc_attr_info_fun(void)
{
  UINT8 aa[4] = {0x12,0x34,0x56,0x70};
  memset((UINT8 *)&rc_attr_info,0x00,sizeof(rc_attr_info));
  rc_attr_info.t0_cont = 5;//1\3\5
  rc_attr_info.t0_time = 2;
  rc_attr_info.t1_time = 2;
  rc_attr_info.t2_time = 20;
  rc_attr_info.period = 2;
  rc_attr_info.led_num = 7;//green
  memcpy(rc_attr_info.led_map,aa,4);
  
}

void def_led_attr_info_fun(void)
{
  UINT16 code = 60488;
  memset((UINT8 *)&epd_attr_info,0x00,sizeof(epd_attr_info));
  epd_attr_info.t0_cont = 3;//1\3\5
  epd_attr_info.t0_time = 1;
  epd_attr_info.t1_time = 1;
  epd_attr_info.t2_time = 30;
  epd_attr_info.led_num = 4;//green
  epd_attr_info.period = 5;
  memcpy(epd_attr_info.secur_code,(UINT8 *)&code,sizeof(code));
  // epd_attr_info.secur_code = 60488;
  epd_attr_info.magnet = 0x00;
  
  attr_led_choose();
}

UINT16 led_house_loop(void)
{
  led_tp_info.all_times += 1;
  
  if((house_loop_times >= led_tp.period) ||(led_tp.period==0))
  {
    led_tp_info.timer_enable = FALSE;
    led_tp_info.led_off = TRUE;
    house_loop_times = 0;
    led_tp_info.all_times = 0;
    magent4_clear_fun(FALSE);
    // led_tp_info.magent2_flag = TRUE;
    return 0;
  }
  
  if(led_tp_info.all_times % (led_tp.t0_cont+1) == 0 )
  {
    // led_tp_info.all_times = 0;
    hose_led_choose++;
    led_tp_info.led_off = TRUE;
    return (led_tp.t2_time);
  }
  if(led_tp_info.all_times %2 == 0)
  {
    
    led_tp_info.led_off = TRUE;
    return (led_tp.t1_time);
  }
  else
  {
    led_tp_info.led_off = FALSE;
    return (led_tp.t0_time);
  }
}

UINT16 led_normal_loop(void)
{
  led_tp_info.all_times += 1;
  
  if((led_tp_info.all_times >=  (UINT32)( (UINT32)(led_tp.t0_cont+1) * (UINT32)(led_tp.period))) || (led_tp.period == 0) )
  {
    led_tp_info.timer_enable = FALSE;
    led_tp_info.led_off = TRUE;
    led_tp_info.all_times=0;
    magent4_clear_fun(FALSE);
    //led_tp_info.magent2_flag = TRUE;
    return 0;
  }
  if(led_tp_info.all_times % (led_tp.t0_cont+1) == 0 )
  {
    led_tp_info.led_off = TRUE;
    return (led_tp.t2_time);
  }
  if(led_tp_info.all_times %2 == 0)
  {
    led_tp_info.led_off = TRUE;
    return (led_tp.t1_time);
  }
  else
  {
    led_tp_info.led_off = FALSE;
    return (led_tp.t0_time);
  }
}

UINT16 led_str_init(void)
{ 
  
  if(led_tp_info.horselamp)
  {
    //return rc_attr_info.t2_time*30;
    return led_house_loop();
  }
  else
    return led_normal_loop();
  
  
}

void led_control_fun(void)
{
  if(led_tp_info.horselamp)
  {
    UINT8 tp;
    tp = get_led_older_fun();
    normal_led_color_define(tp);
    //house_led_color_define(tp);
  }
  else
    normal_led_color_define(led_tp.led_num);
}
void led_init(void)
{
  LED_RED_OUT;
  LED_RED_IO_MODE;
  LED_RED_REN_CLOSE;
  
  LED_GREEN_OUT;
  LED_GREEN_IO_MODE;
  LED_GREEN_REN_CLOSE;
  
  LED_BLUE_OUT;
  LED_BLUE_IO_MODE;
  LED_BLUE_REN_CLOSE;
  led_all_off();
  
  //  def_led_attr_info_fun();
  //  def_rc_attr_info_fun();
  //  led_tp_init_fun(0);
  
}
void load_attribute_pkg_fun(void)
{ 
  UINT16 crc = 0;
  UINT8 i = 0;
  for(i=0;i<3; i++)
  {
    crc = 0;
    memset((UINT8 *)&epd_attr_info,0x00,sizeof(epd_attr_info)); 
    segment_read(EXT_FLASH_BLOCK3_ADDR, (WORD)&epd_attr_info, sizeof(epd_attr_info));
    crc =  my_cal_crc16(crc,(UINT8 *)&epd_attr_info, sizeof(epd_attr_info)-sizeof(epd_attr_info.global_crc));
    if (epd_attr_info.global_crc == crc)
    {
      goto loop;
    }
    Delay_ms(3);
  }
  
  def_led_attr_info_fun();
loop:
  led_tp_init_fun(epd_attr_info.led_num);
  
}
void rc_led_pkg_fun(void)
{
  /*
  UINT16 crc = 0;
  UINT8 i = 0;
  for(i=0;i<3; i++)
  {
  crc = 0;
  memset((UINT8 *)&rc_attr_info,0x00,sizeof(rc_attr_info)); 
  segment_read(EXT_FLASH_BLOCK4_ADDR, (WORD)&rc_attr_info, sizeof(rc_attr_info));
  crc =  my_cal_crc16(crc,(UINT8 *)&rc_attr_info, sizeof(rc_attr_info)-sizeof(rc_attr_info.crc));
  crc =  my_cal_crc16(crc,(UINT8 *)&INFO_DATA.gRFInitData.esl_id, sizeof(RFID));
  if (rc_attr_info.crc == crc)
  {
  break;
}
  Delay_ms(3);
}*/
  def_rc_attr_info_fun();
}
void load_led_attribute_fun(void)
{
  //rc_led_pkg_fun();//遥控器命令不用初始化
  load_attribute_pkg_fun();
  timer_cont_rst();
  //test
  // def_led_attr_info_fun();
}

void led_timer_set(UINT16 cont)
{
  if(cont == 0)
    set_led_timer(FALSE,0,0);
  else   
    set_led_timer(led_tp_info.timer_enable,cont,g_timer_tp.led_30ms_cont);
  
}
