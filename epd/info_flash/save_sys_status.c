#include <string.h>
#include "datatype.h"
#include "sys_init.h"
#include "f_io_rom_flash.h"
#include "crc16.h"
#include "delay.h"
#include "three_protocol.h"
#include "global_variable.h"


static BOOL _save_sys_status(UINT16 addr) {
  //if (memcmp((void *)addr, (void *)&INFO_DATA, sizeof(INFO_DATA)) == 0)
  //  return TRUE;
  rom_segment_erase(addr);
  rom_segment_write(addr, (WORD)&INFO_DATA, sizeof(INFO_DATA));
  return TRUE;
}

void save_info_sys_parameter(void) 
{
  UINT16 crc;
  UINT8 i;
  
  crc = my_cal_crc16(0,(UINT8 *)&INFO_DATA.gRFInitData,sizeof(RFINIT));
  INFO_DATA.idcrc = crc;
  
  for(i = 0 ; i < 3 ;i++) {
    if (_save_sys_status(INFO_ADDR_1) && _save_sys_status(INFO_ADDR_2))
      break;
    Delay_ms(10+i*10);
  }
} 

BOOL load_id(UINT16 addr)
{
  UINT16 crc;
  UINT8 i;
  
  //加载系统配置信息到 INFO_DATA中
  for(i = 0 ; i < 3 ;i++)
  {
    crc = 0;
    //memset((UINT8 *)&INFO_DATA,0,sizeof(INFO_DATA));
    rom_segment_read(addr, (WORD)&INFO_DATA, sizeof(INFO_DATA));
    crc=  my_cal_crc16(crc,(UINT8 *)&INFO_DATA.gRFInitData, sizeof(RFINIT));	
    if (INFO_DATA.idcrc == crc)
      return TRUE;
    Delay_ms(10+i*10);
    
  }
  return FALSE;
}

BOOL load_info_sys_parameter(void)
{ 
  UINT8 id1[]={0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff};
  
  if(load_id(INFO_ADDR_1) || load_id(INFO_ADDR_2))
    return TRUE;
  if(0 == memcmp(&INFO_DATA.gRFInitData, id1, sizeof(id1)))
    return TRUE;
  return FALSE;
  
}
void test_save_id(void)
{
  UINT8 id1[]={0X53,0X78,0x01,0X66};
  UINT8 id2[]={0X58,0X48,0x22,0X99};
  UINT8 id3[]={0x52,0x56,0x78,0x53};
  memset(&INFO_DATA.gRFInitData,0x00,sizeof(RFINIT));
  memcpy((void *)&INFO_DATA.gRFInitData.esl_id,id2,4);   
  memcpy(&INFO_DATA.gRFInitData.wakeup_id,id1,4);   
  memcpy(&INFO_DATA.gRFInitData.master_id,id3,4);      
  INFO_DATA.gRFInitData.grp_wkup_ch = 150 ;
  INFO_DATA.gRFInitData.set_wkup_ch = 150 ;
  INFO_DATA.gRFInitData.esl_data_ch = 150 ;
  INFO_DATA.gRFInitData.esl_netmask = 34;
  save_info_sys_parameter();
}
