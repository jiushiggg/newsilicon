#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include "datatype.h"
#include "mini_fs_conf.h"
#include "mini_fs.h"
#include "osd_cmd.h"
#include "bit_display.h"
#include "global_variable.h"
#include "delay.h"
#include "crc16.h"
#include "three_event.h"
#include "reed.h"
#include "adc10.h"


#define OSD_CMD_END_ORDER	0x76
#define CRC_SIZE     		0x02

UINT32 find_id_mode0(UINT8 id)
{
  return f_len((file_id_t)id);
}


BOOL sava_udaterom_fun(void)
{
  UINT8 i = 0;
  ROM_UP_T  tp;
  for(i=0;i<3; i++)
  {
    memset((UINT8 *)&tp,0xff,sizeof(ROM_UP_T));
    segment_erase(EXT_FLASH_BLOCK2_ADDR);
    segment_write(EXT_FLASH_BLOCK2_ADDR, (WORD)&updata_info, sizeof(ROM_UP_T));
    segment_read(EXT_FLASH_BLOCK2_ADDR, (WORD)&tp, sizeof(ROM_UP_T));
    if((0 == memcmp(&updata_info, &tp, sizeof(ROM_UP_T))))
      return TRUE;
    Delay_ms(5);
  }
  return FALSE;
}
void update_success_fun(void)
{
  ROM_UP_T  tp;
  segment_read(EXT_FLASH_BLOCK2_ADDR, (WORD)&tp, sizeof(ROM_UP_T));
  all_osd_crc = 0;
  memset((UINT8 *)&uprom_tp,0x00,sizeof(uprom_tp));
  uprom_tp.sys_update_query_flag = FALSE;
  if(tp.flag == GROM_UPDATA_SUCCESS)
  {
    uprom_tp.sys_update_success_flag = 0x40;
    
    segment_erase(EXT_FLASH_BLOCK2_ADDR);
  }
  
}
void clear_update_flag_fun(void)
{
  if(uprom_tp.sys_update_query_flag  == TRUE)
  {
    memset((UINT8 *)&uprom_tp,0x00,sizeof(uprom_tp));
  }
}
#define WDTPW_ERROR               (0xf000u)   //利用些错误的口令使系统复位
void update_fom_fun(void)
{
  
#define SYS_RESET_ADDR (0xfffe)
  typedef void(*t_function)() ;  
  
  BOOL ret= FALSE;
  

  if(updata_info.flag == GROM_UPDATA_FLAG)//升级标志正确
    ret = sava_udaterom_fun();
  
  if(ret == TRUE)
  {
//    WDTCTL = WDTPW_ERROR + WDTHOLD;//利用些错误的口令使系统复位 todo
    return ;
  }
  else
  {
    gerr_info = UPROM_ERR;
  }

  memset((UINT8 *)&updata_info,0xff,sizeof (ROM_UP_T));
  segment_erase(EXT_FLASH_BLOCK2_ADDR);
  
}
