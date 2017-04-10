#include "datatype.h"
#include "sys_init.h"
#include "..\ext_flash\extern_flash.h"
#include "..\epd_screen\epd_io.h"
//#include "rf.h"
#include "..\file_sys\mini_fs.h"
#include "..\global_variable\global_variable.h"
#include "..\ext_flash\extern_flash.h"
#include "sys_isr.h"
//#include "rf_a7106.h"
#include "..\reed\reed.h"
//#include "write_id.h"
//#include "nfc_iic.h"
#include "..\protocol\three_protocol.h"
#include "..\protocol\three_event.h"
#include "..\osd2_0\osd2_0_cmd.h"

#include <em_emu.h>

void sys_register_init(void)
{
  
}


BOOL sys_sleepwait(void)
{
	return 1;
}


void sys_aclk(void) {
/*
  volatile UINT8 i ;
  BCSCTL1 |= DIVA_3;                         // aclk/12 
  BCSCTL3 |= LFXT1S_2;                       // 低频时钟选择为VLO ACLK选为VLO  
  IFG1 &= ~OFIFG;                            // Clear OSCFault flag  清除振荡器错误中断标志
  for(i=0xFF; i>0; i--);                    //稳定时间
*/
}

void clock_8M_set(void) 
{
  
  
}

void clock_1M_set(void)
{
  
}

BOOL soft_restart_flag(void) 
{
/*
  if(IFG1&WDTIFG)
  {
    IFG1 &=~(WDTIFG);
    return TRUE;
  }
  return FALSE;
*/
	return TRUE;
}

void WathcDogStop(void)
{
						// Stop watchdog timer
}
void SetWathcDog(void)
{

}
void main_clk_init(void)
{
  //soft_restart_flag();//看门狗复位时此标志位会被置位
  clock_8M_set();
  SetWathcDog();
}

void unused_io_init(void)
{
  UNUSED_GPIO26_OUT;
  UNUSED_GPIO26_IO_MODE;
  UNUSED_GPIO26_REN_CLOSE;
  UNUSED_GPIO26_0;
  
  UNUSED_GPIO27_OUT;
  UNUSED_GPIO27_IO_MODE;
  UNUSED_GPIO27_REN_CLOSE;
  UNUSED_GPIO27_0;
  
}

void disable_peripheral(void)
{
  unused_io_init();
  //禁用外部flash
  disable_extern_flash();
  //电子纸屏幕断电
  epd_powerIO_init();
  close_epd();
  //射频A7106断电
//  rf_powerio_init();
//  rf_power_off();
  
}
static void gpio1_feed_wdg_fun(void)
{
  SetWathcDog();
  //P2OUT ^= BIT6;
  //添加唤醒功能的标准为
}



static void ACLK_VLO(void)
{
  SetWathcDog();
  if(sys_timer_2s_open_flag == TRUE)
  {
    time_wdg_fun(TRUE,def_sys_attr.aclk_timer_cycle);//def_sys_attr.gwor_flag_now根据这个数值去启动相应的定时器
  }
  while (1) 
  {   
    
    if(isr_event_response_fun())
      break;

    EMU_EnterEM2(TRUE);
    gpio1_feed_wdg_fun();                     //不能放到后面，放到后面影响问时间
    if(isr_event_response_fun())
      break;
    
  }
  //time_wdg_fun(FALSE,0);
  SetWathcDog();
  
}

void sys_sleep(void)
{
  f_sync();                             //保存文件系统
  flash_spi_init(FALSE);                //外部flash拉高miso，防止漏电
  aclk_wor_ready_fun();
  //进入休眠时，切换喂狗对应的心跳和wor时间值
  isr_reed_enable(TRUE);                //打开干簧管中断
#ifndef EPD_TEST_MODE
//  WRITEID_ISR_OPEN;                     //打开烧录id中断
#endif
#ifdef NFC_ENABLE 
  NFC_INT_INTERRUPT_OPEN;               //打开NFC中断
#endif
  
  
  // gEventFlag = 0;                       //清空系统任务，会影响led中断

  ACLK_VLO();                           //设置mcu进入 MPL_3 模式
  
  isr_reed_enable(FALSE);               //退出休眠后，关闭干簧管中断
#ifndef EPD_TEST_MODE
//  WRITEID_ISR_CLOSE;                    //退出休眠后，关闭烧录id中断
#endif
#ifdef NFC_ENABLE 
  NFC_INT_INTERRUPT_CLOSE;                    //退出休眠后，关闭NFC中断
#endif
  restore_ext_spi_flashio();            //退出休眠后，初始化外部flash的spi功能
  FLASH_DeepPowerDown(FALSE);           //退出休眠后，启用外部flash功能
}


void sys_load_page_display_fun(void)
{
  
  /*
    说明： 1、系统重新上电，或者重启时 ，之前的显示规则，清0 ，重新开始跑
  */
    segment_read(EXT_FLASH_BLOCK4_ADDR, (WORD)&gSys_tp, sizeof(gSys_tp));
    if(gSys_tp.default_page_id != gSys_tp.gpage_nowid) //当前页不等于default页
     {
        gSys_tp.present_page_id = gSys_tp.default_page_id;//要显示的页码改为默认页
        gEventFlag |= EVENT_FALG_DISPLAY_PAGE;
     }
    gpage.flag = FALSE;//关闭切页定时器标志
    //gscreen_working = FALSE;
    epd_scn_tp.screen_busy_flag = FALSE;
    gSys_tp.change_map =0xff;
}

