/***************************************************************************//**
 * @file main.c
 * @brief This application allows the user to use Button 1 to switch between
 * modes (Duty Cycle, Master, Slave) and use Button 0 to transmit a packet
 * or blast packets using the RAIL library, where blast refers to a continuous
 * flow of packets. The current mode and application status is printed on the
 * LCD screen.
 * @copyright Copyright 2016 Silicon Laboratories, Inc. http://www.silabs.com
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rail.h"
#include "rail_types.h"

#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_emu.h"
#include "em_int.h"

#include "pti.h"
#include "pa.h"
#include "rtcdriver.h"
#include "bsp.h"
#include "gpiointerrupt.h"
#include "graphics.h"
#include "udelay.h"
#include "rail_config.h"
#include "hal_common.h"
#include "response_print.h"
#include "retargetserial.h"


#include "sys_init.h"
//#include "write_id.h"
#include "save_sys_status.h"
#include "led.h"
//#include "rf.h"
#include "extern_flash.h"
#include "mini_fs.h"
#include "global_variable.h"
#include "delay.h"
#include "reed.h"
#include "sys_isr.h"
#include "three_event.h"
#include "osd_flash.h"
#include "reed.h"
//#include "test_spi.h"
#include <string.h>
#include "epd_io.h"
#include "split_dis_layer.h"
#include "bit_display.h"
#include "adc10.h"
#include "main.h"


// Display buffer size
#ifndef APP_DISPLAY_BUFFER_SIZE
#define APP_DISPLAY_BUFFER_SIZE 64
#endif



// General application memory sizes
#define APP_MAX_PACKET_LENGTH (MAX_BUFFER_SIZE - 12) /* sizeof(RAIL_RxPacketInfo_t) == 12) */
#define RAIL_RF_XTAL_FREQ 38400000

// buttons
typedef struct ButtonArray
{
  GPIO_Port_TypeDef   port;
  unsigned int        pin;
} ButtonArray_t;
static const ButtonArray_t buttonArray[ BSP_NO_OF_BUTTONS ] = BSP_GPIO_BUTTONARRAY_INIT;


// Function prototypes
static void changeRadioConfig(int newConfig);
//static void generatePayload(RAIL_TxData_t *txInfo, int txId);
void gpioCallback(uint8_t pin);
void slaveTimerExpiredCallback( RTCDRV_TimerID_t id, void *incomingPacket);
uint32_t change_syncword(uint32_t souce);
void bsp_init(void);
void myRAILtest(void);
/******************************************************************************
 * App constants and variables
 *****************************************************************************/
bool forceStateChange = true;           // call state change irrespective of previous state
int channel = 0;                        // the channel used by the application
int currentConfig = 0;                  // default is first in list
bool button0Pressed = false;            // is button0 pressed?
bool button1Pressed = false;            // is button1 pressed?
uint8_t receiveBuffer[MAX_BUFFER_SIZE];     // buffer used for packet reception

// Parameters used in the initialization of rail
static const RAIL_Init_t railInitParams = {
  APP_MAX_PACKET_LENGTH,
  RAIL_RF_XTAL_FREQ,
};
/******************************************************************************
 * Duty Cycle mode constants and variables
 *****************************************************************************/

#define DUTY_CYCLE_ON_TIME          1600   // time to stay on (us)
#define DUTY_CYCLE_OFF_TIME         3400   // time to stay off (us)
#define DUTY_CYCLE_PREAMBLE_ON_TIME 10000  // time to stay on after preamble received (us)

RAIL_TxData_t dutyCycleTxData;  // application payload sent during a TX

/******************************************************************************
 * Master mode constants and variables
 *****************************************************************************/

#define MASTER_BLAST_TX_SIZE    150    // blast consecutive packets
#define MASTER_ACK_WAITING_TIME 150000 // how long to wait for Ack (us)

int masterTxBlastId = 0;            // ID for a blast
int masterTxBlastAcksReceived = 0;  // ID for ack of a blast
int masterTxBlastPacketsCount = 0;  // count of packets sent in a blast
RAIL_TxData_t masterTxData;         // application payload sent during a TX

/******************************************************************************
 * Slave mode constants and variables
 *****************************************************************************/
/*
#define SLAVE_ON_TIME         15    // time to stay on (ms)
#define SLAVE_OFF_TIME        2500  // time to stay off (ms)
#define SLAVE_BLAST_RX_DELAY  1000    // keep waiting for blast to end (ms)
#define SLEEP_TIME  1000    // keep waiting for blast to end (ms)
*/

extern void setDebugSignal(int argc, char **argv);
void set_iodebug(void)
{
	char *para1[] =  {"mytx", "PC10", "TXACTIVE"};
	char *para2[] =  {"myrx", "PC11", "RXACTIVE"};
	char *para3[] =  {"myrx", "PF3", "PTIDATA"};
	setDebugSignal(3, para1);
	setDebugSignal(3, para2);
	setDebugSignal(3, para3);
}
void myChangeRadioConfig(uint8_t channel, bps_enum bps, uint8_t* ID, uint8_t len)
{
#define BASE_FREQ			2400000000
#define CHANNEL_SPACING		500000
	const uint32_t * p;

	RAIL_RfIdle();
	if (0xff == channel){
		goto bpsconf;
	}
	//channel
	if (0 == channel%2)
	{
		generated_channels[0].baseFrequency = BASE_FREQ + channel*CHANNEL_SPACING;
	}
	if (1 == channel%2)
	{
		generated_channels[0].baseFrequency += CHANNEL_SPACING;
	}
	RAIL_ChannelConfig(channelConfigs[0]); //channel

bpsconf:
	//bps
	switch(bps){
		case TX_BPS: p = configList[1];
			break;
		case RX_BPS: p = configList[2];
			break;
		default:p = NULL;
			break;
	}
	if (RAIL_RadioConfig((void*)p)) { while(1); }

	//ID
	if (NULL != ID)
	{
		syncwordcnf[1] = change_syncword((uint32_t)ID[0]<<24|(uint32_t)ID[1]<<16|(uint32_t)ID[2]<<8 | (uint32_t)ID[3]);
		if (RAIL_RadioConfig((void*)configList[3])) { while(1); }
	}

	//len
	lencnf[1] = len-1;
	if (RAIL_RadioConfig((void*)configList[4])) { while(1); }
}
/*
void test_id(void)
{

	const UINT8 id2[]={0X53,0X78,0x00,0X66, 0X58,0X48,0X22,0X99,  0x52,0x56,0x78,0x53,150, 34, 150,150};
    memcpy((UINT8 *)&gRFInitData, (UINT8 *)&id2, sizeof(id2));
  	grf.crc = my_cal_crc16(0,(UINT8 *)&gRFInitData,sizeof(RFINIT));
}
*/
void rail_status(char * data)
{
	uint8_t a = RAIL_RfStateGet();
	if (0==a){
		//printf("i");
		responsePrint(data, "idle:");
	}else if(1==a){
		responsePrint(data, "rec:");
	}else if(2==a){
		responsePrint(data, "send:");
	}else{
		responsePrint(data, "un:");
	}
}


void sys_working_process(void)
{
  for(;;)
  {
    while(gEventFlag & ALL_EVENT)
    {

      if(epd_scn_tp.screen_busy_flag == TRUE)
      {
        gEventFlag &= EVETT_FALG_CLEAR;
      }

      if(gEventFlag & EVENT_FLAG_EPD_DISPLAY)//屏幕显示任务
      {
        SetWathcDog();
        time_wdg_fun(FALSE,0);
        g_screen_repeat_dis_time=0;
        if( (f_len(F_BMP_RED) !=0) || (f_len(F_BMP_BW) != 0 ))
        {
          if(gSys_tp.present_page_id != gSys_tp.default_page_id )
          {
            sys_page_display_store_fun(gSys_tp.default_page_id,gSys_tp.present_page_id,gSys_tp.present_page_id,gSys_tp.stay_time,0);//保存页显示属性
          }
          gsrc_busy_cont = 0;
          //gscreen_working = TRUE;
          epd_scn_tp.screen_busy_flag = TRUE;
          hlt_busy_times = 0;
          my_ack_buf.srn_time++;
          Init_EPD();
        }
        gEventFlag &= (~EVENT_FLAG_EPD_DISPLAY);
        SetWathcDog();
      }
      if(gEventFlag & EVENT_FLAG_SCREEN_BUSY)//屏幕busy检测任务
      {

        SetWathcDog();
        time_wdg_fun(FALSE,0);
        epd_scn_tp.busy_times++;
        gsrc_busy_cont ++;
        if(FALSE == screen_busy_fun())
        {
          //gscreen_working = FALSE;
          change_next_pageid_fun();//切换到下一页
        }
        if(gsrc_busy_cont >=200)//防止价签屏幕死机busy检测异常导致价签不能更新
          epd_scn_tp.screen_busy_flag = FALSE;

        gEventFlag &= (~EVENT_FLAG_SCREEN_BUSY);
        SetWathcDog();
      }
      //--------------屏幕的優先級最高--------------------------
      if( gEventFlag & EVENT_FLAG_INTER_REED ) //干簧管中断任务
      {
        SetWathcDog();
        Delay_ms(100);
        if(!(REED_READ_INTERRUPT_IO))
          reed_fun_select(epd_attr_info.magnet);
        gEventFlag &= (~EVENT_FLAG_INTER_REED);
        SetWathcDog();
      }
      if(gEventFlag & EVENT_FLAG_INTER_LED) //led定时任务
      {
        UINT16 cont = 0;
        SetWathcDog();
        cont  = led_str_init();
        led_control_fun();
        led_timer_set(cont);
        gEventFlag &= (~EVENT_FLAG_INTER_LED);
        SetWathcDog();
      }

      if( gEventFlag & EVENT_FLAG_SYS_HEART)//心跳任务
      {
        SetWathcDog();
        Delay_ms(1);
        A7106_Tx_Register(CTRL_HBR_NORMAL,1,ghbr_version_flag); //
        gEventFlag &= (~EVENT_FLAG_SYS_HEART);
        if(ghbr_version_flag == 1)
          ghbr_version_flag = 0;
        else
          ghbr_version_flag = 1;
        SetWathcDog();

      }
 //     gEventFlag |= EVENT_FLAG_SYS_HEART;
      // 把干簧管中断和led灯的中断优先级放到rf前面，此做法的目的是让led的中断不会被rf打断，即使有rf事件来时，也要先处理下灯的操作

      if(gEventFlag & EVENT_FLAG_RFWORK) //射频任务
      {
        time_wdg_fun(FALSE,0);
        SetWathcDog();
        led_all_off();
        if(led_tp_info.timer_enable == TRUE )
        {
          // led_all_off();
          set_led_timer(FALSE,0,0);
          //gEventFlag |= EVENT_FLAG_INTER_LED;
        }
        rf_interrupt_into_fun();
        gEventFlag &= (~EVENT_FLAG_RFWORK);
        if((led_tp_info.timer_enable == TRUE)  && (led_tp_info.all_times != 0))
        {
          led_control_fun();
          ledclk_isr_times = (ledclk_isr_times?ledclk_isr_times:1);
          led_timer_set(ledclk_isr_times);
        }
        SetWathcDog();
      }
      if(gEventFlag & EVENT_FLAG_WRITEID)//烧写id任务
      {
        SetWathcDog();
#ifndef EPD_TEST_MODE
//        Write_id_fun(); //todo
#else
        TEST_SPI_WriteString(g_test_buf,27);
        memset(g_test_buf,0x00,30);
#endif
        gEventFlag &= (~EVENT_FLAG_WRITEID);
        SetWathcDog();
      }
      if(gEventFlag & EVENT_FLAG_ERASER_BUFF)//擦除任务
      {
        SetWathcDog();
        event_128_fun();
        gEventFlag &= (~EVENT_FLAG_ERASER_BUFF);
        SetWathcDog();
      }
      if(gEventFlag & EVENT_FLAG_OSD_ANALUSIS)//osd任务
      {
        SetWathcDog();
        main_osd_cmd();
        gEventFlag &= (~EVENT_FLAG_OSD_ANALUSIS);
        SetWathcDog();
      }

      if(gEventFlag & EVENT_FALG_DISPLAY_PAGE)//osd 切页
      {
        SetWathcDog();

        if(gpage_info_fun() != TRUE)//若无册页，直接退出
          goto loop;
        if(change_page_makesure_fun()== FALSE)//显示的页和屏幕页相同且内容不变，不触发显示动作
          goto loop;
        SetWathcDog();
        Delay_ms(1);
        f_erase(F_BMP_BW);
        f_erase(F_BMP_RED);
        SetWathcDog();
        spit_fun(gSys_tp.present_page_id);
        if(dis2screen(FILE_SB,gdis_id_num) != 0)
          gEventFlag |= EVENT_FLAG_EPD_DISPLAY;
      loop:
        gEventFlag &= (~EVENT_FALG_DISPLAY_PAGE);

        SetWathcDog();
      }
      if(gEventFlag & EVENT_FLAG_UPDATA_CHECK)//查询任务
      {
        SetWathcDog();
        f_erase(F_BMP_PKG_1);
        f_erase(F_BMP_PKG_2);
        gEventFlag &= (~EVENT_FLAG_UPDATA_CHECK);
        gEventFlag &= (~EVENT_FLAG_OSD_ANALUSIS);
        gEventFlag &= (~EVENT_FLAG_UPDATA_CHECK);
        SetWathcDog();
      }
      if( gEventFlag & EVENT_FLAG_FLASHUPDATE)//id烧录存储任务
      {
        SetWathcDog();
        save_info_sys_parameter(); //保存到内部info区
        save_extern_rf_info();//保存到外部flash区
        gEventFlag &= (~EVENT_FLAG_FLASHUPDATE);
        SetWathcDog();
      }
      if(gEventFlag & EVENT_FLAG_NETLINK)//组网存储任务
      {
        SetWathcDog();
        INFO_DATA.gRFInitData.wakeup_id = netlink_info.set_wk_id;
        INFO_DATA.gRFInitData.set_wkup_ch = netlink_info.set_wkup_ch;
        INFO_DATA.gRFInitData.grp_wkup_ch = netlink_info.grp_wkup_ch;
        INFO_DATA.gRFInitData.esl_data_ch = netlink_info.esl_data_ch;
        INFO_DATA.gRFInitData.esl_netmask = netlink_info.esl_netmask;
        save_extern_rf_info();
        gEventFlag &= (~EVENT_FLAG_NETLINK);
        SetWathcDog();
      }
      if(gEventFlag & EVENT_FLAG_NCF_CHIP)//nfc任务
      {
        SetWathcDog();
        gEventFlag &= (~EVENT_FLAG_NCF_CHIP);
#ifdef NFC_ENABLE
        nfc_wk_check();
#endif
        SetWathcDog();
      }
      if( gEventFlag & EVENT_FLAG_QUEST_HEART)//心跳请求任务
      {
        SetWathcDog();
        A7106_Tx_Register(CTRL_HBR_NORMAL,1,0);	//todo
        if(rqst_hbr_info.now >= rqst_hbr_info.sum)
          hbr_mode = hbr_mode_bef;
        gEventFlag &= (~EVENT_FLAG_QUEST_HEART);
        SetWathcDog();
      }
      if(gEventFlag & EVENT_FLAG_UPDATA_ROM)//升级rom任务
      {
        SetWathcDog();
        f_sync();                             //保存文件系统
        update_fom_fun();
        gEventFlag &= (~EVENT_FLAG_UPDATA_ROM);
        SetWathcDog();
      }
      /*
      *begin add by zhaoyang for bug 352
      *问题描述：时钟不准
      *解决办法：采用主时钟校准的方式
      */
      if(gEventFlag & EVENT_FLAG_TIMER_CALIBRATION)
      {
        SetWathcDog();

#ifdef SYS_TIMER_CAB
        if(FALSE == timer_calibration_fun())     //第一次校验失败，再测校准下
          timer_cont_rst();
#endif
        gEventFlag &= (~EVENT_FLAG_TIMER_CALIBRATION);
        SetWathcDog();
      }
      /*end add by zhaoyang for bug 352 */
      if(gEventFlag & EVENT_FLAG_SPIERROR)
      {
        SetWathcDog();
        gEventFlag &= (~EVENT_FLAG_SPIERROR);
        SetWathcDog();
      }
      if(gEventFlag & EVENT_FLAG_LTIMER)
      {
        SetWathcDog();
        gEventFlag &= (~EVENT_FLAG_LTIMER);
        SetWathcDog();
      }
//      if(0 == (main_loop_count--))
//        while(1);
    }
    main_loop_count  = 10000;
    eraset_cmd_buf();
    sys_sleep();
  }

}
uint8_t buf[26] = {1,2,3,4,5,6};
RAIL_TxData_t txInfo;
uint32_t t= 0;
int main(void)
{
  bsp_init();
  main_clk_init();                      //主时钟和看门狗初始化
  sys_register_init();                  //初始化2755寄存器
  led_init();                           //关闭led灯
  disable_peripheral();                 //关闭外部设备和未使用的io管脚
//#ifndef EPD_TEST_MODE
//  write_id_IOint();                     //id烧录功能初始化
//#else
//  test_spi_int();
// TEST_SPI_WriteString("0test_begin",12);
//#endif
  isr_reed_init();                      //关闭干簧管中断
//  nfc_init();                           //nfc初始化
  flash_spi_init(TRUE);                 //外部flash，标准SPI初始化
  f_init();
//  test_save_id();
  sys_load_config_info();                //加载系统默认值
  load_led_attribute_fun();                 //加载rc包设置、加载属性报设置
//  rf_a7106_open();                      //初始化射频io	todo
//  if(!A7106_Initial())                  //射频初始化，初始化失败，则等待看门狗重启
//    while(1);
//#ifndef EPD_TEST_MODE
//  WRITEID_ISR_OPEN;                     //烧写id中断打开
//#endif
  test_save_id();
  osd_init();
  update_success_fun();                 //加载系统更新成功标志
#ifdef EPD_POWER_OFTEN_OPEN
  epd_power_deep_sleep_fun();
#endif
  memset((UINT8 *)&my_ack_buf,0x00,sizeof(my_ack_buf));

  sys_load_page_display_fun();
  adc_voltage();
  gEventFlag |= EVENT_FLAG_RFWORK;
  sys_working_process();
}

void myRAILtest(void)
{
	  myChangeRadioConfig(INFO_DATA.gRFInitData.set_wkup_ch, RX_BPS, (UINT8*)&INFO_DATA.gRFInitData.wakeup_id, RF_SET_WAKEUP_PACKET);
	  while(1)
	  {
	    if (button0Pressed == true)
	    {
	      button1Pressed = false;
	      button0Pressed = false;
	      RAIL_RfIdle();
	      txInfo.dataPtr = buf;
	      txInfo.dataLength = 6;

	      RAIL_TxDataLoad(&txInfo);
	      RAIL_TxStart(0, NULL, NULL);
	      while(RAIL_RfStateGet()== RAIL_RF_STATE_TX);
	      UDELAY_Delay(2000);
	      t++;
	    }
	    if (button1Pressed == true)
	    {
	    	t = 0;
	    	button0Pressed = false;
	        RAIL_RfIdle();
	        RAIL_RxStart(0);
	        while(RAIL_RfStateGet()== RAIL_RF_STATE_RX && button0Pressed == false);
	    }
	  }
}
/******************************************************************************
 * App main
 *****************************************************************************/
/*
uint8_t ID[] ={0X53,0X78,0X00,0X66};
volatile uint8_t a = 0;
int main(void)
{
  // Initialize the chip
  CHIP_Init();
  // Initialize the system clocks and other HAL components
  halInit();

  CMU_ClockEnable(cmuClock_GPIO, true);

  // Initialize the BSP
  BSP_Init( BSP_INIT_BCC );
  // Initialize the USART and map LF to CRLF
  RETARGET_SerialInit();
  RETARGET_SerialCrLf(1);
  // Enable the buttons on the board
  for (int i=0; i<BSP_NO_OF_BUTTONS; i++)
  {
    GPIO_PinModeSet(buttonArray[i].port, buttonArray[i].pin, gpioModeInputPull, 1);
  }

  // Initialize the LCD display
  GRAPHICS_Init();
  GRAPHICS_Sleep();
  // Button Interrupt Config
  GPIOINT_Init();
  GPIOINT_CallbackRegister(buttonArray[0].pin, gpioCallback);
  GPIOINT_CallbackRegister(buttonArray[1].pin, gpioCallback);
  GPIO_IntConfig(buttonArray[0].port, buttonArray[0].pin, false, true, true);
  GPIO_IntConfig(buttonArray[1].port, buttonArray[1].pin, false, true, true);

  // Initialize Radio
  RAIL_RfInit(&railInitParams);

  // Configure modem, packet handler
  changeRadioConfig(currentConfig);

  myChangeRadioConfig(8, TX_BPS, ID, 16);
  // Configure RAIL callbacks with no appended info
  RAIL_RxConfig((  RAIL_RX_CONFIG_PREAMBLE_DETECT
                 | RAIL_RX_CONFIG_INVALID_CRC
                 | RAIL_RX_CONFIG_SYNC1_DETECT
                 | RAIL_RX_CONFIG_ADDRESS_FILTERED
                 | RAIL_RX_CONFIG_BUFFER_OVERFLOW  ),
                false);
  RAIL_SetRxTransitions(RAIL_RF_STATE_IDLE, RAIL_RF_STATE_RX, //RAIL_RF_STATE_IDLE
                        RAIL_IGNORE_NO_ERRORS);

  set_iodebug();
  BSP_LedsInit();

  RTCDRV_Init();
  RTCDRV_AllocateTimer( &slaveRtcId ); // Reserve a timer
  RTCDRV_StartTimer(slaveRtcId, rtcdrvTimerTypePeriodic, SET_WOR_PERIOD*2000, slaveTimerExpiredCallback, NULL);
  System_Init();
  test_id();
  while(1)
  {
//todo
		while(0 != gEventFlag){

		}


		while(gEventFlag == 0){

		  EMU_EnterEM2(true);
		  UDELAY_Delay(1000);
		}

  }
}
*/
/******************************************************************************
 * App specific functions
 *****************************************************************************/

/**************************************************************************//**
 * @brief  Change the radio configuration at runtime.
 *
 * @param[in] newConfig The array index of the radio configuration to use. this
 * is chosen from a list of radio configurations defined in rail_config.h
 *   0 = Bluetooth_LE_Test
 *   1 = IEEE802154_2p4GHz
 *   2 = RAIL
 *****************************************************************************/
static void changeRadioConfig(int newConfig)
{
  // Turn off the radio before reconfiguring it
  RAIL_RfIdle();

  // Reconfigure the radio parameters
  RAIL_PacketLengthConfigFrameType(frameTypeConfigList[newConfig]);
  if (RAIL_RadioConfig((void*)configList[newConfig])) 
    { 
      while(1); 
    }

  // Set us to a valid channel for this config and force an update in the main
  // loop to restart whatever action was going on
  RAIL_ChannelConfig(channelConfigs[newConfig]);
  currentConfig = newConfig;
}



/**************************************************************************//**
 * @brief   A utility function for populating a RAIL application payload.
 *
 * @param[in] txInfo  The instance of RAIL_TxData_t to be used in tranmission
 * of the message. This is currently simply populated with the data array 
 * defined above.
 * @param[in] txId  The ID to put in the first element of the packet.
 *****************************************************************************/
/*
static void generatePayload(RAIL_TxData_t *txInfo, int txId)
{
  data[0] = txId;
  txInfo->dataPtr = (uint8_t *)&data[0];
  txInfo->dataLength = sizeof(data);
}
*/
/**************************************************************************//**
 * @brief   A callback to handle a button press on the WSTK
 *
 * @param[in] pin  The pin used to identify the button that has been pressed.
 *****************************************************************************/
void gpioCallback(uint8_t pin)
{
  if (pin == buttonArray[0].pin)
  {
    button0Pressed = true;
  }
  else if (pin == buttonArray[1].pin)
  {
    button1Pressed = true;
  }
}


/******************************************************************************
 * Callback Implementation
 *****************************************************************************/



void RAILCb_TxPacketSent(RAIL_TxPacketInfo_t *txPacketInfo)
{

}

void RAILCb_TxRadioStatus(uint8_t status)
{
    // set to IDLE
    if ((status & RAIL_TX_CONFIG_BUFFER_UNDERFLOW)
        || (status & RAIL_TX_CONFIG_CHANNEL_BUSY)) {
    }
}
void RAILCb_RxRadioStatus(uint8_t status)
{
    if (status & RAIL_RX_CONFIG_PREAMBLE_DETECT) {
      forceStateChange = true;
    }

    // if Sync received, continue listening for packet
    if (status & RAIL_RX_CONFIG_SYNC1_DETECT) {
    	 forceStateChange = true;
    }

    // if failed, switch back to idle
    if ((status & RAIL_RX_CONFIG_INVALID_CRC)
        || (status & RAIL_RX_CONFIG_BUFFER_OVERFLOW)
        || (status & RAIL_RX_CONFIG_ADDRESS_FILTERED)) {
    	 forceStateChange = true;
    }
}

void *RAILCb_AllocateMemory(uint32_t size)
{
  return receiveBuffer;
}

void *RAILCb_BeginWriteMemory(void *handle,
                              uint32_t offset,
                              uint32_t *available)
{
  return ((uint8_t*)handle) + offset;
}

void RAILCb_EndWriteMemory(void *handle, uint32_t offset, uint32_t size)
{
  // Do nothing
}

void RAILCb_FreeMemory(void *ptr)
{
}

void  RAILCb_RfReady(void)
{
  return;
}

void RAILCb_CalNeeded(void)
{
}

void RAILCb_RadioStateChanged(uint8_t state)
{
  //no action
}

void bsp_init(void)
{
	  // Initialize the chip
	  CHIP_Init();
	  // Initialize the system clocks and other HAL components
	  halInit();

	  CMU_ClockEnable(cmuClock_GPIO, true);

	  // Initialize the BSP
	  BSP_Init( BSP_INIT_BCC );
	  // Initialize the USART and map LF to CRLF
	  RETARGET_SerialInit();
	  RETARGET_SerialCrLf(1);
	  // Enable the buttons on the board
	  for (int i=0; i<BSP_NO_OF_BUTTONS; i++)
	  {
	    GPIO_PinModeSet(buttonArray[i].port, buttonArray[i].pin, gpioModeInputPull, 1);
	  }

	  // Initialize the LCD display
	  GRAPHICS_Init();
	  GRAPHICS_Sleep();
	  // Button Interrupt Config
	  GPIOINT_Init();
	  GPIOINT_CallbackRegister(buttonArray[0].pin, gpioCallback);
	  GPIOINT_CallbackRegister(buttonArray[1].pin, gpioCallback);
	  GPIO_IntConfig(buttonArray[0].port, buttonArray[0].pin, false, true, true);
	  GPIO_IntConfig(buttonArray[1].port, buttonArray[1].pin, false, true, true);

	  // Initialize Radio
	  RAIL_RfInit(&railInitParams);

	  // Configure modem, packet handler
	  changeRadioConfig(currentConfig);

	  // Configure RAIL callbacks with no appended info
	  RAIL_RxConfig((  RAIL_RX_CONFIG_PREAMBLE_DETECT
	                 | RAIL_RX_CONFIG_INVALID_CRC
	                 | RAIL_RX_CONFIG_SYNC1_DETECT
	                 | RAIL_RX_CONFIG_ADDRESS_FILTERED
	                 | RAIL_RX_CONFIG_BUFFER_OVERFLOW  ),
	                false);
	  RAIL_SetRxTransitions(RAIL_RF_STATE_IDLE, RAIL_RF_STATE_RX, //RAIL_RF_STATE_IDLE
	                        RAIL_IGNORE_NO_ERRORS);

	  set_iodebug();
	  BSP_LedsInit();

	  RTCDRV_Init();
	  RTCDRV_AllocateTimer( &slaveRtcId ); // Reserve a timer
	  RTCDRV_StartTimer(slaveRtcId, rtcdrvTimerTypePeriodic, 8000, slaveTimerExpiredCallback, NULL);
}


uint32_t change_syncword(uint32_t souce)
{
	uint32_t t = 0;
    uint32_t i = 0;
	for( i = 0;i<4;i++)
	{
		t>>=1;
		if(souce & 0x80000000)
		{
			t|=0x80000000;
		}
		if(souce & 0x08000000)
		{
			t|=0x08000000;
		}
		if(souce & 0x00800000)
		{
			t|=0x00800000;
		}
        if(souce & 0x00080000)
		{
			t|=0x00080000;
		}
         if(souce & 0x00008000)
		{
			t|=0x00008000;
		}
         if(souce & 0x00000800)
		{
			t|=0x00000800;
		}
        if(souce & 0x00000080)
		{
			t|=0x00000080;
		}
        if(souce & 0x00000008)
		{
			t|=0x00000008;
		}
		souce<<=1;
	}
	t = ((t&0x0000000f)<<28) + ((t&0x000000f0)<<20) + ((t&0x00000f00)<<12) +  ((t&0x0000f000)<<4) + ((t&0x000f0000)>>4)+((t&0x00f00000)>>12)+((t&0x0f000000)>>20)+((t&0xf0000000)>>28) ;
	return t;
}

