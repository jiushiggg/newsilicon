#ifndef __HOLTEK_MSP430_OSD_CMD_H__
#define __HOLTEK_MSP430_OSD_CMD_H__

#include "..\file_sys\mini_fs.h"
#include "..\sys_init\datatype.h"
#include "..\osd2_0\osd2_0_cmd.h"
#include "..\osd2_0\split_dis_layer.h"

#define DATA_SIZE               20
#define PRICE_DATA_LEN          12





#pragma pack(1)
//--------------------osd2.0-----------

//-------------------------------------
typedef struct dis_t
{
  UINT32 dstoffset;
  UINT16 all_width;
  UINT8 is_integer; 
  UINT8 dire;
  UINT8 pw; 
  UINT8 ph; 
  UINT8 i;
  UINT8 dw;
  UINT8 dh;
  UINT8 nused;
}dis_t;
typedef struct font_t
{
  UINT32 offset;
  UINT8 id;
  UINT8 h;
  UINT8 w;
  UINT8 isdown;
} font_t;

typedef struct num_t {
  UINT8 cmd;
  UINT8 id;
  UINT32 number;
}num_t;

typedef struct num_attr_t {
  UINT8  type;
  UINT16 start_x, start_y;
  UINT16 end_x, end_y;
  UINT8  font_lid_i;
  UINT8  font_lid_d;
  UINT8  format;
}num_attr_t;

typedef struct layer_t
{
  UINT8  cmd;
  UINT8  layerid;
  UINT16 size;
  UINT16 len; 
} layer_t;



typedef struct eleid_attr_t
{
  UINT32 offset;
  UINT32 size;
  file_id_t dst;
}eleid_attr_t;


typedef struct osd_sttep_t
{
  font_t ft;//sizeof()==8
  num_attr_t num_attr;//sizeof()==12
  UINT8 num_str_buf[PRICE_DATA_LEN];//sizeof()==12
  dis_t dp;//sizeof()==14
  esl_screen_block_t sb;//sizeof()==17
  
}osd_sttep_t;

#pragma pack()

UINT32 find_id_mode0(UINT8 id);
BOOL process_cmd(file_id_t cmdfd);
void update_fom_fun(void);
void update_success_fun(void);
void clear_update_flag_fun(void);
#endif
