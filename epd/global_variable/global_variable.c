#include "sys_init.h"
#include "datatype.h"
#include "global_variable.h"
#include "led.h"
#include <stdbool.h>
//#include "nfc_iic.h"
const UINT32 generated_100kbps[] = {
  0x01010FFCUL, 0x0003C008UL,
  0x00014140UL, 0x008800E2UL,
  0x00014144UL, 0x1153E6C1UL,
  0x0001601CUL, 0x2002C01FUL,
  0x00016024UL, 0x000CD000UL,
  0x00016030UL, 0x00FF2FD0UL,
  0x00016034UL, 0x00000B2EUL,
  0x00016038UL, 0x01880020UL,
  0x00016048UL, 0x11200714UL,
  0x00016050UL, 0x003503B4UL,
  0x0001701CUL, 0x82710060UL,
  0x00017074UL, 0x00000213UL,
  0xFFFFFFFFUL,
};

const UINT32 generated_500kbps[] = {
  0x01010FFCUL, 0x0003C00BUL,
  0x00014140UL, 0x00880067UL,
  0x00014144UL, 0x1153E6C0UL,
  0x0001601CUL, 0x1802C01FUL,
  0x00016024UL, 0x000C5000UL,
  0x00016030UL, 0x00FF0990UL,
  0x00016034UL, 0x000008A2UL,
  0x00016038UL, 0x0102000AUL,
  0x00016048UL, 0x1DE00714UL,
  0x00016050UL, 0x002B03D1UL,
  0x0001701CUL, 0x82720060UL,
  0x00017074UL, 0x00000223UL,
  0xFFFFFFFFUL,
};

uint32_t syncwordcnf[] = {0x00016040UL, 0xCA1E6A4AUL,0xFFFFFFFFUL};
uint32_t lencnf[] = {0x00010018UL, 0x0UL, 0xFFFFFFFFUL};



#pragma pack(1)
//共用全局变量最大64个字节
struct PUBLIC_TEMP_T TEMP;

struct     INFO_DATA_T    INFO_DATA;                            //系统和rf工作参数信息
volatile   UINT8          gRFError = 0;                         //rf位置错误标志
volatile   UINT32         gEventFlag = 0;	                //系统主任务标志位	
volatile   UINT16         main_loop_count  = 10000;             //系统主任务防护，单次流程不能超过10000次，防止死循环
volatile   UINT16          gEslHearcont = 0;
GERR_T gerr_info = NONEERR;
RC_ERR_T grc_err = NONEERR_1;

//UINT8 gnfc_state = NFC_SLEEP;
volatile UINT8 fram1_data_channel = 0;
stNetLink netlink_info;


//-----------------心跳--------------------
volatile HRB_T hbr_mode = NORMAL_HEARTBEAT ;
volatile HRB_T hbr_mode_bef = NORMAL_HEARTBEAT ;
volatile   UINT8          gaclk_times_cont = 0;
volatile   UINT8 ghbr_times = 0;
HRB_STP rqst_hbr_info;
volatile UINT8 ghbr_version_flag = 0;

//-------------------osd----------------------
struct PKG_NUM_T gpkg;
SID_T gsid;
UINT16 all_osd_crc = 0;
file_id_t write_pkg_buff_id;
UINT32 cmd_start_offset = 0;
UINT32 cmd_before_offset = 0;
UINT16 first_lose_pkg = 0;
file_id_t read_pkg_fileid = F_BMP_PKG_1;
file_id_t write_temp_buff_id;
volatile UINT8 gFlag_bit = 1;

//------------rf状态机-----------------------------
RF_T grf_state_flag;
volatile UINT8 gro_wor_times = 0;
volatile UINT8 rc_wor_times = 0;
volatile RF_EVENT_T ret_ack_flag = RF_EVENT_QUERY;

//-------------定时器全局变量-----------------------
volatile UINT16 smclk_isr_times = 0;                            //定时器A的循环计数器
UINT16  sys_wait_time_set = 0;                                 //set唤醒后sort值大于8s标志
volatile UINT16 ledclk_isr_times = 0;                            //led灯的循环计数器
BOOL sys_timer_2s_open_flag = TRUE;

//--------------属性包------------------------------
struct SYS_ATTR_T def_sys_attr;
BOOL  gmagent2_flag = FALSE;
EPD_ATTRIBUTE_PKG_ST epd_attr_info; //属性包
EPD_RC_ST  rc_attr_info;

//-------------------led----------------------------
UINT16 house_loop_times = 0;
volatile UINT8 hose_led_choose = 0;
LED_LOOP_ST led_tp;
LED_STATE led_tp_info ={0};

//--------------------srceen------------------------
UINT8 hlt_busy_times = 0;
volatile SCREEN_T  epd_scn_tp = {0};

//--------------------烧写id----------------------
//烧写id时，缓存数据
UINT8 wirte_spi_data = 0;

//---------------------远程升级---------------------
ROM_UP_T updata_info;
ROM_UP_QRY_T  uprom_tp;

//-----------------------spi转uart输出---------------
#ifdef EPD_TEST_MODE
UINT8 g_test_buf[30];
#endif
//------------------------重新刷新屏幕时间-----------
UINT16 g_screen_repeat_dis_time = 0;

//------------------------定时器校准--------------
/*
*begin add by zhaoyang for bug 352 
*问题描述：时钟不准
*解决办法：采用主时钟校准的方式
*/   
volatile MY_TIMER_T g_timer_tp;
/*end add by zhaoyang for bug 352 */   

//----------------定时器周期切换标准-------
UINT8 tm_add_value = CNT_ADD_VALUE_2S;
//----------------------屏幕更新标志--------
//BOOL gscreen_working = FALSE;

//----------全局crc------------------
osd_crc_t g_crc;
osd_cmd_t gcmd_tp;
UINT8 eraser_file_flag=0;
UINT8 gdis_id_num = 0 ;
//-----------------------页显示属性---------------
dis_page_t gpage ={0};

//-------------------------系统掉电保存数据--------
SYS_LOAD_TYPE gSys_tp;
//-------------------------mytest-----------------
MY_TEST_T my_ack_buf;
//--------------------------end-------------------

//-----------------------set 命令切页-------------------
//glo_dis_page_t gset_dis_page;
//-------------------rc 翻页----------------------------
//grc_dis_page_t grc_dis_page;
//--------------干簧管切页---------------------------
gmagnet_change_page_t gmag_dis_pgae;

//--------------------
BOOL rc_store_inio_flag=FALSE;
//----------电池电量----------
UINT8 gelectric_quantity = 0;
//------当前正在显示的页---------
//UINT8 gpage_nowid = 0xff;
//------------屏幕busy检测测试防护，防止价签无心跳
UINT8 gsrc_busy_cont = 0;

RTCDRV_TimerID_t slaveRtcId;  // RTC timer ID
RAIL_TxData_t slaveTxData;    // application payload sent during a TX
bool slaveRtcRunning;         // boolean for RTC status

fs_t fs;

#pragma pack()
