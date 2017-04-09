#ifndef _CRC_H_
#define _CRC_H_

#include "datatype.h"

UINT16 my_cal_crc16(UINT16 crc,UINT8 *ptr,UINT16 len);
UINT16 RF_crc(UINT8 ctrl , UINT8 *pkg_num , UINT8 *data , UINT8 *id);
UINT16 all_crc16(void);

#endif
