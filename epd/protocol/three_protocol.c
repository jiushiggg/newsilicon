#include "datatype.h"
#include "three_protocol.h"
//#include "rf.h"
#include "three_event.h"
#include <string.h>
#include "sys_isr.h"
#include "global_variable.h"
//#include "rf_a7106.h"
//#include "write_id.h"

#include "rail_types.h"
#include "rail.h"
#include "crc16.h"

const rf_cmd_t cmd_rf[] = {globle_evert_fun,set_get_frame1_fun,set_pkg_trn_fun,set_pkg_chn_fun,set_ctrl_err_fun,\
  get_data_query_fun,get_netlink_fun,get_osd_updata_fun,get_osd_128cmd_fun,get_osd_76cmd_fun,get_link_query_fun,get_rc_pkg_fun,get_sleep_fun};


int rf_state_machine(RF_T *rf)
{
  uint8_t loop = 1;
  uint8_t rf_buf[26];
  RF_ERROR_T err;
  
  while (loop) 
  {
    switch (rf->cmd) 
    {
      //-----------------------------------rf 异常处理流程----------------------      
    case RF_FSM_CMD_RF_POR_RST:                                 //rf射频io错误时，重启rf
//      rf_power_off();	//todo
//      rf_a7106_open();
      rf->next_cmd = RF_FSM_CMD_INIT;
      break; 
      
    case RF_FSM_CMD_INIT:                                        //初始化rf
      err = rf_cmd_init(rf);
      clear_gpkg_fun();
      gsid.old_sid  = 0;
      if (err == RF_ERROR_NONE)
        rf->next_cmd = RF_FSM_CMD_SET_EXIT_LOOP;
      else
        while(1);
      break;
      
      //---------------------------rf set 层流程--------------------------------     
    case RF_FSM_CMD_SET_EXIT_LOOP:                              //退出条件是在set听帧模式下，混杂模式收帧1超时和收到sleep帧后，退出进入set唤醒
      rf_set_wor(rf,TRUE);
      set_wor_flag_fun(RF_FSM_CMD_SET_EXIT_LOOP);                
      rf->next_cmd = RF_FSM_CMD_SLEEP; 
      break; 
      
    case RF_FSM_CMD_SET_WAKEUP:                                 //系统sel wor唤醒入口函数、确保系统在处在set唤醒模式、在rc和grp模式退出时以及在set唤醒后的sorlt值大约8s
      set_wor_flag_fun(RF_FSM_CMD_SET_EXIT_LOOP);              
      rf_set_wor(rf,TRUE);
//      smclk_timer_isr(TRUE,1,RF_GET_WKUP_TIME_3MS);           //保证打开rx的时间是3ms todo
  	  received_tmout = FALSE;
      RAIL_TimerSet(SET_WOR_REC_TIME, RAIL_TIME_DELAY);
      rf->next_cmd = RF_FSM_CMD_SET_RX; 
      break;
      
    case RF_FSM_CMD_SET_RX:                     //接收6字节的set帧
      memset(rf_buf, 0, RF_SET_WAKEUP_PACKET);
      err = rf_cmd_set_rx(rf, rf_buf);
      if (err == RF_ERROR_NONE)
      {
        set_wkup_event(rf, rf_buf[0]);
        rf->next_cmd = RF_FSM_EVENT_SET_HANDLE;
      } 
      else
        rf->next_cmd = set_rx_err(err);
      break;
    case RF_FSM_EVENT_SET_HANDLE:
      /*
      *begin add by zhaoyang for bug  
      *问题描述：grp听帧超时后加入set听帧
      *解决办法：采用主时钟校准的方式
      */  
      set_wor_flag_fun(RF_FSM_CMD_SET_EXIT_LOOP);  //设置状态机为set听帧模式
      /*
      * end
      */
      rf->next_cmd  =  cmd_rf[rf->event](rf_buf);	
      rf->prev_event = rf->event;
      break;
      
      //-----------------------------group 层状态流程---------------------------
      
    case RF_FSM_CMD_GRP_EXIT_LOOP:
      rf_grp_wor(rf);
      set_wor_flag_fun(RF_FSM_CMD_GRP_EXIT_LOOP);
      rf->next_cmd = RF_FSM_CMD_SLEEP; 
      break;
      
    case RF_FSM_CMD_GRP_WAKEUP:
		set_wor_flag_fun(RF_FSM_CMD_GRP_EXIT_LOOP);
		if(gro_wor_times > def_sys_attr.exit_grp_wkup_cont)
		{
			gro_wor_times= 0;
			set_wor_flag_fun(RF_FSM_CMD_SET_EXIT_LOOP);
			rf->next_cmd = RF_FSM_CMD_SET_WAKEUP;
			break;
		}
		rf_grp_wor(rf);
      //smclk_timer_isr(TRUE,1,RF_RP_WKUP_TIME_3MS_HALF);	todo
		received_tmout = FALSE;
		RAIL_TimerSet(SET_WOR_REC_TIME, RAIL_TIME_DELAY);
		gro_wor_times++;
		rf->next_cmd = RF_FSM_CMD_GRP_RX;
		break;
      
    case RF_FSM_CMD_GRP_RX:   
      memset(rf_buf, 0, RF_GRP_WAKEUP_PACKET);
      err = rf_cmd_grp_rx(rf, rf_buf);
      if (err == RF_ERROR_NONE)
      {
        rf->next_cmd =  grp_wkup_event(rf, rf_buf);  
      } 
      else
      {
        rf->next_cmd = grp_rx_err(err);
        /*
        *begin add by zhaoyang for bug  
        *问题描述：grp听帧超时后加入set听帧
        *解决办法：采用主时钟校准的方式
        */ 
        if(rf->next_cmd == RF_FSM_CMD_SLEEP)
          rf->next_cmd  = RF_FSM_CMD_GRP_ADD_SET_RX;
        /*
        *end
        */
      }
      break;
      /*
      *begin add by zhaoyang for bug  
      *问题描述：grp听帧超时后加入set听帧
      *解决办法：采用主时钟校准的方式
      */ 
    case RF_FSM_CMD_GRP_ADD_SET_RX: 
      rf_set_wor(rf,TRUE);
      //smclk_timer_isr(TRUE,1,RF_GET_WKUP_TIME_3MS);           //保证打开rx的时间是3ms todo
      received_tmout = FALSE;
      RAIL_TimerSet(SET_WOR_REC_TIME, RAIL_TIME_DELAY);
      rf->next_cmd = RF_FSM_CMD_SET_RX; 
      break;
      /*
      *end
      */
      //------------------------------------遥控器流程--------------------------
    case RF_FSM_CMD_RC_EXIT_LOOP:                          
      rf_rc_grp_wor(rf);
      def_sys_attr.gwor_flag_now = RF_FSM_CMD_RC_EXIT_LOOP;
      rf->next_cmd = RF_FSM_CMD_SLEEP; 
      
      break;
      
    case RF_FSM_CMD_RC_WAKEUP:    
      def_sys_attr.gwor_flag_now = RF_FSM_CMD_RC_EXIT_LOOP;
      if(rc_wor_times > RC_WOR_CONT)   
      {
        rc_wor_times= 0;
        rf->next_cmd = rc_wor_exit_fun();
        break;
      }
      err = rf_rc_grp_wor(rf); 
      rc_wor_times++;
//      smclk_timer_isr(TRUE,1,RF_GET_WKUP_TIME_3MS);
      received_tmout = FALSE;
  	  RAIL_TimerSet(SET_WOR_REC_TIME, RAIL_TIME_DELAY);
      rf->next_cmd = RF_FSM_CMD_RC_RX; 
      break;
      
    case RF_FSM_CMD_RC_RX:
      memset(rf_buf, 0, RF_GRP_WAKEUP_PACKET);
      err = rf_cmd_grp_rx(rf, rf_buf);
      if (err == RF_ERROR_NONE)
      {
        rf->next_cmd =  rc_wkup_event(rf, rf_buf);  
      } 
      else
        rf->next_cmd = get_rc_err(err);
      break;
      
      //----------------------------------接收帧1流程---------------------------
    case RF_FSM_CONFG_FRAME1_PER:
      rf_frame1_config(rf);
      rf->next_cmd = RF_FSM_CMD_RX_FRAME1;     
      break;
      
    case RF_FSM_CMD_RX_FRAME1:
//      memset(rf_buf , 0, RF_NORMAL_PACKET);
      err = rf_cmd_rx_fram1(rf, rf_buf);
      if (err == RF_ERROR_NONE)
      {
        rf->next_cmd = rx_frame1_event(rf, rf_buf);  
      }
      else
        rf->next_cmd = frame1_rx_err(err);
      break;  
      
      //-------------------------------接收正常的数据流程-----------------------
    case RF_FSM_CMD_RX_DATA_CONFIG:
      err = get_data_rf_config(rf); 
      rf->next_cmd = RF_FSM_CMD_RX_DATA; 
      break;
    case RF_FSM_CMD_RX_DATA:                    //连续接收多个数据包时，是缩小功耗的考虑点，原因是频繁的切换状态在跳转
      err = rf_cmd_rx_data(rf, rf_buf);

      if (err == RF_ERROR_NONE)
      {   
        if(TRUE != crc_normal_pkg_fun(rf_buf))
        {
          rf->next_cmd = RF_FSM_CMD_RX_DATA;
          break;
        }
        
        rx_data_event(rf, rf_buf);
        rf->next_cmd = RF_FSM_EVENT_RX_DATA_HANDLE;
        
      } 
      else
      {
        clear_pkg_nosd_fun();
        rf->next_cmd = rx_data_err(err);
      }
      break;
    case RF_FSM_EVENT_RX_DATA_HANDLE:
      rf->next_cmd  =  cmd_rf[rf->event](rf_buf);	
      rf->prev_event = rf->event;
      break;
      //----------------------------------回ACK正常的数据流程--------------------------------------  
    case RF_FSM_EVENT_TX_ACK_HANDLE:
      rf->next_cmd  = tx_ack_info(rf_buf);
      break;
    case RF_FSM_CMD_TX_DATA_CONFIG:
      ret_ack_rf_config(rf);
      rf->next_cmd = RF_FSM_CMD_TX_DATA; 
      break;
    case RF_FSM_CMD_TX_DATA: 
      err = a7106_tx_data(RF_NORMAL_PACKET, rf_buf);
      if (err == RF_ERROR_NONE)
      {  
#ifdef EPD_TEST_MODE 
        memcpy(g_test_buf+1,rf_buf,RF_NORMAL_PACKET);
        gEventFlag |= EVENT_FLAG_WRITEID;
#endif
        clear_pkg_nosd_fun();
        rf->next_cmd = RF_FSM_CMD_RX_DATA_CONFIG;
      }
      else
      {
        clear_pkg_nosd_fun();
        rf->next_cmd = tx_ack_err(err);
      }
      
      break;
      
      //------------------------------------休眠流程----------------------------      
    case  RF_FSM_CMD_SLEEP:
      clear_pkg_nosd_fun();
      clear_update_flag_fun();
      //smclk_timer_isr(FALSE, 0,0); todo
      //A7106_Cmd(CMD_SLEEP);
	  if (RAIL_TimerIsRunning()) {
		RAIL_TimerCancel();
	  }
      RAIL_RfIdle();
      loop = 0;
      gaclk_times_cont= 0;
      break;
      
    case RF_FSM_CMD_UNKNOWN:
    default:
      rf->next_cmd = RF_FSM_CMD_RF_POR_RST;
      break;
    }
    rf->prev_cmd = rf->cmd;
    rf->cmd = rf->next_cmd;		
  }
  //---------------------
  
  
  return 0;
}

