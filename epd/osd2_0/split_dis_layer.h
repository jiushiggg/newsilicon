#ifndef __HOLTEK_MSP430_SPLIT_DIS_LAYER_H__
#define __HOLTEK_MSP430_SPLIT_DIS_LAYER_H__

#include "..\file_sys\mini_fs.h"
#include "..\sys_init\datatype.h"

#pragma pack(1)
typedef struct esl_image_t 
{
  UINT32  p;
  UINT16 sx, sy;
  UINT16 ex, ey;
} esl_image_t;


typedef struct dis_format_t {
  UINT32 number_variety:3;
  UINT32 prefix:1;
  UINT32 pm:1;
  UINT32 decimals_len:3;
  
  UINT32 time_variety:2;
  UINT32 decimals_flag:1;
  UINT32 thousands_flag:1;
  UINT32 dis_decimals_mode:2;
  UINT32 suffix_flag:1;
  UINT32 reverse_flag:1;
  
  UINT32 horizontal_alignment:2;
  UINT32 vertical_alignment:2;
  UINT32 ispadd:1;
  UINT32 paddinfo:1;
  UINT32 reserve:10;
} dis_format_t;

typedef struct esl_screen_block_t {
  
  UINT32 dst_startbit;		//ˢ����������Ļ�е���ʼbitλ
  UINT32 src_startbit;		//ˢ��������data������ʼbitλ
  file_id_t dstfd;
  file_id_t srcfd;
  UINT16 h;						//ˢ�����ݵ�������Ļ�߶�
  UINT16 w;						//ˢ�����ݵ�������Ļ���
  dis_format_t attr;			//ˢ�����ݵĸ�ʽ
} esl_screen_block_t;


#pragma pack()

void spit_fun(UINT8 page_id);
BOOL gpage_info_fun(void);
void gpage_init_fun(void);
void  change_next_pageid_fun(void);
#endif

