#ifndef __HOLTEK_MSP430_BIT_DISPLAY_H__
#define __HOLTEK_MSP430_BIT_DISPLAY_H__

#include "..\sys_init\datatype.h"
#include "osd_cmd.h"


#define CACHE_SIZE		8
#define CACHE_NOT_CLEAN ((UINT32)(0xffffffff))


#define BUMBER_VARIETY_F  0x00000007
#define PREFIX_F          0x00000008 
#define DIGITAL_PM_F      0x00000010
#define DECIMALS_LEN_F    0x000000e0      

#define TIME_VARIETY_F    0x00000300
#define DECIMALS_FLAG_F   0x00000400
#define THOUSANDS_FLAG_F  0x00000800
#define DIS_MODE_F        0x00003000
#define SUFFIX_FLAG_F     0x00004000
#define REVERSE_FLAG_F    0x00008000

#define HORIZONTAL_F      0x00030000
#define VERTICAL_F        0x000c0000  

#pragma pack(1)
typedef struct bit_cache_t 
{
  
  UINT32 index;
  file_id_t fd;
  UINT8 cache[CACHE_SIZE];
} bit_cache_t;
#pragma pack()

UINT8 dis2screen(file_id_t sbfd, UINT8 sbn) ;
//UINT32 get_dst_start_bit(const num_attr_t *p, const font_t *ft, UINT16 width, UINT8 dire);
UINT32 get_dst_start_bit(const font_arrt_t *p, const num_ft *ft, UINT16 width, UINT8 dire,UINT8 dh,UINT8 dw, dis_format_t *format);
UINT32 get_dst_bit(UINT32 provbit, UINT8 pw, UINT8 ph, const font_arrt_t *p, const num_ft *ft,property_t *pro_tp,dis_format_t *attr_tp);
void alignment_dignum(const UINT8 dire,const UINT8 h ,const UINT8 w , const   UINT8 dis_format,UINT8 num);
UINT8 dis_page_cont(UINT8 i);
#endif
