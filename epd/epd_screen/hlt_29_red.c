#include "delay.h"
#include <string.h>
#include "mini_fs.h"
#include "epd_choose.h"
#include "epd_io.h"
#include "sys_init.h"
#include "global_variable.h"
#include "sys_isr.h"

#ifdef EPD_HLT_209_RED_TEP

const unsigned char init_data[]={
0x90,	0x20,	0x48,	0x10,	0x00,	0x00,	0x00,	0x90,	0x40,	0x48,	0x80,	0x00,	0x00,	0x00,	0x82,	0xA6,
0x48,	0x9B,	0xC0,	0x00,	0x00,	0x82,	0xA6,	0x48,	0x9B,	0xC0,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
0x00,	0x00,	0x00,	0x15,	0x15,	0x02,	0x07,	0x02,	0x0C,	0x0C,	0x05,	0x05,	0x07,	0x1E,	0x05,	0x1E,
0x05,	0x04,	0x0C,	0x0C,	0x09,	0x5A,	0x07,	0x09,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,																		
};

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

static void lcd_chkstatus(void)
{
  
#define WAIT_COUNT	40	//80s超时
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

void EPD_IC_Rst(void)
{
  EPD_W21_RST_1;
  Delay_ms(1);
  EPD_W21_RST_0;		
  Delay_ms(1);
  EPD_W21_RST_1;
  Delay_ms(1);
}

void WRITE_LUT()
{
  unsigned char i;
  
  EPD_W21_WriteCMD(0x32);			// write LUT register
  for(i=0;i<70;i++)			// write LUT register with 29bytes instead of 30bytes 2D13
    EPD_W21_WriteDATA(init_data[i]);
}

void clear_screen(void)
{
#ifdef EPD_NEW_ENCLOSURE
  EPD_W21_WriteCMD(0x4E);		// Set RAM X address counter = 0
  EPD_W21_WriteDATA(0x0F);
  EPD_W21_WriteCMD(0x4F);		// Set RAM Y address counter =295
  EPD_W21_WriteDATA(0x27);
  EPD_W21_WriteDATA(0x01);
#else
  EPD_W21_WriteCMD(0x4E);		// Set RAM X address counter = 0
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteCMD(0x4F);		// Set RAM Y address counter =295
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA(0x00);
#endif
  EPD_W21_WriteCMD(0x24);
}
void clear_screen_red(void)
{
#ifdef EPD_NEW_ENCLOSURE
  EPD_W21_WriteCMD(0x4E);		// Set RAM X address counter = 0
  EPD_W21_WriteDATA(0x0F);
  EPD_W21_WriteCMD(0x4F);		// Set RAM Y address counter =295
  EPD_W21_WriteDATA(0x27);
  EPD_W21_WriteDATA(0x01);
#else
  EPD_W21_WriteCMD(0x4E);		// Set RAM X address counter = 0
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteCMD(0x4F);		// Set RAM Y address counter =295
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA(0x00);
#endif
  EPD_W21_WriteCMD(0x26);
}

void over_screen(void)
{
  EPD_W21_WriteCMD(0x22);
  EPD_W21_WriteDATA(0xf7);		//0XC7   Load LUT from MCU(0x32), Display update   0XF7     FROM 内部
  EPD_W21_WriteCMD(0x20);
  Delay_ms(10);
  if(hlt_busy_1()!= TRUE)
    gerr_info = TR3_SCREEN_ERR1;
  lcd_chkstatus();
}
void display_lcd(file_id_t fd)
{
  UINT16 pcnt;
  UINT8 i;
  UINT8  *data = TEMP.BYTE64;
  clear_screen();
  for (pcnt = 0; pcnt < 296; pcnt++) 
  {	
    f_read(fd, (295-pcnt) * 16 ,data, 16); 
    for (i = 0; i < 16; i++)
    {                 
      EPD_W21_WriteDATA(data[i]);
    }  
  } 
//////  over_screen();
}
void display_lcd_red(file_id_t fd)
{
  UINT16 pcnt;
  UINT8 i;
  UINT8  *data = TEMP.BYTE64;
  clear_screen_red();
  for (pcnt = 0; pcnt < 296; pcnt++) 
  {	
    f_read(fd, (295-pcnt) * 16 ,data, 16); 
    for (i = 0; i < 16; i++)
    {                   
      EPD_W21_WriteDATA(~data[i]);
    }  
  } 
  over_screen();
}
void INIT_SSD1675()
{
  EPD_W21_WriteCMD(0x74);
  EPD_W21_WriteDATA(0x54);
  EPD_W21_WriteCMD(0x75);
  EPD_W21_WriteDATA(0x3b);
  EPD_W21_WriteCMD(0x01);		// Set MUX as 296
  EPD_W21_WriteDATA(0x27);
  EPD_W21_WriteDATA(0x01);
  EPD_W21_WriteDATA(0x00);
//////  EPD_W21_WriteCMD(0x3A);		// Set 100Hz
//////  EPD_W21_WriteDATA(0x30);         // Set 150Hz
//////  EPD_W21_WriteCMD(0x3B);		// Set 100Hz
//////  EPD_W21_WriteDATA(0x0A);         // Set 130Hz
  EPD_W21_WriteCMD(0x11);		// data enter mode
  EPD_W21_WriteDATA(0x03);
  
  EPD_W21_WriteCMD(0x18);		// internal  temperature
  EPD_W21_WriteDATA(0x80);
  
  EPD_W21_WriteCMD(0x44);		// set RAM x address start/end, in page 36
  EPD_W21_WriteDATA(0x00);		// RAM x address start at 00h;
  EPD_W21_WriteDATA(0x0f);		// RAM x address end at 0fh(15+1)*8->128 
  EPD_W21_WriteCMD(0x45);		// set RAM y address start/end, in page 37
  EPD_W21_WriteDATA(0x00);		// RAM y address start at 127h;
  EPD_W21_WriteDATA(0x00);		
  EPD_W21_WriteDATA(0x27);		// RAM y address end at 00h;
  EPD_W21_WriteDATA(0x01);		
//////  EPD_W21_WriteCMD(0x04);		// set VSH,VSL value
//////  EPD_W21_WriteDATA(0x50);		// 	    2D9  15v
//////  EPD_W21_WriteDATA(0xCC);		//	    2D9   5v 
//////  EPD_W21_WriteDATA(0x3E);		//	    2D9  -15v
//////  
//////  EPD_W21_WriteCMD(0x2C);           // vcom
//////  EPD_W21_WriteDATA(0x78);           //-2.6V

  EPD_W21_WriteCMD(0x3C);		// board
  EPD_W21_WriteDATA(0x60);		//GS1-->GS1   0x33

//////  WRITE_LUT();
  
}
  
void EPD_W21_Init(void){
  
#ifndef EPD_POWER_OFTEN_OPEN
  open_epd();
#endif
  epd_spi_init();
  
  EPD_W21_BS_0;		// 4 wire spi mode selected
  EPD_W21_RST_0;		// Module reset
  Delay_ms(100);
  EPD_W21_RST_1;
  Delay_ms(100);
  EPD_W21_WriteCMD(0x12);			//SWRESET
  lcd_chkstatus();
  INIT_SSD1675();
  EPD_W21_WriteCMD(0x21);								//
  EPD_W21_WriteDATA(0x03);
  SetWathcDog();
  display_lcd(F_BMP_BW);
  display_lcd_red(F_BMP_RED);
}
void enterdeepsleep(void)
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
BOOL screen_busy_fun(void)
{
#define TIMES_2S_BUSY 60                //2min
  if(epd_scn_tp.busy_times == TIMES_2S_BUSY)
  {
    while(1);
  }
  else if(!(isEPD_W21_BUSY))         //低不忙
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
  EPD_W21_BS_0;		// 4 wire spi mode selected
  EPD_W21_RST_0;		// Module reset
  Delay_ms(100);
  EPD_W21_RST_1;
  Delay_ms(100);
  EPD_W21_WriteCMD(0x12);			//SWRESET
  lcd_chkstatus();
  screen_deep_sleep_power_on();
}
#endif
