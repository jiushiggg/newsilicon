//#include "msp430g2755.h"
#include "datatype.h"
#include "..\global_variable\global_variable.h"
#include "sys_init.h"
//#include "rf_a7106.h"
#include "sys_isr.h"
//#include "rf.h"
#include "reed.h"
//#include "write_id.h"
//#include "nfc_iic.h"
#include <string.h>



#if 0
#define SMCLK_500MS 	((UINT16)62500)
void sys_wakeup_wait(UINT16 tm)
{
  UINT16 count;
  
  clock_1M_set();
  A7106_SLEEP();
  
  do {
    if (tm <1){
      break;
    } else if (tm < 100){
      count = tm * ((UINT16)625) - 1000;//5*125
      tm = 0;
    }else {
      count = SMCLK_500MS;
      tm -= 100; 
    } 
    smclk_timer_isr(TRUE,1,count);   
    SetWathcDog();
    sys_sleepwait();
    
    //   SetISR_RFException1(FALSE, 0);	 
  } while(tm > 0);
  smclk_timer_isr(FALSE,0,0); 
  clock_8M_set();
}
#else
#define SMCLK_1M_DIV8_500MS 	((UINT16)62500)	//
#define SMCLK_8M_DIV8_5MS	((UINT16)5000)
#define SMCLK_8M_DIV8_10MS	((UINT16)10000)
#define SMCLK_8M_DIV8_50MS	((UINT16)50000-300)//
void sys_wakeup_wait(UINT32 tm)
{
/*		todo
#define TIME_10MS_REDUNDANCY (55)
  UINT32 count;
  
  if(tm <= 2)
    goto loop;
  else
    tm-=2;
  do 
  {
    
    if(tm < 5)
    {
      count = (1000*10*tm-TIME_10MS_REDUNDANCY) ;//500/62500 = 5tm/count => count=tm*5*125 - (4+4+2)ms/8
      tm = 0;
    }
    else 
    {
      count = SMCLK_8M_DIV8_50MS;
      tm -= 5; 
    } 
    if(count > 0)
    {
      smclk_timer_isr(TRUE, 1, count);   
      SetWathcDog();
      sys_sleepwait();   
      smclk_timer_isr(FALSE, 0,0);	
    } 
  } while(tm > 0);
loop:
  gEventFlag &= (~EVENT_FLAG_LTIMER);
*/
	RTCDRV_Delay(tm*10);
}
#endif 

/*-------------------------------ϵͳ��ʱ�Ӷ�ʱ��----------------*/
/*
#pragma vector=TIMER0_A0_VECTOR

__interrupt void Timer_A (void)
{ 
  if(smclk_isr_times == 0)
    smclk_isr_times = 1;
  smclk_isr_times--;
  if (smclk_isr_times == 0) 
  {
    LPM0_EXIT;
    gEventFlag |= EVENT_FLAG_LTIMER;
    CCR0 = 0;//ͣ��ʱ��
    TACTL |= TACLR;//��λ������
    CCTL0 = 0;
    TACTL = 0;
    if(g_timer_tp.flag_600ms == TRUE)
    {
      g_timer_tp.alk_600ms_cont =  TBR;
      g_timer_tp.flag_600ms = FALSE;
    }
  }
}
*/

/**
��ʱn*t ms
*/
VOID smclk_timer_isr(BOOL bOnOff, UINT16 times, UINT16 cycle)
{
/*
  if(bOnOff)
  {
    if (times == 0)
      return;
    smclk_isr_times = times;
    CCR0 = cycle;                              
    TACTL = TASSEL_2 + ID_3 + MC_1;  // SMCLK, up, 1/8 
    CCTL0 = CCIE; // CCR0 interrupt enabled
    _BIS_SR(GIE); // ��ȫ���ж�
  } 
  else 
  { 
    CCR0 = 0;//ͣ��ʱ��
    TACTL |= TACLR;//��λ������
    CCTL0 = 0;
    TACTL = 0;
    smclk_isr_times = 0;
    gEventFlag &= (~EVENT_FLAG_LTIMER); 
  }
*/
}


/*-----------------------12k aclk timers-------------*/


/*
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer_A1 (void)
{ 
  if(ledclk_isr_times == 0)
    ledclk_isr_times =1;
  ledclk_isr_times --;
  if(ledclk_isr_times == 0)
  {
    //sleepʱ��ʱ�ж�ִ�� 
    LPM3_EXIT;                               //�Ѿ�������MCU    
    TA1CCR0 = 0;//ͣ��ʱ��
    TA1CTL |= TACLR;//��λ������
    TA1CCTL0 = 0;
    TA1CTL = 0;
    gEventFlag |= EVENT_FLAG_INTER_LED;
    ledclk_isr_times = 0;
    //led_control_fun();
  }
}
*/


VOID set_led_timer(BOOL bOnOff,UINT16 times, UINT16 n)
{
/*
  if(bOnOff) 
  {
    if (n == 0)
      n = LED_30MS_TIME_CONT;//30ms
    
    ledclk_isr_times = times;
    TA1CCR0 = n;      //  11250  -> 1min      15s==  2411   17.77s=2693   
    TA1CTL = TASSEL_1 + ID_0 + MC_1;  // ACLK=12k, up,  
    TA1CCTL0 = CCIE; // CCR0 interrupt enabled
    _BIS_SR(GIE); // ��ȫ���ж�
  } else {
    TA1CCR0 = 0;//ͣ��ʱ��
    TA1CTL |= TACLR;//��λ������
    TA1CCTL0 = 0;
    TA1CTL = 0;
    gEventFlag &= (~EVENT_FLAG_INTER_LED);
  }
*/
}




//�ⲿ�ж϶���
/*---------------------GPIO1��WTR�ж�-------------------*/
/*
#pragma vector=PORT2_VECTOR
__interrupt void port2_interrupt(void) 
{ 

  if(WRITEID_INTERRUPT_READY)
  {
    WRITEID_ISR_CLOSE;
    gEventFlag |= EVENT_FLAG_WRITEID; 
    
  }
#ifdef NFC_ENABLE 
  if(NFC_INT_INTERRUPT_READY)
  {
    NFC_INT_INTERRUPT_CLOSE;
    gEventFlag |= EVENT_FLAG_NCF_CHIP; 
  }
#endif
  LPM3_EXIT;
}
*/

BOOL isr_event_response_fun(void)
{
/*  //todo
  if(rf_read_gpio2_err() || gRFError)
    while(1);
*/
  //�������ȼ�����
  if(gaclk_times_cont >= (def_sys_attr.aclk_tmr_wor_cont))
  {
    gEventFlag |= EVENT_FLAG_RFWORK;
    gaclk_times_cont= 0;
    sys_wait_time_set = 0;  
  }
  //�ɻɹܡ�LED�ơ���Ļbusy����дid��nfc
  if((gEslHearcont >= def_sys_attr.hbt_cont) && ( hbr_mode == NORMAL_HEARTBEAT))
  {
    gEventFlag |= EVENT_FLAG_SYS_HEART;
    gEslHearcont = 0;
  }

  if((rqst_hbr_info.cont >= rqst_hbr_info.hbt_cont) && ( hbr_mode == REQUST_HEARTBEAT))
  {
    gEventFlag |= EVENT_FLAG_QUEST_HEART;
    rqst_hbr_info.cont =0;
    rqst_hbr_info.now ++;
  }
  if((gpage.flag == TRUE) && (gSys_tp.stay_time_cont >= gSys_tp.stay_time/2) &&( !epd_scn_tp.screen_busy_flag))//��ͨ���ݰ���ҳ
  {
    gSys_tp.present_page_id = gSys_tp.default_page_id;//Ҫ��ʾ��ҳ==Ĭ��ҳ
    gEventFlag |= EVENT_FALG_DISPLAY_PAGE;
  }
  
  if(gEventFlag != 0)
    return TRUE;
  
  return FALSE;
}


//---------------------�ɻɹ��ж�--------------------------
/*
#pragma vector=PORT1_VECTOR
__interrupt void port1_interrupt(void) { 
  
  if (REED_IO_INTERRUPT_READY)
  {  
    REED_IO_INTERRUPT_CLOSE;  
    gEventFlag |= EVENT_FLAG_INTER_REED; 
  }
  LPM3_EXIT;
  
}
*/
//-----------------Timeb-------------------------

/*
#pragma vector=TIMERB0_VECTOR
__interrupt void Timer_B (void)
{ 
  if( def_sys_attr.gwor_flag_now != RF_FSM_CMD_RC_EXIT_LOOP)
  {
    gEslHearcont += tm_add_value;
    rqst_hbr_info.cont++;//����ң�����£���Ҫ��ԭ����ģʽ
    if(gpage.flag == TRUE)//������仰��Ŀ���ǵ���set�������ң����������ʱ���˴�ʱ��˳��
      gSys_tp.stay_time_cont += tm_add_value;
  }
  if(epd_scn_tp.screen_busy_flag == FALSE)
  {  
    gaclk_times_cont += tm_add_value;
#ifdef  SCREEN_RELOAD_OPEN
    g_screen_repeat_dis_time += tm_add_value;
    if(g_screen_repeat_dis_time >= SCREEN_DSIPLAY_RELOAD_CONT )
    {   
      gEventFlag |= EVENT_FLAG_EPD_DISPLAY;
    }
#endif
  }
  else
    gEventFlag |= EVENT_FLAG_SCREEN_BUSY; 
  LPM3_EXIT;
  
}
*/
void slaveTimerExpiredCallback( RTCDRV_TimerID_t id, void *incomingPacket)
{
	  if( def_sys_attr.gwor_flag_now != RF_FSM_CMD_RC_EXIT_LOOP)
	  {
	    gEslHearcont += tm_add_value;
	    rqst_hbr_info.cont++;//����ң�����£���Ҫ��ԭ����ģʽ
	    if(gpage.flag == TRUE)//������仰��Ŀ���ǵ���set�������ң����������ʱ���˴�ʱ��˳��
	      gSys_tp.stay_time_cont += tm_add_value;
	  }
	  if(epd_scn_tp.screen_busy_flag == FALSE)
	  {
	    gaclk_times_cont += tm_add_value;
	#ifdef  SCREEN_RELOAD_OPEN
	    g_screen_repeat_dis_time += tm_add_value;
	    if(g_screen_repeat_dis_time >= SCREEN_DSIPLAY_RELOAD_CONT )
	    {
	      gEventFlag |= EVENT_FLAG_EPD_DISPLAY;
	    }
	#endif
	  }
	  else
	    gEventFlag |= EVENT_FLAG_SCREEN_BUSY;

}
void time_wdg_fun(BOOL bOnOff, UINT16 n)
{
/*
  if(bOnOff) 
  {
    sys_timer_2s_open_flag = FALSE;
    if (n == 0)
      return;
    TBCCR0 = n;      
    TBCTL = TBSSEL_1 + ID_0 + MC_1;  // ACLK=12k/8, up,  
    TBCCTL0 = CCIE; // CCR0 interrupt enabled
    _BIS_SR(GIE); // ��ȫ���ж�
  } 
  else 
  {
    sys_timer_2s_open_flag = TRUE;
    TBCCR0 = 0;//ͣ��ʱ��
    TBCTL |= TBCLR;//��λ������
    TBCCTL0 = 0;
    TBCTL = 0;
  }
*/
	if(bOnOff)
	{
		sys_timer_2s_open_flag = FALSE;
		RTCDRV_StartTimer(slaveRtcId, rtcdrvTimerTypePeriodic, n, slaveTimerExpiredCallback, NULL);
	}
	else
	{
		sys_timer_2s_open_flag = TRUE;
		RTCDRV_IsRunning(slaveRtcId, &slaveRtcRunning);
		if (slaveRtcRunning) {
			RTCDRV_StopTimer(slaveRtcId);
		}
	}
}

/*
*begin add by zhaoyang for bug 352 
*����������ʱ�Ӳ�׼
*����취��������ʱ��У׼�ķ�ʽ
*/ 

//-----------------У׼ʱ�Ӷ�ʱ��---------------




void dst_timer_600ms_fun(BOOL bOnOff, UINT16 n)
{
/*
  if(bOnOff) 
  {
    g_timer_tp.flag_600ms = TRUE;
    TBCCR0 = n;      
    TBCTL = TBSSEL_1 + ID_0 + MC_1;  // ACLK=12k/8, up,  
    TBCCTL0 = CCIE; // CCR0 interrupt enabled
    _BIS_SR(GIE); // ��ȫ���ж�
  } 
  else 
  {
    TBCCR0 = 0;//ͣ��ʱ��
    TBCTL |= TBCLR;//��λ������
    TBCCTL0 = 0;
    TBCTL = 0;
    g_timer_tp.flag_600ms = FALSE;
  }
 */
}


VOID src_timer_smclk_fun(BOOL bOnOff, UINT16 times, UINT16 cycle)
{
/*
  if(bOnOff)
  {
    smclk_isr_times = times;
    CCR0 = cycle;                              
    TACTL = TASSEL_2 + ID_3 + MC_1;  // SMCLK, up, 1/8 
    CCTL0 = CCIE; // CCR0 interrupt enabled
    _BIS_SR(GIE); // ��ȫ���ж�
  } 
  else 
  { 
    CCR0 = 0;//ͣ��ʱ��
    TACTL |= TACLR;//��λ������
    CCTL0 = 0;
    TACTL = 0;
    smclk_isr_times = 0;
    gEventFlag &= (~EVENT_FLAG_LTIMER); 
  }
*/
}
BOOL timer_cont_calibration_check(void)
{
  if( (g_timer_tp.led_30ms_cont <15) || (g_timer_tp.led_30ms_cont > 75) )
    return FALSE;
  if( (g_timer_tp.rc_200ms_cont < 100) || (g_timer_tp.rc_200ms_cont >500) )
    return FALSE;
  if((g_timer_tp.wor_2s_cont < 1000) || (g_timer_tp.wor_2s_cont >5000))
    return FALSE;
  return TRUE;
}

BOOL timer_600ms_cont_check(UINT16 tp1,UINT16 tp2)
{
  UINT16 ret = 0;
  
  if(tp1 >= tp2)
    ret = tp1 - tp2;
  else
    ret = tp2 - tp1;
  
  if(ret >=  9 )//12k/8 -- 1cont=666us--6.6ms��Ϊ�Ǵ��� ����ʱ�ӵ�1%
  {
    return FALSE;
  }
  return TRUE;
}

void timer_calb_retry(void)
{
  memset((UINT8 *)&g_timer_tp,0x00,sizeof(g_timer_tp));
  src_timer_smclk_fun(FALSE,0,0);
  dst_timer_600ms_fun(FALSE,0); 
  src_timer_smclk_fun(TRUE,12,TIMER_SMCLK_50MS_CONT);
  dst_timer_600ms_fun(TRUE,65535);
  while(g_timer_tp.flag_600ms);
  src_timer_smclk_fun(FALSE,0,0);
  dst_timer_600ms_fun(FALSE,0);
}

BOOL timer_calibration_fun(void)
{
  UINT16 alk_600ms_cont_tp1 = 0;
  UINT16 alk_600ms_cont_tp2 = 0;
  led_all_off();
  //-----�ر����ж�ʱ��---------
  time_wdg_fun(FALSE,0);
  set_led_timer(FALSE,0,0);
  smclk_timer_isr(FALSE,0,0);
  //--------end-----------------
  //��һ��600msУ׼
  timer_calb_retry();
  alk_600ms_cont_tp1 = g_timer_tp.alk_600ms_cont;
  //�ڶ���600msУ׼
  timer_calb_retry();
  alk_600ms_cont_tp2 = g_timer_tp.alk_600ms_cont;
  
  //-----�ر����ж�ʱ��---------
  time_wdg_fun(FALSE,0);
  set_led_timer(FALSE,0,0);
  smclk_timer_isr(FALSE,0,0);
  //--------end-----------------
  
  if(TRUE == timer_600ms_cont_check(alk_600ms_cont_tp1,alk_600ms_cont_tp2))
    alk_600ms_cont_tp2 = ((alk_600ms_cont_tp2 + alk_600ms_cont_tp1)>>1)  ;//��ƽ��ֵ
  else
    return FALSE;
  
  if( (alk_600ms_cont_tp1 < 300) || (alk_600ms_cont_tp1 >1500) )
    return FALSE;
  
  g_timer_tp.led_30ms_cont = alk_600ms_cont_tp2 / 20 ;
  g_timer_tp.rc_200ms_cont = alk_600ms_cont_tp2 / 3 ;
  g_timer_tp.wor_2s_cont = g_timer_tp.rc_200ms_cont * 10 ;
  
  return TRUE;
  
//  if(FALSE == timer_cont_calibration_check())
//    return FALSE;
//  else
//    return TRUE;
}

void timer_cont_rst(void)
{
  g_timer_tp.led_30ms_cont = LED_30MS_TIME_CONT;
  g_timer_tp.wor_2s_cont = WOR_TIME_COUNT_2S;
  g_timer_tp.rc_200ms_cont = WOR_TIME_COUNT_200MS;
}
/*end add by zhaoyang for bug 352 */


//--------------------------δ�õ��ж�---------------------
/*
typedef void(*t_function)() ;
#define BOOT_RST_ADDR       (0xfffe)    
volatile t_function  rst_fun = (t_function)BOOT_RST_ADDR;
*/
/*
void my_while(void)
{
  while(1);
}       

#pragma vector=TIMER1_A1_VECTOR
__interrupt void intec_0 (void)
{ 
  my_while();
} 
*/
/*
#pragma vector=TIMER1_A0_VECTOR
__interrupt void intec_1 (void)
{ 
asm("br & 0xEFE2;");
}
*/ 

/*
#pragma vector=PORT1_VECTOR
__interrupt void intec_2 (void)
{ 
asm("br & 0xEFE4;");
}
*/

/*
#pragma vector=PORT2_VECTOR
__interrupt void intec_3 (void)
{ 
asm("br & 0xEFE6;");
}
*/
/*
#pragma vector=TRAPINT_VECTOR
__interrupt void intec_4 (void)
{ 
  my_while();
}

#pragma vector=ADC10_VECTOR
__interrupt void intec_5 (void)
{ 
  my_while();
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void intec_6 (void)
{ 
  my_while();
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void intec_7 (void)
{ 
  my_while();
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void intec_8 (void)
{ 
  my_while();
}
*/
/*
#pragma vector=TIMER0_A0_VECTOR
__interrupt void intec_9 (void)
{ 
asm("br & 0xEFF2;");
}
*/
/*
#pragma vector=WDT_VECTOR
__interrupt void intec_10 (void)
{ 
  my_while();
}

#pragma vector=COMPARATORA_VECTOR
__interrupt void intec_11 (void)
{ 
  my_while();
}

#pragma vector=TIMERB1_VECTOR
__interrupt void intec_12 (void)
{ 
  my_while();
}
*/
/*
#pragma vector=TIMERB0_VECTOR
__interrupt void intec_13 (void)
{ 
asm("br & 0xEFFA;");
}
*/
/*
#pragma vector=NMI_VECTOR
__interrupt void intec_14 (void)
{ 
  my_while();
}
*/

























