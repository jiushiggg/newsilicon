#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "datatype.h"


#define FIRMWARE_VER	13//22����Skyline-M@1.0.1.4036��֮ǰ�İ汾



//�����ǳ�ʼ��������EEPROM��λ��
#define EEPROM_BAK0 0x08											//EEPROM_USER_ADDRESS_START
#define EEPROM_LCD_ADDRESS_START 0x10								//��ʾ���ݶ�Ӧ����ʼ��ַ
#define EEPROM_INIT_BVALID (EEPROM_LCD_ADDRESS_START+RAM_LCD_SIZE)	//bValid��Ӧ��ַ

//������RF��ctrl��crc��EEPROM��λ��
#define EEPROM_RF_DATA_CTRL (EEPROM_LCD_ADDRESS_START+RAM_LCD_SIZE+1)	//data֡��ctrl��ֵ��Ӧ��ַ
#define EEPROM_RF_DATA_CRC (EEPROM_LCD_ADDRESS_START+RAM_LCD_SIZE+2)	//data֡��crc��ֵ��Ӧ��ַ,ռ2���ֽ�

//ȫ��CRC
#define EEPROM_CRC (EEPROM_LCD_ADDRESS_START+RAM_LCD_SIZE+4)			//������Ч����crc��ֵ��Ӧ��ַ,ռ2���ֽ�

//��һ�鱸����
#define EEPROM_BAK1 0x38


/*
�¼�����
�¼�������־(���ȼ�����LSB[��], HSB[��])
*/
#define EVENT_FLAG_RFDATA		(1<<0)		//RF������Ϣ	
#define EVENT_FLAG_RFINIT		(1<<1)		//RF��ʼ����Ϣ
#define EVENT_FLAG_EEPROMUPDATE (1<<2)      //����EEPROM��Ϣ
#define EVENT_FLAG_HTIMER		(1<<3)		//��ʱ��Ϣ[H]
#define EVENT_FLAG_SPIERROR		(1<<4)		//RF��ʼ������ͨѶʧ��
#define EVENT_FLAG_LCDUPDATE	(1<<5)		//��ʾ����
#define EVENT_FLAG_LTIMER		(1<<6)		//��ʱ��Ϣ[L]
#define EVENT_FLAG_IDLE			(1<<7)		//����
#define EVENT_FLAG_LED			((UINT16)1<<8)
//#define EVENT_FLAG_ROLLINGLCD	((UINT16)1<<9)	//�������ơ�	
#define EVENT_FLAG_HEARTBEAT	((UINT16)1<<10)	//��Ļ
#define EVENT_FLAG_REMOTE		((UINT16)1<<11)	//��Ļ
#define EVENT_FLAG_VOLTAGE		((UINT16)1<<12)	//��ѹ���


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


//E2�洢��־
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

#define LCD_213


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
void myChangeRadioConfig(UINT8 channel, bps_enum bps, UINT8* ID, UINT8 len);

#endif


