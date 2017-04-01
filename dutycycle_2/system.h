#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "datatype.h"


#define FIRMWARE_VER	13//22已用Skyline-M@1.0.1.4036和之前的版本



//以下是初始化参数在EEPROM的位置
#define EEPROM_BAK0 0x08											//EEPROM_USER_ADDRESS_START
#define EEPROM_LCD_ADDRESS_START 0x10								//显示数据对应的起始地址
#define EEPROM_INIT_BVALID (EEPROM_LCD_ADDRESS_START+RAM_LCD_SIZE)	//bValid对应地址

//以下是RF的ctrl及crc在EEPROM的位置
#define EEPROM_RF_DATA_CTRL (EEPROM_LCD_ADDRESS_START+RAM_LCD_SIZE+1)	//data帧的ctrl的值对应地址
#define EEPROM_RF_DATA_CRC (EEPROM_LCD_ADDRESS_START+RAM_LCD_SIZE+2)	//data帧的crc的值对应地址,占2个字节

//全局CRC
#define EEPROM_CRC (EEPROM_LCD_ADDRESS_START+RAM_LCD_SIZE+4)			//所有有效数据crc的值对应地址,占2个字节

//第一块备份区
#define EEPROM_BAK1 0x38


/*
事件类型
事件驱动标志(优先级定义LSB[高], HSB[低])
*/
#define EVENT_FLAG_RFDATA		(1<<0)		//RF数据消息	
#define EVENT_FLAG_RFINIT		(1<<1)		//RF初始化消息
#define EVENT_FLAG_EEPROMUPDATE (1<<2)      //更新EEPROM信息
#define EVENT_FLAG_HTIMER		(1<<3)		//定时消息[H]
#define EVENT_FLAG_SPIERROR		(1<<4)		//RF初始化参数通讯失败
#define EVENT_FLAG_LCDUPDATE	(1<<5)		//显示更新
#define EVENT_FLAG_LTIMER		(1<<6)		//定时消息[L]
#define EVENT_FLAG_IDLE			(1<<7)		//空闲
#define EVENT_FLAG_LED			((UINT16)1<<8)
//#define EVENT_FLAG_ROLLINGLCD	((UINT16)1<<9)	//翻屏控制。	
#define EVENT_FLAG_HEARTBEAT	((UINT16)1<<10)	//屏幕
#define EVENT_FLAG_REMOTE		((UINT16)1<<11)	//屏幕
#define EVENT_FLAG_VOLTAGE		((UINT16)1<<12)	//电压检测


#define SINGLE_DISPLAY	(UINT8)0X01
#define GLITTER_DISPLAY	(UINT8)0X03
#define MULT_DISPLAY	(UINT8)0X04

#define SET_WAKEUP		0X09
/*
**	system reset mode
**	0: none
**   1: normal
**   2: 12-hour system reset
*/
#define SYS_RST_MODE_0		0
#define SYS_RST_MODE_1		1
#define SYS_RST_MODE_2		2


//E2存储标志
#define SAVE_SCREEN1	(UINT16)0X01
#define SAVE_SCREEN2	(UINT16)0X02
#define SAVE_SCREEN3	(UINT16)0X04
#define SAVE_SCREEN4	(UINT16)0X08
#define SAVE_SCREEN5	(UINT16)0X10
#define SAVE_SCREEN6	(UINT16)0X20
#define SAVE_SCREEN7	(UINT16)0X40
#define SAVE_SCREEN8	(UINT16)0X80
#define SAVE_GLITTER 	(UINT16)0X0100
#define SAVE_DISPLAY_CNF 	(UINT16)0X0200
#define SAVE_SYS_CNF 	(UINT16)0X0400
#define SAVE_DEV_INFO	(UINT16)0X0800


#define INFO_ADDR		0X00
#define INFO_ADDR_BAK	0X18

#define DATA_ADDR		0X30
#define LCD_DATA_LEN	(20)
#define LCD_SCRREN1_ADDR	0X30
#define LCD_SCRREN2_ADDR		(LCD_SCRREN1_ADDR + LCD_DATA_LEN)
#define LCD_SCRREN3_ADDR		(LCD_SCRREN2_ADDR + LCD_DATA_LEN)
#define LCD_SCRREN4_ADDR		(LCD_SCRREN3_ADDR + LCD_DATA_LEN)
#define LCD_SCRREN5_ADDR		(LCD_SCRREN4_ADDR + LCD_DATA_LEN)
#define LCD_SCRREN6_ADDR		(LCD_SCRREN5_ADDR + LCD_DATA_LEN)
#define LCD_SCRREN7_ADDR		(LCD_SCRREN6_ADDR + LCD_DATA_LEN)
#define LCD_SCRREN8_ADDR		(LCD_SCRREN7_ADDR + LCD_DATA_LEN)
#define LCD_GLITTER_ADDR		(LCD_SCRREN8_ADDR + LCD_DATA_LEN)
#define DISPLAY_CNF_ADDR		(LCD_GLITTER_ADDR + LCD_DATA_LEN)
#define LCD_CNF_LEN		20		//
	
#define SYS_CNF_ADDR	(DISPLAY_CNF_ADDR + LCD_CNF_LEN)
#define SYS_CNF_LEN		4
		
#define END_ADDR 		0XFF
#define _CLRWDT


extern const UINT32 generated_500kbps[];
extern const UINT32 generated_100kbps[];
extern UINT32 syncwordcnf[];
extern UINT32 lencnf[];

void System_Init(void);
void System_EnterSleep(void);
void System_ClrSleepISR(void);

UINT8 BitNumber(UINT16);
UINT8 multi_display_bit(void);
BOOL glitter_display_bit(void);
void system_save_data(UINT16 flg);
void read_data(UINT16 flg);
UINT8 load_sys_status(void);
void eeprom(void);
void remote(void);
UINT8 hb_setwor_fun(void);
UINT32 change_syncword(UINT32 souce);


#endif


