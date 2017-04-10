#ifndef __HOLTEK_MSP430_OSD_FLASH_H__
#define __HOLTEK_MSP430_OSD_FLASH_H__

#include "three_event.h"

UINT8 main_osd_cmd(void);
void osd_init(void);
void eraset_cmd_buf(void);
#endif