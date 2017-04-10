#include "delay.h"
#include <string.h>
#include "mini_fs.h"
/*
#include "public_temp.h"
#include "epd.h"
#include "display_id.h"
#include "isr.h"
*/


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


#ifdef WF_JX_58_BW
static UINT8 get_FPC_flash_pll_data();
void EPD_FLASH_CMD(UINT8 command);

static void SPI_IOInit(){
  P3DIR |=  BIT6;
  P3SEL &= ~(BIT6) ;
  P3SEL2 &= ~(BIT6) ;
  EPD_W21_RST_0;
  
  Delay_ms(500);
  
  
  P3DIR |= BIT1+BIT4 + BIT5  + BIT7;
  P3SEL &= ~(BIT1+BIT4 + BIT5  + BIT7) ;
  P3SEL2 &= ~(BIT1+BIT4 + BIT5  + BIT7) ;
  P3OUT &= ~BIT1;
  
  P2DIR |= BIT3 + BIT4;
  P2SEL &= ~(BIT3+BIT4 ) ;
  P2SEL2 &= ~(BIT3+BIT4) ;
  
  
  P2DIR &= ~BIT5;
  P2SEL &= ~(BIT5) ;
  P2SEL2 &= ~(BIT5) ;
  P2REN |= BIT5;
  P2OUT &= ~BIT5;  
}

void SPI_off()
{
  
  P3DIR |= (BIT1+BIT4 + BIT5 + BIT6 + BIT7);
  P3SEL &= ~(BIT1+BIT4 + BIT5 + BIT6 + BIT7);
  P3SEL2 &= ~(BIT1+BIT4 + BIT5 + BIT6 + BIT7);
  P3OUT |= BIT4;
  P3OUT &= ~BIT5; 
  P3OUT |= BIT6;
  P3OUT |= BIT7;
  P3OUT |= BIT1;
  
  P2DIR |= (BIT3 + BIT4);
  P2SEL &= ~(BIT3+BIT4 ) ;
  P2SEL2 &= ~(BIT3+BIT4) ;
  P2OUT &= ~BIT3;
  P2OUT &= ~BIT4;
  
  
  P2DIR &= ~BIT5;
  P2SEL &= ~(BIT5) ;
  P2SEL2 &= ~(BIT5) ;
  P2REN &= ~ BIT5;
  P2OUT &= ~BIT5;
  
  P3DIR &= ~BIT2;
  P3SEL &= ~(BIT2) ;
  P3SEL2 &= ~(BIT2) ;
  P3REN &= ~ BIT2;
  P3OUT &= ~BIT2;
  
  
  
  
  
  
  
}

static void SPI_Write(UINT8 value){                                                           
  UINT8 i;
  
  for(i = 0; i < 8; i++)
  {
    SPI_Delay(2);
    EPD_W21_CLK_0; 
    
    if(value & 0x80)
      EPD_W21_MOSI_1;
    else
      EPD_W21_MOSI_0;		
    value = (value << 1); 	
    EPD_W21_CLK_1; 
  }
}

static void SPI_Delay(UINT8 xrate)
{
  ;
}

static void EPD_W21_WriteCMD(UINT8 command)
{  
  EPD_W21_CS_0; 
  EPD_W21_CS1_1;                  
  EPD_W21_DC_0;		// command write
  SPI_Write(command);
  EPD_W21_CS_1; 
  EPD_W21_CS1_1;
}

static void EPD_W21_WriteDATA(UINT8 command)
{
  EPD_W21_CS_0; 
  EPD_W21_CS1_1;                   
  EPD_W21_DC_1;		// data write
  SPI_Write(command);
  EPD_W21_CS_1; 
  EPD_W21_CS1_1;
}

static UINT8 EPD_W21_ReadDATA_Flash(void)
{
  
  UINT8 i,j;
  
  P3DIR &= ~BIT2;  
  P3OUT &= ~BIT2; 
  
  
  EPD_W21_DC_1;	 // data write 
  j=0; 
  
  for(i=0; i<8; i++) 
  { 
    EPD_W21_CLK_0; 
    j=(j<<1); 
    if((P3IN &BIT2)==BIT2) 
      j|=0x01; 
    else 
      j&=0xfe;	 
    EPD_W21_CLK_1; 
  } 
  
  P3DIR |=BIT2; 
  return(j); 
}

static UINT8 EPD_W21_ReadDATA(void)
{
  
  UINT8 i,j; 
  
  P2OUT &= ~BIT4; 
  P2DIR &= ~BIT4; 
  
  EPD_W21_CS_0; 
  EPD_W21_CS1_1;
  EPD_W21_DC_1;	
  
  j=0; 
  
  for(i=0; i<8; i++) 
  { 
    EPD_W21_CLK_0; 
    j=(j<<1); 
    if((P2IN &BIT4)==BIT4) 
      j|=0x01; 
    EPD_W21_CLK_1; 
  } 
  
  P2DIR |=BIT4; 
  EPD_W21_DC_0;
  EPD_W21_CS_1; 
  EPD_W21_CS1_1;
  
  
  return(j); 
  
}

static void lcd_chkstatus(void)
{
  while(!(isEPD_W21_BUSY));
}

//四个bit位表示一个像素
static void EPD_W21_WriteDATA_58(UINT8 command)
{
  int i = 6, j;
  UINT8 val1 , val2;
  for(j = 0; j < 4; j++)
  {
    val1 = command >> i;
    val1 = (val1 & 0x03);
    switch(val1)
    {
    case 0x00: val2 = 0x00;break;           //显示黑色
    case 0x01: val2 = 0x03;break;
    case 0x02: val2 = 0x30;break;
    case 0x03: val2 = 0x33;break;
    
    }
    i -= 2;
    EPD_W21_WriteDATA(val2);
  }
  
}
static void power_saving(void)
{
  
  EPD_W21_WriteCMD(DAM);
  EPD_W21_WriteDATA(0x01);
  EPD_FLASH_CMD(0xB9);
  EPD_W21_WriteCMD(DAM);
  EPD_W21_WriteDATA(0x00);
  
  EPD_W21_WriteCMD(DSLP);
  EPD_W21_WriteDATA(0xA5);
  
}

static void out_of_power_saving(void)
{
  
  EPD_W21_WriteCMD(DAM);
  EPD_W21_WriteDATA(0x01);
  EPD_FLASH_CMD(0xAB);
  EPD_W21_WriteCMD(DAM);
  EPD_W21_WriteDATA(0x00);
  
}
static void Ultrachip(UINT8 color)					 //black
{//	chip[]=G_Ultrachip[];
  UINT16 i,j;
  get_FPC_flash_pll_data();
  EPD_W21_WriteCMD(PON);  
  lcd_chkstatus();
  EPD_W21_WriteCMD(DTM1);
  SetWathcDog();
  //显示图片
  
  for( i=0; i < 448; i++)
  {
    SetWathcDog();
    for(j = 0; j < 75; j++)
    {
      EPD_W21_WriteDATA_58(color);
      
    }
  }
  EPD_W21_WriteCMD(DRF);
  lcd_chkstatus();
  EPD_W21_WriteCMD(POF);  
  lcd_chkstatus(); 
  power_saving();	
  SPI_off();
}


static void EPD_W21_ToFlash(UINT8 command){
  
  SPI_Delay(1);                 
  SPI_Write(command);
  EPD_W21_MOSI_0;
}
static void EPD_FLASH_CMD(UINT8 command){
  
  EPD_W21_CS1_0;  
  EPD_W21_CS_1;                
  SPI_Write(command);
  EPD_W21_MOSI_0;
  EPD_W21_CS1_1;  
  EPD_W21_CS_1;  
}

static void Send_Address_F(UINT8 address_23_16, unsigned int address_15_0)
{
  
  EPD_W21_ToFlash(address_23_16);
  EPD_W21_MOSI_0;
  EPD_W21_ToFlash((UINT8)(address_15_0 >> 8));
  EPD_W21_MOSI_0;
  EPD_W21_ToFlash((UINT8)address_15_0);
  EPD_W21_MOSI_0;
}



extern UINT8 EPD_W21_ReadDATA_Flash(void);
//从flash中读取数据
static void ReadData_F(UINT8 address_23_16, UINT16 address_15_0, UINT8 *read_data, UINT16 counter)
{
  UINT16 i;
  
  EPD_W21_CS1_0;  
  EPD_W21_CS_1;
  EPD_W21_ToFlash(0X03);
  Send_Address_F(address_23_16,address_15_0);
  for(i=0;i<counter;i++)
  {
    
    *read_data=EPD_W21_ReadDATA_Flash();
    read_data++;
  }
  //P3DIR |= BIT2;
  EPD_W21_CS1_1;  //片选端
  EPD_W21_CS_1;
}


//从flash中抓取VCOM数值
static UINT8 get_FPC_flash_vcom_data()
{
  UINT8 vcomFlashBuff = 0x1F;
  EPD_W21_WriteCMD(DAM);                       //使能flash
  EPD_W21_WriteDATA(0x01);
  
  Delay_ms(200);
  ReadData_F(0x00,0x6400,&vcomFlashBuff,1);   //从flash中读取数据   应该是读取的vm表的首地址
  Delay_ms(200);
  
  EPD_W21_WriteCMD(DAM);                          
  EPD_W21_WriteDATA(0x00);
  
  return vcomFlashBuff;
}

static UINT8 get_FPC_flash_pll_data()  
{
  UINT8 pllFlashBuff[10];
  UINT8 tempFlashBuff[10];
  
  UINT8 pllData;
  UINT8 tempData;
  UINT8 tempData2;
  int LUT_idx;
  
  EPD_W21_WriteCMD(DAM);//发送65开启从flash中读取数据
  EPD_W21_WriteDATA(0x01);   //使能flash
  Delay_ms(1000);
  
  ReadData_F(0x00,0x61AA,tempFlashBuff,10);
  Delay_ms(1000);
  ReadData_F(0x00,0x6410,pllFlashBuff,10);
  
  SetWathcDog();
  
  Delay_ms(1000);
  EPD_W21_WriteCMD(DAM);
  EPD_W21_WriteDATA(0x00);
  
  Delay_ms(1000);
  EPD_W21_WriteCMD(TSC);  
  lcd_chkstatus();
  
  tempData = EPD_W21_ReadDATA();
  tempData2 = EPD_W21_ReadDATA();
  tempData = tempData<<1;
  tempData2 = tempData2>>7;
  
  LUT_idx = (int)tempData + tempData2;
  //LUT_idx = 65;
  
  
  if(LUT_idx<tempFlashBuff[0])
  {
    pllData = pllFlashBuff[0];
  }
  else if( LUT_idx<tempFlashBuff[1])
  {
    pllData = pllFlashBuff[1];
  }
  else if(LUT_idx<tempFlashBuff[2])
  {
    pllData = pllFlashBuff[2];
  }
  else if( LUT_idx<tempFlashBuff[3])
  {
    pllData = pllFlashBuff[3];
  }
  else if( LUT_idx<tempFlashBuff[4])
  {
    pllData = pllFlashBuff[4];
  }
  else if( LUT_idx<tempFlashBuff[5])
  {
    pllData = pllFlashBuff[5];
  }
  else if(LUT_idx<tempFlashBuff[6])
  {
    pllData = pllFlashBuff[6];
  }
  else if( LUT_idx<tempFlashBuff[7])
  {
    pllData = pllFlashBuff[7];
  }
  else if(LUT_idx<tempFlashBuff[8])
  {
    pllData = pllFlashBuff[8];
  }
  else if(tempFlashBuff[8]<=LUT_idx)
  {
    pllData = pllFlashBuff[9];
  }
  
  
  
  
  EPD_W21_WriteCMD(PLL);
  EPD_W21_WriteDATA(pllData);
  //EPD_W21_WriteDATA(0x2E);
  
  return pllData;
}






const UINT8 EPD_cmd_data[] = {PLL,0x2C,TSE,0x00,CDI,0x77,TCON,0x22};



static void EPD_W21_Init(void){
  
  UINT8 vcmData = 0;
  
  
  SPI_IOInit();
  
  EPD_W21_BS_0;		// 4 wire spi mode selected
  EPD_W21_RST_0;		// Module reset
  Delay_ms(100);
  EPD_W21_RST_1;
  Delay_ms(100);	
  
  out_of_power_saving();
  EPD_W21_WriteCMD(PON);//上电
  lcd_chkstatus();     //检测忙
  
  EPD_W21_WriteCMD(PWR); 
  EPD_W21_WriteDATA (0x37);	    //POWER SETTING
  EPD_W21_WriteDATA (0x00);
  EPD_W21_WriteDATA (0x05);
  EPD_W21_WriteDATA (0x05);
  
  EPD_W21_WriteCMD(PSR);			//PANNEL SETTING  
  EPD_W21_WriteDATA(0xcf);                     //从flash中读取LUT
  EPD_W21_WriteDATA(0x08);
  
  EPD_W21_WriteCMD(0xe5);      //
  EPD_W21_WriteDATA(0x03);
  
  EPD_W21_WriteCMD(PFS);		          //power off sequence setting
  EPD_W21_WriteDATA(0x00);
  
  EPD_W21_WriteCMD(BTST);                      //boost设定
  EPD_W21_WriteDATA (0x28);
  EPD_W21_WriteDATA (0x28);
  EPD_W21_WriteDATA (0x28);
  
  EPD_W21_WriteCMD(PLL);
  EPD_W21_WriteDATA(0x3C);
  
  
  EPD_W21_WriteCMD(TSE);
  EPD_W21_WriteDATA(0x00);
  
  EPD_W21_WriteCMD(CDI);
  EPD_W21_WriteDATA(0x77);
  
  EPD_W21_WriteCMD(TCON);
  EPD_W21_WriteDATA(0x22);
  
  EPD_W21_WriteCMD(TRES);                       //tres
  EPD_W21_WriteDATA (0x02);
  EPD_W21_WriteDATA (0x58);
  EPD_W21_WriteDATA (0x01);
  EPD_W21_WriteDATA (0xc0);
  
  vcmData = get_FPC_flash_vcom_data(); 
  EPD_W21_WriteCMD(VDCS);
  EPD_W21_WriteDATA(vcmData);
  
  
  EPD_W21_WriteCMD(DAM);			//FLASH CONTROL
  EPD_W21_WriteDATA(0x00);
  
  EPD_W21_WriteCMD(POF);  
  
  
}

static void my_init_epd(BOOL test) {
  EPD_W21_Init();
  SetWathcDog();
  Ultrachip(0xff);       //30S  
  SetWathcDog();
  SPI_off();
  
}

void Init_EPD(void) {
  
  P3DIR |=  BIT2;
  P3SEL &= ~BIT2;
  P3SEL2 &= ~BIT2;
  
  my_init_epd(FALSE);
  gEventFlag &= ~EVENT_FLAG_DISPLAYID;
  
  
  //clock_16M_set();
}

#endif


