#include "delay.h"
#include <string.h>
#include "mini_fs.h"
#include "epd_choose.h"
#include "epd_io.h"
#include "sys_init.h"
#include "global_variable.h"
#include "sys_isr.h"


#ifdef EPD_HLT_154_WB

const unsigned char init_data[]={  //30 bytes
  0x66,
  0x66,
  0x99,
  0x99,
  0x89,
  0x66,
  0x56,
  0x64,
  0x66,
  0xAA,
  0x11,
  0x88,
  0x11,
  0x88,
  0x11,
  0x88,
  0x00,
  0x00,
  0x00,
  0x00,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xBF,
  0x4F,
  0x0F,
  0x00,
};

void EPD_IC_Rst(void)
{
  EPD_W21_RST_1;
  Delay_ms(20);
  EPD_W21_RST_0;		
  Delay_ms(20);
  EPD_W21_RST_1;
  Delay_ms(20);
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


void clear_screen(void)
{
  EPD_W21_WriteCMD(0x4E);		// Set RAM X address counter = 0
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteCMD(0x4F);		// Set RAM Y address counter =200
  EPD_W21_WriteDATA(0xC7);
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteCMD(0x24); 
  
  
}

void over_screen(void)
{
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
  
  clear_screen();  
  for(i=0;i<8000;i++)	     
    EPD_W21_WriteDATA(color); 
  over_screen(); 
}

void display_lcd(file_id_t fd)
{
  UINT16 pcnt;
  UINT8 i;
  UINT8  *data = TEMP.BYTE64,*data_red = TEMP.BYTE64+30;
  clear_screen(); 
  for (pcnt = 0; pcnt < 200; pcnt++) 
  {	
    f_read(fd, (pcnt) * 25 ,data, 25); 
    f_read(F_BMP_RED, (pcnt) * 25 ,data_red, 25);  
    for (i = 0; i < 25; i++)
    {                 
      data[i] &= (data_red[i]);    
      EPD_W21_WriteDATA(data[i]);
    }  
  } 
  over_screen();
}

void INIT_SSD1608()
{
  EPD_W21_WriteCMD(0x11);		// data enter mode
  EPD_W21_WriteDATA(0x01);
  EPD_W21_WriteCMD(0x44);		// set RAM x address start/end, in page 36
  EPD_W21_WriteDATA(0x00);		// RAM x address start at 00h;
  EPD_W21_WriteDATA(0x18);		// RAM x address end at 0fh(24+1)*8->200
  EPD_W21_WriteCMD(0x45);		// set RAM y address start/end, in page 37
  EPD_W21_WriteDATA(0xC7);		// RAM y address start at c7h;
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA(0x00);		// RAM y address end at 00h;
  EPD_W21_WriteDATA(0x00);
  
  
  EPD_W21_WriteCMD(0xF0);      // Set lower PREVGH as 22V, PREVGL as -22.5V
  EPD_W21_WriteDATA(0x1A);    
  
  EPD_W21_WriteCMD(0x03);      // Set lower VGH as 20V, Keep VGL as -20V
  EPD_W21_WriteDATA(0xAA);
  
  
  EPD_W21_WriteCMD(0x2C);      // vcom
  EPD_W21_WriteDATA(0x61);     //-2V
  
  EPD_W21_WriteCMD(0x3C);		// board
  EPD_W21_WriteDATA(0x73);		//GS1-->GS1
  
  WRITE_LUT();
  
}
void EPD_W21_Init(void){
  
#ifndef EPD_POWER_OFTEN_OPEN
  open_epd();
#endif
  epd_spi_init();
  
  EPD_W21_BS_0;		// 4 wire spi mode selected
  EPD_W21_WriteCMD(0x12);			//SWRESET
  lcd_chkstatus();
  INIT_SSD1608();
  EPD_W21_WriteCMD(0x21);       // Option for Display Update
  EPD_W21_WriteDATA(0x83);    // A[7]=1(Enable bypass)  A[4]=0全黑(value will be used as for bypass)
  // Ultrachip(0x00);       // 全黑到全白清屏，这样可防止开机出现花屏的问题
  display_lcd(F_BMP_BW);
  
}

void enterdeepsleep()
{
  EPD_W21_WriteCMD(0x10);
  EPD_W21_WriteDATA(0x01);
}

void screen_deep_sleep_power_on(void)
{
  enterdeepsleep();
  epd_deep_sleep_io();
}

void screen_close(void)
{
  enterdeepsleep();
  close_epd();
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
  EPD_W21_WriteCMD(0x12);			//SWRESET
  lcd_chkstatus();
  screen_deep_sleep_power_on();
}

#endif


