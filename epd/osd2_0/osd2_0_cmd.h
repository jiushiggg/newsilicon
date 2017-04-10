#ifndef __HOLTEK_MSP430_OSD2__0CMD_H__
#define __HOLTEK_MSP430_OSD2__0CMD_H__

#include "..\file_sys\mini_fs.h"
#include "..\sys_init\datatype.h"

#define OSD_CMD_SUN     (0x08)
#define OSD_END_CMD     (0x76)
#define PAGE_LEN        (512)
#define ALL_PAGE_SUM    (0x07)
#define PAGE_TIME_OFF   (0XFFF0)
#define NUMBER_ALL   (12)                       //数字缓冲区长度

#pragma pack(1)

typedef struct write_layer_t
{
  UINT8  cmd;
  UINT32 len;
  UINT16  layerid;
  UINT32 property; 
} write_layer_t;

typedef struct wrt_layer_arrt_t
{
  UINT32 offset;
  UINT32 len;
  file_id_t src_file;
}wrt_layer_arrt_t;

typedef struct store_page_cmd_t
{
  UINT8 cmd;
  UINT16 len;
  UINT8 page_sum;
}store_page_cmd_t;

typedef struct page_into_t
{
  UINT16 page_len;
  UINT8  page_id;
  UINT16 layer_len;
}page_into_t;

typedef struct dis_page_cmd_t
{
  UINT8  cmd;
  UINT16 len;
  UINT8  default_page_id;
 
}dis_page_cmd_t;

typedef struct alter_num_t
{
  UINT8 cmd;
  UINT16 len;
  UINT8 flag;
}alter_num_t;
typedef struct page_num_sum_t
{
  UINT8  page_id;
  UINT16 page_num_len;
}page_num_sum_t;

typedef struct number_t
{
  UINT16 layer_id;
  UINT32 number;
}number_t;

typedef struct cmd1_t
{
  UINT8 cmd;
  UINT16 len;
  UINT8 default_page_id;
}cmd1_t;

typedef struct font_arrt_t
{
  UINT16 layer_id;
  UINT32 property;
  UINT16 sx;
  UINT16 sy;
  UINT16 ex;
  UINT16 ey;
  UINT16 font_layerid_i;
  UINT16 font_layerid_d; 
  UINT16 font_layerid_s; 
  UINT32 format; 
}font_arrt_t;

typedef struct num_ft
{  
  UINT16 layerid;
  UINT32 number; 
  UINT32 offset;
  UINT32 dstoffset; 
  UINT8  font_h;
  UINT8  font_w;
  UINT8  type;
}num_ft;
typedef struct property_t
{
    UINT8 color;
    UINT8 direction;
}property_t;
//-----------------------

#pragma pack()
BOOL change_page_makesure_fun(void);
BOOL sys_page_display_store_fun(UINT8 default_page_id,UINT8 present_page_id,UINT8 page_id,UINT16 stay_time,UINT16 stay_time_cont);// 默认页、当前页、切换的页id号、停留时间，已经走过的时间、状态标志
#endif
