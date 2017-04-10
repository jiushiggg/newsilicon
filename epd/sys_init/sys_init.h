#ifndef __HOLTEK_MSP430_SYSINT_H__
#define __HOLTEK_MSP430_SYSINT_H__

//#include "msp430g2755.h"
#include "datatype.h"
#include "..\protocol\three_protocol.h"

#define BATTERY_BASELINE    0x02

#define EXT_FLASH_BLOCK0_ADDR  ((WORD)0x0000)
#define EXT_FLASH_BLOCK1_ADDR  ((WORD)0x1000)
#define EXT_FLASH_BLOCK2_ADDR  ((WORD)0x2000)
#define EXT_FLASH_BLOCK3_ADDR  ((WORD)0x3000)//属性包
#define EXT_FLASH_BLOCK4_ADDR  ((WORD)0x4000)//页属性

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
//是否打开上下拉电阻
#define UNUSED_GPIO27_REN_CLOSE
#define UNUSED_GPIO27_REN_OPEN
//输出高低或者是上拉下拉
#define UNUSED_GPIO27_0
#define UNUSED_GPIO27_1


#define UNUSED_GPIO26_IN
#define UNUSED_GPIO26_OUT
#define UNUSED_GPIO26_IO_MODE
//是否打开上下拉电阻
#define UNUSED_GPIO26_REN_CLOSE
#define UNUSED_GPIO26_REN_OPEN
//输出高低或者是上拉下拉
#define UNUSED_GPIO26_0
#define UNUSED_GPIO26_1


/*--------------------系统任务-----------------*/

/** 事件类型
*	事件驱动标志(优先级定义LSB[高], HSB[低])
*/
#define EVENT_FLAG_RFWORK		((UINT32)(1<<0))		/* RF数据消息 */	
#define EVENT_FLAG_EPD_DISPLAY	        ((UINT32)(1<<1))		/* 屏幕更新 */
#define EVENT_FLAG_WRITEID		((UINT32)(1<<2))		/* 烧写配置id信息 */
#define EVENT_FLAG_NETLINK	        ((UINT32)(1<<3))		/* 组网保存消息 */
#define EVENT_FLAG_OSD_ANALUSIS		((UINT32)(1<<4))		/*  解析cmd流*/
#define EVENT_FLAG_FLASHUPDATE          ((UINT32)(1<<5))                /* 保存烧写id的配置任务 */


#define EVENT_FLAG_ERASER_BUFF		((UINT32)(1<<6))		/**<擦除外部flash，即osd的128命令 */
#define EVENT_FLAG_UPDATA_CHECK		((UINT32)(1<<7))		/**osd查询命令 */
#define EVENT_FLAG_SYS_HEART		((UINT32)(1<<8))		/**心跳*/

#define EVENT_FLAG_NCF_CHIP             ((UINT32)(1<<9))               /*干簧管中断*/
#define EVENT_FLAG_INTER_REED           ((UINT32)(1<<10))               /*干簧管中断*/
#define EVENT_FLAG_INTER_LED            ((UINT32)(1<<11))               /*led中断*/
#define EVENT_FLAG_QUEST_HEART		((UINT32)(1<<12))		/**QUEST心跳*/
#define EVENT_FLAG_LTIMER		((UINT32)(1<<13))		/* 定时器消息 */
#define EVENT_FLAG_SPIERROR	        ((UINT32)(1<<14))
#define EVENT_FLAG_UPDATA_ROM           ((UINT32) ((UINT32)1<<(UINT32)15))
#define EVENT_FLAG_SCREEN_BUSY	        ((UINT32) ((UINT32)1<<(UINT32)16))		/* 屏幕忙不忙检测 */
#define EVENT_FLAG_TIMER_CALIBRATION	((UINT32) ((UINT32)1<<(UINT32)17))		/* 定时器校准 */  // add by zhaoyang for bug 352 
#define EVENT_FALG_DISPLAY_PAGE         ((UINT32) ((UINT32)1<<(UINT32)18)) //osd 显示
//#define EVENT_FALG_RC_CHANGE_PAGE      ((UINT32) ((UINT32)1<<(UINT32)20)) //rc命令切页
//#define EVENT_FALG_EXIT_RC_CHANGE_PAGE      ((UINT32) ((UINT32)1<<(UINT32)21)) //退出rc命令切页
//#define EVENT_FALG_EXIT_MAGNET_CHANGE_PAGE      ((UINT32) ((UINT32)1<<(UINT32)22)) //退出magnet命令切页

#define  EVETT_FALG_CLEAR ((UINT32) 0xFFF9E8FE)

/*---------------系统自定义类型----------------*/
#pragma pack(1)

struct INFO_DATA_T {
  UINT16 idcrc;	
  RFINIT 		gRFInitData;			//gRFInitData	18B   RF 工作数据	
};

#pragma pack()





/*-------------------函数声明------------------*/
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
