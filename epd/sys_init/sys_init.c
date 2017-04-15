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
  BCSCTL3 |= LFXT1S_2;                       // ��Ƶʱ��ѡ��ΪVLO ACLKѡΪVLO  
  IFG1 &= ~OFIFG;                            // Clear OSCFault flag  ������������жϱ�־
  for(i=0xFF; i>0; i--);                    //�ȶ�ʱ��
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
  //soft_restart_flag();//���Ź���λʱ�˱�־λ�ᱻ��λ
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
  //�����ⲿflash
  disable_extern_flash();
  //����ֽ��Ļ�ϵ�
  epd_powerIO_init();
  close_epd();
  //��ƵA7106�ϵ�
//  rf_powerio_init();
//  rf_power_off();
  
}
static void gpio1_feed_wdg_fun(void)
{
  SetWathcDog();
  //P2OUT ^= BIT6;
  //��ӻ��ѹ��ܵı�׼Ϊ
}



static void ACLK_VLO(void)
{
  SetWathcDog();
  if(sys_timer_2s_open_flag == TRUE)
  {
    time_wdg_fun(TRUE,def_sys_attr.aclk_timer_cycle);//def_sys_attr.gwor_flag_now���������ֵȥ������Ӧ�Ķ�ʱ��
  }
  while (1) 
  {   
    
    if(isr_event_response_fun())
      break;

    EMU_EnterEM2(TRUE);
    gpio1_feed_wdg_fun();                     //���ܷŵ����棬�ŵ�����Ӱ����ʱ��
    if(isr_event_response_fun())
      break;
    
  }
  //time_wdg_fun(FALSE,0);
  SetWathcDog();
  
}

void sys_sleep(void)
{
  f_sync();                             //�����ļ�ϵͳ
  flash_spi_init(FALSE);                //�ⲿflash����miso����ֹ©��
  aclk_wor_ready_fun();
  //��������ʱ���л�ι����Ӧ��������worʱ��ֵ
  isr_reed_enable(TRUE);                //�򿪸ɻɹ��ж�
#ifndef EPD_TEST_MODE
//  WRITEID_ISR_OPEN;                     //����¼id�ж�
#endif
#ifdef NFC_ENABLE 
  NFC_INT_INTERRUPT_OPEN;               //��NFC�ж�
#endif
  
  
  // gEventFlag = 0;                       //���ϵͳ���񣬻�Ӱ��led�ж�

  ACLK_VLO();                           //����mcu���� MPL_3 ģʽ
  
  isr_reed_enable(FALSE);               //�˳����ߺ󣬹رոɻɹ��ж�
#ifndef EPD_TEST_MODE
//  WRITEID_ISR_CLOSE;                    //�˳����ߺ󣬹ر���¼id�ж�
#endif
#ifdef NFC_ENABLE 
  NFC_INT_INTERRUPT_CLOSE;                    //�˳����ߺ󣬹ر�NFC�ж�
#endif
  restore_ext_spi_flashio();            //�˳����ߺ󣬳�ʼ���ⲿflash��spi����
  FLASH_DeepPowerDown(FALSE);           //�˳����ߺ������ⲿflash����
}


void sys_load_page_display_fun(void)
{
  
  /*
    ˵���� 1��ϵͳ�����ϵ磬��������ʱ ��֮ǰ����ʾ������0 �����¿�ʼ��
  */
    segment_read(EXT_FLASH_BLOCK4_ADDR, (WORD)&gSys_tp, sizeof(gSys_tp));
    if(gSys_tp.default_page_id != gSys_tp.gpage_nowid) //��ǰҳ������defaultҳ
     {
        gSys_tp.present_page_id = gSys_tp.default_page_id;//Ҫ��ʾ��ҳ���ΪĬ��ҳ
        gEventFlag |= EVENT_FALG_DISPLAY_PAGE;
     }
    gpage.flag = FALSE;//�ر���ҳ��ʱ����־
    //gscreen_working = FALSE;
    epd_scn_tp.screen_busy_flag = FALSE;
    gSys_tp.change_map =0xff;
}

