#ifndef __HOLTEK_MSP430_SYSINT_H__
#define __HOLTEK_MSP430_SYSINT_H__

//#include "msp430g2755.h"
#include "datatype.h"
#include "..\protocol\three_protocol.h"

#define BATTERY_BASELINE    0x02

#define EXT_FLASH_BLOCK0_ADDR  ((WORD)0x0000)
#define EXT_FLASH_BLOCK1_ADDR  ((WORD)0x1000)
#define EXT_FLASH_BLOCK2_ADDR  ((WORD)0x2000)
#define EXT_FLASH_BLOCK3_ADDR  ((WORD)0x3000)//���԰�
#define EXT_FLASH_BLOCK4_ADDR  ((WORD)0x4000)//ҳ����

/*
#define   WOR_TIME_COUNT_200MS   ((UINT16)265)
#define   WOR_TIME_COUNT_1S      ((UINT16)1325)
#define   WOR_TIME_COUNT_2S      ((UINT16)2650)
#define   WOR_TIME_COUNT_8S      ((UINT16)10600)  //9440    
*/
#define   WOR_TIME_COUNT_200MS   ((UINT16)200)
#define   WOR_TIME_COUNT_1S      ((UINT16)1000)
#define   WOR_TIME_COUNT_2S      ((UINT16)2000)
#define   WOR_TIME_COUNT_8S      ((UINT16)8000)  //9440

#define   CNT_ADD_VALUE_2S	((UINT8)1)
#define   CNT_ADD_VALUE_8S	((UINT8)4)
#define   WOR_PERIOD_16S	((UINT8)8)



/*-----------------unused io-------------------*/

#define UNUSED_GPIO27_IN
#define UNUSED_GPIO27_OUT
#define UNUSED_GPIO27_IO_MODE
//�Ƿ������������
#define UNUSED_GPIO27_REN_CLOSE
#define UNUSED_GPIO27_REN_OPEN
//����ߵͻ�������������
#define UNUSED_GPIO27_0
#define UNUSED_GPIO27_1


#define UNUSED_GPIO26_IN
#define UNUSED_GPIO26_OUT
#define UNUSED_GPIO26_IO_MODE
//�Ƿ������������
#define UNUSED_GPIO26_REN_CLOSE
#define UNUSED_GPIO26_REN_OPEN
//����ߵͻ�������������
#define UNUSED_GPIO26_0
#define UNUSED_GPIO26_1


/*--------------------ϵͳ����-----------------*/

/** �¼�����
*	�¼�������־(���ȼ�����LSB[��], HSB[��])
*/
#define EVENT_FLAG_RFWORK		((UINT32)(1<<0))		/* RF������Ϣ */	
#define EVENT_FLAG_EPD_DISPLAY	        ((UINT32)(1<<1))		/* ��Ļ���� */
#define EVENT_FLAG_WRITEID		((UINT32)(1<<2))		/* ��д����id��Ϣ */
#define EVENT_FLAG_NETLINK	        ((UINT32)(1<<3))		/* ����������Ϣ */
#define EVENT_FLAG_OSD_ANALUSIS		((UINT32)(1<<4))		/*  ����cmd��*/
#define EVENT_FLAG_FLASHUPDATE          ((UINT32)(1<<5))                /* ������дid���������� */


#define EVENT_FLAG_ERASER_BUFF		((UINT32)(1<<6))		/**<�����ⲿflash����osd��128���� */
#define EVENT_FLAG_UPDATA_CHECK		((UINT32)(1<<7))		/**osd��ѯ���� */
#define EVENT_FLAG_SYS_HEART		((UINT32)(1<<8))		/**����*/

#define EVENT_FLAG_NCF_CHIP             ((UINT32)(1<<9))               /*�ɻɹ��ж�*/
#define EVENT_FLAG_INTER_REED           ((UINT32)(1<<10))               /*�ɻɹ��ж�*/
#define EVENT_FLAG_INTER_LED            ((UINT32)(1<<11))               /*led�ж�*/
#define EVENT_FLAG_QUEST_HEART		((UINT32)(1<<12))		/**QUEST����*/
#define EVENT_FLAG_LTIMER		((UINT32)(1<<13))		/* ��ʱ����Ϣ */
#define EVENT_FLAG_SPIERROR	        ((UINT32)(1<<14))
#define EVENT_FLAG_UPDATA_ROM           ((UINT32) ((UINT32)1<<(UINT32)15))
#define EVENT_FLAG_SCREEN_BUSY	        ((UINT32) ((UINT32)1<<(UINT32)16))		/* ��Ļæ��æ��� */
#define EVENT_FLAG_TIMER_CALIBRATION	((UINT32) ((UINT32)1<<(UINT32)17))		/* ��ʱ��У׼ */  // add by zhaoyang for bug 352 
#define EVENT_FALG_DISPLAY_PAGE         ((UINT32) ((UINT32)1<<(UINT32)18)) //osd ��ʾ
//#define EVENT_FALG_RC_CHANGE_PAGE      ((UINT32) ((UINT32)1<<(UINT32)20)) //rc������ҳ
//#define EVENT_FALG_EXIT_RC_CHANGE_PAGE      ((UINT32) ((UINT32)1<<(UINT32)21)) //�˳�rc������ҳ
//#define EVENT_FALG_EXIT_MAGNET_CHANGE_PAGE      ((UINT32) ((UINT32)1<<(UINT32)22)) //�˳�magnet������ҳ

#define  EVETT_FALG_CLEAR ((UINT32) 0xFFF9E8FE)

/*---------------ϵͳ�Զ�������----------------*/
#pragma pack(1)

struct INFO_DATA_T {
  UINT16 idcrc;	
  RFINIT 		gRFInitData;			//gRFInitData	18B   RF ��������	
};

#pragma pack()





/*-------------------��������------------------*/
void WathcDogStop(void);
void SetWathcDog(void);
void main_clk_init(void);
void disable_peripheral(void);
void sys_sleep(void);
void clock_8M_set(void);
void clock_1M_set(void);
BOOL sys_sleepwait(void);
void sys_register_init(void);
void sys_load_page_display_fun(void);
#endif
