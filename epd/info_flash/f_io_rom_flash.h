#ifndef __HOLTEK_MSP430_ROMFLASH_H__
#define __HOLTEK_MSP430_ROMFLASH_H__

/**< 保存在INFO B区的配置信息 */


#define INFO_ADDR_1			0x1000u
#define INFO_ADDR_2	                0x1040u
#define INFO_ADDR_3                     0x1080u
//#define INFO_ADDR_4                     0x10c0u


void rom_segment_erase(WORD seg_addr);
void rom_segment_write(WORD addr, WORD data, WORD len);
void rom_segment_read(WORD addr,WORD buf, WORD len) ;
#endif