#include "datatype.h"
#include "..\global_variable\global_variable.h"
#include <string.h>
#include "..\crc16\crc16.h"
#include "..\info_flash\save_sys_status.h"
#include "three_event.h"
#include "delay.h"
#include "sys_config.h"


//----------------load rf info------------------------

BOOL save_sys_load_page_info(void)
{
  UINT8 i = 0;
  SYS_LOAD_TYPE tp;
  for(i=0;i<3; i++)
  {
    segment_erase(EXT_FLASH_BLOCK4_ADDR);
    segment_write(EXT_FLASH_BLOCK4_ADDR, (WORD)&gSys_tp, sizeof(gSys_tp));
    segment_read(EXT_FLASH_BLOCK4_ADDR, (WORD)&tp, sizeof(gSys_tp));
    if((0 == memcmp(&gSys_tp, &tp, sizeof(tp))))
      return TRUE ;
    Delay_ms(5);
  }
  segment_erase(EXT_FLASH_BLOCK4_ADDR);//写入错误时，擦除清空flash中的id
  return FALSE;
}


BOOL load_extern_rf_info(void)
{
  UINT8 id1[]={0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff};
  UINT16 crc = 0;
  UINT8 i = 0;
  for(i=0;i<3; i++)
  {
    crc = 0;
    memset((UINT8 *)&INFO_DATA,0x00,sizeof(INFO_DATA)); 
    segment_read(EXT_FLASH_BLOCK0_ADDR, (WORD)&INFO_DATA, sizeof(INFO_DATA));
    crc=  my_cal_crc16(crc,(UINT8 *)&INFO_DATA.gRFInitData, sizeof(RFINIT));
    if (INFO_DATA.idcrc == crc)
    {
      if(0 == memcmp(&INFO_DATA.gRFInitData, id1, sizeof(id1)))
        return FALSE;
      return TRUE;
    }
    Delay_ms(5);
  }
  
  return FALSE; 
  
}

void save_extern_rf_info(void)
{
  UINT16 crc = 0;
  UINT8 i = 0;
  struct     INFO_DATA_T  tp;
  for(i=0;i<3; i++)
  {
    crc = 0;
    INFO_DATA.idcrc =  my_cal_crc16(crc,(UINT8 *)&INFO_DATA.gRFInitData, sizeof(RFINIT));
    segment_erase(EXT_FLASH_BLOCK0_ADDR);
    segment_write(EXT_FLASH_BLOCK0_ADDR, (WORD)&INFO_DATA, sizeof(INFO_DATA));
    segment_read(EXT_FLASH_BLOCK0_ADDR, (WORD)&tp, sizeof(INFO_DATA));
    if((0 == memcmp(&INFO_DATA, &tp, sizeof(tp))))
      return ;
    Delay_ms(5);
  }
  segment_erase(EXT_FLASH_BLOCK0_ADDR);//写入错误时，擦除清空flash中的id
}
//-------------------------保存属性包---------------------------------
void save_state_info_fun(void)
{
  UINT8 i = 0;
  EPD_ATTRIBUTE_PKG_ST tp;
  for(i=0;i<3; i++)
  {
    memset((UINT8 *)&tp,0x00,sizeof(EPD_ATTRIBUTE_PKG_ST));
    segment_erase(EXT_FLASH_BLOCK3_ADDR);
    segment_write(EXT_FLASH_BLOCK3_ADDR, (WORD)&epd_attr_info, sizeof(EPD_ATTRIBUTE_PKG_ST));
    segment_read(EXT_FLASH_BLOCK3_ADDR, (WORD)&tp, sizeof(EPD_ATTRIBUTE_PKG_ST));
    if((0 == memcmp(&epd_attr_info, &tp, sizeof(tp))))
      return ;
    Delay_ms(5);
  }
  //属性包错误是否置全局错误标志
}
//--------------------------------------------------------------------

//------------------load sys configuration---------------------------
BOOL load_sys_config_info(void)
{
  UINT16 crc = 0;
  UINT8 i= 0;
  BOOL  ret = FALSE ;
  for(i = 0 ; i < 3 ;i++)
  {
    crc=0;
    memset((UINT8 *)&def_sys_attr,0x00,sizeof(def_sys_attr));
    segment_read(EXT_FLASH_BLOCK1_ADDR, (WORD)&def_sys_attr, sizeof(def_sys_attr));
    crc=  my_cal_crc16(crc,((UINT8 *)(&def_sys_attr))+sizeof(def_sys_attr.attrcrc), sizeof(def_sys_attr)-sizeof(def_sys_attr.attrcrc));
    if (def_sys_attr.attrcrc == crc)
    {   
      ret =  TRUE;
      break;
    }
    else
      Delay_ms(5);
    
  }
  
  if(def_sys_attr.sys_def_attribute.grp_wkup_time == 0)
    ret =  FALSE;
  if(def_sys_attr.sys_def_attribute.heartbit_time == 0)
    ret =  FALSE; 
  if(def_sys_attr.sys_def_attribute.set_wkup_time == 0)
    ret =  FALSE; 
  if(def_sys_attr.sys_def_attribute.work_time == 0)
    ret =  FALSE; 
  if(def_sys_attr.aclk_timer_cycle == 0)
    ret =  FALSE;  
  return ret;
  
}
void sys_config_deaf_cont(void)
{
  def_sys_attr.sys_def_attribute.set_wkup_time = 2;
  def_sys_attr.sys_def_attribute.grp_wkup_time = 4;
  def_sys_attr.sys_def_attribute.heartbit_time= 180;
  def_sys_attr.sys_def_attribute.work_time = 4;
  def_sys_attr.aclk_timer_cycle = WOR_TIME_COUNT_2S;
  
  def_sys_attr.rf_wk_md = RF_A7106_SET_WKUP_MODE;
  def_sys_attr.exit_grp_wkup_cont = GROUP_MAX_TIMES;
  def_sys_attr.gwor_flag_now = RF_FSM_CMD_SET_EXIT_LOOP;
  def_sys_attr.gwor_flag_before = RF_FSM_CMD_SET_EXIT_LOOP;
  
}

/*
*begin add by zhaoyang for bug 409
*问题描述：修改心跳信息，上传setwor时间
*/ 

UINT8 hb_setwor_fun(void)
{
  UINT8 ret =3;
  
  if( (def_sys_attr.sys_def_attribute.set_wkup_time == 0) || ( (def_sys_attr.sys_def_attribute.set_wkup_time %2) !=0 ) ||(def_sys_attr.sys_def_attribute.set_wkup_time > 64))
  {
    ret = 7;
    goto loop;
  }
  
  if(def_sys_attr.sys_def_attribute.set_wkup_time <= 2 )
  {
    ret = 0;
  }
  else if(def_sys_attr.sys_def_attribute.set_wkup_time <= 4)
  {
    ret = 1;
  }
  else if( (def_sys_attr.sys_def_attribute.set_wkup_time <=8))
  {
    ret = 2;
  }
  else if( (def_sys_attr.sys_def_attribute.set_wkup_time <=16))
  {
    ret = 3;
  }
  else if((def_sys_attr.sys_def_attribute.set_wkup_time <=32))
  {
    ret = 4;
  }
  else if( (def_sys_attr.sys_def_attribute.set_wkup_time <=48))
  {
    ret = 5;
  }
  else 
  {
    ret = 6;
  }
  
loop:
  
  ret =  ret<<5;
  ret |= 0x08;
  return ret;
  
}
/*end add by zhaoyang for bug 409 */  
BOOL save_sys_config_info(void)
{
  UINT16 crc = 0;
  UINT8 i= 0;
  struct SYS_ATTR_T tp;
  for(i=0;i<3; i++)
  {
    crc = 0;
    crc=  my_cal_crc16(crc,((UINT8 *)(&def_sys_attr))+sizeof(def_sys_attr.attrcrc), sizeof(def_sys_attr)-sizeof(def_sys_attr.attrcrc));
    def_sys_attr.attrcrc = crc;
    segment_erase(EXT_FLASH_BLOCK1_ADDR);
    segment_write(EXT_FLASH_BLOCK1_ADDR, (WORD)&def_sys_attr, sizeof(def_sys_attr));
    segment_read(EXT_FLASH_BLOCK1_ADDR, (WORD)&tp, sizeof(def_sys_attr));
    if((0 == memcmp(&def_sys_attr, &tp, sizeof(tp))))
      return TRUE;
    Delay_ms(5);
  }
  return TRUE;
}

void sys_load_config_info(void)
{
  BOOL ret = FALSE;
  
  hbr_mode = NORMAL_HEARTBEAT;                          //心跳上电加载为默认心跳模式
  ret = load_sys_config_info();                         //加载rf工作参数
  if(ret == FALSE)
  {
    sys_config_deaf_cont();
    save_sys_config_info();                             //存储区的配置信息错误，是否要把默认值写回去
  }
  
  def_sys_attr.exit_grp_wkup_cont = GROUP_MAX_TIMES ;                 //grp唤醒最大次数为3分钟
  ret = load_extern_rf_info();                          //加载rf配置信息
  if(ret == TRUE)                                       //外部配置信息正确
    return ;
  
  ret = load_info_sys_parameter();                      //读取内部info信息  
  
  /* LED_RED_ON;
  LED_GREEN_ON;
  LED_BLUE_OFF;
  while(1);
  */
  if(ret == FALSE)                                      //加载RF射频测试错误  
  {
    while(1);
  }
  
  save_extern_rf_info();
  
}
