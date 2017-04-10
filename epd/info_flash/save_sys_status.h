#ifndef __HOLTEK_MSP430_SAVE_SYS_STATUS_H__
#define __HOLTEK_MSP430_SAVE_SYS_STATUS_H__

BOOL load_info_sys_parameter(void);
void save_info_sys_parameter(void);
BOOL load_id(UINT16 addr);
void test_save_id(void);
#endif
