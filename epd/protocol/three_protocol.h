#ifndef _RF_PROTOCOL_H_
#define _RF_PROTOCOL_H_

#include "..\sys_init\datatype.h"

#define RF_SET_WAKEUP_PACKET       0x06		//SET唤醒数据包大小
#define RF_GRP_WAKEUP_PACKET       0x02		//GRP唤醒数据包大小
#define RF_NORMAL_PACKET           0x1a         //正常数据包大小
#define HBR_LEN 					16
#pragma pack(1)

typedef struct _hrb
{
  UINT8 sum;
  UINT8 now;
  UINT8 cont;
  UINT8 hbt_cont;
}HRB_STP;
/** RF工作设置参数 */

typedef struct _rfid
{
  UINT8 id0;
  UINT8 id1;
  UINT8 id2;
  UINT8 id3;
} RFID;

typedef struct _rfinit
{
  
  RFID wakeup_id;
  RFID esl_id;
  RFID master_id;
  UINT8 esl_data_ch;			
  UINT8 esl_netmask;				
  UINT8 set_wkup_ch;			
  UINT8 grp_wkup_ch;			  
  
} RFINIT;

typedef struct esl_frame_t {
  
  UINT8 ctrl;
  UINT16 package_num;
  UINT8 addr;
  UINT8 data[20];
  UINT8 crc[2];
}esl_frame_t;

#pragma pack()


typedef enum{
  NORMAL_HEARTBEAT,
  REQUST_HEARTBEAT,
  STOP_HEARTBEAT
}HRB_T;

typedef enum {
 // RF_EVENT_NONE,
  RF_EVENT_GLOBLE_CMD,
  RF_EVENT_GET_FRAME1,
  RF_EVENT_PKG_TRN,
  RF_EVENT_SET_PKG_CH, 
  RF_EVENT_SET_ERR,
  
  
  RF_EVENT_QUERY,
  RF_EVENT_NETLINK,
  RF_EVENT_OSD,
  RF_EVENT_OSD_128CMD,
  RF_EVENT_OSD_76CMD,
  RF_EVENT_LINK_QUERY,
  RF_EVENT_RC_INFO,
  RF_EVENT_SLEEP,
  RF_ENENT_ACK,
  
  RF_EVENT_UNKNOWN,
} RF_EVENT_T;

typedef enum {
  RF_ERROR_NONE,
  RF_ERROR_RF_CRC,
  RF_ERROR_ENTER_TXRX,
  RF_ERROR_TX_TIMEOUT,
  RF_ERROR_RX_TIMEOUT,
  RF_ERROR_CAL,
  RF_ERROR_UNKNOWN,
} RF_ERROR_T;

typedef enum {
  RF_FSM_CMD_NONE,
  
  RF_FSM_CMD_RF_POR_RST,
  RF_FSM_CMD_INIT,
  
  RF_FSM_CMD_SET_EXIT_LOOP,
  RF_FSM_CMD_SET_WAKEUP,
  RF_FSM_CMD_SET_RX,
  RF_FSM_EVENT_SET_HANDLE, 
  
  RF_FSM_CMD_GRP_EXIT_LOOP,
  RF_FSM_CMD_GRP_WAKEUP,
  RF_FSM_CMD_GRP_RX,
  
  RF_FSM_CMD_GRP_ADD_SET_RX,
  
  RF_FSM_EVENT_GRP_HANDLE,
  
  RF_FSM_CONFG_FRAME1_PER,
  RF_FSM_CMD_RX_FRAME1,

  RF_FSM_CMD_RX_DATA_CONFIG,
  RF_FSM_CMD_RX_DATA,
  RF_FSM_EVENT_RX_DATA_HANDLE,
  
  RF_FSM_EVENT_TX_ACK_HANDLE,
  RF_FSM_CMD_TX_DATA_CONFIG,
  RF_FSM_CMD_TX_DATA,
  
  RF_FSM_CMD_RC_EXIT_LOOP,
  RF_FSM_CMD_RC_WAKEUP,
  RF_FSM_CMD_RC_RX,
  
  
  RF_FSM_CMD_SLEEP,
  

  RF_FSM_ERROR_HANDLE,
  
  RF_FSM_CMD_UNKNOWN,
} RF_CMD_T;

typedef struct RF_T {
  RF_EVENT_T event;			//记录当前事件
  RF_EVENT_T prev_event;	//记录上一次事件
  
  RF_CMD_T prev_cmd;		//记录上一次CMD
  RF_CMD_T cmd;				//记录当前CMD
  RF_CMD_T next_cmd;		//记录下一次CMD
  
  RF_ERROR_T error;			//记录RF错误标识
  uint8_t ack;				//RF ACK值
} RF_T;

int rf_state_machine(RF_T *rf);
#endif

