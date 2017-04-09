#include "system.h"
//#include "eeprom.h"
#include "crc.h"
//#include "isr.h"
//#include "screen.h"
#include "rf_protocol.h"
#include "bank2.h"
#include <string.h>
#include "udelay.h"


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

static void value_init(void);
static void display_init(void);
static UINT8 check_write(UINT8 addr, UINT8* data, UINT8 n);
#define REMOTE_HB (UINT16)0x01
#define REMOTE_LED (UINT16)0x02
#define TRIGGER_LED	(UINT16)0X04
#define TRANSFER	(UINT16)0X08
#define CHANGE_PAGE1 (UINT16)0X10
#define CHANGE_PAGE2 (UINT16)0X20
#define CHANGE_PAGE3 (UINT16)0X30
#define CHANGE_PAGE4 (UINT16)0X40
#define CHANGE_PAGE5 (UINT16)0X50
#define CHANGE_PAGE6 (UINT16)0X60
#define CHANGE_PAGE7 (UINT16)0X70
#define CHANGE_PAGE8 (UINT16)0X80
#define CHANGE_PAGE_MASK (UINT16)0XF0

//extern volatile UINT16 tm2pcount;

/*
void remote(void)
{
	UINT16 tmp = gcnf.magnit;
	if (tmp == 0){
		_intc0 &= ~(INT0F);			//处理完一次事件后开中断
		_intc0 |= INT0E;		
		return;	
	}
	Delay_MS(100);

	if (SW_IO == SW_FALSE){					//是抖动
		_intc0 &= ~(INT0F);			
		_intc0 |= INT0E;
		return;
	}
	if (TRIGGER_LED&tmp){
#ifdef NEW_PCB
		LEDOFF;	
		LED_PWM_ON;
		Delay_MS(60);
		LEDOFF;	
#else
		LEDON(0X20);
		Delay_MS(30);
		LEDOFF;			
#endif											
	
	}		
	if (REMOTE_HB & tmp){
		st_remote_init();
	}
	if(TRANSFER & tmp){
			gEventFlag |= EVENT_FLAG_RFDATA;
//			brf_set_flg = 0;
			swor_cnt = 10;									
			if (gRF_mode.current_st <= RF_ST_GROUP_TRANSFER){		
				gmode_backup = RF_ST_SET_IDLE;
			}else{
				gmode_backup = RF_ST_GROUP_IDLE;
			}
			gRF_mode.next_st = RF_ST_REMOTE_WOR;			
	}else if(REMOTE_LED & tmp){
		tmp &= REMOTE_LED;
		if(gLEDstate==LED_GLITTER_DISABLE){		//开LED
//			bremote_led = FALSE;
			gLEDstate = LED_INIT;								
			EEPROM_Read(DISPLAY_CNF_ADDR+8,(UINT8 *)&gled, sizeof(stLEDSet));
			led_map();	//led映射关系
			gEventFlag |= EVENT_FLAG_LED;					
		} else {						//关LED
//			bremote_led = TRUE;
			gLEDstate = LED_GLITTER_DISABLE;														
			gEventFlag |= EVENT_FLAG_LED;						
		}
	}else{
		if (CHANGE_PAGE_MASK & tmp){			
			gdisplay_time = gcnf.magnet_display_time;
			gdsp_mode = LCD_REMOTE_BEGIN;				
			gEventFlag |= EVENT_FLAG_LCDUPDATE;	
			_intc0 &= ~(INT0F);			//处理完一次事件后开中断
			_intc0 |= INT0E;	
					
			switch(CHANGE_PAGE_MASK & tmp){
				case CHANGE_PAGE1:		
					gpage_num = 0X00;
					break;
				case CHANGE_PAGE2:		
					gpage_num = 0X01;
					break;
				case CHANGE_PAGE3:	
					gpage_num = 0X02;
					break;
				case CHANGE_PAGE4:		
					gpage_num = 0X03;
					break;
				case CHANGE_PAGE5:		
					gpage_num = 4;
					break;
				case CHANGE_PAGE6:		
					gpage_num = 5;
					break;
				case CHANGE_PAGE7:		
					gpage_num = 6;
					break;
				case CHANGE_PAGE8:		
					gpage_num = 7;		
					break;																							
				default:
					Delay_MS(100);	
					gdsp_mode = LCD_NORMAL;				
					gEventFlag &= ~EVENT_FLAG_LCDUPDATE;														
					break;	
			}		
		}	
	}				
	if (((REMOTE_HB|REMOTE_LED|TRIGGER_LED)&tmp) && 0==(tmp& ~(REMOTE_HB|REMOTE_LED|TRIGGER_LED))){		
		Delay_MS(100);
		_intc0 &= ~(INT0F);			//处理完一次事件后开中断
		_intc0 |= INT0E;	
	}
}
*/


void Delay_MS(UINT8 n)
{
	UINT8 i = 0;
	for (i=0; i<n; i++)
		UDELAY_Delay(1000);
}
void save_sys_status(void) 
{
  UINT8 ret=0, ret1 = 0;
  UINT8 i = 0;
//  UINT16 crc = 0;
    memcpy((UINT8 *)&gRFInitData, (UINT8 *)&bak, sizeof(bak));
  	grf.crc = my_cal_crc16(0,(UINT8 *)&gRFInitData,sizeof(RFINIT));
  	
	for(i = 0 ; i < 3 ;i++) {
		ret = check_write(INFO_ADDR,(UINT8 *)&grf, sizeof(stRFInit));
		ret1 = check_write(INFO_ADDR_BAK,(UINT8 *)&grf, sizeof(stRFInit));
		if (ret & ret1){
			break;
		}
		Delay_MS(5+i*10);
	}
} 

BOOL load_id(UINT16 addr)
{
	UINT16 crc;
	UINT8 i;
	
	//加载系统配置信息到 INFO_DATA中
	for(i = 0 ; i < 3 ;i++){
		crc = 0;
		//memset((UINT8 *)&INFO_DATA,0,sizeof(INFO_DATA));
//		EEPROM_Read(addr, (UINT8*)&grf, sizeof(grf));
		crc =  my_cal_crc16(crc,(UINT8 *)&gRFInitData, sizeof(RFINIT));	
		if (crc == grf.crc){  
			return TRUE;
		}
		Delay_MS(10+i*10);   
	}
	return FALSE;
}
UINT8 load_sys_status(void)
{ 
//	const UINT8 id1[]={0X58,0X00,0X44,0X66, 0X58,0X08,0X2A,0X99,  0x52,0x56,0x78,0x53,200,200,0,200, 0};
	
	if(load_id(INFO_ADDR) || load_id(INFO_ADDR_BAK)){		
		return TRUE;
	}
	return FALSE;
//	if(0 == memcpy((UINT8*)&gRFInitData, (UINT8*)id1, sizeof(id1))){		//for debug
//		return TRUE;
//	}
    
//   while(1);
//   memset((UINT8 *)&gRFInitData,0,sizeof(RFINIT));
//   memcpy(&gRFInitData, id1, sizeof(RFINIT));
}



void eeprom(void)
{
	if (TRUE == gEEPROM){
		gEEPROM = FALSE;
		system_save_data(gsave_flg);
		gEventFlag |= EVENT_FLAG_LCDUPDATE;	
		bscroll = TRUE;
	} 
	else {
		read_data(gsave_flg);
	} 	
	gsave_flg = 0;
			
	bglitter_en = glitter_display_bit();
	bmulti_en = multi_display_bit();	
	if (bmulti_en == TRUE){
		screen_num = 0x01;	
	}											
	 
	if (gled.cnt > 0){		//基站发 遥控器的LED数据
//		led_map();	//led映射关系
		gLEDstate = LED_INIT;
		gEventFlag |= EVENT_FLAG_LED;
	}else {
		gLEDstate = LED_GLITTER_DISABLE;	
		gEventFlag |= EVENT_FLAG_LED;
	}	
}

static UINT8 check_write(UINT8 addr, UINT8* data, UINT8 n)
{
	return TRUE;
/*
	UINT8 i=0, j=0;
	UINT8 *p = NULL;
	for (i=0; i<3; i++){
		EEPROM_Write(addr, data, n);
		EEPROM_Read(addr, (UINT8*)eeprom_buff, n);
		for (j=0, p = data; j<n; j++, p++){
			if (*p != eeprom_buff[j]){
				break;
			}
		}
		
		if (j == n){
			return TRUE;
		}			
	}		
	
	return FALSE;
*/
}


void system_save_data(UINT16 flg)
{
	UINT16 i = 0x01;
	UINT8 j=0, k=0;
	
	for (j=0; j<16 && 0!=flg; j++){
		while (0==(flg&i) && 0!=i){
			i <<= 1;	
		}							
		switch(i){
			case SAVE_SCREEN1:
//				EEPROM_Write(LCD_SCRREN1_ADDR,(UINT8 *)&gdisplay.buff[0], sizeof(gdisplay.buff[0]));			
 				check_write(LCD_SCRREN1_ADDR,(UINT8 *)&gdisplay.buff[0], sizeof(gdisplay.buff[0]));						
				break;
			case SAVE_SCREEN2:
				//EEPROM_Write(LCD_SCRREN2_ADDR,(UINT8 *)&gdisplay.buff[1], sizeof(gdisplay.buff[0]));
				check_write(LCD_SCRREN2_ADDR,(UINT8 *)&gdisplay.buff[1], sizeof(gdisplay.buff[0]));
				break;
			case SAVE_SCREEN3:
				//EEPROM_Write(LCD_SCRREN3_ADDR,(UINT8 *)&gdisplay.buff[2], sizeof(gdisplay.buff[0]));
				check_write(LCD_SCRREN3_ADDR,(UINT8 *)&gdisplay.buff[2], sizeof(gdisplay.buff[0]));
				break;
			case SAVE_SCREEN4:
				//EEPROM_Write(LCD_SCRREN4_ADDR,(UINT8 *)&gdisplay.buff[3], sizeof(gdisplay.buff[0]));
				check_write(LCD_SCRREN4_ADDR,(UINT8 *)&gdisplay.buff[3], sizeof(gdisplay.buff[0]));
				break;
			case SAVE_SCREEN5:
				//EEPROM_Write(LCD_SCRREN5_ADDR,(UINT8 *)&gdisplay1.buff[0], sizeof(gdisplay1.buff[0]));
				check_write(LCD_SCRREN5_ADDR,(UINT8 *)&gdisplay1.buff[0], sizeof(gdisplay1.buff[0]));
				break;
			case SAVE_SCREEN6:
				//EEPROM_Write(LCD_SCRREN6_ADDR,(UINT8 *)&gdisplay1.buff[1], sizeof(gdisplay1.buff[0]));
				check_write(LCD_SCRREN6_ADDR,(UINT8 *)&gdisplay1.buff[1], sizeof(gdisplay1.buff[0]));
				break;
			case SAVE_SCREEN7:
				//EEPROM_Write(LCD_SCRREN7_ADDR,(UINT8 *)&gdisplay1.buff[2], sizeof(gdisplay1.buff[0]));
				check_write(LCD_SCRREN7_ADDR,(UINT8 *)&gdisplay1.buff[2], sizeof(gdisplay1.buff[0]));
				break;
			case SAVE_SCREEN8:
				//EEPROM_Write(LCD_SCRREN8_ADDR,(UINT8 *)&gdisplay1.buff[3], sizeof(gdisplay1.buff[0]));
				check_write(LCD_SCRREN8_ADDR,(UINT8 *)&gdisplay1.buff[3], sizeof(gdisplay1.buff[0]));
				break;
			case SAVE_GLITTER:
				//EEPROM_Write(LCD_GLITTER_ADDR,(UINT8 *)&gglitter_seg, sizeof(gglitter_seg));
				check_write(LCD_GLITTER_ADDR,(UINT8 *)&gglitter_seg, sizeof(gglitter_seg));
				break;				
			case SAVE_DISPLAY_CNF:
				//EEPROM_Write(DISPLAY_CNF_ADDR,(UINT8 *)&gcnf, sizeof(stDisplayCnf));
				check_write(DISPLAY_CNF_ADDR,(UINT8 *)&gcnf, sizeof(stDisplayCnf));
				break;
			case SAVE_SYS_CNF:
				//EEPROM_Write(SYS_CNF_ADDR,(UINT8 *)&gsyscnf, sizeof(gsyscnf));
				check_write(SYS_CNF_ADDR,(UINT8 *)&gsyscnf, sizeof(gsyscnf));
				break;
			case SAVE_DEV_INFO:
				for (k=0; k<3; k++){
					save_sys_status();
				//EEPROM_Write(LCD_GLITTER_ADDR,(UINT8 *)&gsyscnf, sizeof(gsyscnf));
					if (load_sys_status()){
						break;	
					}					
				}

				break;			
			default:
				break;				
		}	
		flg	&= ~i;			//清除位
	}
	
}

void read_data(UINT16 flg)
{
/*
	UINT16 i = 0x01;
	UINT8 j = 0;
	
	for (j=0; j<16 && 0!=flg; j++){
		while (0==(flg&i) && 0!=i){
			i <<= 1;	
		}							
		switch(i){
			case SAVE_SCREEN1:
				EEPROM_Read(LCD_SCRREN1_ADDR,(UINT8 *)&gdisplay.buff[0], sizeof(gdisplay.buff[0]));			
				break;
			case SAVE_SCREEN2:
				EEPROM_Read(LCD_SCRREN2_ADDR,(UINT8 *)&gdisplay.buff[1], sizeof(gdisplay.buff[0]));
				break;
			case SAVE_SCREEN3:
				EEPROM_Read(LCD_SCRREN3_ADDR,(UINT8 *)&gdisplay.buff[2], sizeof(gdisplay.buff[0]));
				break;
			case SAVE_SCREEN4:
				EEPROM_Read(LCD_SCRREN4_ADDR,(UINT8 *)&gdisplay.buff[3], sizeof(gdisplay.buff[0]));
				break;
			case SAVE_SCREEN5:
				EEPROM_Read(LCD_SCRREN5_ADDR,(UINT8 *)&gdisplay1.buff[0], sizeof(gdisplay1.buff[0]));
				break;
			case SAVE_SCREEN6:
				EEPROM_Read(LCD_SCRREN6_ADDR,(UINT8 *)&gdisplay1.buff[1], sizeof(gdisplay1.buff[0]));
				break;
			case SAVE_SCREEN7:
				EEPROM_Read(LCD_SCRREN7_ADDR,(UINT8 *)&gdisplay1.buff[2], sizeof(gdisplay1.buff[0]));
				break;
			case SAVE_SCREEN8:
				EEPROM_Read(LCD_SCRREN8_ADDR,(UINT8 *)&gdisplay1.buff[3], sizeof(gdisplay1.buff[0]));
				break;
			case SAVE_GLITTER:
				EEPROM_Read(LCD_GLITTER_ADDR,(UINT8 *)&gglitter_seg, sizeof(gglitter_seg));
//				gEventFlag |= EVENT_FLAG_LCDUPDATE+EVENT_FLAG_ROLLINGLCD;	//更新显示	
				gEventFlag |= EVENT_FLAG_LCDUPDATE;
				bscroll = TRUE;	
				break;				
			case SAVE_DISPLAY_CNF:
				EEPROM_Read(DISPLAY_CNF_ADDR,(UINT8 *)&gcnf, sizeof(stDisplayCnf));	
//				gEventFlag |= EVENT_FLAG_LCDUPDATE+EVENT_FLAG_ROLLINGLCD;	//更新显示	
				gEventFlag |= EVENT_FLAG_LCDUPDATE;
				bscroll = TRUE;	
				break;
			case SAVE_SYS_CNF:
				EEPROM_Read(SYS_CNF_ADDR,(UINT8 *)&gsyscnf, sizeof(gsyscnf));
				if (gsyscnf.hb_time>465 || 0==gsyscnf.hb_time){
					gsyscnf.hb_time = 90;
				}

				if (gsyscnf.set_wor_period>0x20 || 0==gsyscnf.set_wor_period){
					gsyscnf.set_wor_period=24;
				}			
				break;
			case SAVE_DEV_INFO:
				EEPROM_Read(INFO_ADDR,(UINT8 *)&grf, sizeof(grf));
				break;			
			default:
				break;				
		}	
		flg	&= ~i;			//清除位
	}
*/
}

static void display_init(void)
{
	UINT8 i=0, j=0;
	UINT8 *p = NULL;	
	//LCD
	for(j=0; j<4; j++){										
		for (i=0; i<20; i++){		
			gdisplay.buff[j][i] = 0xff;	
			gdisplay1.buff[j][i] = 0xff;		
		}		
	} 	

	for (i=0; i<20; i++){
		gglitter_seg[i] = 0x00;	
	}
	gcnf.rollingtm[0] = 0x04;
	//屏翻页时间
	for (i=1; i<4; i++){
		gcnf.rollingtm[i] = 0;	
	}
	for (p=(UINT8*)&grf; p<sizeof(grf)+(UINT8*)&grf; p++){		
		*p = 0xff;			
	}		
	//配置信息
	gcnf.security_code = 0;
	gcnf.led.cnt = 20;
	gcnf.led.num = 0x01;
	gcnf.led.off1 = 1;
	gcnf.led.off2 = 30;
	gcnf.led.on = 1;	
	gcnf.magnit = 0;	
	gcnf.all_crc = all_crc16();	
	
	//系统配置
	gsyscnf.set_wor_period = SET_WOR_PERIOD;	//set WOR听帧周期
	gsyscnf.hb_time = HB_PERIOD;						//心跳间隔
	
	gsave_flg = 0x7ff;
	gEEPROM = TRUE;
	gEventFlag |= EVENT_FLAG_EEPROMUPDATE;
}

static void value_init(void)
{
	//-----------bank0--------------
	gEventFlag = 0;
	var_0.var = 0;
	var_1.var = 0;
	var_2.var = 0;
	bheartbeat_en = TRUE;
//	bremote_led = TRUE;	
	//RF
	gRF_mode.current_st = RF_ST_INIT;
	gRF_mode.next_st = RF_ST_SET_WOR;
	gRF_mode.error = RF_ERROR_NONE;
	gRF_mode.ack = 0;
	gmode_backup = 	RF_ST_BACKUP;

	
	grp_wor_period = GROUP_WOR_PERIOD;	//wor听帧周期
	work_time = 2;						//开RX时间
		
	gwor_cnt = GROUP_WOR_MAX_VALUE;		//group wor 听帧计数
	gsyscnf.set_wor_period = SET_WOR_PERIOD;
	//lcd
	screen_num = 0x01;
	gbt_trigger = 0;
	glcd_config = 0;
	
	//led
	gLEDstate = LED_GLITTER_DISABLE;
	
	//-----------bank2--------------
	gvol_value = 0X07;		
	//-----------bank3--------------
	gdisplay_time = 4;
	gdsp_mode = LCD_NORMAL;
}

void System_Init(void)
{	
	UINT8 tmp = 0xff, i=0;
	
	value_init();
	for (i=0; i<3; i++){
//		EEPROM_Read(0x2f,&tmp, 1);
		if (tmp != 0x55){
			continue;	
		}else{
			break;	
		}
	}
	if (0x55 != tmp){
		display_init();
		tmp = 0x55;
//		EEPROM_Write(0x2f,&tmp, 1);
	}else{
		read_data(0xfff);	
	}
	if (gsyscnf.set_wor_period > 0X20){
		gsyscnf.set_wor_period = SET_WOR_PERIOD;	//set WOR听帧周期
		grp_wor_period = GROUP_WOR_PERIOD;	//wor听帧周期
		work_time = 2;						//开RX时间
		gsyscnf.hb_time = HB_PERIOD;						//心跳间隔
	}		
	
	bmulti_en = multi_display_bit();
	bglitter_en = glitter_display_bit();
	System_ClrSleepISR();
}

void System_EnterSleep(void)
{	
/*begin 2016/11/25 add by gaolongfei for bug 221 */
	if (FALSE == bwtd){
//		ISR_set_timer_ccrp(TRUE, tm2pcount, LIRC_2S);
	}		
/*end 2016/11/25 add by gaolongfei for bug 221 */	
/*
	do{	
		IDLE0_MODE();		
		GCC_DELAY(16);
		NOMAL_MODE();		
	}while(0 == gEventFlag);
*/
}
void System_ClrSleepISR(void)
{
//	_int0f=0;
//	_int1f=0;
}
UINT8 BitNumber(UINT16 x)
{
   UINT8 n;
   for(n=0; x; n++)
      x &= x-1;
    return n;
}

UINT8 multi_display_bit(void)
{
	UINT8 n = 0, i = 0x01, j = 0x10 ;
	gdisplay_cnf = 0;
	for(n=0; n<4; n++){
		if (0 != (gcnf.rollingtm[n] & 0x0f)){
			gdisplay_cnf |= i;
		} 
		i <<= 1;
		if (0 != (gcnf.rollingtm[n] & 0xf0)){
			gdisplay_cnf |= j;
		} 
		j <<= 1;
	}
	if (BitNumber(gdisplay_cnf) > 1){
		return TRUE;	
	} else{
		return FALSE;	
	}
}
BOOL glitter_display_bit(void) 
{
	UINT8 i = 0;
	for (i=0; i<sizeof(gglitter_seg); i++){
		if (0 != gglitter_seg[i]){
			return TRUE;
		}			
	}
	return FALSE;		
}

UINT8 hb_setwor_fun(void)
{
  UINT8 ret =5;
  
  if( (gsyscnf.set_wor_period == 0) || ( (gsyscnf.set_wor_period %2) !=0 ) ||(gsyscnf.set_wor_period > 32))
  {
    ret = 7;
    goto loop;
  }
  
  if(gsyscnf.set_wor_period == 1 )
  {
    ret = 0;
  }
  else if(gsyscnf.set_wor_period == 2)
  {
    ret = 1;
  }
  else if((gsyscnf.set_wor_period >2) && (gsyscnf.set_wor_period <=4))
  {
    ret = 2;
  }
  else if((gsyscnf.set_wor_period >4) && (gsyscnf.set_wor_period <=8))
  {
    ret = 3;
  }
  else if((gsyscnf.set_wor_period >8) && (gsyscnf.set_wor_period <=16))
  {
    ret = 4;
  }
  else if((gsyscnf.set_wor_period >16) && (gsyscnf.set_wor_period <=24))
  {
    ret = 5;
  }
  else 
  {
    ret = 6;
  }
  
loop:
  ret =  ret<<5;
  ret |= 0x08;
  return ret;
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
