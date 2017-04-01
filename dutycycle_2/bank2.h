#ifndef _BANK2_H_
#define _BANK2_H_

#include "datatype.h"

typedef struct {
	unsigned char b0 : 1;
	unsigned char b1 : 1;
	unsigned char b2 : 1;
	unsigned char b3 : 1;
	unsigned char b4 : 1;
	unsigned char b5 : 1;
	unsigned char b6 : 1;
	unsigned char b7 : 1;
} _stbits;
typedef union{
	_stbits bvar;
	UINT8   var;
}bits;

bits var_0;
#define gRxOverflow 	var_0.bvar.b0
#define bmulti_en 		var_0.bvar.b1
#define bglitter_dsp 	var_0.bvar.b2
#define bglitter_en   	var_0.bvar.b3
#define gEEPROM 		var_0.bvar.b4
#define blow_vol_flg 	var_0.bvar.b5
#define bled_glitter 	var_0.bvar.b6
#define grx3ms_overflow var_0.bvar.b7

bits var_1;
#define boverflow 	var_1.bvar.b0
#define bnetlink	var_1.bvar.b1
#define bquery		var_1.bvar.b2
#define bheartbeat_en		var_1.bvar.b5
#define brf_set_flg		var_1.bvar.b6
#define bremote_flg		var_1.bvar.b7
bits var_2;
#define hbreq_en		var_2.bvar.b1
#define blast_pkg_flg   var_2.bvar.b2
#define bscroll		var_2.bvar.b3
#define bwtd		var_2.bvar.b4
#define gRFInitData grf.info

//bank0中变量

UINT8 gRFError;
UINT8 gRFPowerFlag;
stHBReq hbreq;
UINT8 gpkg_cnt;		//包号计数
UINT8 * tmp_p;
eRF_STATE gmode_backup;			//RF模式
UINT8 screen_num;
stRF_ST gRF_mode;				//7
stRFInit grf;					//*	
UINT16 RF_DataCRC;
UINT8 spidata;
UINT16 gbt_trigger;	//bt触发中断值
UINT16 gEventFlag;	//事件触发标志

UINT16 	glvd_cnt;	//
eLCD_STATE gdsp_mode;	//显示模式



 stRecvBuff gRFbuf;		//26个字节
 UINT8 gdisplay_cnf ;	//屏显示位
 stDisplayCnf gcnf;	//显示配置
 UINT16 gbit_map;		//收到数据包的位图
 UINT8 swor_cnt;
 UINT8 gpkg_cnt_bak;
 UINT8 gvol_value;

UINT16 stb1_cnt;		//翻页计数	
UINT16 gsave_flg;	//存储标志
UINT8 gled_bit;	//存储标志
 UINT16 gsolt;	//solt睡眠时间
UINT8 ghb_request;	//
UINT8 gLEDstate;	//
UINT16 glcd_config;	//LCD控制
UINT16 gall_crc;			//set wor 周期 *

UINT16 gwor_cnt;			//wor听帧计数   
UINT16 remote_display_cnt;		

UINT8 gpage_num;				//遥控器控制显示页
UINT16 gdisplay_time;			//遥控器控制显示时间


stDisplayBuff gdisplay;//显示缓存84BYTE *
UINT8 gdisplay_buf[2][20];//显示缓存

stDisplayBuff gdisplay1;//显示缓存 *
UINT8 gglitter_seg[20]; //闪烁位*
stLEDSet gled;          //LED控制 *
stSysCnf gsyscnf;       //系统配置 *
UINT8 work_time;       //系统配置 *
UINT8 grp_wor_period;       //系统配置 *

 RFINIT bak; 
st_tx_buff gtx;	//26字节
UINT16 crc_buff[9];	//18字节
UINT8 eeprom_buff[22];	//22字节

#endif
