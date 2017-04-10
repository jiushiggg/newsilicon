#include "delay.h"
#include <string.h>
#include "mini_fs.h"
#include "epd_choose.h"
#include "epd_io.h"
#include "sys_init.h"
#include "global_variable.h" 
#include "sys_isr.h"


#define PSR         0x00
#define PWR         0x01
#define POF         0x02
#define PFS         0x03
#define PON         0x04
#define BTST        0x06
#define DSLP        0x07
#define DTM1        0x10
#define DSP         0x11
#define DRF         0x12

#define PLL         0x30
#define TSC         0x40
#define TSE         0x41

#define CDI         0x50
#define LPD         0x51
#define TCON        0x60
#define TRES        0x61
#define DAM         0x65
#define REV         0x70
#define FLG         0x71
#define AMV         0x80
#define VV          0x81
#define VDCS        0x82


#ifdef WF_JX_75_RED

void EPD_IC_Rst(void)
{
  EPD_W21_RST_1;
  Delay_us(300);
  EPD_W21_RST_0;		
  Delay_us(300);
  EPD_W21_RST_1;
  Delay_us(300);
}
static void SPI_Delay(UINT8 xrate)
{
  ;
}
static void SPI_Write(UINT8 value){                                                           
  UINT8 i;
  
  
  SPI_Delay(1);
  for(i = 0; i < 8; i++)
  {
    EPD_W21_CLK_0;
    SPI_Delay(2);
    
    if(value & 0x80)
      EPD_W21_MOSI_1;
    else
      EPD_W21_MOSI_0;		
    value = (value << 1); 	
    SPI_Delay(2);
    __NOP();
    __NOP();
    EPD_W21_CLK_1; 
    SPI_Delay(2);
  }
}

static void EPD_W21_WriteCMD(UINT8 command){
  
  SPI_Delay(1);
  EPD_W21_CS_0;                   
  EPD_W21_DC_0;		// command write
  SPI_Write(command);
  EPD_W21_CS_1;
}
static void EPD_W21_WriteDATA(UINT8 command)
{
  
  SPI_Delay(1);
  EPD_W21_CS_0;                   
  EPD_W21_DC_1;		// command write
  SPI_Write(command);
  EPD_W21_CS_1;
}
/*
static void EPD_W21_WriteDATA_led(UINT8 command)
{

SPI_Delay(1);
EPD_W21_CS_0;                   
EPD_W21_DC_1;		// command write
send_2byte(command);
EPD_W21_CS_1;
}
*/
/*static UINT8 EPD_W21_ReadDATA(void)
{

  UINT8 i,j; 

  EPD_W21_MOSI_0;
  EPD_WF_SDA_IN;
  EPD_W21_CS_0; 
  EPD_W21_DC_1;	 // command write 
  j=0; 
  SPI_Delay(2); 
  for(i=0; i<8; i++) 
  { 
  EPD_W21_CLK_0; 
  SPI_Delay(20); 
  j=(j<<1); 
  if(EPD_READ_SDA) 
  j|=0x01; 
      else 
  j&=0xfe;	
  SPI_Delay(5); 

  EPD_W21_CLK_1; 
  SPI_Delay(5); 
    } 
  EPD_W21_CS_1; 
  EPD_WF_SDA_OUT;
  return(j); 
}*/
static void lcd_chkstatus(void)
{
  
#define WAIT_COUNT	80	//160s超时
  SetWathcDog();
  UINT8 i=WAIT_COUNT;
  
  SetWathcDog();  
  
  do
  {
    sys_wakeup_wait(400); 
    i--;
  }
  while(!(isEPD_W21_BUSY) && (0 != i) ); 
  SetWathcDog();  
  if(i == 0)
    while(1);
  
}

static void EPD_750red_data(UINT8 bw_data,UINT8 red_data)
{
  UINT8 i=6,j,val1,val2;
  for(j = 0 ; j < 4 ; j++)
  {
    val1 = bw_data >> i;
    val1 = (val1 & 0x03);
    switch(val1)
    {
    case 0x00: val2 = 0x00;break;           //显示黑色
    case 0x01: val2 = 0x03;break;
    case 0x02: val2 = 0x30;break;
    case 0x03: val2 = 0x33;break;
    }
    
    val1 = red_data >> i;
    val1 = (val1 & 0x03);
    switch(val1)
    {
    case 0x00:
      val2 = 0x44;
      break;           //显示黑色
    case 0x01: 
      val2 &= 0x0f;
      val2 |= 0x40;
      break;
    case 0x02:
      val2 &= 0xf0;
      val2 |= 0x04;
      break;
    case 0x03: 
      
      break;
    }
    
    i -= 2;
    EPD_W21_WriteDATA(val2);
  }
}


static void Ultrachip_new(file_id_t fd)				
{
  UINT16 i,j,k;
  UINT8  *data = TEMP.BYTE64,*data_red = TEMP.BYTE64+28;
  
  SetWathcDog(); 
  //显示图片
  
  for( i=384; i >0 && i<=384; i--)
  {
    SetWathcDog(); 
    
    for(k=0;k<4;k++)
    {
      f_read(F_BMP_BW, (i-1) * (80) + (k*20) ,data, 20); 
      f_read(F_BMP_RED, (i-1) * (80) + (k*20) ,data_red, 20); 
      
      for(j = 0; j < 20; j++)
      {
        EPD_750red_data(data[j],data_red[j]);
//        EPD_750red_data(0X0F,0XF0);
      }
    }
  }
  
}
static void MCU_write_flash(unsigned char command)
{       
  unsigned char i ;
  for (i=0;i<8;i++)	 
  {
    EPD_W21_CLK_0;          //		SCK = 0;                                   
    if (command&0x80)
      EPD_W21_MOSI_1;       //		SDA = 1;
    else          
      EPD_W21_MOSI_0;      //		SDA = 0;		
    command <<= 1;
    EPD_W21_CLK_1;         //		SCK = 1;
  }
  EPD_W21_CLK_0;       	  //	       SCK = 0;                            	   	
}
static void EPD_W21_Init(void){
  
  epd_spi_init();
  
  EPD_W21_BS_0;		// 4 wire spi mode selected
  EPD_W21_RST_0;		// Module reset
  Delay_ms(100);
  EPD_W21_RST_1;
  Delay_ms(100);	
  
  EPD_W21_WriteCMD(0x06);         //booster  set
  EPD_W21_WriteDATA (0xc7);	   	
  EPD_W21_WriteDATA (0xcc);
  EPD_W21_WriteDATA (0x28);
  
  EPD_W21_WriteCMD(0X65);			//FLASH CONTROL
  EPD_W21_WriteDATA(0x01);
  
  EPD_W21_CS1_0;					//MFCSB à-μí
  MCU_write_flash(0xAB);
  EPD_W21_CS1_1;					//MFCSB à-??
  
  EPD_W21_WriteCMD(0X65);			//FLASH CONTROL
  EPD_W21_WriteDATA(0x00);
  /**********************************release flash sleep**********************************/	
  EPD_W21_WriteCMD(0x01); 
  EPD_W21_WriteDATA (0x37);	    //POWER SETTING
  EPD_W21_WriteDATA (0x00);
  //	 	EPD_W21_WriteDATA (0x3c);
  //		EPD_W21_WriteDATA (0x3c);
  
  EPD_W21_WriteCMD(0x04);	 	    //POWER ON		 
  lcd_chkstatus();
  
  EPD_W21_WriteCMD(0X00);			//PANNEL SETTING
  EPD_W21_WriteDATA(0xCF);
  EPD_W21_WriteDATA(0x00);
  
  EPD_W21_WriteCMD(0x30);			//PLL setting
  EPD_W21_WriteDATA (0x3a);        
  
//////  EPD_W21_WriteCMD(0X41);			//TEMPERATURE SETTING
//////  EPD_W21_WriteDATA(0x00);
  
  EPD_W21_WriteCMD(0X50);			//VCOM AND DATA INTERVAL SETTING
  EPD_W21_WriteDATA(0x77);
  
  EPD_W21_WriteCMD(0X60);			//TCON SETTING
  EPD_W21_WriteDATA(0x22);
  
  EPD_W21_WriteCMD(0x61);        	//tres
  EPD_W21_WriteDATA (0x02);		//source 640
  EPD_W21_WriteDATA (0x80);
  EPD_W21_WriteDATA (0x01);		//gate 384
  EPD_W21_WriteDATA (0x80);
  
  EPD_W21_WriteCMD(0X82);			//VDCS SETTING
  EPD_W21_WriteDATA(0x26);		//decide by LUT file
  
  EPD_W21_WriteCMD(0xe5);			//FLASH MODE		   	
  EPD_W21_WriteDATA(0x03);	
  EPD_W21_WriteCMD(0x10);	       //?aê?′?ê?í???
  
}

//extern UINT8 chip_spi_flag;
//static void my_init_epd(BOOL test) {
//  open_epd();
//  EPD_W21_Init();
//  SetWathcDog(); 
//  
//  Ultrachip_new(F_BMP_BW);
//  SetWathcDog(); 
//  
//  EPD_W21_WriteCMD(0x04);	 	    //POWER ON		 
//  lcd_chkstatus();
//  
//  EPD_W21_WriteCMD(0x12); 
//  Delay_ms(100);
//  lcd_chkstatus();
//  
//  /**********************************flash sleep**********************************/
//  EPD_W21_WriteCMD(0X65);			//FLASH CONTROL
//  EPD_W21_WriteDATA(0x01);
//  
//  EPD_W21_CS1_0;					//MFCSB à-μí
//  MCU_write_flash(0xB9);
//  EPD_W21_CS1_1;					//MFCSB à-??
//  
//  EPD_W21_WriteCMD(0X65);			//FLASH CONTROL
//  EPD_W21_WriteDATA(0x00);
//  /**********************************flash sleep**********************************/	
//  
//  EPD_W21_WriteCMD(0x02);
//  lcd_chkstatus();
//  
//  EPD_W21_WriteCMD(0x07);
//  EPD_W21_WriteDATA(0xa5);
//  
//  
//}
void Init_EPD(void) {
  open_epd(); 
  EPD_W21_Init();
  SetWathcDog();  
  Ultrachip_new(F_BMP_BW);
  SetWathcDog(); 
 
  EPD_W21_WriteCMD(0x12); 
}

void screen_close(void)
{
    
  EPD_W21_WriteCMD(0X65);			//FLASH CONTROL
  EPD_W21_WriteDATA(0x01);
  
  EPD_W21_CS1_0;					//MFCSB à-μí
  MCU_write_flash(0xB9);
  EPD_W21_CS1_1;					//MFCSB à-??
  
  EPD_W21_WriteCMD(0X65);			//FLASH CONTROL
  EPD_W21_WriteDATA(0x00);	
  
  EPD_W21_WriteCMD(0x02);
  lcd_chkstatus();
  
  EPD_W21_WriteCMD(0x07);
  EPD_W21_WriteDATA(0xa5);
  close_epd();
}
BOOL screen_busy_fun(void)
{
#define TIMES_2S_BUSY 60                //2min
  if(epd_scn_tp.busy_times == TIMES_2S_BUSY)
  {
    while(1);
  }
  else if((isEPD_W21_BUSY))         //高不忙
  {
    epd_scn_tp.busy_times=0;
    epd_scn_tp.screen_busy_flag = FALSE;
    screen_close();
    return FALSE;
  }
  else
  {
    epd_scn_tp.screen_busy_flag = TRUE;
    return TRUE;
  }
}
#endif


