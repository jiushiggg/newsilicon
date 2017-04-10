#ifndef __HOLTEK_MSP430_SYSCONFIG_H__
#define __HOLTEK_MSP430_SYSCONFIG_H__


#pragma pack(1)
typedef enum {
  RF_A7106_SET_WKUP_MODE, 
  RF_A7106_GRP_WKUP_MODE, 
} RF_WORK_MOD_T;


typedef struct _sys_config
{
  UINT8 set_wkup_time;
  UINT8 grp_wkup_time;
  UINT16 heartbit_time;
  UINT8 work_time;
  
}MCU_DEF_ATTR;


struct SYS_ATTR_T {
  UINT16 attrcrc;
  UINT16 aclk_timer_cycle;
  UINT16 aclk_tmr_wor_cont;
/*begin add by zhaoyang for bug  266 */ 
  UINT16 hbt_cont;
/*end add by zhaoyang for bug  266 */ 
  RF_WORK_MOD_T rf_wk_md;
  UINT8 exit_grp_wkup_cont;
  
  
  RF_CMD_T gwor_flag_before;
  RF_CMD_T gwor_flag_now;
  MCU_DEF_ATTR sys_def_attribute;
};
#pragma pack()
void sys_load_config_info(void);
void save_extern_rf_info(void);
BOOL load_extern_rf_info(void);
BOOL save_sys_config_info(void);
void save_state_info_fun(void);
UINT8 hb_setwor_fun(void);
BOOL save_sys_load_page_info(void);
#endif