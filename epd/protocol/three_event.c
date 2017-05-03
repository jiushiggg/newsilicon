#include "..\sys_init\datatype.h"
#include "..\global_variable\global_variable.h"
#include "..\delay\delay.h"
#include "..\sys_init\sys_isr.h"
#include "three_event.h"
#include "..\sys_init\sys_isr.h"
#include "..\crc16\crc16.h"
#include <string.h>
#include "..\adc10\adc10.h"
//#include "test_spi.h"
#include "..\reed\reed.h"
#include "..\osd\bit_display.h"
#include "main.h"

#include <rail.h>

RAIL_AppendedInfo_t appendedInfo;
volatile uint8_t received_data = FALSE;
volatile uint8_t received_tmout = FALSE;


void RAILCb_RxPacketReceived(void *rxPacketHandle)
{
  RAIL_RxPacketInfo_t* rxPacketInfo = rxPacketHandle;
//  uint8_t len = sizeof(RAIL_AppendedInfo_t)+sizeof(rxPacketInfo->dataLength)+rxPacketInfo->dataLength;
  memcpy((uint8_t*)&appendedInfo.timeUs, (uint8_t*)&rxPacketInfo->appendedInfo.timeUs, sizeof(RAIL_AppendedInfo_t));
  memcpy(receiveBuffer, rxPacketInfo->dataPtr, RF_NORMAL_PACKET);
  received_data = TRUE;
}
void RAILCb_TimerExpired(void)
{
	received_tmout = TRUE;
}

void clear_pkg_nosd_fun(void)
{
  if(ret_ack_flag != RF_EVENT_OSD )
    clear_gpkg_fun();  
}
UINT8 check_GPIO1_init(void)
{
/* todo
  if(A7106_CheckDataing())
  {
    smclk_timer_isr(FALSE, 0,0);
    return 0; // 待机状态错误
  }
  return 1;
*/
	return 1;
}

UINT8 check_gpio1_finish(void)
{
/*	//todo
  do {
    if(gEventFlag&EVENT_FLAG_LTIMER)
    {
      smclk_timer_isr(FALSE, 0,0);
      return 0;//定时器超时
    }
  } while(A7106_CheckDataing());
*/
	do{
		if(TRUE == received_tmout)
		{
			RAIL_TimerCancel();
			return 0;
		}
	}while(FALSE==received_data);
  return 1;
}

RF_ERROR_T rf_cmd_init(RF_T *rf) 
{
/*	//todo
  if (A7106_Initial()) 
  {
//    A7106_Cmd(CMD_STBY);
//    Delay_ms(4);
    return RF_ERROR_NONE;
  }
  return RF_ERROR_CAL;
 */
	RAIL_RfIdle();
	return RF_ERROR_NONE;
}

RF_ERROR_T rf_into_stby(RF_T *rf) 
{  
//  A7106_SleepToStby();	todo
  RAIL_RfIdle();
  return RF_ERROR_NONE;
}

RF_ERROR_T rf_set_wor(RF_T *rf,BOOL slp_flag) 
{  
	RFID setwkupid_temp = {0};
//	memcpy((UINT8*)&setwkupid_temp, (UINT8*)&INFO_DATA.gRFInitData.wakeup_id, 4);
	setwkupid_temp = INFO_DATA.gRFInitData.wakeup_id;
	setwkupid_temp.id2 = 0x00;
/*					todo
  if(slp_flag)
    A7106_SleepToStby(); 
  A7106_SetBPS(RX_BPS);
  A7106_SetChannel(INFO_DATA.gRFInitData.set_wkup_ch); //工作频率
  A7106_WriteID(setwkupid_temp); //写入唤醒ID
  A7106_clr_rx_fifo(RF_SET_WAKEUP_PACKET);
*/
	RAIL_RfIdle();
	myChangeRadioConfig(INFO_DATA.gRFInitData.set_wkup_ch, RX_BPS, (UINT8*)&setwkupid_temp.id0, RF_SET_WAKEUP_PACKET);
    return RF_ERROR_NONE;
}

//只需要接收数据，直接调用底层rf函数即可
RF_ERROR_T rf_cmd_set_rx(RF_T *rf, uint8_t *buf)
{
  //todo
/*
  A7106_clr_rx_fifo(RF_SET_WAKEUP_PACKET);
  
  A7106_StartRX(); 
  
  if(!check_gpio1_finish())
  {
    return RF_ERROR_RX_TIMEOUT;//定时器超时 
  }
  // Delay_us(400);
  //Delay_us(200);
  if(!A7106_CheckCRC())
    return RF_ERROR_RF_CRC;//crc错误
  
  A7106_ReadFIFO(buf, RF_SET_WAKEUP_PACKET);
  return RF_ERROR_NONE;//正确
*/
	memset(receiveBuffer , 0, RF_SET_WAKEUP_PACKET);
	memset((uint8_t*)&appendedInfo , 0, sizeof(RAIL_AppendedInfo_t));
	received_data = false;
	received_tmout = false;
	RAIL_RxStart(0);
	if(!check_gpio1_finish())
	{
		return RF_ERROR_RX_TIMEOUT;//定时器超时
	}
	if (FALSE == appendedInfo.crcStatus){
		return RF_ERROR_RF_CRC;//crc错误
	}
	memcpy(buf, receiveBuffer, RF_SET_WAKEUP_PACKET);
	return RF_ERROR_NONE;//正确
}

RF_CMD_T set_rx_err(RF_ERROR_T err)
{
  if(err == RF_ERROR_ENTER_TXRX)
  {
    //smclk_timer_isr(FALSE,0,0); todo
	if (RAIL_TimerIsRunning()) {
		RAIL_TimerCancel();
	}
    return RF_FSM_CMD_RF_POR_RST;
  }
  else if( err == RF_ERROR_RX_TIMEOUT )
  {
    //smclk_timer_isr(FALSE,0,0);
    //A7106_Cmd(CMD_STBY);
	if (RAIL_TimerIsRunning()) {
		RAIL_TimerCancel();
	}
    RAIL_RfIdle();
    return RF_FSM_CMD_SLEEP;//此做法的目的是当接收set唤醒时，超时，退出是不再配置rf参数，直接进入到sleep
  }
  else if(err == RF_ERROR_RF_CRC )
  {
    return RF_FSM_CMD_SET_RX;
  }
  else
  {
    return RF_FSM_CMD_SET_RX;
  }
  
}

void set_wkup_event(RF_T *rf, UINT8 ctrl)
{
  switch(ctrl&(UINT8)SET_CTRL_INFO)
  {
  case SET_GLOBAL_CTRL:
    //smclk_timer_isr(FALSE,0,0);	todo
	if (RAIL_TimerIsRunning()) {
		RAIL_TimerCancel();
	}
    rf->event = RF_EVENT_GLOBLE_CMD;
    break;
  case SET_GET_FRAME1_CTRL:
    rf->event = RF_EVENT_GET_FRAME1;
    break;
  case SET_PKG_TRN_CTRL:
    rf->event = RF_EVENT_PKG_TRN;
    break;
  case SET_PKG_CH_CTRL:
    rf->event = RF_EVENT_SET_PKG_CH;
    break;
  default:
//    smclk_timer_isr(FALSE,0,0); todo
	if (RAIL_TimerIsRunning()) {
		RAIL_TimerCancel();
	}
    rf->event = RF_EVENT_SET_ERR;
    break;
  }
}
//-----------------------------------globle_cmd_fun-----------------------------

void globle_cmd_hbrqus_fun(UINT8 tp)
{
  
#define GBL_HBRQUS_FLAG 0x80
  if((tp &0xe0) != GBL_HBRQUS_FLAG)
    return ;
  
  if((hbr_mode_bef != REQUST_HEARTBEAT) && (hbr_mode != REQUST_HEARTBEAT))//防止重复数据包，导致心跳模式错误
    hbr_mode_bef = hbr_mode;
  
  hbr_mode = REQUST_HEARTBEAT;
  rqst_hbr_info.cont = 0;
  rqst_hbr_info.now = 0;
  rqst_hbr_info.sum = ((tp&0x03)+1) * 3;
  rqst_hbr_info.hbt_cont= ((( (tp>>2)&0x07 )+1)*2 )/ aclk_timer_choose_cycle_fun(def_sys_attr.aclk_timer_cycle);
}
void globle_cmd_hbrconf_fun(UINT8 tp)
{
#define GBL_HBRCONF_FLAG 0x20
  if((tp &0xE0) != GBL_HBRCONF_FLAG)
    return ;
  if((tp&0x1f) == 0x1f)
  {
    gEslHearcont = 0;
    hbr_mode = STOP_HEARTBEAT;
    return;
  }
  hbr_mode_bef = NORMAL_HEARTBEAT;
  hbr_mode = NORMAL_HEARTBEAT;
  def_sys_attr.sys_def_attribute.heartbit_time = (UINT16)( ( (tp & 0x1f) + 1) * 30);
  //保存数据到基础参数区
  save_sys_config_info();
  gEslHearcont = 0;
  
}
void globle_cmd_setconf_fun(UINT8 tp)
{
#define GBL_SETCONF_FLAG 0x40
  if((tp &0xE0) != GBL_SETCONF_FLAG)
    return ;
  def_sys_attr.sys_def_attribute.set_wkup_time = (UINT16) (((tp&0x1f) + 1) * 2);
  save_sys_config_info(); 
  gEventFlag |= EVENT_FLAG_TIMER_CALIBRATION;
}
void globle_cmd_ledconf_fun(UINT8 tp)
{
  
#define GLO_LED_ON  0x72
#define GLO_LED_OFF 0x70
  if((tp != GLO_LED_ON) &&(tp != GLO_LED_OFF))
    return ;
  
  set_led_timer(FALSE,0,0);
  led_clear_fun();
  led_all_off();
  
  if(tp == GLO_LED_OFF)
  {
    magent4_clear_fun(FALSE);//如果magent==2 ，干簧管下一个动作点灯
    return;
  }
  if(epd_attr_info.period == 0)
    return;
  
  if(epd_attr_info.led_num == 0)//属性包不支持跑马灯
    return;
  
  led_tp_info.horselamp = FALSE;
  led_tp_init_fun(epd_attr_info.led_num);//选择led的闪烁模式，0是遥控器包的跑马灯配置 1是属性包的闪烁模式
  led_tp_info.timer_enable = TRUE;
  magent4_clear_fun(TRUE);//如果magent==2 ，干簧管下一个动作关灯
  gEventFlag |= EVENT_FLAG_INTER_LED;  
  led_all_off();
  
}
void globle_cmd_page_change_fun(UINT8 tp)
{
  
#define GLO_PAGE_FLAG 0xc0
#define SET_PAGE_ID_T 0x07
#define SET_PAGE_TIME 0x18
#define SET_PAGE_30MIN ((UINT16)1800)
  
  UINT16 time = 0;
  
  time = SET_PAGE_30MIN;
  if((tp &0xe0) != GLO_PAGE_FLAG)
    return ;
  time = ( time <<(((tp & SET_PAGE_TIME)>>3)) );//修改时间30、60、120、240
  
  gpage.flag = FALSE;//停止之前的计数
  if(FALSE == sys_page_display_store_fun(gSys_tp.default_page_id,(tp & SET_PAGE_ID_T),gSys_tp.gpage_nowid,time ,0))//保存页显示属性
  {
    gerr_info = SYS_SAVE_ERR;
  } 
  gEventFlag |= EVENT_FALG_DISPLAY_PAGE;
}


void globle_def_cmd_fun(UINT8 tp)
{
  return;
}


RF_CMD_T globle_evert_fun(UINT8 *buf)
{
#define GRP_NETMASK_START    0x02
#define GRP_PARA_START       0x05
  
  UINT8 mask_i=0,mask_m = 0;
  mask_i =  INFO_DATA.gRFInitData.wakeup_id.id2%24/8;
  mask_m =  INFO_DATA.gRFInitData.wakeup_id.id2%24%8;
  
  if ( 0 == (buf[GRP_NETMASK_START+mask_i] & (0x01<<mask_m) ))          //grp子网码是0的表示此组价签不需要工作，直接休眠
    return RF_FSM_CMD_SLEEP;
  
  if( GBL_CMD_HB_REQ == buf[1] )
  {
    globle_cmd_hbrqus_fun(buf[GRP_PARA_START]);
  }
  else if( GBL_CMD_HB_CFG == buf[1])
  {
    globle_cmd_hbrconf_fun(buf[GRP_PARA_START]);
  }
  else if( GBL_CMD_SET_CFG == buf[1])
  {
    globle_cmd_setconf_fun(buf[GRP_PARA_START]);
  }
  else if(GBL_CMD_PAGE_CHG == buf[1])
  {
    globle_cmd_page_change_fun(buf[GRP_PARA_START]);
  }
  else if( GBL_CMD_LED_CFG == buf[1])
  {
    globle_cmd_ledconf_fun(buf[GRP_PARA_START]);
  }
  else
    return RF_FSM_CMD_SLEEP;
  
  return RF_FSM_CMD_SLEEP;
}
//--------------------------------------end-------------------------------------
UINT8  set_rf_work_time_fun(UINT8 buf)
{
  UINT8 ret = 4;
#define RF_WORK_1S      0x00
#define RF_WORK_2S      0x01
#define RF_WORK_4S      0x03
#define RF_WORK_6S      0x09
#define RF_WORK_8S      0x0a
  
  
  switch(buf&0x0f)
  {
  case RF_WORK_1S:
  case RF_WORK_2S:
    ret = 2;
    break;
  case RF_WORK_4S:
    ret = 4;
    break;
  case RF_WORK_6S:
    ret = 6;
    break;
  case RF_WORK_8S:
    ret = 8;
    break;
  default :
    ret = 4;
    
  }
  return ret;
}


RF_CMD_T set_pkg_trn_fun(UINT8 *buf)
{
#define GRP_NETMASK_START    0x02  
#define TIME_CONT_6S         (UINT16)0x258
#define TIME_CONT_5S         (UINT16)0x1F4
#define TIME_CONT_8S         (UINT16)0x320
#define TIME_CONT_2S         (UINT16)0x00c8
#define TIME_CONT_1S         (UINT16)0x0064
  
  UINT8 mask_i=0,mask_m = 0;
  UINT16 set_slort = 0;
  
  mask_i =  INFO_DATA.gRFInitData.wakeup_id.id2%24/8;
  mask_m =  INFO_DATA.gRFInitData.wakeup_id.id2%24%8;
  
//  smclk_timer_isr(FALSE,0,0);	todo
  if (RAIL_TimerIsRunning()) {
	RAIL_TimerCancel();
  }
  if ( 0 == (buf[GRP_NETMASK_START+mask_i] & (0x01<<mask_m) ))
    return RF_FSM_CMD_SLEEP;
  
  gro_wor_times = 0; //从set层到grp唤醒前，清空grp唤醒的次数
  def_sys_attr.sys_def_attribute.grp_wkup_time = set_rf_work_time_fun(buf[5]>>4);
  def_sys_attr.sys_def_attribute.work_time  = set_rf_work_time_fun(buf[5]);
  
  set_slort = (UINT16)((UINT16)(buf[0]&0x1F)*256+(UINT16)buf[1]);
  
  if(set_slort > TIME_CONT_8S)
  {
    sys_wait_time_set = (set_slort - TIME_CONT_6S) / TIME_CONT_2S;
    set_wor_flag_fun(RF_FSM_CMD_SET_EXIT_LOOP); 
    return RF_FSM_CMD_SLEEP;
  }
//  A7106_SLEEP();	//todo
  
  sys_wakeup_wait(set_slort);
  
  my_ack_buf.set_wor_time++;
  return RF_FSM_CMD_GRP_WAKEUP;
}

RF_CMD_T set_get_frame1_fun(UINT8 *buf)
{
  
#define GRP_NETMASK_START    0x02  
#define TIME_CONT_6S         (UINT16)0x258
#define TIME_CONT_8S         (UINT16)0x320
#define TIME_CONT_2S         (UINT16)0x00c8
  
  UINT8 mask_i=0,mask_m = 0;
  UINT16 set_slort = 0;
  
  mask_i =  INFO_DATA.gRFInitData.wakeup_id.id2%24/8;
  mask_m =  INFO_DATA.gRFInitData.wakeup_id.id2%24%8;
  
//  smclk_timer_isr(FALSE,0,0);	//todo
  if (RAIL_TimerIsRunning()) {
	RAIL_TimerCancel();
  }
  if ( 0 == (buf[GRP_NETMASK_START+mask_i] & (0x01<<mask_m) ))
    return RF_FSM_CMD_SLEEP;
  gro_wor_times = 0; //从set层到grp唤醒前，清空grp唤醒的次数
  set_slort = (UINT16)((UINT16)(buf[0]&0x1F)*256+(UINT16)buf[1]);
  
  if(set_slort > TIME_CONT_8S)
  {
    sys_wait_time_set = (set_slort - TIME_CONT_6S) / TIME_CONT_2S;
    set_wor_flag_fun(RF_FSM_CMD_SET_EXIT_LOOP); 
    return RF_FSM_CMD_SLEEP;
  }
  
//  A7106_SLEEP();	todo
  sys_wakeup_wait(set_slort);
//  A7106_SleepToStby();
  def_sys_attr.sys_def_attribute.grp_wkup_time = set_rf_work_time_fun(buf[5]>>4);
  def_sys_attr.sys_def_attribute.work_time  = set_rf_work_time_fun(buf[5]);
  
//  smclk_timer_isr(TRUE, 200,SMCLK_8M_DIV8_5MS);
  RAIL_TimerSet(WKUP1_REC_TIME, RAIL_TIME_DELAY);
  set_wor_flag_fun(RF_FSM_CMD_GRP_EXIT_LOOP);
  my_ack_buf.set_wor_firme1_time++;
  return RF_FSM_CONFG_FRAME1_PER;
}


RF_CMD_T set_pkg_chn_fun(UINT8 *buf)
{
  
  RFID tp;
  UINT16 crc =0;
  
  tp =INFO_DATA.gRFInitData.wakeup_id;
  tp.id2 = 0x00;
  crc= my_cal_crc16(crc,buf,4);
  crc= my_cal_crc16(crc,(UINT8 *)&tp,sizeof(RFID));
  
  if (0 != memcmp(buf+4,(UINT8 *)&crc,sizeof(crc)))
    return RF_FSM_CMD_SET_RX;
  
  INFO_DATA.gRFInitData.set_wkup_ch = buf[1];
  INFO_DATA.gRFInitData.grp_wkup_ch = buf[2];
  INFO_DATA.gRFInitData.esl_data_ch = buf[3];
  
  save_extern_rf_info();
//  smclk_timer_isr(FALSE,0,0);
  if (RAIL_TimerIsRunning()) {
	RAIL_TimerCancel();
  }
  return RF_FSM_CMD_SLEEP;
}

RF_CMD_T set_ctrl_err_fun(UINT8 *buf)
{
  
  return RF_FSM_CMD_SLEEP;
}


//-------------------------group层函数-------------------------
RF_ERROR_T rf_grp_wor(RF_T *rf) 
{  
/*	//todo
  A7106_SleepToStby(); 
  A7106_SetBPS(RX_BPS);
  A7106_SetChannel(INFO_DATA.gRFInitData.grp_wkup_ch); //工作频率
  A7106_WriteID(INFO_DATA.gRFInitData.wakeup_id); //写入唤醒ID
  A7106_clr_rx_fifo(RF_GRP_WAKEUP_PACKET);
*/
    RAIL_RfIdle();
    myChangeRadioConfig(INFO_DATA.gRFInitData.grp_wkup_ch, RX_BPS, (UINT8*)&INFO_DATA.gRFInitData.wakeup_id, RF_GRP_WAKEUP_PACKET);
    return RF_ERROR_NONE;
}

//只需要接收数据，直接调用底层rf函数即可
RF_ERROR_T rf_cmd_grp_rx(RF_T *rf, uint8_t *buf)
{
	//todo
/*
  A7106_clr_rx_fifo(RF_GRP_WAKEUP_PACKET);
  
  A7106_StartRX();
  
  if(!check_gpio1_finish())
    return RF_ERROR_RX_TIMEOUT;//定时器超时  
  // Delay_us(400);
  // Delay_us(200);
  if(!A7106_CheckCRC())
    return RF_ERROR_RF_CRC;//crc错误
  
  A7106_ReadFIFO(buf, RF_GRP_WAKEUP_PACKET);
  return RF_ERROR_NONE;//正确
*/
//	BSP_LedToggle(0);
	memset(receiveBuffer , 0, RF_GRP_WAKEUP_PACKET);
	memset((uint8_t*)&appendedInfo , 0, sizeof(RAIL_AppendedInfo_t));
	received_data = FALSE;
	RAIL_RxStart(0);
//	BSP_LedToggle(0);
	if(!check_gpio1_finish())
	{
		return RF_ERROR_RX_TIMEOUT;//定时器超时
	}
	if (FALSE == appendedInfo.crcStatus){
		return RF_ERROR_RF_CRC;//crc错误
	}
	memcpy(buf, receiveBuffer, RF_SET_WAKEUP_PACKET);
	return RF_ERROR_NONE;//正确
}
RF_CMD_T grp_rx_err(RF_ERROR_T err)
{
  if(err == RF_ERROR_ENTER_TXRX)
  {
//    smclk_timer_isr(FALSE,0,0);
    if (RAIL_TimerIsRunning()) {
	  RAIL_TimerCancel();
    }
    return RF_FSM_CMD_RF_POR_RST;
  }
  else if( err == RF_ERROR_RX_TIMEOUT )
  {
    //smclk_timer_isr(FALSE,0,0);
    //A7106_Cmd(CMD_STBY);
    if (RAIL_TimerIsRunning()) {
    	RAIL_TimerCancel();
    }
    RAIL_RfIdle();
    return ((gro_wor_times > def_sys_attr.exit_grp_wkup_cont) ? (RF_FSM_CMD_GRP_WAKEUP): (RF_FSM_CMD_SLEEP));
  }
  else if(err == RF_ERROR_RF_CRC )
    return RF_FSM_CMD_GRP_RX;
  else
  {
    return RF_FSM_CMD_GRP_RX;
  }
  
}
RF_CMD_T grp_wkup_event(RF_T *rf, UINT8 *buf)
{
  UINT16 wakeup1_sort = 0;
  //todo
 /*
  if((buf[0] & GRP_WKUP_CTRL_INFO) != GRP_WKUP_READY)
    return RF_FSM_CMD_GRP_RX;
  wakeup1_sort = (UINT16)((UINT16)(buf[0]&0x1F)*256+(UINT16)buf[1]);
  smclk_timer_isr(FALSE, 0,0);
  A7106_SLEEP();
  sys_wakeup_wait(wakeup1_sort);
  A7106_SleepToStby();
  //接收帧1定时器超时时间是1s
  smclk_timer_isr(TRUE, 200,SMCLK_8M_DIV8_5MS);
  my_ack_buf.grp_wor_time++;
*/
	if((buf[0] & GRP_WKUP_CTRL_INFO) != GRP_WKUP_READY)
	return RF_FSM_CMD_GRP_RX;
	wakeup1_sort = (UINT16)((UINT16)(buf[0]&0x1F)*256+(UINT16)buf[1]);
	if (RAIL_TimerIsRunning()) {
		RAIL_TimerCancel();
	}
	sys_wakeup_wait(wakeup1_sort);
	received_data = false;
	received_tmout = false;
	RAIL_TimerSet(WKUP1_REC_TIME, RAIL_TIME_DELAY);
	my_ack_buf.grp_wor_time++;
	return RF_FSM_CONFG_FRAME1_PER;
}

//-------------------帧1层函数--------------------------------


RF_ERROR_T rf_frame1_config(RF_T *rf) 
{
	//todo
/*
  A7106_SetBPS(RX_BPS);
  A7106_SetChannel(INFO_DATA.gRFInitData.grp_wkup_ch); //工作频率
  A7106_WriteID(INFO_DATA.gRFInitData.wakeup_id); //写入唤醒ID
  A7106_clr_rx_fifo(RF_NORMAL_PACKET);
*/
  myChangeRadioConfig(INFO_DATA.gRFInitData.grp_wkup_ch, RX_BPS, (uint8_t*)&INFO_DATA.gRFInitData.wakeup_id, RF_NORMAL_PACKET);
//  memset(receiveBuffer , 0, RF_NORMAL_PACKET);
//  memset((uint8_t*)&appendedInfo , 0, sizeof(RAIL_AppendedInfo_t));
  return RF_ERROR_NONE;
}

RF_ERROR_T rf_cmd_rx_fram1(RF_T *rf, uint8_t *buf)
{
	//todo
/*
  A7106_clr_rx_fifo(RF_NORMAL_PACKET);
  
  A7106_StartRX();
  
  if(!check_gpio1_finish())
    return RF_ERROR_RX_TIMEOUT;//定时器超时  
  // Delay_us(400);
  //Delay_us(200);
  if(!A7106_CheckCRC())
    return RF_ERROR_RF_CRC;//crc错误
  
  A7106_ReadFIFO(buf, RF_NORMAL_PACKET);
*/
	memset(receiveBuffer , 0, RF_NORMAL_PACKET);
	memset((uint8_t*)&appendedInfo , 0, sizeof(RAIL_AppendedInfo_t));
	received_data = FALSE;
	RAIL_RxStart(0);
//todo 20170503

	if(!check_gpio1_finish())
	{
		return RF_ERROR_RX_TIMEOUT;//定时器超时
	}
	if (FALSE == appendedInfo.crcStatus){
		return RF_ERROR_RF_CRC;//crc错误
	}
	memcpy(buf, receiveBuffer, RF_NORMAL_PACKET);
	return RF_ERROR_NONE;//正确
}


RF_CMD_T frame1_rx_err(RF_ERROR_T err)
{
  if(err == RF_ERROR_ENTER_TXRX)
  {
    //smclk_timer_isr(FALSE,0,0);//todo
	if (RAIL_TimerIsRunning()) {
		RAIL_TimerCancel();
	}
    return RF_FSM_CMD_RF_POR_RST;
  }
  else if( err == RF_ERROR_RX_TIMEOUT )
  { 
    //smclk_timer_isr(FALSE,0,0);
    //A7106_Cmd(CMD_STBY); //todo
	if (RAIL_TimerIsRunning()) {
		RAIL_TimerCancel();
	}
    RAIL_RfIdle();
    return rx_frame1_tout_fun();
  }
  else if(err == RF_ERROR_RF_CRC )
    return RF_FSM_CMD_RX_FRAME1;
  else
    return RF_FSM_CMD_RX_FRAME1;
  
}

RF_CMD_T rx_frame1_tout_fun( void)
{
  
  if(def_sys_attr.gwor_flag_before == RF_FSM_CMD_GRP_EXIT_LOOP)
  { 
    return RF_FSM_CMD_GRP_EXIT_LOOP; 
  }
  else
    return RF_FSM_CMD_SET_EXIT_LOOP;
}

RF_CMD_T rx_frame1_mask_exit_fun( void)
{
  rc_wor_times= 0;
  if(def_sys_attr.gwor_flag_now == RF_FSM_CMD_GRP_EXIT_LOOP)
  {
    return RF_FSM_CMD_GRP_EXIT_LOOP;
    
  }
  else
    return RF_FSM_CMD_SET_EXIT_LOOP;
}

UINT16 set_rf_work_time_cont_fun(void)
{
  return 200 * def_sys_attr.sys_def_attribute.work_time;
}

RF_CMD_T rx_frame1_event(RF_T *rf, UINT8 *buf)
{
  UINT8 mask_i=0,mask_m = 0;
  UINT16 time_cont = 0;
  
  if((buf[0] & GRP_WKUP_CTRL_INFO) != FRAME1_CTRL_INFO)
    return RF_FSM_CMD_RX_FRAME1;
  
  if(TRUE != rf_pgk_crc(buf,RF_NORMAL_PACKET,INFO_DATA.gRFInitData.wakeup_id))
    return RF_FSM_CMD_RX_FRAME1;
  
//  smclk_timer_isr(FALSE,0,0);  todo
  if (RAIL_TimerIsRunning()) {
	RAIL_TimerCancel();
  }
  mask_i =  INFO_DATA.gRFInitData.esl_netmask % 160 / 8;
  mask_m =  INFO_DATA.gRFInitData.esl_netmask % 160 % 8;
  if (buf[2+mask_i] & (0x01<<mask_m) )
  {
    fram1_data_channel = buf[1];
    /*
#ifdef RED_SYSRET_LOAD
    //当正确收到帧一时，红色屏幕24小时刷新屏幕计时清0
    gred_replay = 0;
#endif
    */
    //当收到正确的帧1时，且下一步的动作是接收正常的osd数据包，则3分钟的心跳，定时器清0
    //gEslHearcont = 0;
    
    time_cont = set_rf_work_time_cont_fun();
    //smclk_timer_isr(TRUE, time_cont,SMCLK_8M_DIV8_5MS);
	received_tmout = FALSE;
    RAIL_TimerSet(time_cont*5000, RAIL_TIME_DELAY);
    return RF_FSM_CMD_RX_DATA_CONFIG;
  }
  
  return rx_frame1_mask_exit_fun();
}
RF_CMD_T rc_wkup_event(RF_T *rf, UINT8 *buf)
{
  UINT16 wakeup1_sort = 0;
  
  if((buf[0] & GRP_WKUP_CTRL_INFO) != GRP_WKUP_READY)
    return RF_FSM_CMD_RC_RX;
  wakeup1_sort = (UINT16)((UINT16)(buf[0]&0x1F)*256+(UINT16)buf[1]);
//  smclk_timer_isr(FALSE, 0,0);	//todo
  if (RAIL_TimerIsRunning()) {
	RAIL_TimerCancel();
  }
//  A7106_SLEEP();
  sys_wakeup_wait(wakeup1_sort);
//  A7106_SleepToStby();
  //接收帧1定时器超时时间是1s
//  smclk_timer_isr(TRUE, 200,SMCLK_8M_DIV8_5MS);
  received_data = FALSE;
  received_tmout = FALSE;
  RAIL_TimerSet(WKUP1_REC_TIME, RAIL_TIME_DELAY);
  return RF_FSM_CONFG_FRAME1_PER;
}
//----------------------------------遥控器层函数--------------------


RF_ERROR_T rf_rc_grp_wor(RF_T *rf) 
{  
/* todo
  A7106_SleepToStby(); 
  A7106_SetBPS(RX_BPS);
  A7106_SetChannel(INFO_DATA.gRFInitData.esl_data_ch); //工作频率
  A7106_WriteID(INFO_DATA.gRFInitData.esl_id); //写入唤醒ID
  A7106_clr_rx_fifo(RF_GRP_WAKEUP_PACKET);
  return RF_ERROR_NONE;
*/
	RAIL_RfIdle();
	myChangeRadioConfig(INFO_DATA.gRFInitData.esl_data_ch, RX_BPS, (UINT8*)&INFO_DATA.gRFInitData.esl_id, RF_GRP_WAKEUP_PACKET);
	return RF_ERROR_NONE;
}
RF_ERROR_T rf_rc_rx(RF_T *rf, uint8_t *buf)
{
/* todo
  A7106_SetBPS(RX_BPS);
  A7106_SetChannel(INFO_DATA.gRFInitData.grp_wkup_ch); //工作频率
  A7106_WriteID(INFO_DATA.gRFInitData.wakeup_id); //写入唤醒ID
  A7106_SetFIFOLen(RF_GRP_WAKEUP_PACKET);
  
  A7106_StartRX();
  
  if(!check_gpio1_finish())
    return RF_ERROR_RX_TIMEOUT;//定时器超时  
  // Delay_us(400);
  //Delay_us(200);
  if(!A7106_CheckCRC())
    return RF_ERROR_RF_CRC;//crc错误
  
  A7106_ReadFIFO(buf, RF_GRP_WAKEUP_PACKET);
  return RF_ERROR_NONE;//正确
*/
	RAIL_RfIdle();
	myChangeRadioConfig(INFO_DATA.gRFInitData.grp_wkup_ch, RX_BPS, (UINT8*)&INFO_DATA.gRFInitData.wakeup_id, RF_GRP_WAKEUP_PACKET);
    memset(receiveBuffer, 0, RF_NORMAL_PACKET);
	memset((uint8_t*)&appendedInfo , 0, sizeof(RAIL_AppendedInfo_t));
	received_data = FALSE;
	RAIL_RxStart(0);
	if(!check_gpio1_finish())
	{
		return RF_ERROR_RX_TIMEOUT;//定时器超时
	}
	if (FALSE == appendedInfo.crcStatus){
		return RF_ERROR_RF_CRC;//crc错误
	}
	memcpy(buf, receiveBuffer, RF_SET_WAKEUP_PACKET);
	return RF_ERROR_NONE;//正确
}
RF_CMD_T rc_wor_exit_fun( void)
{
  if(def_sys_attr.gwor_flag_before == RF_FSM_CMD_GRP_EXIT_LOOP)
  {
    set_wor_flag_fun(RF_FSM_CMD_GRP_EXIT_LOOP);
    return RF_FSM_CMD_GRP_WAKEUP; 
  } 
  else
  {
    set_wor_flag_fun(RF_FSM_CMD_SET_EXIT_LOOP);
    return RF_FSM_CMD_SET_WAKEUP;
  }
}

RF_CMD_T get_rc_err(RF_ERROR_T err)
{
  if(err == RF_ERROR_ENTER_TXRX)
  {
    rc_wor_times= 0;
    //smclk_timer_isr(FALSE,0,0);  todo
    if (RAIL_TimerIsRunning()) {
      RAIL_TimerCancel();
    }
    return RF_FSM_CMD_RF_POR_RST;
  }
  else if( err == RF_ERROR_RX_TIMEOUT )
  {
    //smclk_timer_isr(FALSE,0,0);
    //A7106_Cmd(CMD_STBY);
    if (RAIL_TimerIsRunning()) {
  	  RAIL_TimerCancel();
    }
    RAIL_RfIdle();
    return ((rc_wor_times > RC_WOR_CONT)?RF_FSM_CMD_RC_WAKEUP:RF_FSM_CMD_SLEEP);
  }
  else if(err == RF_ERROR_RF_CRC )
    return RF_FSM_CMD_RC_RX;
  else
    return RF_FSM_CMD_RC_RX;
  
}
//---------------------------------正常数据收发----------------------------------------



RF_ERROR_T get_data_rf_config(RF_T *rf) 
{
/* todo
  A7106_SetBPS(RX_BPS);
  A7106_SetChannel(fram1_data_channel); //工作频率
  A7106_WriteID(INFO_DATA.gRFInitData.esl_id); //写入唤醒ID
  return RF_ERROR_NONE;
*/
	RAIL_RfIdle();
	myChangeRadioConfig(fram1_data_channel, RX_BPS, (UINT8*)&INFO_DATA.gRFInitData.esl_id,   RF_NORMAL_PACKET);
	UDELAY_Delay(500);
	return RF_ERROR_NONE;
}

RF_ERROR_T rf_cmd_rx_data(RF_T *rf, uint8_t *buf)
{
  /* todo
  A7106_clr_rx_fifo(RF_NORMAL_PACKET);
  
  A7106_StartRX();
  
  if(!check_gpio1_finish())
    return RF_ERROR_RX_TIMEOUT;//定时器超时  
  // Delay_us(400);
  //Delay_us(200);
  if(!A7106_CheckCRC())
    return RF_ERROR_RF_CRC;//crc错误
  
  A7106_ReadFIFO(buf, RF_NORMAL_PACKET);
  return RF_ERROR_NONE;//正确
  */
	memset(receiveBuffer , 0, RF_NORMAL_PACKET);
	memset((uint8_t*)&appendedInfo , 0, sizeof(RAIL_AppendedInfo_t));
	received_data = FALSE;
	RAIL_RxStart(0);
	if(!check_gpio1_finish())
	{
		return RF_ERROR_RX_TIMEOUT;//定时器超时
	}
	if (FALSE == appendedInfo.crcStatus){
		return RF_ERROR_RF_CRC;//crc错误
	}
	memcpy(buf, receiveBuffer, RF_NORMAL_PACKET);
	return RF_ERROR_NONE;//正确
}

RF_CMD_T data_timeout_exit_fun( void)
{
  if(def_sys_attr.gwor_flag_now == RF_FSM_CMD_SET_EXIT_LOOP)
    return RF_FSM_CMD_SET_EXIT_LOOP;
  else  if(def_sys_attr.gwor_flag_now == RF_FSM_CMD_GRP_EXIT_LOOP)
    return RF_FSM_CMD_GRP_EXIT_LOOP;
  else  if(def_sys_attr.gwor_flag_now == RF_FSM_CMD_RC_EXIT_LOOP)
    return RF_FSM_CMD_RC_EXIT_LOOP;
  else
    return RF_FSM_CMD_SET_EXIT_LOOP;
}


RF_CMD_T rx_data_err(RF_ERROR_T err)
{
  if(err == RF_ERROR_ENTER_TXRX)
  {
    //smclk_timer_isr(FALSE,0,0);
	if (RAIL_TimerIsRunning()) {
		  RAIL_TimerCancel();
	}
    return RF_FSM_CMD_RF_POR_RST;
  }
  else if( err == RF_ERROR_RX_TIMEOUT )
  {
    //smclk_timer_isr(FALSE,0,0);
    //A7106_Cmd(CMD_STBY);
	if (RAIL_TimerIsRunning()) {
		RAIL_TimerCancel();
	}
    RAIL_RfIdle();
    return rx_tx_timeout_fun();
    // return data_timeout_exit_fun();
  }
  else if(err == RF_ERROR_RF_CRC )
    return RF_FSM_CMD_RX_DATA;
  else
    return RF_FSM_CMD_RX_DATA;
  
}
//-----------------------------------osd 函数-----------------------------------
#define OSD_PKG_START_ADDR      0x01
/*
RF_CMD_T sid_cmpare(UINT8 tp)
{
if(tp == gsid.now_sid )
return  RF_FSM_EVENT_TX_ACK_HANDLE;

if(gflash_data_error != 0)
return  RF_FSM_EVENT_TX_ACK_HANDLE;

return  RF_FSM_CMD_RX_DATA;
}
*/
BOOL sid_cmpare(UINT8 tp)
{
  if(tp == gsid.now_sid )
    return  TRUE;
  
  if(gerr_info != 0)
    return  TRUE;
  
  return  FALSE;
}

BOOL check_128_fun(UINT8 *buf,UINT8 offset)
{
  UINT8 tp[10];
  const UINT8 ml_0x80[] = {0x01,0xc0,0x00,0x80};
  memcpy(tp,buf+offset,4);
  tp[2] = 0x00;
  if(0 == memcmp(tp,ml_0x80,4))
    return TRUE;
  return FALSE;
  
}

BOOL check_0x76_fun(UINT8 *buf,UINT8 offset)
{
  UINT8 tp[10];
  const UINT8 ml_0x76[] = {0x01,0xc0,0x00,0x76};
  
  //查询命令
  memcpy(tp,buf+offset,4);
  tp[2] = 0x00;
  if(0 == memcmp(tp,ml_0x76,4))
    return TRUE;
  return FALSE;
}

RF_EVENT_T osd_mode_fun(UINT8 * temp)
{
  gsid.now_sid = temp[0] & OSD_SID_FALG ;
  
  if(TRUE == check_128_fun(temp,OSD_PKG_START_ADDR))
  {
    return RF_EVENT_OSD_128CMD;
  }
  
  if(TRUE == check_0x76_fun(temp,OSD_PKG_START_ADDR) )
  {
    return RF_EVENT_OSD_76CMD;
  }
  return RF_EVENT_OSD;
}

void rx_data_event(RF_T *rf, UINT8 *buf)
{ 
  
  switch(buf[0]&(UINT8)SET_CTRL_INFO)
  {
  case CTRL_QUERY:  
    rf->event = RF_EVENT_QUERY;
    break;
  case CTRL_NETLINK:
    rf->event = RF_EVENT_NETLINK;
    break;
  case CTRL_OSD:
    rf->event = osd_mode_fun(buf);
    break;
  case CTRL_LINK_QUERY:
    rf->event = RF_EVENT_LINK_QUERY;
    break;
  case CTRL_RC_PKG:
    rf->event = RF_EVENT_RC_INFO;
    break;
  case CTRL_SLEEP:
    //缺少一个属性包和led灯的拣货包
  default:
    rf->event = RF_EVENT_SLEEP;
    break;
  }
}
BOOL crc_normal_pkg_fun(UINT8 *buf)
{
  if(TRUE != rf_pgk_crc(buf,RF_NORMAL_PACKET,INFO_DATA.gRFInitData.esl_id))
    return FALSE;
  return TRUE;
//	return rf_pgk_crc(buf,RF_NORMAL_PACKET,INFO_DATA.gRFInitData.esl_id);
}
RF_CMD_T get_data_query_fun(UINT8 *buf)
{
  gpkg.pkg_num = 1;
  ret_ack_flag = RF_EVENT_QUERY;  
  return RF_FSM_CMD_RX_DATA;
}
RF_CMD_T get_netlink_fun(UINT8 *buf)
{
  ret_ack_flag = RF_EVENT_NETLINK;
  gpkg.pkg_num = 1;  
  memset((UINT8 *)&netlink_info,0x00,sizeof(netlink_info));
  memcpy((UINT8 *)&netlink_info,buf,sizeof(netlink_info));
  gEventFlag |= EVENT_FLAG_NETLINK; 
  
  return RF_FSM_CMD_RX_DATA;
}


file_id_t swp_pkg(file_id_t src)
{
  if(src == F_BMP_PKG_1)
    return F_BMP_PKG_2;
  if(src == F_BMP_PKG_2)
    return F_BMP_PKG_1;
  return F_BMP_PKG_1;
  
}

void sid_change_eraflag(UINT8 ctrl)
{
  write_pkg_buff_id  = swp_pkg(write_pkg_buff_id);
  if(f_len(write_pkg_buff_id) != 0 )
      f_erase(write_pkg_buff_id);
  
  gsid.old_sid = gsid.now_sid;  
  gFlag_bit =1;
  clear_gpkg_fun();//不能删除，原因是osd更新时，sid不同时表示一个新的周期
  
  //all_osd_crc = 0;
  cmd_start_offset = find_id_mode0(F_BMP_DATA);
  cmd_before_offset = cmd_start_offset; 
  first_lose_pkg = 0;  
}

enum {END_PACKET = 0x03, NORMAL_PACKET = 0x00};

BOOL check_pkg_validity(UINT8 pkg)
{
  if(pkg != END_PACKET && pkg != NORMAL_PACKET )
  {
    return FALSE;
  }
  return TRUE;
}

UINT8 packet_bitmap(UINT16 packet_num,file_id_t file_id)
{
  UINT16 packet_byte = 0,offset = 0;
  UINT8 packet_bit = 0,des = 0xff,tp=0;
  
  
  packet_num -=1;
  packet_byte = packet_num / 8;
  packet_bit  = (UINT8)(packet_num % 8);
  offset += packet_byte;
  
  f_read(file_id,(WORD)offset,&des,sizeof(des));
  if( (des & (0x01<<packet_bit)) == 0)
    return 0;
  tp = (0x01<<packet_bit);
  tp = ~tp;
  des &= tp;
  f_write_direct(file_id,offset,&des,sizeof(des));
  
  return 1;
  
  
}

RF_CMD_T get_osd_updata_fun(UINT8 *buf)
{
  esl_frame_t osd_data;
  UINT8 pkg_tpye = 0;
  
  
  memcpy((UINT8 *) &osd_data,buf,RF_NORMAL_PACKET);
  
  if( 0 == (buf[0] & OSD_SID_FALG ))	//osd更新sid不能是0	                                    
    return RF_FSM_CMD_RX_DATA;
  
  if( gsid.old_sid != gsid.now_sid)
  {
    sid_change_eraflag(buf[0]);
    // memset(&my_ack_buf.updae_ack_time,0x00,sizeof(my_ack_buf)-7);
    my_ack_buf.updae_ack_time = 0;
    my_ack_buf.query_ack_time = 0;
    my_ack_buf.receive_pkg_sum = 0;
    my_ack_buf.finish_pkg_num = 0;
    my_ack_buf.query_pkg_num = 0; 
    uprom_tp.sys_update_success_flag = 0;
  }
  
  my_ack_buf.sid1 =  write_pkg_buff_id;
  
  pkg_tpye = (osd_data.package_num & RF_PKG_TYPE)>>14;
  
  if(TRUE != check_pkg_validity(pkg_tpye))
    return RF_FSM_CMD_RX_DATA;
  
  osd_data.package_num = (osd_data.package_num &0x3fff);  
  
  switch (pkg_tpye) 
  {
  case END_PACKET:  
    gpkg.osd_pkg_all = osd_data.package_num;
  case NORMAL_PACKET:  
    
    read_pkg_fileid = write_pkg_buff_id;
    if( packet_bitmap(osd_data.package_num, write_pkg_buff_id))
    { 
      gpkg.pkg_num +=1;
    }
    
    my_ack_buf.receive_pkg_sum = gpkg.pkg_num;
    my_ack_buf.finish_pkg_num = gpkg.osd_pkg_all;
    
    if(DATA_SIZE != f_write_direct(write_temp_buff_id,(WORD)((WORD)(osd_data.package_num -1)* (WORD)DATA_SIZE + cmd_start_offset),osd_data.data,DATA_SIZE))
    {
      gerr_info = TR3_OSD_WPKG_BIT_ERR;
      break;
    } 
    ret_ack_flag = RF_EVENT_OSD;
    gEventFlag |= EVENT_FLAG_OSD_ANALUSIS;  
    break;
  }
  
  
  return RF_FSM_CMD_RX_DATA;
}

RF_CMD_T get_osd_128cmd_fun(UINT8 *buf)
{
  UINT16 crc_temp = 0;
  ret_ack_flag = RF_EVENT_OSD_128CMD;
  // gpkg.pkg_sum =(UINT16)buf[OSD_PKG_START_ADDR] + (UINT16)(buf[OSD_PKG_START_ADDR+1]*256);
  gpkg.pkg_num  = 1;
  gEventFlag |= EVENT_FLAG_ERASER_BUFF;
  
  eraser_file_flag = buf[5];//128命令后面的字节：1表示全擦，2：擦除存储区 4：擦除页数字区
  
  crc_temp = my_ack_buf.flash_crc;
  memset((UINT8 *)&my_ack_buf,0x00,sizeof(my_ack_buf));
  my_ack_buf.flash_crc = crc_temp;
  return RF_FSM_CMD_RX_DATA;
}

RF_CMD_T get_osd_76cmd_fun(UINT8 *buf)
{
  
  ret_ack_flag = RF_EVENT_OSD_76CMD;
  gpkg.pkg_num  = 1;
  UINT8 temp[8]={0};
//  if((g_crc.crc_cmd==0)&&(g_crc.crc_layer==0)&&(g_crc.crc_page_info==0)&&(g_crc.crc_page_num==0))
 if(0 == memcmp(&g_crc, &temp,sizeof(g_crc)))
 {
    gerr_info = TR3_OSD_QUERY_ERR;
  }
  else if(gerr_info == 0)
  {
    gEventFlag |= EVENT_FLAG_UPDATA_CHECK;
  }
  else
  {
    gEventFlag |= EVENT_FLAG_UPDATA_CHECK;  
  }
  return RF_FSM_CMD_RX_DATA;
}


/*
RF_CMD_T get_pkg_finish(UINT8 *buf)
{
gpkg.pkg_sum =(UINT16)buf[2] + (UINT16)(buf[3]*256);
if(gpkg.pkg_sum == gpkg.pkg_num)
return RF_FSM_EVENT_TX_ACK_HANDLE;
return RF_FSM_CMD_RX_DATA;
}
*/
void clear_gpkg_fun(void)
{
  memset((UINT8 *)&gpkg,0x00,sizeof(gpkg));
  ret_ack_flag = RF_EVENT_UNKNOWN;  
}
BOOL all_pkg_cmp_fun(void)
{
  if(gpkg.pkg_num != gpkg.pkg_sum)
    return FALSE;
  return TRUE;
}

RF_CMD_T get_osd_cmd_fun(UINT8 *buf)
{
  
  return RF_FSM_EVENT_TX_ACK_HANDLE;
}

BOOL check_link_query_fun(UINT8 *buf)
{
#define CHECK_CTRL_ADDR 0x04
#define CHECK_OSD_TP_ADDR 0x05
  RF_EVENT_T tp;
  
  
  switch(buf[CHECK_CTRL_ADDR]&(UINT8)SET_CTRL_INFO)
  {
  case CTRL_QUERY:  
    tp = RF_EVENT_QUERY;
    break;
  case CTRL_NETLINK:
    tp = RF_EVENT_NETLINK;
    break;
  case CTRL_OSD:
    if(TRUE == check_128_fun(buf,CHECK_OSD_TP_ADDR))
      tp = RF_EVENT_OSD_128CMD;
    else if(TRUE == check_0x76_fun(buf,CHECK_OSD_TP_ADDR) )
      tp = RF_EVENT_OSD_76CMD;
    else
      tp= RF_EVENT_OSD;
    break;
  case CTRL_LINK_QUERY:
    tp = RF_EVENT_LINK_QUERY;
    break;
  case CTRL_RC_PKG:
    tp = RF_EVENT_RC_INFO;
    break;
  case CTRL_SLEEP:
    //缺少一个属性包和led灯的拣货包
  default:
    tp =  RF_EVENT_UNKNOWN;
    return FALSE;
    break;  
  }
  if(ret_ack_flag != tp)
  {  
    return FALSE;
  }
  return TRUE;
}

RF_CMD_T get_link_query_fun(UINT8 *buf)
{
  //#define LINK_QUERY_OSD_ADDR 0x
 
  if(check_link_query_fun(buf) == FALSE)
    return RF_FSM_CMD_RX_DATA;
  
  gpkg.ctrl = buf[0];
  gpkg.sroft= buf[1]; 
  gpkg.pkg_sum =(UINT16)buf[2] + (UINT16)(buf[3]*256);
  
  switch(ret_ack_flag)
  {
  case RF_EVENT_QUERY:
  case RF_EVENT_NETLINK: 
    return RF_FSM_EVENT_TX_ACK_HANDLE;
    break;
  case RF_EVENT_OSD:
    my_ack_buf.query_pkg_num = gpkg.pkg_sum;
    return get_osd_cmd_fun(buf);
    break;
  case RF_EVENT_OSD_128CMD:
    if(TRUE != all_pkg_cmp_fun())
      return RF_FSM_CMD_RX_DATA;
    if(sid_cmpare(buf[0] & OSD_SID_FALG))
      return RF_FSM_EVENT_TX_ACK_HANDLE;
    else
      return RF_FSM_CMD_RX_DATA;
    break;
  case RF_EVENT_OSD_76CMD:
    if(TRUE != all_pkg_cmp_fun())
      return RF_FSM_CMD_RX_DATA;
    if(sid_cmpare(buf[0] & OSD_SID_FALG))
      return RF_FSM_EVENT_TX_ACK_HANDLE;
    else
      return RF_FSM_CMD_RX_DATA;
    break;
  case RF_EVENT_RC_INFO: 
    return RF_FSM_EVENT_TX_ACK_HANDLE;
    break;
  default:
    clear_gpkg_fun();
    gsid.old_sid  = 0;
    return RF_FSM_CMD_RX_DATA;
  }
  //return RF_FSM_EVENT_TX_ACK_HANDLE;
}

void led_tp_init_fun(UINT8 ledtp_flag)
{
  
  if(ledtp_flag)//属性包
  {
    led_tp.t0_cont =  epd_attr_info.t0_cont;
    led_tp.t0_time = epd_attr_info.t0_time;
    led_tp.t1_time = epd_attr_info.t1_time;
    led_tp.t2_time = epd_attr_info.t2_time;
    led_tp.period = epd_attr_info.period;
    led_tp.led_num = epd_attr_info.led_num;
  }
  else
  {
    led_tp.t0_cont =   rc_attr_info.t0_cont;
    led_tp.t0_time =   rc_attr_info.t0_time;
    led_tp.t1_time =  rc_attr_info.t1_time;
    led_tp.t2_time = rc_attr_info.t2_time;
    led_tp.period =  rc_attr_info.period;
    led_tp.led_num = rc_attr_info.led_num;
    led_tp.led_map[0] = rc_attr_info.led_map[0];
    led_tp.led_map[1] = rc_attr_info.led_map[1];
    led_tp.led_map[2] = rc_attr_info.led_map[2];
    led_tp.led_map[3] = rc_attr_info.led_map[3];
  }
  if( led_tp.t0_cont <= 1)
    led_tp.t0_cont = 3;
  if(led_tp.t0_cont >> 1 == 0)
    led_tp.t0_cont += 1;
  
  if(led_tp.t0_time ==0)
    led_tp.t0_time=1;
  if(led_tp.t1_time ==0)
    led_tp.t1_time=1;
  if(led_tp.t2_time ==0)
    led_tp.t2_time=1;
  
}



void lcd_reset_init(void)
{
  if(rc_attr_info.period)
  {
    led_tp_init_fun(0);
    rc_led_choose();
    led_tp_info.timer_enable = TRUE;
    led_tp_info.all_times  = 0;
    house_loop_times = 0;
    gEventFlag |= EVENT_FLAG_INTER_LED;
    led_all_off();
    magent4_clear_fun(TRUE);
  }
  
}
RF_CMD_T get_rc_pkg_fun(UINT8 *buf)
{
  grc_err = NONEERR_1;
  gpkg.pkg_num = 1;
  ret_ack_flag = RF_EVENT_RC_INFO; 
  memcpy((UINT8 *)&rc_attr_info,buf,sizeof(rc_attr_info));
  if((0 != memcmp(rc_attr_info.secur_code, epd_attr_info.secur_code,2)))
  {
    grc_err = RC_CODE_ERR;
    goto loop;
  }
  
/*
  if((rc_attr_info.t0_time ==0)&&(rc_attr_info.t1_time ==0)&&(rc_attr_info.t2_time ==0)&&(rc_attr_info.period ==0) ||(rc_attr_info.period>0))
  {
    led_tp_init_fun(0);
    rc_led_choose();
    led_tp_info.timer_enable = TRUE;
    led_tp_info.all_times  = 0;
    house_loop_times = 0;
    gEventFlag |= EVENT_FLAG_INTER_LED;
    led_all_off();
    magent4_clear_fun(TRUE);
  }
  */
  rc_led_init();
  rc_store_inio_flag =  TRUE;
  
  if(rc_attr_info.page_num != gSys_tp.page_map[rc_attr_info.page_num])
  {
    grc_err = PAGE_ID_ERR2;
    goto loop;
  }
  if(rc_attr_info.display_time)
  {
    gpage.flag = FALSE;//停止之前的计数  
    gEventFlag |= EVENT_FALG_DISPLAY_PAGE;
  }
  else
  {
    rc_attr_info.page_num = gSys_tp.present_page_id;
    rc_attr_info.display_time= gSys_tp.stay_time;
  }
  
loop:
  
  return RF_FSM_CMD_RX_DATA;
  
}
RF_CMD_T get_sleep_fun(UINT8 *buf)
{ 
  return tx_exit_fun();
}
//----------------------------------回ack---------------------------------------

RF_CMD_T tx_exit_fun( void)
{
  rc_wor_times= 0;
  if(def_sys_attr.gwor_flag_before == RF_FSM_CMD_GRP_EXIT_LOOP)
  {
    set_wor_flag_fun(RF_FSM_CMD_GRP_EXIT_LOOP);
    //aclk_wor_ready_fun();
    return RF_FSM_CMD_GRP_EXIT_LOOP;
    
  } 
  else  
  {
    def_sys_attr.gwor_flag_now =RF_FSM_CMD_SET_EXIT_LOOP; 
    //aclk_wor_ready_fun();
    return RF_FSM_CMD_SET_EXIT_LOOP;
  }
}
void query_ack_buf_fun(UINT8 *tp)
{
  UINT16 crc =0;
  
  tp[0]= gpkg.ctrl ;
  tp[1] = gpkg.sroft + 1;
  if(TRUE != all_pkg_cmp_fun())
  {
    tp[2]= 0x01;//丢一个包
    tp[4] =0x01;//包号是1
  }
  else
  {
    /*
    if(gelectric_quantity >=0x02)
      tp[4] = 0x40;
    else
      tp[4] =0x02;
    */
     tp[4] = EPD_3_VERSION;
  }
  
  crc = ack_crc_fun(tp,RF_NORMAL_PACKET);
  memcpy(tp+24,(UINT8 *)&crc,sizeof(crc));
}
void netlink_ack_buf_fun(UINT8 *tp)
{
  UINT16 crc =0;
  
  tp[0]= gpkg.ctrl ;
  tp[1] = gpkg.sroft + 1;
  if(TRUE != all_pkg_cmp_fun())
  {
    tp[2]= 0x01;//丢一个包
    tp[4] =0x01;//包号是1
  }
  else
  {
    tp[4] = 0x40;
  }
  crc = ack_crc_fun(tp,RF_NORMAL_PACKET);
  memcpy(tp+24,(UINT8 *)&crc,sizeof(crc));
}
void osd128_ack_buf_fun(UINT8 *tp)
{
  UINT16 crc =0;
  
  tp[0]= gpkg.ctrl ;
  tp[1] = gpkg.sroft + 1;
  if(TRUE != all_pkg_cmp_fun())
  {
    tp[2]= 0x01;//丢一个包
    tp[4] =0x01;//包号是1
  }
  else
  {
    my_ack_buf.sid1 &= 0x0f;
    my_ack_buf.sid1 |= 0x30;
    memcpy(tp+5,(UINT8 *)&my_ack_buf,sizeof(my_ack_buf));
    //tp[23]= 0x33;
    memcpy(tp+20,(UINT8 *)&INFO_DATA.gRFInitData.esl_id,4);
    tp[4] = 0x40;
  }
  crc = ack_crc_fun(tp,RF_NORMAL_PACKET);
  memcpy(tp+24,(UINT8 *)&crc,sizeof(crc));
}
void osd0x76_ack_buf_fun(UINT8 *tp)
{
#define ROM_UPDATE_QUERY_ACK  0x30
  UINT16 crc =0;
  
  tp[0]= gpkg.ctrl ;
  tp[1] = gpkg.sroft + 1;
  
  my_ack_buf.query_ack_time++;
  
  if(TRUE != all_pkg_cmp_fun())
  {
    tp[2]= 0x01;//丢一个包
    tp[4] =0x01;//包号是1
  }
  else if(gerr_info != 0)
  {
    tp[4] = gerr_info;
    my_ack_buf.sid1 &= 0x0f;
    my_ack_buf.sid1 |= 0x20;
    memcpy(tp+5,(UINT8 *)&my_ack_buf,sizeof(my_ack_buf));
    //tp[23]= 0x22; 
    memcpy(tp+20,(UINT8 *)&INFO_DATA.gRFInitData.esl_id,4);
  }
  else if((uprom_tp.sys_update_success_flag == 0x40) && (all_osd_crc ==0))
  {
    tp[4] = ROM_UPDATE_QUERY_ACK;
  }
  else
  {
    tp[4] = 0x40;
    my_ack_buf.sid1 &= 0x0f;
    my_ack_buf.sid1 |= 0x20;
    memcpy(tp+5,(UINT8 *)&my_ack_buf,sizeof(my_ack_buf));
    //tp[23]= 0x22; 
    memcpy(tp+20,(UINT8 *)&INFO_DATA.gRFInitData.esl_id,4);
    
  }
  crc = ack_crc_fun(tp,RF_NORMAL_PACKET);
  memcpy(tp+24,(UINT8 *)&crc,sizeof(crc));
}

UINT16 lose_pkg_statistics(UINT16 pkg_all,file_id_t readid,UINT8 *buf)
{
  UINT16 j,all_pkg_byte,lose, n;
  UINT8 temp[64],i, read_byte,k, len = 0;
  
  all_pkg_byte = ( (pkg_all/8) +1);
  for( j = ((first_lose_pkg)/8>0)?(first_lose_pkg)/8 -1:0; j < all_pkg_byte ; j+= read_byte) 
  {
    
    if(all_pkg_byte - j >= 64)
      read_byte = 64;
    else
      read_byte = all_pkg_byte - j;
    
    f_read(readid,j,temp,read_byte);
    
    for(i = 0 ; i < read_byte ; i++)
    {
      if(temp[i] == 0x00)
        continue;
      
      for(k = 0; k < 8; k++)
      {
        n = (j + i) * 8 + k;
        if(n == pkg_all)
          goto End;       
        if((temp[i] & ( 0x01<<k)) == 0)
          continue;
        if(len == 20)
          goto End;
        
        lose =  n +1;
        buf[len++] = lose & 0xff;
        buf[len++] = (lose >> 8) & 0xff;
      }
    }
    
  }
  
End:
  if (len == 0) {
    first_lose_pkg = pkg_all;
  } else {
    first_lose_pkg = buf[0] + buf[1]*256;
  }
  
  return len /2;
}

BOOL osd_ack_buf_fun(UINT8 *tp)
{
  UINT16 crc =0, num=0;
  
  tp[0]= gpkg.ctrl ;
  tp[1] = gpkg.sroft + 1;
  
  my_ack_buf.updae_ack_time++;
  
  if(gerr_info != 0)
  {
    tp[4] = gerr_info;
    
    my_ack_buf.sid1 &= 0x0f;
    my_ack_buf.sid1 |= 0x10;
    memcpy(tp+5,(UINT8 *)&my_ack_buf,sizeof(my_ack_buf));
    //tp[23] = 0x11;
    memcpy(tp+20,(UINT8 *)&INFO_DATA.gRFInitData.esl_id,4);
    goto loop;
  }
  
  num = (UINT16)lose_pkg_statistics(gpkg.pkg_sum,read_pkg_fileid,tp+4);
  
  if(num == 0)
  {
    tp[4] = 0x40;
    my_ack_buf.sid1 &= 0x0f;
    my_ack_buf.sid1 |= 0x10;
    memcpy(tp+5,(UINT8 *)&my_ack_buf,sizeof(my_ack_buf));
    memcpy(tp+20,(UINT8 *)&INFO_DATA.gRFInitData.esl_id,4);
    if(gpkg.pkg_num == 0) //不能等于结束包gpkg.osd_pkg_all,gpkg.pkg_sum，必须等于链路层查询包总数
    {
      gsid.old_sid  = 0;
      return FALSE;
    }
  }
  else
    memcpy(tp+2,(UINT8 *)&num,sizeof(num));
  
loop:
  
  crc = ack_crc_fun(tp,RF_NORMAL_PACKET);
  memcpy(tp+24,(UINT8 *)&crc,sizeof(crc));
  return TRUE;
}

void rc_ack_buf_fun(UINT8 *tp)
{
  UINT16 crc =0;
  
  tp[0]= gpkg.ctrl ;
  tp[1] = gpkg.sroft + 1;
  if(TRUE != all_pkg_cmp_fun())
  {
    tp[2]= 0x01;//丢一个包
    tp[4] =0x01;//包号是1
  }
  else
  {
    if( grc_err)
      tp[4] =grc_err;
    else
     tp[4] = 0x40;
  }
  
  crc = ack_crc_fun(tp,RF_NORMAL_PACKET);
  memcpy(tp+24,(UINT8 *)&crc,sizeof(crc));
}
RF_CMD_T tx_ack_info(UINT8 *buf)
{
  memset(buf,0x00,RF_NORMAL_PACKET);
  switch(ret_ack_flag)
  {
  case RF_EVENT_QUERY: 
    query_ack_buf_fun(buf);
    break;
  case RF_EVENT_NETLINK:   
    netlink_ack_buf_fun(buf);
    break;
  case RF_EVENT_OSD_128CMD:
    osd128_ack_buf_fun(buf);
    break;
  case RF_EVENT_OSD_76CMD:
    osd0x76_ack_buf_fun(buf);
    break;
  case RF_EVENT_OSD: 
    if(FALSE == osd_ack_buf_fun(buf))
    {
      return  RF_FSM_CMD_RX_DATA;
     //  return RF_FSM_CMD_RX_DATA_CONFIG;这个目前就是接受状态，是否不用退回到rx配置状态
    }
    break;
  case  RF_EVENT_RC_INFO:
    rc_ack_buf_fun(buf);
    break;
  default:
    return RF_FSM_CMD_RX_DATA_CONFIG;
    break;
  }
  return RF_FSM_CMD_TX_DATA_CONFIG;
  
}
RF_ERROR_T ret_ack_rf_config(RF_T *rf) 
{  
/*
  A7106_SetBPS(TX_BPS);
  A7106_SetChannel(fram1_data_channel); //工作频率
  A7106_WriteID(INFO_DATA.gRFInitData.master_id); //写入唤醒ID
  return RF_ERROR_NONE;
*/
  RAIL_RfIdle();
  myChangeRadioConfig(fram1_data_channel, TX_BPS, (UINT8*)&INFO_DATA.gRFInitData.master_id,   UNCHANGED_LEN);
  return RF_ERROR_NONE;
}

RF_ERROR_T a7106_tx_data(UINT8 len, UINT8 *buf) 
{
/* todo
  A7106_clr_tx_fifo(len);
  A7106_WriteFIFO(buf, len);
  
  A7106_StartTX();	
  
  for (i = 0; i < 25; i++) {  //最多等5ms
    if(!A7106_CheckDataing())
    {
      // Delay_us(400);  
      return RF_ERROR_NONE;//正确
    }    
    Delay_us(200);
  }
  return RF_ERROR_TX_TIMEOUT;//定时器超时
   */
	RAIL_TxData_t txInfo;
	UINT8 i=0, ret;
	RAIL_RfIdle();
	txInfo.dataPtr = buf;
	txInfo.dataLength = len;
	RAIL_TxDataLoad(&txInfo);
	while(i<25){
		ret = RAIL_TxStart(0, NULL, NULL);
		if (0 == ret){
			while(RAIL_RF_STATE_IDLE != RAIL_RfStateGet());
			return RF_ERROR_NONE;//正确
		}else {
			RAIL_RfIdleExt(RAIL_IDLE_ABORT, TRUE);
		}
		i++;
	}
	return RF_ERROR_TX_TIMEOUT;
}
RF_CMD_T rx_tx_timeout_fun( void)
{
  rc_wor_times= 0;
  if(def_sys_attr.gwor_flag_now == RF_FSM_CMD_GRP_EXIT_LOOP)
  {
    return RF_FSM_CMD_GRP_EXIT_LOOP;
    // return RF_FSM_CMD_GRP_WAKEUP;
  }
  else
  {
    //return RF_FSM_CMD_SET_WAKEUP;
    return RF_FSM_CMD_SET_EXIT_LOOP;
  }
  
}

RF_CMD_T tx_ack_err(RF_ERROR_T err)
{
  if(err == RF_ERROR_ENTER_TXRX)
  {
    //smclk_timer_isr(FALSE,0,0);
    if (RAIL_TimerIsRunning()) {
  	  RAIL_TimerCancel();
    }
    return RF_FSM_CMD_RF_POR_RST;
  }
  else 
    return rx_tx_timeout_fun();
  
}
//--------------------------------------sleep-----------------------------------
RF_ERROR_T rf_cmd_sleep(RF_T *rf)
{
  //smclk_timer_isr(FALSE, 0,0);
  // A7106_Cmd(CMD_STBY); todo
  //Delay_ms(3);
  //  A7106_SLEEP();
  if (RAIL_TimerIsRunning()) {
	RAIL_TimerCancel();
  }
  RAIL_RfIdleExt(RAIL_IDLE_FORCE_SHUTDOWN, TRUE);
  return RF_ERROR_NONE;
  
}
//------------------------------------------------------------------------------
void rf_interrupt_into_fun(void)
{
  
  if(def_sys_attr.gwor_flag_now == RF_FSM_CMD_GRP_EXIT_LOOP)
    grf_state_flag.cmd= RF_FSM_CMD_GRP_WAKEUP;
  else if(def_sys_attr.gwor_flag_now == RF_FSM_CMD_RC_EXIT_LOOP)
    grf_state_flag.cmd= RF_FSM_CMD_RC_WAKEUP;
  else
    grf_state_flag.cmd = RF_FSM_CMD_SET_WAKEUP;
  
  rf_state_machine(&grf_state_flag);
  if(rc_store_inio_flag)
  {
    rc_store_inio_flag=FALSE;
    if(FALSE == sys_page_display_store_fun( gSys_tp.default_page_id,rc_attr_info.page_num,gSys_tp.gpage_nowid ,rc_attr_info.display_time,0))//保存页显示属性 默认页、要显示的页、屏幕当前显示的页、停留时间，已经走过的时间、状态标志
    {
      gerr_info = SYS_SAVE_ERR;
    }
  }
}

UINT8 aclk_timer_choose_cycle_fun(UINT16 tp)
{
  return 2;
  /*
  if(tp == WOR_TIME_COUNT_1S)
  return 1;
  else if(tp == WOR_TIME_COUNT_2S)
  return 2;
  else if(tp == WOR_TIME_COUNT_8S)
  return 8;
  else
  return 2;      
  */
}
void aclk_wor_ready_fun(void)
{
  //gaclk_times_cont= 0;
  if(def_sys_attr.gwor_flag_now == RF_FSM_CMD_GRP_EXIT_LOOP)
  {
    
    def_sys_attr.aclk_timer_cycle = g_timer_tp.wor_2s_cont;
    def_sys_attr.aclk_tmr_wor_cont  = def_sys_attr.sys_def_attribute.grp_wkup_time  / aclk_timer_choose_cycle_fun(def_sys_attr.aclk_timer_cycle);
    def_sys_attr.hbt_cont =  def_sys_attr.sys_def_attribute.heartbit_time / aclk_timer_choose_cycle_fun(def_sys_attr.aclk_timer_cycle);
    def_sys_attr.exit_grp_wkup_cont = GROUP_MAX_TIMES ;                 //grp唤醒最大次数为3分钟
    tm_add_value = CNT_ADD_VALUE_2S;
  }
  else if(def_sys_attr.gwor_flag_now == RF_FSM_CMD_RC_EXIT_LOOP)
  {
    def_sys_attr.aclk_timer_cycle = g_timer_tp.rc_200ms_cont;
    def_sys_attr.aclk_tmr_wor_cont  = 1;
    def_sys_attr.hbt_cont =  900;
  }
  else
  {
    UINT8 set_wktime = 0;
    def_sys_attr.aclk_timer_cycle = g_timer_tp.wor_2s_cont;
    set_wktime = def_sys_attr.sys_def_attribute.set_wkup_time;
    if(sys_wait_time_set !=0 ) //set wor时间超过8秒
    {
      set_wktime = 2 * sys_wait_time_set;//2s的定时器周期，一共sys_wait_time_set次
    }
    
    def_sys_attr.aclk_tmr_wor_cont  = set_wktime  / aclk_timer_choose_cycle_fun(def_sys_attr.aclk_timer_cycle);
    def_sys_attr.hbt_cont =  def_sys_attr.sys_def_attribute.heartbit_time / aclk_timer_choose_cycle_fun(def_sys_attr.aclk_timer_cycle);
    def_sys_attr.exit_grp_wkup_cont =GROUP_MAX_TIMES;                 //grp唤醒最大次数为3分钟
    def_sys_attr.gwor_flag_now = RF_FSM_CMD_SET_EXIT_LOOP;
    /*
    *begin add by zhaoyang for bug  
    *问题描述：修改mcu唤醒周期
    *解决办法：采用主时钟校准的方式
    */  
    if ((def_sys_attr.aclk_tmr_wor_cont>=WOR_PERIOD_16S) && (hbr_mode==NORMAL_HEARTBEAT) && (gpage.flag == FALSE) && ( epd_scn_tp.screen_busy_flag == FALSE))							//wor>=16s, 正常心跳模式，睡眠时间>8s
    {		
      tm_add_value = CNT_ADD_VALUE_8S;	
      def_sys_attr.aclk_timer_cycle = g_timer_tp.wor_2s_cont<<2;
    }
    else
    {
      def_sys_attr.aclk_timer_cycle = g_timer_tp.wor_2s_cont;
      tm_add_value = CNT_ADD_VALUE_2S;	
    }
    /*
    *end
    */
  }
  
}

void set_wor_flag_fun(RF_CMD_T tp)
{
  switch (tp)
  {
  case RF_FSM_CMD_RC_EXIT_LOOP:
    def_sys_attr.gwor_flag_now = RF_FSM_CMD_RC_EXIT_LOOP;
    break;
  case RF_FSM_CMD_GRP_EXIT_LOOP:
    def_sys_attr.gwor_flag_now = RF_FSM_CMD_GRP_EXIT_LOOP;
    def_sys_attr.gwor_flag_before = RF_FSM_CMD_GRP_EXIT_LOOP; 
    break;
  default :
    def_sys_attr.gwor_flag_now = RF_FSM_CMD_SET_EXIT_LOOP;
    def_sys_attr.gwor_flag_before = RF_FSM_CMD_SET_EXIT_LOOP;
    break;
  }
}


void event_128_fun(void)
{
  switch (eraser_file_flag)
  {
  case 2:
    f_erase(F_PAGE_INFO1);
    f_erase(F_PAGE_INFO2);
  case 4:
    f_erase(F_PAGE_NUM_1);
    f_erase(F_PAGE_NUM_2);
    break;
  default:
    fs_erase_all();
    f_init();
    break;
  }
  gerr_info = NONEERR;
  clear_gpkg_fun();
  gsid.now_sid = 0;
  gsid.old_sid = 0;
}

void rf_hbr_conf_into(void)
{
#define HBR_CHANNEL 2
	/*
  A7106_SleepToStby();
  //--------把注册功率切换成-1.75db-----------
  // A7106_WriteReg(REG_TXTEST,RF_TX_L175DB);
  //------------------------------------------
  A7106_SetBPS(TX_BPS);
  A7106_WriteID(INFO_DATA.gRFInitData.master_id);
  A7106_SetChannel(HBR_CHANNEL);                //工作频率
*/
	RAIL_RfIdle();
	myChangeRadioConfig(HBR_CHANNEL, TX_BPS, (UINT8*)&INFO_DATA.gRFInitData.master_id, HBR_LEN);
}
UINT16 A7106_Tx_HBR_info(UINT8 *buf)
{

  UINT8 i = 0, ret = 0;
  RAIL_TxData_t txInfo;
  txInfo.dataPtr = buf;
  txInfo.dataLength = HBR_LEN;
  RAIL_TxDataLoad(&txInfo);
  while(i<3){
  	ret = RAIL_TxStart(0, NULL, NULL);
  	if (0 == ret){
  		while(RAIL_RF_STATE_IDLE != RAIL_RfStateGet());
  		break;
  	}
  	i++;
  }
/*
  A7106_Cmd(CMD_TX);
  Delay_ms(1);

  for (i = 0; i < 150; i++)
  {  //最多等3ms
    if(!A7106_Check_Dataing())
      goto done;
    Delay_us_20();
  }

done:
*/
  return 0;
}
void rf_hbr_conf_leave(void)
{
  //把发射功率修改成1dbm
  // A7106_WriteReg(REG_TXTEST,RF_TX_1DB);
  //A7106_SLEEP();
//  A7106_Cmd(CMD_SLEEP);
}
UINT16 A7106_Tx_Register(UINT8 ctrl,UINT8 count,UINT8 vsion_flag)
{
  UINT8 i= 0;
#pragma pack(1)
  struct esl_regist {
    UINT8 ctrl;
    UINT8 version;
    UINT8 set_wkup_id[3];
    RFID eslid;
    UINT8 set_wkup_chn;
    UINT8 grp_wkup_chn;
    UINT8 data_chn;
    UINT8 esl_netmask;
    UINT8 esl_infomation;
    UINT8 crc[2];
  }esl_regist_buf;
#pragma pack()

  UINT16 crc;

//  memset((UINT8 *)&esl_regist_buf,0x00,sizeof(esl_regist_buf));
  if(vsion_flag == TRUE)
  {
    esl_regist_buf.esl_infomation = EPD_3_VERSION;
    esl_regist_buf.ctrl = CTRL_HBR_ESL_VERSION & 0xf0;
  }
  else
  {
     esl_regist_buf.ctrl = ctrl & 0xf0;
     esl_regist_buf.esl_infomation  = hb_setwor_fun();
     esl_regist_buf.esl_infomation  &= 0xf8;

    // esl_regist_buf.esl_infomation |= (adc_voltage());
     if(def_sys_attr.gwor_flag_now == RF_FSM_CMD_SET_EXIT_LOOP)
     {
        adc_voltage();
     }
     esl_regist_buf.esl_infomation |= gelectric_quantity;
  }

  esl_regist_buf.ctrl |= (UINT8)((VERSION_OP5>>8) & 0x0f);
  esl_regist_buf.version = (UINT8)(VERSION_OP5 & 0x00ff);
  memcpy(esl_regist_buf.set_wkup_id,(UINT8 *)&INFO_DATA.gRFInitData.wakeup_id,3);
  memcpy(&esl_regist_buf.eslid,(UINT8 *)&INFO_DATA.gRFInitData.esl_id,4);
  esl_regist_buf.set_wkup_chn =  INFO_DATA.gRFInitData.set_wkup_ch;
  esl_regist_buf.grp_wkup_chn = INFO_DATA.gRFInitData.grp_wkup_ch;
  esl_regist_buf.data_chn = INFO_DATA.gRFInitData.esl_data_ch;
  esl_regist_buf.esl_netmask = INFO_DATA.gRFInitData.esl_netmask;


  crc = my_cal_crc16(0,(UINT8 *)&esl_regist_buf,14);
  crc = my_cal_crc16(crc,(UINT8 *)&INFO_DATA.gRFInitData.esl_id,4);
  esl_regist_buf.crc[0] = (crc & 0x00ff);
  esl_regist_buf.crc[1] =(crc >> 8);



  rf_hbr_conf_into();
  for(i = 0;i<count-1;i++)
  {
    A7106_Tx_HBR_info((UINT8 *)&esl_regist_buf);
    Delay_ms(1);
  }
  A7106_Tx_HBR_info((UINT8 *)&esl_regist_buf);
  rf_hbr_conf_leave();
  return 0;

}
//------------------------------------------------------------------------------
