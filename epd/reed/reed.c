#include "reed.h"
#include "..\delay\delay.h"
#include "..\led\led.h"
#include "..\protocol\three_protocol.h"
#include "..\sys_init\sys_init.h"
#include "..\protocol\three_event.h"
#include "..\global_variable\global_variable.h"
#include "..\sys_init\sys_isr.h"
#include <string.h>

void isr_reed_enable(BOOL onoff)
{
  if(onoff)
  {
    REED_IO_INTERRUPT_OPEN;
  }
  else
  {
    REED_IO_INTERRUPT_CLOSE;
  }
}
void isr_reed_init(void)
{
  REED_IO_INTERRUPT_MODE;
  isr_reed_enable(FALSE);
}



void reed_fun_select( UINT16 magnet)
{
#define MAGNET_CMD_REMOTE_HB    0x0001
#define MAGNET_CMD_REMOTE_LED   0x0002
#define MAGNET_CMD_TRIGGER_LED  0x0004
#define MAGNET_CMD_TRANSFER     0x0008
#define MAGNET_CHANGE_PAGE1     0x0010
#define MAGNET_CHANGE_PAGE2     0x0020
#define MAGNET_CHANGE_PAGE3     0x0030
#define MAGNET_CHANGE_PAGE4     0x0040
#define MAGNET_CHANGE_PAGE5     0x0050
#define MAGNET_CHANGE_PAGE6     0x0060
#define MAGNET_CHANGE_PAGE7     0x0070
#define MAGNET_CHANGE_PAGE8     0x0080
  
  if(magnet & MAGNET_CMD_TRIGGER_LED)
  {
    LED_GREEN_ON;
    Delay_ms(30);
    LED_GREEN_OFF;
  }
  if(magnet & MAGNET_CMD_REMOTE_HB)
  {
    SetWathcDog();
    A7106_Tx_Register(CTRL_HBR_REED,5,0);
    SetWathcDog();
  }
  if(magnet & MAGNET_CMD_TRANSFER)
  {
    def_sys_attr.gwor_flag_now = RF_FSM_CMD_RC_EXIT_LOOP;
    gEventFlag |= EVENT_FLAG_RFWORK;
    
  }
  else if(magnet & MAGNET_CMD_REMOTE_LED)
  {
    led_tp_info.all_times = 0;
    house_loop_times = 0;
    if(gmagent2_flag == TRUE)
    { 
      set_led_timer(FALSE,0,0);
      led_clear_fun();
      led_all_off();
      gmagent2_flag = FALSE;
      return;
    }
    gmagent2_flag =TRUE;
    gEventFlag |= EVENT_FLAG_INTER_LED;
    led_all_off();
    led_tp_info.timer_enable = TRUE;
    attr_led_choose();
    led_tp_init_fun(1);
    
  }
  else if(magnet & 0x00f0)
  {
    
    UINT8 pageid=(UINT8)((magnet & 0x00f0)>>4) -1; 
    gpage.flag = FALSE;//停止之前的计数
    if(FALSE == sys_page_display_store_fun(gSys_tp.default_page_id,pageid,gSys_tp.gpage_nowid ,epd_attr_info.mag_dis_time,0))//保存页显示属性 默认页、当前页、切换的页id号、停留时间，已经走过的时间、状态标志
    {
      gerr_info = SYS_SAVE_ERR;
    }
    gEventFlag |= (EVENT_FALG_DISPLAY_PAGE); 
    
  }
  else
    ;
  
}

void magent4_clear_fun(BOOL led_open)
{
  if(epd_attr_info.magnet != MAGNET_CMD_REMOTE_LED)//不等于2表示默认设置为打开
    gmagent2_flag = FALSE;
  
  if(led_open == TRUE) //要开灯
    gmagent2_flag = TRUE;//下个干簧管动作是关灯
  else
    gmagent2_flag = FALSE;
}
