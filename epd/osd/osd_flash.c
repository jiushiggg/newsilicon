#include <string.h>
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

BOOL compare_receive_data_ok(UINT16 get_success_pkg_num,UINT16 pkgnum_sum)
{ 
  if(0 == pkgnum_sum )
    return FALSE;
  if(get_success_pkg_num < pkgnum_sum )
    return FALSE; 
  //当收到结束包，且数据包总数比比实际大时，表示错误
  if(get_success_pkg_num > pkgnum_sum )
  {
    gerr_info = TR3_PKG_ERR;
    return FALSE;
  } 
  return TRUE;  
}

UINT8 cmd_main_pkg_check()
{ 
  if(!compare_receive_data_ok(gpkg.pkg_num,gpkg.osd_pkg_all))
  {
    return 0;
  }
  clear_gpkg_fun();
  return 1;
}


void flash_all_data_crc(UINT16  *crc,const file_id_t id,const UINT32 len,const UINT32 offset)
{
#define TEMP_MAX_BUF  1024
  UINT8 temp[1024];
  UINT32 i = 0 ,wirte_size = 0;
  if(id == 200)//非法ID
    memset(temp, 0xff,TEMP_MAX_BUF);
  for(i=0; i < len ;)
  {
    if( (len - i) < TEMP_MAX_BUF )
      wirte_size  = len - i;        
    else
      wirte_size  = TEMP_MAX_BUF ;   
    
    if(id != 200)
      f_read(id,i + offset,temp,wirte_size);       
    *crc = my_cal_crc16(*crc,temp,wirte_size);
    i += wirte_size;   
  }
}





UINT16 all_ele_crc(void)
{
  UINT8 i ;
  UINT16  crc= 0;
  wrt_layer_arrt_t tp;
  UINT32 all_size = 0;
  UINT8 num = (find_id_mode0(F_LAY_MAP) / sizeof(tp));
  
  for(i = 0 ;i < num ;i++)
  {
    f_read(F_LAY_MAP,i * sizeof(tp),(UINT8 *)&tp,sizeof(wrt_layer_arrt_t));
    flash_all_data_crc(&crc,tp.src_file,tp.len,tp.offset);
    all_size += tp.len;
  }
  return crc;
  //flash_all_data_crc(&crc,(file_id_t)200,ELEMENT_DATA_BITMAP -all_size,0 );
  //memcpy((UINT8 *)g_crc.crc_layer,(UINT8 *)&crc,sizeof(crc));
}

UINT8 cmd_main_data_crc()
{ 
  UINT16 tp_crc =0;
  
  //命令流crc
  flash_all_data_crc(&tp_crc,F_BMP_DATA,gcmd_tp.len,gcmd_tp.start_addr );
  if((0 != memcmp((UINT8 *)&tp_crc,(UINT8 *)g_crc.crc_cmd , sizeof(g_crc.crc_cmd))))
  {
    gerr_info = TR3_STREAM_CRC_ERR;
    return 0;
  }
  //所有图层crc
  tp_crc =all_ele_crc();
  
  if(0 != memcmp((UINT8 *)&tp_crc,g_crc.crc_layer,2))
  {
    gerr_info = TR3_STREAM_CRC_ERR;
    return 0;
  }
  //页存储crc
  tp_crc =0;
  flash_all_data_crc(&tp_crc,gSys_tp.use_page_info_fileid,f_size(gSys_tp.use_page_info_fileid),0);
  if(0 != memcmp((UINT8 *)&tp_crc,g_crc.crc_page_info,2))
  {
    gerr_info = TR3_OSD_QUERY_PAGE_INFO_CRC_ERR;
    return 0;
  }
  //数字存储crc
  tp_crc =0;
  flash_all_data_crc(&tp_crc,gSys_tp.use_page_num_fileid,f_size(gSys_tp.use_page_info_fileid),0);
  if(0 != memcmp((UINT8 *)&tp_crc,g_crc.crc_page_num,2))
  {
    gerr_info = TR3_OSD_QUERY_PAGE_INFO_CRC_ERR;  
    return 0;
  }
  return 1;
}

BOOL osd_other_mode_fun(void)
{
#define CMD_NATRE_PKG    0x75
#define CMD_RC_PKG       0x74
  UINT8 cmd = 0;
  f_read(write_temp_buff_id,cmd_before_offset, &cmd, 1);
  if(cmd == CMD_NATRE_PKG)
  {
    return TRUE;
  }
  else if(cmd == CMD_RC_PKG)
  {
    return TRUE;
  }
  else
    return FALSE;
}

UINT8 main_osd_cmd(void)
{
  if(!cmd_main_pkg_check())
    return 0;
  SetWathcDog();   //喂狗  
  if(TRUE != process_cmd(write_temp_buff_id))
  {
    gerr_info = TR3_OSD_CMDERR;
    gEventFlag &= (~EVENT_FALG_DISPLAY_PAGE); 
    return 0;
  }
  
  SetWathcDog();   
  if(cmd_main_data_crc() ==0)
    gEventFlag &= (~EVENT_FALG_DISPLAY_PAGE); //crc 计算错误时，不刷新屏幕
  
  SetWathcDog();   //喂狗 
  return 1;
  
}




void eraset_cmd_buf(void)
{
  
  if(gFlag_bit)
  {
    volatile file_id_t eraser_id  = swp_pkg(write_pkg_buff_id);
    f_erase(eraser_id);//擦除未用的接受缓冲区
    gFlag_bit = 0;
   
  }
}

void osd_init(void)
{
  write_temp_buff_id = F_BMP_DATA;
  write_pkg_buff_id = F_BMP_PKG_1; 
  
  f_erase(F_BMP_PKG_1);
  f_erase(F_BMP_PKG_2);
  gerr_info = NONEERR;
  memset((UINT8 *)&g_crc,0x00,sizeof(g_crc));
}