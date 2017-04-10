#include "delay.h"
#include <string.h>
#include "mini_fs.h"
#include "epd_choose.h"
#include "epd_io.h"
#include "sys_init.h"
#include "global_variable.h"
#include "sys_isr.h"


#ifdef EPD_HLT_420_BW


UINT8 casmode=2;	

const unsigned char init_data[]={  //30 bytes
0x00,
0xA6,
0x65,
0xA6,
0x2A,
0x1A,
0x6A,
0x64,
0x54,
0x66,
0x00,
0x55,
0x99,
0x11,
0x88,
0x11,
0x88,
0x88,
0x11,
0x88,
0xF0,
0xFF,
0xFF,
0xFF,
0xFF,
0xF2,
0xFF,
0xFF,
0xDF,
0x2F,
};
void EPD_IC_Rst(void)
{
  EPD_W21_RST_1;
  Delay_ms(150);
  EPD_W21_RST_0;		
  Delay_ms(150);
  EPD_W21_RST_1;
  Delay_ms(150);
}

static void SSD_CS(UINT8 ic_cs)
{
  if (ic_cs==0)
  {
    switch (casmode)
    {
    case 0:
      EPD_W21_CS_0;
      break;
    case 1:
      EPD_W21_CS1_0;
      break;
    default:
      EPD_W21_CS_0;
      EPD_W21_CS1_0;
      break;
    }
    
  }
  else
  {
    switch (casmode)
    {
    case 0:
      EPD_W21_CS_1;
      break;
    case 1:
      EPD_W21_CS1_1;
      break;
    default:
      EPD_W21_CS_1;
      EPD_W21_CS1_1;
      break;
    }
  }
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
  
  SSD_CS(1);		//nCS_H;
  SSD_CS(0);		//nCS_L;  
  EPD_W21_CLK_1;
  EPD_W21_DC_0;		// command write
  SPI_Write(command);
  SSD_CS(1);		//nCS_H;
}
static void EPD_W21_WriteDATA(UINT8 command)
{
  
  SSD_CS(1);		//nCS_H;
  SSD_CS(0);		//nCS_L; 
  EPD_W21_CLK_1;
  EPD_W21_DC_1;		// command write
  SPI_Write(command);
  SSD_CS(1);		//nCS_H;
}

void WRITE_LUT()
{
  unsigned char i;
  
  EPD_W21_WriteCMD(0x32);			// write LUT register
  for(i=0;i<30;i++)			// write LUT register with 29bytes instead of 30bytes 2D13
    EPD_W21_WriteDATA(init_data[i]);
}

static void lcd_chkstatus(void)
{
  
#define WAIT_COUNT	10	//20s超时
  SetWathcDog();
  UINT8 i=WAIT_COUNT;
  
  smclk_timer_isr(FALSE, 0,0); 
  Delay_ms(20);
  while((isEPD_W21_BUSY) && (0 != i) ) //高表示忙
  {
    sys_wakeup_wait(200); 
    i--;
  }
  
  smclk_timer_isr(FALSE, 0,0); 
  if(i == 0)
    while(1);
  
}
static BOOL hlt_busy_1(void)
{
#define _nop_() asm("nop")
  _nop_();
  if((isEPD_W21_BUSY) != 0)
    return TRUE;
  else
    return FALSE;
}


void clear_screen(UINT8 ic_choose)
{
  if(ic_choose)
    casmode=0;					//Master Chip
  else
    casmode = 1;					//Slave Chip
  
  EPD_W21_WriteCMD(0x4E);		// Set RAM X address counter = 0
  EPD_W21_WriteDATA(0x18);
  EPD_W21_WriteCMD(0x4F);		// Set RAM Y address counter =200
  EPD_W21_WriteDATA(0x2B);
  EPD_W21_WriteDATA(0x01);
  EPD_W21_WriteCMD(0x24); 
  
  
}

void over_screen(void)
{
  casmode=2;					//Both Chip
  EPD_W21_WriteCMD(0x22);
  EPD_W21_WriteDATA(0xC7);		//Load LUT from MCU(0x32), Display update
  EPD_W21_WriteCMD(0x20);
  Delay_ms(10);
  if(hlt_busy_1()!= TRUE)
    gerr_info = TR3_SCREEN_ERR1;
}
void Ultrachip(UINT8 color)
{
  unsigned int i;
  
  clear_screen(0);  
  for(i=0;i<7500;i++)	     
    EPD_W21_WriteDATA(color); 
  clear_screen(1);  
  for(i=0;i<7500;i++)	     
    EPD_W21_WriteDATA(color); 
  over_screen(); 
}

void display_lcd(file_id_t fd)
{
  UINT16 j;
  UINT8 i;
  UINT8  *data = TEMP.BYTE64,*data_red = TEMP.BYTE64+30;
  
  clear_screen(0);  
  clear_screen(1);  
  for(j=0;j<300;j++)	
  {
    f_read(fd, (299-j) * 50 ,data, 25); 
    f_read(F_BMP_RED, (299-j) * 50 ,data_red, 25); 
    SSD_CS(2);
    for(i=0;i<25;i++)
    {    
      data[i] &= (data_red[i]);       
      casmode=0x01;     
      EPD_W21_WriteDATA(data[i]);
      
    }
    
    f_read(fd, (299-j) * 50+25 ,data, 25); 
    f_read(F_BMP_RED, (299-j) * 50+25 ,data_red, 25); 
    for(i=25;i<50;i++)
    {    
      data[i-25] &= (data_red[i-25]);    
      casmode=0x00;	
      EPD_W21_WriteDATA(data[i-25]);        
    } 
    
  }
  over_screen();
}

void INIT_SSD1608()
{
  EPD_W21_WriteCMD(0x01);		// Display Setting
  EPD_W21_WriteDATA(0x2B);		// Set Mux for 300
  EPD_W21_WriteDATA(0x01);
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteCMD(0x3A);		// set dummy line for 50Hz frame freq
  EPD_W21_WriteDATA(0x16);
  EPD_W21_WriteCMD(0x3B);		// set gate line for 50Hz frame freq
  EPD_W21_WriteDATA(0x08);
  
  EPD_W21_WriteCMD(0x3C);		// board
  EPD_W21_WriteDATA(0x33);		//GS1-->GS1
  
  EPD_W21_WriteCMD(0x11);		// data enter mode
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteCMD(0x44);		// set RAM x address start/end, in page 36
  EPD_W21_WriteDATA(0x18);		// RAM x address start at 00h;
  EPD_W21_WriteDATA(0x00);		// RAM x address end at 0fh(24+1)*8->200
  EPD_W21_WriteCMD(0x45);		// set RAM y address start/end, in page 37
  EPD_W21_WriteDATA(0x2B);		// RAM y address start at 127h;
  EPD_W21_WriteDATA(0x01);		
  EPD_W21_WriteDATA(0x00);		// RAM y address end at 00h;
  EPD_W21_WriteDATA(0x00);		
  
  
  EPD_W21_WriteCMD(0xF0);      // Set lower PREVGH as 22V, PREVGL as -22.5V
  EPD_W21_WriteDATA(0x1A);    
  
  EPD_W21_WriteCMD(0x03);      // Set lower VGH as 20V, Keep VGL as -20V
  EPD_W21_WriteDATA(0xAA);  
  
  
  EPD_W21_WriteCMD(0x2C);      // vcom
  EPD_W21_WriteDATA(0x55);     //-1V

  WRITE_LUT();
  
}
void EPD_W21_Init(void){
  
#ifndef EPD_POWER_OFTEN_OPEN
  open_epd();
#endif
  epd_spi_init();
  casmode=2;					//Both Chip
  
  EPD_W21_BS_0;		// 4 wire spi mode selected
  EPD_W21_WriteCMD(0x12);			//SWRESET
  lcd_chkstatus();
  INIT_SSD1608();
  EPD_W21_WriteCMD(0x21);       // Option for Display Update
  EPD_W21_WriteDATA(0x83);    // A[7]=1(Enable bypass)  A[4]=0全黑(value will be used as for bypass)
  Ultrachip(0x00);       // 全黑到全白清屏，这样可防止开机出现花屏的问题
  
}

void enterdeepsleep()
{
  EPD_W21_WriteCMD(0x10);
  EPD_W21_WriteDATA(0x01);
}

void screen_close(void)
{
  enterdeepsleep();
  close_epd();
}
void screen_deep_sleep_power_on(void)
{
  enterdeepsleep();
  epd_deep_sleep_io();
}

void Init_EPD(void)
{
  SetWathcDog();
  EPD_W21_Init(); 
  
}
void hlt_normal_display(void)
{
  EPD_W21_WriteCMD(0x21);       // Option for Display Update
  EPD_W21_WriteDATA(0x03);    // 后面刷新恢复正常的前后2幅图比较
  EPD_W21_WriteCMD(0x3C);		// board
  EPD_W21_WriteDATA(0x73);		//GS1-->GS1
  SetWathcDog();
  display_lcd(F_BMP_BW);
}

BOOL screen_busy_fun(void)
{
#define TIMES_2S_BUSY 60                //2min
  if(epd_scn_tp.busy_times == TIMES_2S_BUSY)
  {
    while(1);
  }
  else if((!(isEPD_W21_BUSY)) && (hlt_busy_times == 0))         //低不忙
  {
    hlt_normal_display();
    epd_scn_tp.busy_times=0;
    epd_scn_tp.screen_busy_flag = TRUE;
    hlt_busy_times++;
    return TRUE;
  }
  else if((!(isEPD_W21_BUSY)) && (hlt_busy_times >= 1))         //低不忙
  {
    epd_scn_tp.busy_times=0;
    epd_scn_tp.screen_busy_flag = FALSE;
    hlt_busy_times = 0;
#ifdef EPD_POWER_OFTEN_OPEN
    screen_deep_sleep_power_on();
#else
    screen_close();
#endif
    return FALSE;
  }
  else
  {
    epd_scn_tp.screen_busy_flag = TRUE;
    return TRUE;
  }
}

void epd_power_deep_sleep_fun(void)
{
  open_epd();
  epd_spi_init();
  casmode=2;					//Both Chip
  
  EPD_W21_BS_0;		// 4 wire spi mode selected
  EPD_W21_WriteCMD(0x12);			//SWRESET
  lcd_chkstatus();
  screen_deep_sleep_power_on();
}

#endif


