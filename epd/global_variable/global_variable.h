#ifndef __HOLTEK_MSP430_GLOBAL_VARIABLE_H__
#define __HOLTEK_MSP430_GLOBAL_VARIABLE_H__

#include "..\sys_init\datatype.h"
#include "..\osd\osd_cmd.h"
#include "..\led\led.h"
#include "..\sys_init\sys_init.h"
#include "..\sys_init\sys_config.h"
#include <stdbool.h>
#include <rtcdriver.h>
#include <rail_types.h>
//#include "write_id.h"

#define EPD_3_VERSION   (0x20)
#define  SYS_TIMER_CAB                  // add by zhaoyang for bug 352 


#define HEART_TIME_3M           ((UINT8)(0x17))  //8s听帧的心跳值
#define RC_WOR_CONT             ((UINT8)0x0a)    //遥控器200模式wor的2分钟计数 
#define GROUP_MAX_TIMES          ((UINT8 )30)    //120s
#define GROM_UPDATA_FLAG        ((UINT32)0x66886688)
#define GROM_UPDATA_SUCCESS     ((UINT32)0x99779977) 

#pragma pack(1)

typedef enum 
{
  NONEERR,
  TR3_PKG_ERR,
  TR2_LAYER_ID_ERR,
  TR3_OSD_CMDERR,
  TR2_CMD_CRC_ERR,
  TR3_STREAM_CRC_ERR,
  TR3_OSD_WPKG_BIT_ERR,
  TR3_OSD_SB_START_ERR,
  TR3_OSD_SB_END_ADDR_ERR,
  TR3_OSD_SB_ERR,
  TR3_SCREEN_ERR1, 
  TR3_OSD_QUERY_PAGE_INFO_CRC_ERR,
  TR3_OSD_QUERY_PAGE_NUM_CRC_ERR,
  TR3_OSD_CMD6_ERR,
  TR3_OSD_QUERY_ERR,
  UPROM_ERR,
  SYS_SAVE_ERR,
  PAGE_ID_ERR,
  OSD_SID_ERR, 
}GERR_T;

typedef enum
{
  NONEERR_1,
  RC_CODE_ERR,
  PAGE_ID_ERR2,
}RC_ERR_T;
struct PKG_NUM_T 
{
  UINT16 pkg_num;
  UINT16 pkg_sum;
  UINT16 osd_pkg_all;
  UINT8 sroft;
  UINT8 ctrl;
};

typedef struct _rf_cbrt_ 
{
  UINT8 cbrt;
  UINT8 ifal1;
  UINT8 vcoal;
  UINT8 vcoal1;
  UINT8 r_rcoc;
}RF_CBL;

struct PUBLIC_TEMP_T {
  union
  {
    UINT32 unuserd; //必须放在首位保证4字节对齐
    osd_sttep_t td;
    // RF_FILE_DATA rfd;
    UINT8 BYTE64[MAX_TEMP_BUF];		//最大空间为128字节
    UINT8 RF_RCV[64];						//接收BUF为64字节
    UINT8 HAFF_CODE[64];						//HAFFMAN解码时的码元
  };
};


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
typedef struct _osdsid
{
  UINT8 old_sid;
  UINT8 now_sid;
}SID_T;

typedef struct _romupdat_t
{
  UINT16 crc;
  UINT32 flag;
  UINT32 addr;  
  
}ROM_UP_T;

typedef struct _romupqry_t
{
  UINT8 sys_update_success_flag;
  BOOL  sys_update_query_flag;
  
}ROM_UP_QRY_T;

typedef struct _screen_t
{
  BOOL screen_busy_flag;
  UINT8 busy_times;
}SCREEN_T;

typedef struct _sys_init_load_t
{
  file_id_t use_page_info_fileid;
  file_id_t use_page_num_fileid;
  UINT8     default_page_id;//默认页
  UINT8     present_page_id;//要显示的页
  UINT8     gpage_nowid;//当前显示的页
  UINT8     page_map[8];
  UINT16    stay_time;
  UINT16    stay_time_cont;
  UINT8    change_map;//此变量的作用是标明8页中那个页被改变了，0表示未改变，1表示数据发送变化
 // UINT8 gflag_dispage_state; //0:空闲 1：osd 命令 2：set命令 4：rc命令
}SYS_LOAD_TYPE;

typedef struct my_test_t
{
  UINT16 flash_crc;
  UINT8 srn_time;
  UINT8 sid1;
  UINT8 set_wor_time;
  UINT8 set_wor_firme1_time;
  UINT8 grp_wor_time;
  UINT8 updae_ack_time;
  UINT8 query_ack_time;
  UINT16 receive_pkg_sum;
  UINT16 finish_pkg_num;
  UINT16 query_pkg_num; 
}MY_TEST_T;

/*
*begin add by zhaoyang for bug 352 
*问题描述：时钟不准
*解决办法：采用主时钟校准的方式
*/ 
typedef struct my_timer_t
{
  UINT16 led_30ms_cont;
  UINT16 wor_2s_cont;
  UINT16 rc_200ms_cont;
  UINT16 alk_600ms_cont;
  BOOL flag_600ms;
}MY_TIMER_T;

/*end add by zhaoyang for bug  352*/  

typedef struct osd_crc_t
{
  UINT8 crc_cmd[2];
  UINT8 crc_layer[2];
  UINT8 crc_page_info[2];
  UINT8 crc_page_num[2];
}osd_crc_t;

typedef struct osd_cmd_t
{
  UINT32 start_addr;
  UINT32 len;
}osd_cmd_t;

typedef struct dis_page_t
{
  BOOL  flag;   
}dis_page_t;

typedef struct glo_dis_page_t
{
  UINT8 dis_pageid;
  UINT8 default_pageid;
  UINT8 before_state;//set切页的上一个状态的值
  BOOL  before_flag;//用来保存上一个状态标志
  BOOL  flag;//保存set切页的状态值
  UINT16 time_cont;
  UINT16 time_sum;
}glo_dis_page_t;

typedef struct grc_dis_page_t
{
  UINT8 dis_pageid;
  UINT8 default_pageid;
  UINT8 before_state;//set切页的上一个状态的值
  BOOL  before_flag;//用来保存上一个状态标志
  BOOL  flag;//保存set切页的状态值
  UINT16 time_cont;
  UINT16 time_sum;
}grc_dis_page_t;
typedef struct gmagnet_change_page_t
{
  UINT8 dis_pageid;
  UINT8 default_pageid;
  UINT8 before_state;//set切页的上一个状态的值
  BOOL  before_flag;//用来保存上一个状态标志
  BOOL  flag;//保存set切页的状态值
  UINT16 time_cont;
  UINT16 time_sum;
}gmagnet_change_page_t;
typedef enum{
	TX_BPS,
	RX_BPS,
	DEFAULT_BPS
}bps_enum;
#pragma pack()



#define ALL_EVENT        (0x0FFFFFFFF)

extern struct INFO_DATA_T INFO_DATA;
extern volatile UINT8 gRFError;
extern volatile UINT32 gEventFlag;
extern volatile UINT16 main_loop_count;
extern LED_STATE led_tp_info;
extern UINT8 gnfc_state;
extern struct SYS_ATTR_T def_sys_attr;
extern volatile UINT8 gro_wor_times;
extern volatile UINT8 fram1_data_channel;
extern volatile UINT8 rc_wor_times;
extern volatile RF_EVENT_T ret_ack_flag;
extern  RF_T grf_state_flag;
extern volatile HRB_T hbr_mode;
extern volatile HRB_T hbr_mode_bef ;
extern HRB_STP rqst_hbr_info;
extern volatile   UINT8 ghbr_times;
extern struct PUBLIC_TEMP_T TEMP;

extern volatile   UINT16 smclk_isr_times;  
extern UINT8 wirte_spi_data;
extern volatile   UINT16  gEslHearcont;
extern volatile   UINT8  gaclk_times_cont;
extern struct PKG_NUM_T gpkg;
extern stNetLink netlink_info;
extern SID_T gsid;
extern GERR_T gerr_info;
extern UINT16 all_osd_crc;
extern file_id_t write_pkg_buff_id;
extern UINT32 cmd_start_offset;
extern UINT32 cmd_before_offset;
extern UINT16 first_lose_pkg;
extern file_id_t read_pkg_fileid;
extern file_id_t write_temp_buff_id;
extern volatile UINT8 gFlag_bit;
extern EPD_ATTRIBUTE_PKG_ST epd_attr_info;
extern EPD_RC_ST  rc_attr_info;
extern LED_LOOP_ST led_tp;
extern volatile UINT8 hose_led_choose;
extern ROM_UP_T updata_info;
extern ROM_UP_QRY_T  uprom_tp;;
extern UINT16  sys_wait_time_set ;  
extern BOOL sys_timer_2s_open_flag;
extern volatile SCREEN_T  epd_scn_tp;
extern UINT16 house_loop_times;
extern UINT8 hlt_busy_times;
extern BOOL  gmagent2_flag;
extern volatile UINT8 ghbr_version_flag ;
extern volatile UINT16 ledclk_isr_times;                            //led灯的循环计数器
extern UINT16 g_screen_repeat_dis_time;
/*
*begin add by zhaoyang for bug 352 
*问题描述：时钟不准
*解决办法：采用主时钟校准的方式
*/  
extern volatile MY_TIMER_T g_timer_tp;
/*end add by zhaoyang for bug 352 */ 
#ifdef EPD_TEST_MODE
extern UINT8 g_test_buf[30];
#endif
extern SYS_LOAD_TYPE gSys_tp;
extern  MY_TEST_T my_ack_buf;

extern osd_crc_t g_crc;
extern osd_cmd_t gcmd_tp;
extern UINT8 eraser_file_flag;
extern UINT8 gdis_id_num ;
extern dis_page_t gpage ;
extern UINT8 tm_add_value;
//extern BOOL gscreen_working ;
extern BOOL rc_store_inio_flag;
extern RC_ERR_T grc_err;
extern UINT8 gelectric_quantity ;
//extern UINT8 gpage_nowid;
//------------屏幕busy检测测试防护，防止价签无心跳
extern UINT8 gsrc_busy_cont;
//extern glo_dis_page_t gset_dis_page;
//extern grc_dis_page_t grc_dis_page;
//extern gmagnet_change_page_t gmag_dis_pgae;
extern const UINT32 generated_500kbps[];
extern const UINT32 generated_100kbps[];
extern UINT32 syncwordcnf[];
extern UINT32 lencnf[];
extern RTCDRV_TimerID_t slaveRtcId;  // RTC timer ID
extern RAIL_TxData_t slaveTxData;    // application payload sent during a TX
extern bool slaveRtcRunning;         // boolean for RTC status

#endif
