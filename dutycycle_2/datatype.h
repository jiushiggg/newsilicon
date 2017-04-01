#ifndef _DATATYPE_H_
#define _DATATYPE_H_

typedef char 			INT8;
typedef unsigned char 	UINT8;
typedef unsigned char 	BOOL;
typedef short 			INT16;
typedef unsigned short 	UINT16;
typedef long 			INT32;
typedef unsigned long 	UINT32;


typedef struct
{
	UINT8 ID[4];
} RFID;


typedef struct{
	UINT8 value;
	UINT8 time;
	UINT8 count;
}stHBReq;

typedef struct{
    UINT8 ctrl;
    UINT8 version;
    UINT8 wkup_id0;
    UINT8 wkup_id1;
    UINT8 wkup_id2;
    RFID SlaveID;
	UINT8 set_wkup_ch;
	UINT8 grp_wkup_ch;
	UINT8 esl_data_ch;
    UINT8 net_id;
    UINT8 info;
    UINT16 crc;
}st_resigter_hb;


typedef struct{
	UINT8 ctrl;
	UINT8 sequence;
	UINT16 lose_package_num;
	UINT16 lose_package[10];
	UINT16 crc;
}st_ack;

typedef union{
	st_resigter_hb rhb_buff;
	st_resigter_hb reed_buff;
	st_ack         ack;
	UINT8 		   buff[26];
}st_tx_buff;



typedef enum {
  RF_ERROR_NONE = (UINT8)0,
  RF_ERROR_RF_CRC = (UINT8)0x01,
  RF_ERROR_SW_CRC = (UINT8)0x02,
  RF_ERROR_ENTER_TXRX =(UINT8)0x04,
  RF_ERROR_TX_TIMEOUT = (UINT8)0x08,
  RF_ERROR_RX_TIMEOUT = (UINT8)0x10,
  RF_ERROR_CTRL	=(UINT8)0x20,
  RF_ERROR_CAL =(UINT8)0x40,
  RF_ERROR_UNKNOWN = (UINT8)0x80,
} RF_ERROR_T;

typedef enum {
	RF_ST_INIT = 0X00,	
	RF_ST_SET_IDLE = 0X01,	
	RF_ST_SET_WOR = 0X02,  
	RF_ST_GROUP_SLEEP = 0X04,			//10
	RF_ST_GROUP_WKUP = 0X08,
	RF_ST_GROUP_TRANSFER = 0X10,

	RF_ST_GROUP_IDLE = 0X20,
	RF_ST_GROUP_WOR = 0X40,	
	RF_ST_ESL_SLEEP = 0X80,			//15
	RF_ST_ESL_WKUP = 0X100,
	RF_ST_ESL_TRANSFER = 0X200, 

	RF_LINK_EVENT_HANDLE = 0X400,
	RF_GRP_DATA_EVENT_HANDLE = 0X800,
	RF_ESL_DATA_EVENT_HANDLE = 0X1000,	//20	
	RF_REMOTE_EVENT_HANDLE = 0X2000,
	
	RF_ERROR_HANDLE = 0X4000,
	RF_ST_BACKUP = 0X8000,
	
	RF_ST_REMOTE_INIT = 0X2002,
	RF_ST_REMOTE_IDLE = 0X2004,
	RF_ST_REMOTE_WOR = 0X2006,		
	RF_ST_REMOTE_WKUP = 0X2008,	
	RF_ST_REMOTE_TRANSFER = 0X2020,
	RF_ST_REMOTE_OUT = 0X2040,
	RF_ST_GROUP_SLEEP1 = 0XC0,			//15	
	RF_ST_UNKNOWN = 0XA0		
}eRF_STATE;


typedef enum {
	LED_GLITTER_DISABLE =(UINT8)0x01,
	LED_ON1 =(UINT8)0,
	LED_OFF1 = (UINT8)2,
	LED_ON2 = (UINT8)4,
	LED_OFF2 = (UINT8)8,
	LED_INIT = (UINT8)0X10
}eLED_STATE;

typedef enum {
	LCD_NORMAL =(UINT8)0x01,
	LCD_REMOTE_BEGIN =(UINT8)0,
	LCD_REMOTE_FINISH = (UINT8)2,	
}eLCD_STATE;



typedef struct{			//8
	UINT8 reserved;	
	UINT8 num;	
	UINT8 on;
	UINT8 off1;
	UINT16 off2;
	UINT16 cnt;	
}stLEDSet;

typedef struct{			//20
	UINT16 security_code;
	UINT8 rollingtm[4];
	UINT8 magnet_display_time;
	UINT8 reserved;
	stLEDSet led;	
	UINT16 magnit;	
	UINT16 all_crc;
}stDisplayCnf;

typedef struct{
	UINT16 hb_time;			//心跳时间   *
	UINT8 set_wor_period;	//set wor 周期 *
	UINT8 reserved;
}stSysCnf;



typedef struct{
//  RF_EVENT_T current_event;	//记录当前事件
//  RF_EVENT_T prev_event;	//记录上一次事件
  
//  eRF_STATE prev_st;		//记录上一次st
  eRF_STATE current_st;		//记录当前st
  eRF_STATE next_st;		//记录下一次st
  
  RF_ERROR_T error;			//记录RF错误标识
  UINT8 ack;				//RF ACK值
} stRF_ST;

/*
RF工作设置参数
*/

/*显示RAM block，占用20Bytes*/
#define RAM_LCD_START		0x80
#define RAM_LCD_END			0xff
#define	RAM_LCD_SIZE		(UINT8)(20)

typedef struct
{
	RFID grp_wkup_id;
	RFID esl_id;
	RFID master_id;	
	UINT8 esl_data_ch;
	UINT8 esl_netmask;
	UINT8 set_wkup_ch;
	UINT8 grp_wkup_ch;
	UINT8 CheckMode;			//检查	
	UINT8 esl_hb_ch;	
	UINT8 reserve;
	BOOL bValid;		
} RFINIT;

typedef struct	//22
{
	UINT16 crc;
	RFINIT	info;
} stRFInit;

//多屏控制帧26个字节
typedef struct{
//	UINT8 total_pkg :5;
//	UINT8 ctrl :3;
	UINT8 ctrl;
	UINT8 package_num[2];
	UINT8 screen_num;
	UINT8 data[20];	
	UINT16 crc;
}stMultiPageCtrl;
//配置帧
typedef struct{
//	UINT8 total_pkg :5;
//	UINT8 ctrl :3;
	UINT8 ctrl;
	UINT8 package_num[2];
	UINT8 reserved;
	UINT16 security_code;
	UINT8 rollingtm[4];
	UINT8 magnit_display_time;
	UINT8 reserved2[2];
	UINT8 led_num;
	UINT8 t0;
	UINT8 t1;
	UINT16 t2;
	UINT16 period;
	UINT16 magnit;
	UINT16 all_crc;
	UINT16 crc;
}stConfigCtrl;

//查询数据帧
typedef struct{
	UINT8 ctrl;
	UINT8 package_num[2];
	UINT8 query_cmd;
	UINT8 reserved[20];
	UINT16 crc;
}stQueryCtrl;

//查询数据帧
typedef struct{
	UINT8 ctrl;
	UINT8 sequence;
	UINT16 package_num;
	UINT8 reserved[20];
	UINT16 crc;
}stLinkQueryCtrl;


typedef struct{
	UINT8 ctrl;
	UINT8 package_num[2];
	RFID set_wk_id;
	RFID esl_id;
	UINT8 set_wkup_ch;
	UINT8 grp_wkup_ch;
	UINT8 esl_data_ch;
	UINT8 esl_netmask;
	UINT8 reserved[9];
	UINT16 crc;
}stNetLink;

//睡眠帧
typedef struct{
	UINT8 ctrl;
	UINT8 reserved[23];
	UINT16 crc;
}stSleepCtrl;

typedef struct{
	UINT8 ctrl;
	UINT8 solt_l;
	UINT8 grp_mask[3];
	UINT8 para;	
}stSetWkUpPkg;

typedef struct{
	UINT8 ctrl;
	UINT8 cmd;
	UINT8 grp_mask[3];
	UINT8 para;	
}stSetWkUpPkgGlb;

typedef struct{
	UINT8  ctrl;
	UINT8 set_wkup_ch;
	UINT8 grp_wkup_ch;
	UINT8 esl_data_ch;
	UINT16 crc;	
}stSetWkUpPkgCh;

typedef struct{
	UINT8 ctrl;
	UINT8 channel;
	UINT8 esl_mask[20];
	UINT8 range;
	UINT8 reserved;
	UINT16 crc;	
}stGrp_WkUpPkg1;

typedef struct{
	UINT8 ctrl;	
	UINT8 solt;	
}stGrp_WkUpPkg0;

typedef struct{
	UINT8 ctrl;
	UINT8 data[23];
	UINT16 crc;
}stData;

typedef struct{
	UINT8 ctrl;
	UINT8 data;	
}stRemoteWkUp;

typedef struct{
	UINT8 ctrl;
	UINT8 package_num[2];
	UINT8 reserved;
	UINT16 security_code;		
	UINT8 reserved2;
	UINT8 led_num;
	UINT8 t0;
	UINT8 t1;
	UINT16 t2;
	UINT16 period;
	UINT16 display_time;
	UINT8 page_num;
	UINT8 reserved3[7];
	UINT16 crc;	
}stRemote;


typedef union{
	stConfigCtrl 	cc;	
	stMultiPageCtrl mpc;
	stQueryCtrl 	qc;
	stLinkQueryCtrl lqc;
	stNetLink 		nl;
	stSleepCtrl	 	sc;
	stSetWkUpPkg	swp;
	stSetWkUpPkgGlb swpg;
	stSetWkUpPkgCh	swpc;
	stGrp_WkUpPkg1	gwp1;
	stGrp_WkUpPkg0	gwp0;
	stData			d;	
	stRemoteWkUp	rwp;
	stRemote 		rdata;
	UINT8 buff[26];
}stRecvBuff;


#define DISPLAY_ID	4
typedef struct{
	UINT8 buff[DISPLAY_ID][RAM_LCD_SIZE];	//
}stDisplayBuff;


typedef enum{
	BPS100,
	BPS500,
	BPSREMAIN
}bps_enum;

#define TRUE 1
#define FALSE 0
//#define NULL ((void*)0)

#endif
