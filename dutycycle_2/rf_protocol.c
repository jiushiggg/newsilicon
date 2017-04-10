#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "rail.h"
#include "rail_types.h"
#include "em_int.h"
#include "rtcdriver.h"
#include "bsp.h"

#include "system.h"
#include "crc.h"
#include "bank2.h"
#include "rf_protocol.h"
#include "main.h"

#define IO_GPIO2	_pd6
#define IO_WAIT_RX_TX		(UINT8)1
#define IO_RX_TX_FINISH		(UINT8)0

#define MAX_GROUP			(UINT8)24
#define RF_ACKPKG_LEN		(UINT8)26
#define RF_WAKEUP_PACKET	(UINT8)2


#ifdef LCD_213
#define VERSION_OP5			(UINT8)30
#endif

#ifdef LCD_213F
#define VERSION_OP5			(UINT8)32
#endif

#ifdef LCD_154
#define VERSION_OP5			(UINT8)31
#endif

#ifdef LCD_154_BAT2477
#define VERSION_OP5			(UINT8)33
#endif


//发射功率
#define RF_TX_1DB           (UINT8)0x1f
#define RF_TX_L175DB        (UINT8)0x0f
#define RF_TX_L3DB          (UINT8)0x15


//数据层格式:
//控制字
//000：查询
//001：属性
//010：多页更新
//011：保留
//100：遥控操作
//101：组网
//110：保留
//111：睡眠
#define DATA_CTRL_QUERY		(UINT8)0X00
#define DATA_CTRL_ATTR		(UINT8)0X20
#define DATA_CTRL_MULT_P	(UINT8)0x40
#define LINK_CTRL_QUERY		(UINT8)0X60
#define DATA_CTRL_RM_CTL	(UINT8)0x80
#define DATA_CTRL_NETLINK	(UINT8)0xA0
#define DATA_CTRL_GRP_PKG1	(UINT8)0XC0
#define DATA_CTRL_SLEEP		(UINT8)0xE0
#define DATA_CTRL_MASK		(UINT8)0XE0



//0x72：价签回复ACK后直接睡眠
//0x53：价签回复ACK后，基站将进行数据传输
//0x74：价签回复电池电压状态
#define DATA_QUERY_SLEEP	(UINT8)0X72
#define DATA_QUERY_TRANS	(UINT8)0X53
#define DATA_QUERY_VOLATGE	(UINT8)0X74

//链路层格式
//ctrl：
//001:global命令
//cmd:
//0011：Heartbeat Requst
//0101:Heartbeat config
//1001:set config
//1010:page change
//1011:led config
//1111:lcd config


#define LINK_CTRL_GLB		(UINT8)0X20
#define LINK_CTRL_JUMP_GROUP1 (UINT8)0X60
#define LINK_CTRL_CHL		(UINT8)0X80
#define LINK_CTRL_SET_WK	(UINT8)0X40
#define LINK_CTRL_GRP_PKG1	(UINT8)0XC0
#define LINK_CTRL_GRP_PKG0	(UINT8)0XA0
#define LINK_CTRL_REMOTE_WK (UINT8)0XB0				//question
#define LINK_CTRL_MASK		(UINT8)0XE0

//链路层命令
#define LINK_CMD_HB_REQ		(UINT8)0X3C
#define LINK_CMD_HB_CNF		(UINT8)0X5A
#define LINK_CMD_SET_CNF		(UINT8)0X96
#define LINK_CMD_PG_CHNG		(UINT8)0XA5
#define LINK_CMD_LED_CNF		(UINT8)0XB4
#define LINK_CMD_LCD_CNF		(UINT8)0XF0
#define LINK_CMD_MASK		(UINT8)0XFF

//链路层参数
#define LINK_PARA_HREQ		(UINT8)0X80
#define LINK_PARA_HCNF		(UINT8)0X20
#define LINK_PARA_SCNF		(UINT8)0X40
#define LINK_PARA_PCHNG		(UINT8)0X60
#define LINK_PARA_LEDCNF	(UINT8)0XA0
#define LINK_PARA_LCDCNF	(UINT8)0XC0
#define LINK_PARA_MASK		(UINT8)0XE0

//心跳控制字
#define HB_CTRL_NORMAL		(UINT8)0XC0
#define HB_CTRL_REED		(UINT8)0X10
#define HB_CTRL_ACK			(UINT8)0X20
#define HB_CTRL_NFC			(UINT8)0X30
#define HB_CTRL_IB			(UINT8)0XC0
#define HB_CTRL_TCHS		(UINT8)0XD0
#define HB_CTRL_HBREQ		(UINT8)0XE0
#define HB_CTRL_MASK		(UINT8)0XF0



#define SET_SLOT_MASK		(UINT16)0X1FFF
#define WORK_TIME_MASK		(UINT8)0X0F


#define DATA_LEN			(UINT8)20
#define REMOTE_HB			(UINT8)5		
#define GLITTER_SCREEN_NUM	(UINT16)0X0100


#define MINUTE_30		(UINT16)900


//RAIL
#define SET_WOR_REC_TIME	300000	//3ms
#define GRP_WOR_REC_TIME	3000	//3ms
#define WKUP1_REC_TIME		1000000	//1s
#define DATA_REC_TIME		4000000	//4s

#define DEFAULT_CHANNEL 0XFF

void wakeup_wait(UINT16 tm);

static void st_RF_init(void);
static void st_idle(UINT8 n);
static void st_sleep(void);
static RF_ERROR_T st_wkup(RFID id, UINT8 chl);

static RF_ERROR_T st_set_wor(void);
//static RF_ERROR_T rf_wor_receive(UINT8 *buf, UINT8 len);
static RF_ERROR_T rf_receive(void);
static RF_ERROR_T st_group_WOR(void);

static void data_layer_ack(void);	
static void link_layer_cmd(void);
static void mode_change(void);

static BOOL rf_loop_condition(void);
//static void data_layer_cmd(stRecvBuff *p);
void data_layer_cmd(stRecvBuff *p);

static RF_ERROR_T st_remote_wor(void);
static UINT8 a7106_tx_data(RFID id, UINT8 len, UINT8 *buf,bps_enum kbps);
//extern BOOL EEPROM_Read(UINT8 addr, UINT8* dst, UINT8 n);
//extern void led_map(void);
extern void myChangeRadioConfig(uint8_t channel, bps_enum bps, uint8_t* ID, uint8_t len);
extern RTCDRV_TimerID_t slaveRtcId;  // RTC timer ID
extern bool slaveRtcRunning;         // boolean for RTC status

RAIL_AppendedInfo_t appendedInfo;
uint8_t received_data = false;
volatile uint8_t received_tmout = 0;

RF_ERROR_T myret = 0;

void RAILCb_RxPacketReceived(void *rxPacketHandle)
{
  RAIL_RxPacketInfo_t* rxPacketInfo = rxPacketHandle;
//  uint8_t len = sizeof(RAIL_AppendedInfo_t)+sizeof(rxPacketInfo->dataLength)+rxPacketInfo->dataLength;
  memcpy((uint8_t*)&appendedInfo.timeUs, (uint8_t*)&rxPacketInfo->appendedInfo.timeUs, sizeof(RAIL_AppendedInfo_t));
  memcpy(gRFbuf.buff , rxPacketInfo->dataPtr, sizeof(gRFbuf.buff));
  received_data = true;
}

void RAILCb_TimerExpired (void)	//
{
	received_tmout = true;
}

void remote_data(void)
{
/*
	gtx.rhb_buff.ctrl = 0x10;  

	gtx.rhb_buff.ctrl |= ((VERSION_OP5 & 0xfff) >> 8);   
	gtx.rhb_buff.version = (VERSION_OP5 & 0x00ff); 
	memcpy((UINT8 *)&gtx.rhb_buff.SlaveID,(UINT8 *)&gRFInitData.esl_id,4);
	memcpy((UINT8 *)&gtx.rhb_buff.wkup_id0,(UINT8 *)&gRFInitData.grp_wkup_id,3);

	gtx.rhb_buff.set_wkup_ch = gRFInitData.set_wkup_ch; 
	gtx.rhb_buff.grp_wkup_ch = gRFInitData.grp_wkup_ch; 
	gtx.rhb_buff.esl_data_ch = gRFInitData.esl_data_ch; 
	gtx.rhb_buff.net_id = gRFInitData.esl_netmask;
	gtx.rhb_buff.info = blow_vol_flg;
	gtx.rhb_buff.crc = my_cal_crc16(0,(UINT8 *)&gtx.rhb_buff,sizeof(gtx.rhb_buff)-2);
	gtx.rhb_buff.crc = my_cal_crc16(gtx.rhb_buff.crc,(UINT8 *)&gRFInitData.esl_id, 4);
*/
}
void st_remote_init(void)
{
	/*
	UINT8 i = 0,j = 0;
	remote_data();	
  	A7106_Cmd(CMD_STBY);	
  	Delay_MS(3);	
	A7106_WriteID(gRFInitData.master_id);
	A7106_SetChannel(2);
	A7106_SetFIFOLen(sizeof(gtx.rhb_buff));
	A7106_WriteFIFO((UINT8 *)&gtx.rhb_buff, sizeof(gtx.rhb_buff));
	A7106_SetBPS(TX_BPS);	  	
	for(j=0; j<REMOTE_HB; j++){
		_emi = 0;
		A7106_StartTX();	
		for (i = 0; i < 25; i++) {  //最多等5ms
			if(!A7106_CheckDataing())
			{
			  Delay_100US(4);  
			  break;//正确
			}    		
		}
		_emi = 1;
		Delay_100US(100);
	}
	*/
}

static void st_remote_idle(void)
{	
	/*
	A7106_Cmd(CMD_SLEEP);
	T0_RX_3MS_OFF;							//关3msRX
	ISR_SetEXIT1(TRUE); 						//开ID烧录 
	if (TRUE==bmulti_en || TRUE==bglitter_en){
		TB_ON;								//闪烁	
	}
	if (LED_GLITTER_DISABLE!=gLEDstate && gled.cnt!=0){				
		LED_30MS_ON;							//LED灯 		
	}
	ISR_set_timer_ccra(TRUE, 1, LIRC_200MS);	//开T2
	*/
}

static RF_ERROR_T st_remote_wor(void)
{
	RF_ERROR_T ret = RF_ERROR_NONE;
	/*
	RFID buff;
	ISR_set_timer_ccra(FALSE, 0, LIRC_200MS);			//关T2
	gRxOverflow = FALSE;
	TB_OFF;											//关闪烁
	LED_30MS_OFF;
	LEDOFF;

	A7106_Cmd(CMD_STBY);							//调整到代码入口位置
	Delay_MS(3);									//优化delay时间，配置时间长短
	A7106_SetBPS(RX_BPS);
	A7106_SetChannel(gRFInitData.esl_data_ch);	//写入ESL参数
	A7106_WriteID(gRFInitData.esl_id);	
	A7106_SetFIFOLen(sizeof(gRFbuf.rwp));

	T0_RX_3MS_ON;
	ret = rf_wor_receive((UINT8*)&gRFbuf.rwp, sizeof(gRFbuf.rwp));
	T0_RX_3MS_OFF;
	grx3ms_overflow = FALSE;	
	*/
	return ret;

}

static void st_sleep(void)
{
//	A7106_Cmd(CMD_STBY);
//	A7106_Cmd(CMD_SLEEP);
	RAIL_RfIdle();
/*
	ISR_SetEXIT1(FALSE);						//ID烧录
	TB_OFF;								    //闪烁	
	LED_30MS_OFF;
	LEDOFF;	
*/
	if (gsolt > 5){
		gsolt -= 5;
	}
	BSP_LedToggle(0);
	wakeup_wait(gsolt);
	BSP_LedToggle(0);
}
/*
static RF_ERROR_T rf_wor_receive(UINT8 *buff, UINT8 len)
{

	RF_ERROR_T ret = RF_ERROR_NONE;

	do{
		A7106_StartRX();
		while (A7106_CheckDataing() && FALSE==grx3ms_overflow){			
		}
							
		if (TRUE==grx3ms_overflow){
			A7106_Cmd(CMD_STBY);
			ret = RF_ERROR_RX_TIMEOUT;
		} else {						
			if(FALSE == A7106_CheckCRC()){
				gRF_mode.error |= RF_ERROR_RF_CRC;
				ret = RF_ERROR_RF_CRC;
				continue;
			}		
			A7106_ReadFIFO(buff, len);
			gRF_mode.error = RF_ERROR_NONE;
			ret = RF_ERROR_NONE;
			break;			
		}						
	}while(FALSE==grx3ms_overflow);

	return ret;	
}
	*/
static RF_ERROR_T rf_receive(void)
{
/*
	UINT16 tmp = 0;
	RF_ERROR_T ret = RF_ERROR_NONE;
	_CLRWDT;                                            //清除看门狗
	do{
		A7106_StartRX();
		while (A7106_CheckDataing() && FALSE==gRxOverflow){			
		}
		if (TRUE==gRxOverflow){							//超时
			A7106_Cmd(CMD_STBY);
			gRF_mode.error |= RF_ERROR_RX_TIMEOUT;
			ret = RF_ERROR_RX_TIMEOUT;							
		}else {	
			if(FALSE == A7106_CheckCRC()){	
				gRF_mode.error |= RF_ERROR_RF_CRC;
				ret = RF_ERROR_RF_CRC;
				continue;								//硬件CRC错误
			}						
			A7106_ReadFIFO((UINT8 *)gRFbuf.buff,26);

			if (DATA_CTRL_GRP_PKG1 == (gRFbuf.buff[0]&0xe0)){							
				tmp = my_cal_crc16(0, (UINT8 *)&gRFbuf.buff[0], sizeof(gRFbuf.buff)-2);
				tmp = my_cal_crc16(tmp, (UINT8 *)&gRFInitData.grp_wkup_id, 4);								
			}else{
				tmp = my_cal_crc16(0, (UINT8 *)&gRFbuf.buff[0], sizeof(gRFbuf.buff)-2);
				tmp = my_cal_crc16(tmp, (UINT8 *)&gRFInitData.esl_id, 4);
			}
			if (gRFbuf.gwp1.crc != tmp) {	
				gRF_mode.error |= RF_ERROR_SW_CRC;
				ret = RF_ERROR_SW_CRC;
				continue;								//软件CRC错误							
			}
			gRF_mode.error = RF_ERROR_NONE;
			ret = RF_ERROR_NONE;
			break;		
		}	
	}while(FALSE==gRxOverflow);	
	
	return ret;
	*/

	/*
	RAIL_RfIdle();
	myChangeRadioConfig(gRFInitData.set_wkup_ch, RX_BPS, (UINT8*)&id.ID, sizeof(stRecvBuff));
	rail_status("a");
//	RAIL_RxStart(0);
	*/
	RF_ERROR_T ret = RF_ERROR_UNKNOWN;
	UINT16 tmp = 0;
	memset(gRFbuf.buff , 0, sizeof(gRFbuf.buff));
	memset((uint8_t*)&appendedInfo.timeUs, 0, sizeof(RAIL_AppendedInfo_t));
	received_data = false;
	received_tmout = false;
	do{
		RAIL_RxStart(0);
		while (false == RAIL_TimerExpired()  && false==received_data);
		if (true == received_tmout){
			ret |= RF_ERROR_RX_TIMEOUT;
			RAIL_RfIdle();
//			UDELAY_Delay(1000);
//			rail_status("b");
			break;
		}
		if (0 == appendedInfo.crcStatus){
			ret |= RF_ERROR_RF_CRC;
			continue;
		}
		if (DATA_CTRL_GRP_PKG1 == (gRFbuf.buff[0]&0xe0)){
			tmp = my_cal_crc16(0, (UINT8 *)&gRFbuf.buff[0], sizeof(gRFbuf.buff)-2);
			tmp = my_cal_crc16(tmp, (UINT8 *)&gRFInitData.grp_wkup_id, 4);
		}else{
			tmp = my_cal_crc16(0, (UINT8 *)&gRFbuf.buff[0], sizeof(gRFbuf.buff)-2);
			tmp = my_cal_crc16(tmp, (UINT8 *)&gRFInitData.esl_id, 4);
		}
		if (gRFbuf.gwp1.crc != tmp) {
			gRF_mode.error |= RF_ERROR_SW_CRC;
			ret = RF_ERROR_SW_CRC;
			continue;								//软件CRC错误
		}
		gRF_mode.error = RF_ERROR_NONE;
		ret = RF_ERROR_NONE;
		break;
	}while(false == RAIL_TimerExpired());

	return ret;
}

static void st_idle(UINT8 n)
{										
//	ISR_set_timer_ccra(FALSE, 0, LIRC_2S);
//	gRxOverflow = FALSE;
//	ISR_set_timer_ccrp(TRUE, n, LIRC_2S);	//开ggrp_wor_period S的定时
//	A7106_Cmd(CMD_SLEEP);
	RAIL_RfIdle();
	n = (0==n) ? 1:n;
	RTCDRV_StartTimer(slaveRtcId, rtcdrvTimerTypePeriodic, n*2000, slaveTimerExpiredCallback, NULL);
/*
	T0_RX_3MS_OFF;							//关3msRX
	ISR_SetEXIT1(TRUE);						//开ID烧录 
	if (TRUE==bmulti_en || TRUE==bglitter_en){
		TB_ON;										//闪烁	
	}
	if (LED_GLITTER_DISABLE!=gLEDstate && gled.cnt!=0){				
		LED_30MS_ON;		
	}
*/
}
static RF_ERROR_T st_wkup(RFID id, UINT8 chl)//todo
{
	RF_ERROR_T ret = RF_ERROR_UNKNOWN;
//    A7106_Cmd(CMD_STBY);
//    Delay_MS(2);
//	A7106_SetChannel(chl); 		//写入group参数
//	A7106_WriteID(id);
//	A7106_SetFIFOLen(sizeof(stRecvBuff));
//	ISR_set_timer_ccra(TRUE, 1, LIRC_2S_A);
//	ret = rf_receive();
//	ISR_set_timer_ccra(FALSE, 0, LIRC_100MS);

	if (RAIL_TimerIsRunning()) {
		RAIL_TimerCancel();
	}
	RAIL_RfIdle();
	myChangeRadioConfig(gRFInitData.set_wkup_ch, RX_BPS, (UINT8*)&id.ID, sizeof(stRecvBuff));
	memset(gRFbuf.buff , 0, sizeof(gRFbuf.buff));
	received_data = false;
	received_tmout = false;
//	rail_status("a");
//	RAIL_RxStart(0);
	RAIL_TimerSet(WKUP1_REC_TIME, RAIL_TIME_DELAY);
	ret = rf_receive();
	RAIL_TimerCancel();

	gRxOverflow = FALSE;
	gbit_map = 0;
	gpkg_cnt = 0;
	gsave_flg = 0;
	gpkg_cnt_bak = 0;
	gall_crc = 0;
	blast_pkg_flg = FALSE;
	bnetlink = FALSE;
	bquery = FALSE;
	bremote_flg = FALSE;
	memset((UINT8*)crc_buff,0, sizeof(crc_buff));
	
	return ret;
}

static void st_RF_init(void)
{
	gRF_mode.current_st = RF_ST_INIT;	
//	A7106_Init();
}

static RF_ERROR_T st_set_wor(void)//
{
	RFID buff;	
	RF_ERROR_T ret = RF_ERROR_UNKNOWN;
//	ISR_set_timer_ccrp(FALSE, 0, LIRC_2S);			//关闭T2 CCRP
	RTCDRV_IsRunning(slaveRtcId, &slaveRtcRunning);
	if (slaveRtcRunning) {
	    RTCDRV_StopTimer(slaveRtcId);
    }
	buff.ID[0] = gRFInitData.grp_wkup_id.ID[0];		
	buff.ID[1] = gRFInitData.grp_wkup_id.ID[1];
	buff.ID[2] = 0;
	buff.ID[3] = gRFInitData.grp_wkup_id.ID[3];
//	A7106_Cmd(CMD_STBY);							//调整到代码入口位置
//	Delay_MS(3);									//优化delay时间，配置时间长短
//	A7106_SetBPS(RX_BPS);
//	A7106_SetChannel(gRFInitData.set_wkup_ch); 	//写入SET参数
//	A7106_WriteID(buff);
//	A7106_SetFIFOLen(sizeof(gRFbuf.swp));
//	T0_RX_3MS_ON;
//	ret = rf_wor_receive((UINT8*)&gRFbuf.swp, sizeof(gRFbuf.swp));
//	T0_RX_3MS_OFF;
//	grx3ms_overflow = FALSE;

//	if (RAIL_TimerIsRunning()) {
//		RAIL_TimerCancel();
//	}
	RAIL_RfIdle();
	myChangeRadioConfig(gRFInitData.set_wkup_ch, RX_BPS, (UINT8*)&buff.ID, sizeof(gRFbuf.swp));
	memset(gRFbuf.buff , 0, sizeof(gRFbuf.buff));
	received_data = false;
	received_tmout = false;
//	rail_status("a");
//	RAIL_RxStart(0);
	RAIL_TimerSet(SET_WOR_REC_TIME, RAIL_TIME_DELAY);
	do{
		RAIL_RxStart(0);
		while (false == RAIL_TimerExpired()  && false==received_data);
		if (true == received_tmout){
			ret |= RF_ERROR_RX_TIMEOUT;
			RAIL_RfIdle();
//			UDELAY_Delay(1000);
//			rail_status("b");
			break;
		}
		if (1 == appendedInfo.crcStatus){
			ret = RF_ERROR_NONE;
			break;
		}else {
			ret |= RF_ERROR_RF_CRC;
		}
	}while(false == RAIL_TimerExpired());
	RAIL_TimerCancel();
	switch(gRFbuf.buff[0]&0XE0){
		case LINK_CTRL_GLB:
		case LINK_CTRL_JUMP_GROUP1:
		case LINK_CTRL_CHL:
		case LINK_CTRL_SET_WK:
			break;
		default:
			ret |= RF_ERROR_UNKNOWN;
			break;
	}	
	return ret;
}

static RF_ERROR_T st_group_WOR(void)
{

//	ISR_set_timer_ccrp(FALSE, 0, LIRC_2S);	//关 gsyscnf.grp_wor_period S的定时
//	A7106_Cmd(CMD_STBY);		//写入group参数
//	Delay_MS(3);
//	A7106_SetBPS(RX_BPS);
//	A7106_SetChannel(gRFInitData.grp_wkup_ch);		//工作频率
//	A7106_WriteID(gRFInitData.grp_wkup_id);
//	A7106_SetFIFOLen(sizeof(gRFbuf.gwp0));
	//	T0_RX_3MS_ON;
	//	ret = rf_wor_receive((UINT8*)&gRFbuf.gwp0, sizeof(gRFbuf.gwp0));
	//	T0_RX_3MS_OFF;
	//	grx3ms_overflow = FALSE;
	RF_ERROR_T ret = RF_ERROR_UNKNOWN;
	if (RAIL_TimerIsRunning()) {
		RAIL_TimerCancel();
	}
	RAIL_RfIdle();
	myChangeRadioConfig(gRFInitData.grp_wkup_ch, RX_BPS, (uint8_t*)&gRFInitData.grp_wkup_id, sizeof(gRFbuf.gwp0));
	memset(gRFbuf.buff , 0, sizeof(gRFbuf.buff));
	received_data = false;
	received_tmout = false;
//	rail_status("c");
//	RAIL_RxStart(0);
	RAIL_TimerSet(SET_WOR_REC_TIME, RAIL_TIME_DELAY);
	do{
		RAIL_RxStart(0);
		while (false == RAIL_TimerExpired()  && false==received_data);
		if (true == received_tmout){
			ret |= RF_ERROR_RX_TIMEOUT;
			RAIL_RfIdle();
//			UDELAY_Delay(1000);
//			rail_status("d");
			break;
		}
		if (1 == appendedInfo.crcStatus){
			ret = RF_ERROR_NONE;
			break;
		}else {
			ret |= RF_ERROR_RF_CRC;
		}
	}while(false == RAIL_TimerExpired());
	RAIL_TimerCancel();

	return ret;
}

static void data_layer_ack(void)		//question
{ 
	UINT16 i = 0;
	UINT8 j = 0;
			
	gtx.ack.ctrl = gRFbuf.lqc.ctrl;
	gtx.ack.sequence = gRFbuf.lqc.sequence+1;
	if (gpkg_cnt != gRFbuf.lqc.package_num){	

		gtx.ack.lose_package_num = 0;
		for (i = 0; i < 10; i++)
			gtx.ack.lose_package[i] = 0;
			
		for(i = 0; i < gRFbuf.lqc.package_num; i++) {
			if (!(gbit_map & (1 << (i+1)))){
				gtx.ack.lose_package_num++;
				gtx.ack.lose_package[j] = i+1;
				j++;
				if (j >= 10){
					break;
				}
			}
		}
	}else {
		gtx.ack.lose_package[0] = 0x40;	
		if (bnetlink){
			bnetlink = FALSE;
			gEEPROM = TRUE;		
		}else if(bquery){
			bquery = FALSE;						
		}else if(bremote_flg && gpkg_cnt == 1){
			bremote_flg = FALSE;	
		}else {						
			if (gcnf.all_crc == gall_crc){
				gEEPROM = TRUE;	
			} else {
				gEEPROM = FALSE;	
				gtx.ack.lose_package[0] = 0x50;	
			}				
		}

		gEventFlag |= EVENT_FLAG_EEPROMUPDATE;
		gtx.ack.lose_package_num = 0;	
		for(i=1; i<10; i++){
			gtx.ack.lose_package[i] = 0x0;	
		}
	}
	gtx.ack.crc = my_cal_crc16(0, (UINT8 *)&gtx.buff[0], sizeof(gtx)-2);
	gtx.ack.crc = my_cal_crc16(gtx.ack.crc, (UINT8 *)&gRFInitData.esl_id, sizeof(gRFInitData.esl_id));		
}
//
void wakeup_wait(UINT16 tm)
{
	/*
	volatile UINT16 i=0;
	UINT16 j = 0;
	j = tm>800 ? 400: tm/2;
	_CLRWDT;
	for (i=0; i<j; i++){		
		GCC_DELAY(20000);
	}
	_CLRWDT;	
	for (i=0; i<j; i++){		
		GCC_DELAY(20000);
	}
	_CLRWDT;	
	*/
	RTCDRV_Delay(tm*10);
	/*
	if (RAIL_TimerIsRunning()) {
		RAIL_TimerCancel();
	}
	RAIL_TimerSet(tm*10, RAIL_TIME_DELAY);
	while (false == RAIL_TimerExpired());
	*/
}

static BOOL rf_loop_condition(void)
{

	if (gRF_mode.current_st==RF_ST_SET_IDLE){
		return FALSE;	
	} 
	if (gRF_mode.current_st==RF_ST_GROUP_IDLE){
		return FALSE;	
	}
	if (gRF_mode.current_st==RF_ST_REMOTE_IDLE){
		return FALSE;	
	}
	return TRUE;	
}

void rf_fsm(void)//todo
{
//	RF_ERROR_T err = RF_ERROR_NONE;
	UINT8 tmp = 0;
	do {
		switch(gRF_mode.next_st){
			case RF_LINK_EVENT_HANDLE:
				link_layer_cmd();
				break;		
			case RF_GRP_DATA_EVENT_HANDLE:
			case RF_ESL_DATA_EVENT_HANDLE:
			case RF_REMOTE_EVENT_HANDLE:
				data_layer_cmd((stRecvBuff*)&gRFbuf);
				mode_change();
				break;
			case RF_ST_SET_IDLE:				
				if (gRF_mode.current_st == RF_ST_GROUP_SLEEP || gRF_mode.current_st == RF_ST_GROUP_SLEEP1){			//需多次唤醒						
				    	tmp = gsolt/200;	    			    											
				}else {													//48秒听帧
					tmp = gsyscnf.set_wor_period;										
				}	
				gwor_cnt = GROUP_WOR_MAX_VALUE;			//设置GROUP最大听帧。
				gRF_mode.current_st = RF_ST_SET_IDLE;					
				st_idle(tmp);
				gRF_mode.next_st = RF_ST_SET_WOR;
				break;
			case RF_ST_SET_WOR:
				gRF_mode.current_st = RF_ST_SET_WOR;
				
				if (RF_ERROR_NONE == st_set_wor()){
					gRF_mode.next_st = RF_LINK_EVENT_HANDLE;
				} else {
					gRF_mode.next_st = RF_ST_SET_IDLE;
				}
				break;	
			case RF_ST_GROUP_SLEEP:
				gRF_mode.current_st = RF_ST_GROUP_SLEEP;
								
			    if (gsolt > 800){				//大于4000ms
					gsolt -= 600;
			    	gRF_mode.next_st = RF_ST_SET_IDLE;
			    } else {
					st_sleep();
					gRF_mode.next_st = RF_ST_GROUP_WKUP;																	
			    }
				break;
			case RF_ST_GROUP_SLEEP1:
				gRF_mode.current_st = RF_ST_GROUP_SLEEP1;

			    if (gsolt > 800){				//大于4000ms
			    	gsolt -= 600;
			    	gRF_mode.next_st = RF_ST_SET_IDLE;
			    } else {
					st_sleep();//
					gRF_mode.next_st = RF_ST_GROUP_WOR;																	
			    } 																										  
				break;				
			case RF_ST_GROUP_WKUP:
				gRF_mode.current_st = RF_ST_GROUP_WKUP;
									
				if (RF_ERROR_NONE == st_wkup(gRFInitData.grp_wkup_id, gRFInitData.grp_wkup_ch)){
					gRF_mode.next_st = RF_LINK_EVENT_HANDLE;
				} else {
					gRF_mode.next_st = RF_ST_GROUP_IDLE;
				}				
				break;
			case RF_ST_GROUP_TRANSFER:
				gRF_mode.current_st = RF_ST_GROUP_TRANSFER;
				
				if (RF_ERROR_NONE == rf_receive()){
					gRF_mode.next_st = RF_GRP_DATA_EVENT_HANDLE;
				} else {
					_CLRWDT;
//					ISR_set_timer_ccra(FALSE, 0, LIRC_2S);
					RAIL_TimerCancel();
					gRxOverflow = FALSE;							
					gEventFlag |= EVENT_FLAG_EEPROMUPDATE;
					gRF_mode.next_st = RF_ST_GROUP_IDLE;
				}				
				break;
				
			case RF_ST_GROUP_IDLE:
				gRF_mode.current_st = RF_ST_GROUP_IDLE;					
				st_idle(grp_wor_period);
				gRF_mode.next_st = RF_ST_GROUP_WOR;
				break;
			case RF_ST_GROUP_WOR:
				gRF_mode.current_st = RF_ST_GROUP_WOR;
				if (0==gwor_cnt || 0==--gwor_cnt){								//GROUP听帧计数	
					gRF_mode.next_st = RF_ST_SET_WOR;				
				} else{							
					if (RF_ERROR_NONE == st_group_WOR()){
						gRF_mode.next_st = RF_LINK_EVENT_HANDLE;	
					}else{
						gRF_mode.next_st = RF_ST_GROUP_IDLE;	
					}
				}
				break;
			case RF_ST_ESL_SLEEP:
				gRF_mode.current_st = RF_ST_ESL_SLEEP;
				st_sleep();
				if (RF_ERROR_NONE == st_wkup(gRFInitData.grp_wkup_id, gRFInitData.grp_wkup_ch)){
					gRF_mode.next_st = RF_ESL_DATA_EVENT_HANDLE;					
				} else {
					gRF_mode.next_st = RF_ST_GROUP_IDLE;				
				}			
				break;
			case RF_ST_ESL_TRANSFER:
				gRF_mode.current_st = RF_ST_ESL_TRANSFER;								
			
				if (RF_ERROR_NONE == rf_receive()){
					
					gRF_mode.next_st = RF_ESL_DATA_EVENT_HANDLE;	
				}else {
					_CLRWDT;
					//ISR_set_timer_ccra(FALSE, 0, LIRC_2S);
					RAIL_TimerCancel();
					gRxOverflow = FALSE;					
					gRF_mode.next_st = RF_ST_GROUP_IDLE;	
					gEventFlag |= EVENT_FLAG_EEPROMUPDATE;
				}								
				break;									
			case RF_ST_REMOTE_INIT:
				gRF_mode.current_st = RF_ST_REMOTE_INIT;
				st_remote_init();
				gRF_mode.next_st = RF_ST_REMOTE_WOR;
				break;
			case RF_ST_REMOTE_IDLE:
				gRF_mode.current_st = RF_ST_REMOTE_IDLE;
				st_remote_idle();
				gRF_mode.next_st = RF_ST_REMOTE_WOR;				
				break;
			case RF_ST_REMOTE_WOR:
				gRF_mode.current_st = RF_ST_REMOTE_WOR;
				if (0 == --swor_cnt){								//遥控器听帧计数	
					gRF_mode.next_st = gmode_backup;					//还原到遥控器触发之前的状态。
					gmode_backup = RF_ST_BACKUP;
//					_intc0 &= ~(INT0F);			//
//					_intc0 |= INT0E;			//处理完一次事件后再开中断
				} else{	
					if (RF_ERROR_NONE == st_remote_wor()){
						gRF_mode.next_st = RF_LINK_EVENT_HANDLE;	
					}else {
						gRF_mode.next_st = RF_ST_REMOTE_IDLE;							
					}							
				}
				break;
			case RF_ST_REMOTE_WKUP:
				gRF_mode.current_st = RF_ST_REMOTE_WKUP;
				st_sleep();
				
				if (RF_ERROR_NONE == st_wkup(gRFInitData.grp_wkup_id, gRFInitData.grp_wkup_ch)){
					gRF_mode.next_st = RF_LINK_EVENT_HANDLE;
				} else {
					gRF_mode.next_st = gmode_backup;					//还原到遥控器触发之前的状态。
					gmode_backup = RF_ST_BACKUP;	
//					_intc0 &= ~(INT0F);			//
//					_intc0 |= INT0E;			//处理完一次事件后再开中断
				}			
				break;					
			case RF_ST_REMOTE_TRANSFER:
				gRF_mode.current_st = RF_ST_REMOTE_TRANSFER;								
			
				if (RF_ERROR_NONE == rf_receive()){
					gRF_mode.next_st = RF_REMOTE_EVENT_HANDLE;	
				}else {
					_CLRWDT;
//					ISR_set_timer_ccra(FALSE, 0, LIRC_2S);
					gRxOverflow = FALSE;					
					gRF_mode.next_st = gmode_backup;					//还原到遥控器触发之前的状态。
					gmode_backup = RF_ST_BACKUP;
//					_intc0 &= ~(INT0F);			//
//					_intc0 |= INT0E;			//处理完一次事件后再开中断
				}								
				break;	
			case RF_ST_INIT:
				gRF_mode.current_st = RF_ST_INIT;
				st_RF_init();
				gRF_mode.next_st = RF_ST_SET_WOR;
				break;
			case RF_ERROR_HANDLE:	
			default:
				gRF_mode.current_st = RF_ERROR_HANDLE;
				gRF_mode.next_st = RF_ST_INIT;
//				_intc0 &= ~(INT0F);		//
//				_intc0 |= INT0E;
				break;																																
		}			
	}while(TRUE == rf_loop_condition());//48s听帧或启动WOR.
}

static UINT8 set_time(UINT8 n)
{
	UINT8 ret=1;
	switch(n){
		case 0:
			ret = 1;
			break;
		case 1:
			ret = 1;
			break;
		case 3:
			ret = 2;
			break;
		case 9:
			ret = 3;
			break;
		case 10:
			ret = 4;
			break;			
		default:
			ret = 2;
			break;										
	}	
	return ret;
}

static void link_layer_cmd(void)
{
	UINT8 tmp = 0;
	UINT16 crc = 0;
	
	switch(gRFbuf.buff[0]&LINK_CTRL_MASK){
		case LINK_CTRL_GLB:	
			tmp = gRFInitData.grp_wkup_id.ID[2]%MAX_GROUP;
			if(gRFbuf.swp.grp_mask[tmp/8] &  0x01<<(tmp&0x07)){			//group mask 匹配											//set global
				switch(gRFbuf.swpg.cmd){	//解析参数
					case LINK_CMD_HB_REQ:
						hbreq_en = TRUE;
						hbreq.value = 3*((gRFbuf.swpg.para&0x03)+1);
						hbreq.time = ((gRFbuf.swpg.para>>2 & 0x07)+1);
						hbreq.count = 0;
						break; 	
					case LINK_CMD_HB_CNF:				
						if (0X3F == gRFbuf.swpg.para){
							bheartbeat_en = FALSE;	
						}else {
							bheartbeat_en = TRUE;
							gsyscnf.hb_time = ((gRFbuf.swpg.para&0X1F)+1)*15;						
							gEEPROM = TRUE;
							gsave_flg |= SAVE_SYS_CNF;
							gEventFlag |=EVENT_FLAG_EEPROMUPDATE;								
						}						
						break; 	
					case LINK_CMD_SET_CNF:
						gsyscnf.set_wor_period = (gRFbuf.swpg.para&0X1F)+1;
						gEEPROM = TRUE;
						gsave_flg |= SAVE_SYS_CNF;
						gEventFlag |=EVENT_FLAG_EEPROMUPDATE;
						break; 	
					case LINK_CMD_PG_CHNG:
						gdisplay_time = ((gRFbuf.swpg.para>>3 & 0x03)+1)*MINUTE_30;
						gdsp_mode = LCD_REMOTE_BEGIN;				
						gEventFlag |= EVENT_FLAG_LCDUPDATE;			
						gpage_num = gRFbuf.swpg.para&0x07;					
						break; 	
					case LINK_CMD_LED_CNF:
						if(0x72 == gRFbuf.swpg.para){		//开LED
							gLEDstate = LED_INIT;								
//							EEPROM_Read(DISPLAY_CNF_ADDR+8,(UINT8 *)&gled, sizeof(stLEDSet));
//							led_map();	//led映射关系
							gEventFlag |= EVENT_FLAG_LED;					
						} else {							//关LED
							gLEDstate = LED_GLITTER_DISABLE;														
							gEventFlag |= EVENT_FLAG_LED;						
						}
						break;
/*
					case LINK_CMD_LCD_CNF:
						if (0xA0 == (gRFbuf.swpg.para)){
							_lcdc0 |= LCD_EN;
							glcd_config = 0;							
						}else{
							_lcdc0 &= ~LCD_EN;
							glcd_config = MINUTE_30*((0x1f&gRFbuf.swpg.para)+1);	
						}
						break;
*/
					default:
						gRF_mode.error |= RF_ERROR_CTRL;			
						break;																								
				}	
					
			} 
			gRF_mode.next_st = RF_ST_SET_IDLE;	
			break;	
		case LINK_CTRL_CHL:												//set channel
			{
			static UINT8 schl_cnt = 0;
		    UINT8 buff[4]={0};
			memcpy(buff,(UINT8 *)&gRFInitData.grp_wkup_id, sizeof(gRFInitData.grp_wkup_id)); 
			buff[2] = 0;	
			crc = my_cal_crc16(0, (UINT8*)&gRFbuf.swpc, sizeof(gRFbuf.swpc)-2);
			crc = my_cal_crc16(crc, buff, sizeof(gRFInitData.grp_wkup_id));
			if (gRFbuf.swpc.crc == crc){
				schl_cnt = 0;
				memcpy((UINT8 *)&bak.master_id, (UINT8 *)&gRFInitData.master_id, sizeof(bak.master_id));
				memcpy((UINT8 *)&bak.grp_wkup_id,(UINT8 *)&gRFInitData.grp_wkup_id, sizeof(bak.grp_wkup_id));
				memcpy((UINT8 *)&bak.esl_id,(UINT8 *)&gRFInitData.esl_id, sizeof(gRFInitData.esl_id));
				bak.set_wkup_ch = gRFbuf.swpc.set_wkup_ch;
				bak.grp_wkup_ch = gRFbuf.swpc.grp_wkup_ch;
				bak.esl_data_ch = gRFbuf.swpc.esl_data_ch;	
				bak.reserve = gRFbuf.swpc.esl_data_ch;
				bak.esl_netmask	= gRFInitData.esl_netmask;
				gEEPROM = TRUE;
				gsave_flg |= SAVE_DEV_INFO;
				gEventFlag |= EVENT_FLAG_EEPROMUPDATE;									
				gRF_mode.next_st = RF_ST_SET_IDLE;									
			}else {
				if (++schl_cnt >=3 ){
					schl_cnt = 0;
					gRF_mode.next_st = RF_ST_SET_IDLE;	
				} else {
					gRF_mode.next_st = RF_ST_SET_WOR;
				}
			}	
			break;	
			}
		case LINK_CTRL_JUMP_GROUP1:
			tmp = gRFInitData.grp_wkup_id.ID[2]%MAX_GROUP;
			if (gRFbuf.swp.grp_mask[tmp/8] &  (0x01 << (tmp&0x07))){			//group mask 匹配				
				gsolt =  (UINT16)(gRFbuf.buff[0]&0X1f)<<8  |  gRFbuf.swp.solt_l;					

				work_time = set_time(gRFbuf.swp.para & WORK_TIME_MASK);								
				gRF_mode.next_st = RF_ST_GROUP_SLEEP1;								
			} else {															//不匹配
				gRF_mode.next_st = RF_ST_SET_IDLE;		
			}									
			break;									
		case LINK_CTRL_SET_WK:												//基站SET WAKEUP帧		
			tmp = gRFInitData.grp_wkup_id.ID[2]%MAX_GROUP;
			if (gRFbuf.swp.grp_mask[tmp/8] &  (0x01 << (tmp&0x07))){			//group mask 匹配				
				gsolt =  (UINT16)(gRFbuf.buff[0]&0X1f)<<8  |  gRFbuf.swp.solt_l;					
				grp_wor_period = set_time(gRFbuf.swp.para >> 4);								//
				work_time = set_time(gRFbuf.swp.para & WORK_TIME_MASK);								
				gRF_mode.next_st = RF_ST_GROUP_SLEEP;								
			} else {															//不匹配
				gRF_mode.next_st = RF_ST_SET_IDLE;		
			}									
			break;
		case LINK_CTRL_GRP_PKG1:
			tmp = gRFInitData.esl_netmask%160;											//基站和遥控器唤醒帧1		
			if (RF_ST_REMOTE_WKUP == gRF_mode.current_st){					//遥控器模式
				if (!(gRFbuf.gwp1.esl_mask[tmp/8] & 		//esl mask 不匹配
					0x01<<(tmp&0x07))){
					gRF_mode.next_st = gmode_backup;					//还原到遥控器触发之前的状态。
					gmode_backup = RF_ST_BACKUP;
//					_intc0 &= ~(INT0F);			//
//					_intc0 |= INT0E;			//处理完一次事件后再开中断
					break;
				}
				gRFInitData.reserve = gRFbuf.gwp1.channel;	
//				A7106_SetChannel(gRFInitData.reserve);	//工作频率
//				A7106_WriteID(gRFInitData.esl_id);
//				ISR_set_timer_ccra(TRUE, work_time, LIRC_2S_A);					//TM2开超时
				myChangeRadioConfig(gRFInitData.reserve, DEFAULT_BPS, (uint8_t*)&gRFInitData.esl_id, 26);
				RAIL_TimerSet(WKUP1_REC_TIME, RAIL_TIME_DELAY);
				gRF_mode.next_st = RF_ST_REMOTE_TRANSFER;	
				break;				
			}
			if (!(gRFbuf.gwp1.esl_mask[tmp/8] &  		//esl mask 不匹配
				0x01<<(tmp&0x07))){
				gRF_mode.next_st = RF_ST_GROUP_IDLE;
				break;
			}
			gRFInitData.reserve = gRFbuf.gwp1.channel;
//			A7106_SetChannel(gRFInitData.reserve);	//工作频率
//			A7106_WriteID(gRFInitData.esl_id);
//			ISR_set_timer_ccra(TRUE, work_time, LIRC_2S_A);					//TM2开超时
			myChangeRadioConfig(gRFInitData.reserve, DEFAULT_BPS, (uint8_t*)&gRFInitData.esl_id, 26);
			RAIL_TimerSet(WKUP1_REC_TIME, RAIL_TIME_DELAY);
			gRF_mode.next_st = RF_ST_GROUP_TRANSFER;
			break;			
		case LINK_CTRL_GRP_PKG0:											//基站唤醒帧0
			if (RF_ST_GROUP_WOR == gRF_mode.current_st){
				gsolt = (UINT16)(gRFbuf.gwp0.ctrl&0x1f)<<8 | gRFbuf.gwp0.solt;
				gRF_mode.next_st = RF_ST_ESL_SLEEP; 
			} else if (RF_ST_REMOTE_WOR == gRF_mode.current_st){	
				gsolt = (UINT16)(gRFbuf.gwp0.ctrl&0x1f)<<8 | gRFbuf.gwp0.solt;
				gRF_mode.next_st = RF_ST_REMOTE_WKUP;
			}else{
				gRF_mode.error |= RF_ERROR_CTRL;
				gRF_mode.next_st = RF_ERROR_HANDLE;					
			}
			break;	
		default:
			gRF_mode.error |= RF_ERROR_CTRL;
			gRF_mode.next_st = RF_ERROR_HANDLE;		
			break;								
	}	
}

void data_layer_cmd(stRecvBuff *p)
{
	volatile UINT8 tmp=0;
	switch(p->buff[0]&DATA_CTRL_MASK){
		case DATA_CTRL_QUERY:	
			gbit_map = 0;
			switch(p->qc.query_cmd){	
				case DATA_QUERY_VOLATGE:					
					if (p->qc.package_num[1] & 0x80){
						gbit_map |= (UINT16)1<< p->cc.package_num[0];
					}				
					bquery = TRUE;
					break;	
				default:
					break;															
			}			
			break;	
		case LINK_CTRL_QUERY:

			break;
		case DATA_CTRL_ATTR:							//保存属性			
			gbit_map |= (UINT16)1<< p->cc.package_num[0];
			memcpy((UINT8 *)&gcnf.security_code, (UINT8 *)&p->cc.security_code, sizeof(stDisplayCnf));	
			gsave_flg |= SAVE_DISPLAY_CNF;

			break;	
		case DATA_CTRL_MULT_P:									//多屏	
			gbit_map |= (UINT16)1<<p->mpc.package_num[0];
			tmp = p->mpc.screen_num&0x07;						//只接收0-7屏	
			
			if (p->mpc.screen_num & 0xf0){						//	闪烁段
				tmp_p = (UINT8 *)&gglitter_seg[0];
				crc_buff[8] = gRFbuf.gwp1.crc;
				gsave_flg |= SAVE_GLITTER;					
			} else {
				crc_buff[tmp] = gRFbuf.gwp1.crc;
				gsave_flg |= 0x01 << tmp;					
				if (tmp < 4){									// 一个RAM存不下,所以分两个数组										
					tmp_p = (UINT8 *)&gdisplay.buff[tmp][0];	//编译器BUG(参数中不能有乘法)，必须这么写。						
				}else{					
					tmp -= 4;
					tmp_p = (UINT8 *)&gdisplay1.buff[tmp][0];
				}					
			}	
			memcpy((UINT8 *)tmp_p, (UINT8 *)&p->mpc.data[0], sizeof(gdisplay.buff[0]));
 			break;
		case DATA_CTRL_NETLINK:							//保存组网数据	
			bnetlink = TRUE;
			gbit_map = 0;				
			gbit_map |= (UINT16)1<< p->nl.package_num[0];	
			memcpy((UINT8 *)&bak.master_id, (UINT8 *)&gRFInitData.master_id, sizeof(gRFInitData.master_id));
			memcpy((UINT8 *)&bak.grp_wkup_id,(UINT8 *)&p->nl.set_wk_id, sizeof(bak.grp_wkup_id));
			memcpy((UINT8 *)&bak.esl_id,(UINT8 *)&gRFInitData.esl_id, sizeof(gRFInitData.esl_id));
			bak.set_wkup_ch = p->nl.set_wkup_ch;
			bak.grp_wkup_ch = p->nl.grp_wkup_ch;
			bak.esl_data_ch = p->nl.esl_data_ch;	
			bak.reserve = p->nl.esl_data_ch;
			bak.esl_netmask	= p->nl.esl_netmask;
			gsave_flg |= SAVE_DEV_INFO;
			break;
		case DATA_CTRL_GRP_PKG1:										//esl层唤醒帧1
			if (gRF_mode.current_st == RF_ST_REMOTE_TRANSFER){			//遥控器模式
				gRFInitData.reserve = p->gwp1.channel;	
//				A7106_SetChannel(gRFInitData.reserve);	//工作频率
//				A7106_WriteID(gRFInitData.esl_id);
//				ISR_set_timer_ccra(TRUE, work_time, LIRC_2S_A);					//TM2开超时
				myChangeRadioConfig(gRFInitData.reserve, RX_BPS, (uint8_t*)&gRFInitData.esl_id, 26);
				RAIL_TimerSet(WKUP1_REC_TIME, RAIL_TIME_DELAY);
				gRF_mode.current_st = RF_ST_REMOTE_TRANSFER;
				break;
			}
			if (gRF_mode.current_st <= RF_ST_GROUP_TRANSFER){			// 不是esl层的数据传输，退出
				gRF_mode.next_st = RF_ST_GROUP_IDLE;
				gRF_mode.error |= RF_ERROR_CTRL;				 				
				break;
			}
			tmp = gRFInitData.esl_netmask%160;
			if (!(p->gwp1.esl_mask[tmp/8] & 0x01<<(tmp&0x07))){//esl mask 不匹配
				gRF_mode.next_st = RF_ST_GROUP_IDLE;
				break;
			}
				gRFInitData.reserve = p->gwp1.channel;	
//				A7106_SetChannel(gRFInitData.reserve);	//工作频率
//				A7106_WriteID(gRFInitData.esl_id);
//				ISR_set_timer_ccra(TRUE, work_time, LIRC_2S_A);					//TM2开超时
				myChangeRadioConfig(gRFInitData.reserve, RX_BPS, (uint8_t*)&gRFInitData.esl_id, 26);
				RAIL_TimerSet(DATA_REC_TIME, RAIL_TIME_DELAY);
				gRF_mode.next_st = RF_ST_ESL_TRANSFER;
			break;
		case DATA_CTRL_RM_CTL:
			bremote_flg = TRUE;	
			gbit_map |= (UINT16)1<<p->rdata.package_num[0];
			if (gcnf.security_code != p->rdata.security_code){
				break;
			}
			if (p->rdata.period > 0){
				memcpy((UINT8 *)&gled.num, (UINT8 *)&p->rdata.led_num, sizeof(stLEDSet)-1);
			}
			if (0==p->rdata.period && 0==p->rdata.t1 && 0==p->rdata.t2 && 0==p->rdata.t0){
				memcpy((UINT8 *)&gled.num, (UINT8 *)&p->rdata.led_num, sizeof(stLEDSet)-1);
			}
						
			if (p->rdata.display_time > 0){
				gdisplay_time = p->rdata.display_time;
				gdsp_mode = LCD_REMOTE_BEGIN;
				gpage_num = p->rdata.page_num;
				gEventFlag |= EVENT_FLAG_LCDUPDATE;	
			}
			if (0==p->rdata.display_time && 0XFF==p->rdata.page_num){
				gdsp_mode = LCD_REMOTE_FINISH;
				gEventFlag |= EVENT_FLAG_LCDUPDATE;		
			}							
			break;								
		case DATA_CTRL_SLEEP:
			gEventFlag |= EVENT_FLAG_EEPROMUPDATE;
			break;	
		default:
			gRF_mode.next_st = RF_ST_GROUP_WOR;
			gRF_mode.error |= RF_ERROR_CTRL; 
			break;							
	}
	
	gpkg_cnt = BitNumber(gbit_map);	
	if (0x80&p->rdata.package_num[1]){		//最后一包		
		gpkg_cnt_bak = 	p->rdata.package_num[0];
		blast_pkg_flg = TRUE;
	}
	if (gpkg_cnt_bak==gpkg_cnt && TRUE==blast_pkg_flg){
		gpkg_cnt_bak = 0;
		blast_pkg_flg = FALSE;	
		gall_crc = all_crc16();	
	}
}
void mode_change(void)//todo
{
	switch(gRFbuf.buff[0]&DATA_CTRL_MASK){
		case LINK_CTRL_QUERY:
			data_layer_ack();
			a7106_tx_data(gRFInitData.master_id, sizeof(st_tx_buff), (UINT8*)gtx.buff,TX_BPS);
//			A7106_SetBPS(RX_BPS);
//			A7106_WriteID(gRFInitData.esl_id);

			if (RF_ST_GROUP_TRANSFER == gRF_mode.current_st){
				gRF_mode.next_st = RF_ST_GROUP_TRANSFER;
			}else if (RF_ST_ESL_TRANSFER == gRF_mode.current_st){
				gRF_mode.next_st = RF_ST_ESL_TRANSFER;
			}else{
				if (RF_ST_REMOTE_TRANSFER == gRF_mode.current_st){
					gRF_mode.next_st = RF_ST_REMOTE_TRANSFER;
				}
			}						
			break;
		case DATA_CTRL_QUERY:						
		case DATA_CTRL_ATTR:							//保存属性					
		case DATA_CTRL_MULT_P:							//多屏					
		case DATA_CTRL_NETLINK: 						//保存组网数据	
		case DATA_CTRL_RM_CTL:		
			if (RF_ST_GROUP_TRANSFER == gRF_mode.current_st){
				gRF_mode.next_st = RF_ST_GROUP_TRANSFER;
			}else if (RF_ST_ESL_TRANSFER == gRF_mode.current_st){
				gRF_mode.next_st = RF_ST_ESL_TRANSFER;
			}else{
				if (RF_ST_REMOTE_TRANSFER == gRF_mode.current_st){
					gRF_mode.next_st = RF_ST_REMOTE_TRANSFER;
				}
			}						
			break;		
		case DATA_CTRL_GRP_PKG1:										//esl层唤醒帧1															
			break;			
		case DATA_CTRL_SLEEP:
			if (RF_ST_GROUP_TRANSFER == gRF_mode.current_st){
				gRF_mode.next_st = RF_ST_GROUP_IDLE;
			}else if (RF_ST_ESL_TRANSFER == gRF_mode.current_st){
				gRF_mode.next_st = RF_ST_GROUP_IDLE;
			}else{
				if (RF_ST_REMOTE_TRANSFER == gRF_mode.current_st){
					gRF_mode.next_st = gmode_backup;					//还原到遥控器触发之前的状态。
					gmode_backup = RF_ST_BACKUP;
//					_intc0 &= ~(INT0F);			//
//					_intc0 |= INT0E;			//处理完一次事件后再开中断
				}
			}										
			break;	
		default:
//			ISR_set_timer_ccra(FALSE, 0, LIRC_100MS);
			gRF_mode.next_st = RF_ST_GROUP_WOR;
			gRF_mode.error |= RF_ERROR_CTRL; 
			break;			
	}

}

static UINT8 a7106_tx_data(RFID id, UINT8 len, UINT8 *buf, bps_enum kbps)
{
/*
	UINT8 i;
  	A7106_WriteID(id);
	A7106_SetBPS(kbps);
    A7106_SetChannel(gRFInitData.reserve); 
  	A7106_SetFIFOLen(len);
  	A7106_WriteFIFO(buf, len);
  
	A7106_StartTX();
	for (i = 0; i < 25; i++) {  //最多等5ms
	if(!A7106_CheckDataing())
	{
	  Delay_100US(4);
	  return RF_ERROR_NONE;//正确
	}
	Delay_100US(2);
  }
*/
	UINT8 i = 0, ret = 0;
	RAIL_TxData_t txInfo;
	RAIL_RfIdle();
	myChangeRadioConfig(gRFInitData.reserve, kbps, (UINT8*)&id.ID, len);
	txInfo.dataPtr = buf;
	txInfo.dataLength = len;
    RAIL_TxDataLoad(&txInfo);
    while(i<3){
    	ret = RAIL_TxStart(0, NULL, NULL);
    	if (0 == ret){
    		break;
    	}
    	i++;

    }
    return ret;
  
}

void HeartBeat(void)
{  
#define HEARTBEAT_CHANNEL 10
	static uint8_t hb_info_cnt = 0;
	RAIL_TxData_t txInfo;
//  	UINT8 i = 0;
	if(hbreq_en){
		gtx.rhb_buff.ctrl = 0xe0;
	}else {
		gtx.rhb_buff.ctrl = 0xf0; 	
	}
	 

	gtx.rhb_buff.ctrl |= ((VERSION_OP5 & 0x0f00) >> 8);   
	gtx.rhb_buff.version = (VERSION_OP5 & 0x00ff); 
	memcpy((UINT8 *)&gtx.rhb_buff.SlaveID,(UINT8 *)&gRFInitData.esl_id,4);
	memcpy((UINT8 *)&gtx.rhb_buff.wkup_id0,(UINT8 *)&gRFInitData.grp_wkup_id,3);

	gtx.rhb_buff.set_wkup_ch = gRFInitData.set_wkup_ch; 
	gtx.rhb_buff.grp_wkup_ch = gRFInitData.grp_wkup_ch; 
	gtx.rhb_buff.esl_data_ch = gRFInitData.esl_data_ch; 
	gtx.rhb_buff.net_id = gRFInitData.esl_netmask;

#ifdef HB_SET_WOR
	gtx.rhb_buff.info = hb_setwor_fun();
#else
	if (hb_info_cnt++ >= 0x10){
		hb_info_cnt = 0;	
	}	
	gtx.rhb_buff.info = hb_info_cnt<<4;
#endif	
	gtx.rhb_buff.info = (gtx.rhb_buff.info&0xf8) | gvol_value;	
	gtx.rhb_buff.crc = my_cal_crc16(0,(UINT8 *)&gtx.rhb_buff,sizeof(gtx.rhb_buff)-2);
	gtx.rhb_buff.crc = my_cal_crc16(gtx.rhb_buff.crc,(UINT8 *)&gRFInitData.esl_id, 4);

//  	A7106_Cmd(CMD_STBY);
	RAIL_RfIdle();
//  Delay_MS(3);

//	A7106_WriteID(gRFInitData.master_id);
//	A7106_SetChannel(2);
//	A7106_SetFIFOLen(sizeof(gtx.rhb_buff));
//	A7106_WriteFIFO((UINT8 *)&gtx.rhb_buff, sizeof(gtx.rhb_buff));
//	A7106_SetBPS(TX_BPS);
	myChangeRadioConfig(HEARTBEAT_CHANNEL, TX_BPS, (uint8_t*)&gRFInitData.master_id, sizeof(gtx.rhb_buff));
	txInfo.dataPtr = gtx.buff;
	txInfo.dataLength = sizeof(gtx.rhb_buff);
    RAIL_TxDataLoad(&txInfo);
//	_emi = 0;
	INT_Disable();
    RAIL_TxStart(0, NULL, NULL);

//	for (i = 0; i < 25; i++) {	//最多等5ms
//	  if(!A7106_CheckDataing())
//	  {
//		Delay_100US(4);
//		break;
//	  }
//	  Delay_100US(1);
//	}
//	_emi = 1;
	INT_Enable();
//	A7106_Cmd(CMD_SLEEP);
}


