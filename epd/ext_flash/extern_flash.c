#include "extern_flash.h"
#include "delay.h"
#include "datatype.h"


#define  SPI_NOT_BUSY  spi_not_busy()

//此函数只提供超时退出机制，不返回是否成功失败
//MONITOR static void spi_not_busy(void) UPDATA_SEGMENT
static void spi_not_busy(void) 
{
/*
  UINT32 i = FLASH_TIMEOUT;
  while(i--) {
    if (!(UCA0STAT & UCBUSY))
      return;
  } 
*/
}


//MONITOR static UINT8 FLASH_SPI_ReadByte(void) UPDATA_SEGMENT
static UINT8 FLASH_SPI_ReadByte(void) 
{
/*
  SPI_NOT_BUSY; 
  UCA0TXBUF = 0xFF;
  SPI_NOT_BUSY; 	
  return UCA0RXBUF;
*/
  return 1;
}

//MONITOR static void FLASH_SPI_WriteByte(UINT8 Data) UPDATA_SEGMENT
static void FLASH_SPI_WriteByte(UINT8 Data)
{
/*
  SPI_NOT_BUSY; 
  UCA0TXBUF = Data;
*/
}

//MONITOR static void FLASH_SPI_Close(void) UPDATA_SEGMENT
static void FLASH_SPI_Close(void) 
{
/*
  SPI_NOT_BUSY;
  Delay_us_20();
  FLASH_CS_1;
*/
}

static void FLASH_SPI_Open(void) 
{
/*
  FLASH_CS_0;
*/
}



//读状态寄存器,包括两种读操作：读高字节和读低字节操作
//MONITOR UINT8 FLASH_StateRegisterRD(UINT8 readtype) UPDATA_SEGMENT
UINT8 FLASH_StateRegisterRD(UINT8 readtype) 
{  
  UINT8 byte;
  FLASH_SPI_Open();
  FLASH_SPI_WriteByte(readtype);
  byte=FLASH_SPI_ReadByte();
  FLASH_SPI_Close();
  return(byte);  
}

//等flash忙操作结束
//MONITOR void FLASH_WaitBusy(void) UPDATA_SEGMENT
void FLASH_WaitBusy(void) 
{
#if 0
  while(FLASH_StateRegisterRD(FLASH_RDSR_L) & State_REG_BIT_WIP)
    ;
#else
  UINT32 i = FLASH_TIMEOUT;
  //坏的情况下，至少会delay10秒，
  //等待WEL忙标志结束,用在program/erase/write status register这些命令之前	
  while (i--) {
    if (!((FLASH_StateRegisterRD(FLASH_RDSR_L)) & State_REG_BIT_WIP))
      break;
  }
#endif
}

//写状态寄存器,包括两个操作:只写低字节和写16数据
//MONITOR void FLASH_StateRegisterWR(void) UPDATA_SEGMENT
void FLASH_StateRegisterWR(void)
{  	       
  FLASH_SPI_Open();
  FLASH_SPI_WriteByte(FLASH_WRSR);	
  
  FLASH_SPI_WriteByte(2);	
  FLASH_SPI_WriteByte(0);
  
  FLASH_SPI_Close(); 
  FLASH_WaitBusy();
}

//写使能信号
//MONITOR static void FLASH_WriteEnable(BOOL bOnOff) UPDATA_SEGMENT
static void FLASH_WriteEnable(BOOL bOnOff) 
{
  FLASH_SPI_Open();
  FLASH_SPI_WriteByte(FLASH_WRITE_ENABLE);	
  FLASH_SPI_Close(); 	
}

//MONITOR static void send_spi_addr(WORD addr) UPDATA_SEGMENT
static void send_spi_addr(WORD addr) 
{
  FLASH_SPI_WriteByte((addr >> 16) & 0xff);
  //FLASH_SPI_WriteByte(0);
  FLASH_SPI_WriteByte((addr >> 8) & 0xff);
  FLASH_SPI_WriteByte((addr >> 0) & 0xff);
}

//写页操作,写N个字节到Flash
//MONITOR void FLASH_PageProgram(WORD start_addr, UINT8 *writebuf, WORD writecnt)UPDATA_SEGMENT
void FLASH_PageProgram(WORD start_addr, UINT8 *writebuf, WORD writecnt)
{
  WORD i;	
  
  FLASH_WriteEnable(TRUE);
  
  FLASH_SPI_Open();
  FLASH_SPI_WriteByte(FLASH_WRITE_BYTE);
  send_spi_addr(start_addr);
  for (i = 0; i < writecnt; i++)
    FLASH_SPI_WriteByte(writebuf[i]);
  FLASH_SPI_Close(); 
  
  FLASH_WaitBusy();
  
}

//读取操作：包括两种类型的读取操作:快速方式及普通读取操作
//MONITOR void FLASH_Read(WORD start_addr, UINT8 * readBuf, WORD readcnt) UPDATA_SEGMENT
void FLASH_Read(WORD start_addr, UINT8 * readBuf, WORD readcnt) 
{
  WORD i;
  
  FLASH_SPI_Open();
  FLASH_SPI_WriteByte(FLASH_FASTREAD);
  
  send_spi_addr(start_addr);
  //发送一个不确定的数据
  FLASH_SPI_WriteByte(0xFF);
  
  for (i = 0; i < readcnt; i++)
    readBuf[i] =FLASH_SPI_ReadByte(); 
  
  FLASH_SPI_Close();
  
  FLASH_WaitBusy();
}

#if 0
//芯片ID读取,包括两种读操作：一种是ID有效位为16位,一种为24位
UINT32 FLASH_ReadID(UINT8 readtype){
  UINT32 Manufact_ID1 = 0, Product_ID1 = 0 ,Product_ID2 = 0;
  UINT32 Chip_ID; 
  FLASH_SPI_Open();
  
  if(readtype==FLASH_READ_ID){
    FLASH_SPI_WriteByte(FLASH_READ_ID);
    FLASH_SPI_WriteByte(0);
    FLASH_SPI_WriteByte(0);
    FLASH_SPI_WriteByte(0);
    Manufact_ID1 = FLASH_SPI_ReadByte();
    Product_ID1 = FLASH_SPI_ReadByte();
    Chip_ID = ((Manufact_ID1 << 8) | Product_ID1);
  } else if(readtype==FLASH_RDID) {
    FLASH_SPI_WriteByte(FLASH_RDID);
    Manufact_ID1 = FLASH_SPI_ReadByte();
    Product_ID1 = FLASH_SPI_ReadByte();
    Product_ID2 = FLASH_SPI_ReadByte();
    Chip_ID = ((Manufact_ID1 << 16) | (Product_ID1<< 8)| Product_ID2);  
  } 
  FLASH_SPI_Close();
  return(Chip_ID);    
}
#endif

//Deep Powerdown
//MONITOR void FLASH_DeepPowerDown(BOOL bOnOff) UPDATA_SEGMENT
#if 0
void FLASH_DeepPowerDown(BOOL bOnOff) 
{
  FLASH_SPI_Open();	
  FLASH_SPI_WriteByte(bOnOff ? FLASH_SLEEP : FLASH_WAKEUP);
  FLASH_SPI_Close();		
}
#else
void FLASH_DeepPowerDown(BOOL bOnOff) 
{
  FLASH_SPI_Open();	
  FLASH_SPI_WriteByte(bOnOff ? FLASH_SLEEP : FLASH_WAKEUP);
  if(!bOnOff)
  {
      FLASH_SPI_WriteByte(0);
      FLASH_SPI_WriteByte(0);
      FLASH_SPI_WriteByte(0);
      if(!FLASH_SPI_ReadByte()) 
      {
        Delay_us_20();
        Delay_us_20();
      }
  }
  FLASH_SPI_Close();		
}
#endif

#if 0
//Flash擦除操作,包括三种操作:芯片擦除,扇区擦除及块擦除
void FLASH_Erase(UINT8 erasetype, WORD start_addr){	
  // 发送写使能命令
  FLASH_WriteEnable(TRUE);
  FLASH_SPI_Open();
  switch (erasetype) {   
  case FLASH_EREASE_CHIP:      	//full chip		
    FLASH_SPI_WriteByte(FLASH_EREASE_CHIP);
    break;		
  case FLASH_EREASE_SECTOR:       // 4k
    FLASH_SPI_WriteByte(FLASH_EREASE_SECTOR);
    send_spi_addr(start_addr);				
    break;
  case FLASH_EREASE_BLOCK:		//32k
    FLASH_SPI_WriteByte(FLASH_EREASE_BLOCK);
    send_spi_addr(start_addr);		
    break;
  default:
    break;		
  }
  FLASH_SPI_Close();
  FLASH_WaitBusy();	
}
#else
//MONITOR void FLASH_Erase(UINT8 erasetype, WORD start_addr) UPDATA_SEGMENT
void FLASH_Erase(UINT8 erasetype, WORD start_addr)	
{	
  // 发送写使能命令
  FLASH_WriteEnable(TRUE);
  FLASH_SPI_Open();
  FLASH_SPI_WriteByte(erasetype);
  if (erasetype == FLASH_EREASE_SECTOR || erasetype == FLASH_EREASE_BLOCK)
    send_spi_addr(start_addr);		 
  FLASH_SPI_Close();
  FLASH_WaitBusy();	
}
#endif


void fs_erase_all(void)
{

  UINT32 block_num = (EF_EADDR- EF_STAA_DDR)/EF_BLOCK_SIZE,i =0,offset=0;
  for(i= 0;i<block_num;i++)
  {
    FLASH_Erase(FLASH_EREASE_SECTOR, EF_STAA_DDR+offset);
    offset += EF_BLOCK_SIZE;
  }

}
//MONITOR void segment_erase(WORD seg_addr) UPDATA_SEGMENT
void segment_erase(WORD seg_addr) 
{
  FLASH_Erase(FLASH_EREASE_SECTOR, seg_addr);
}

//MONITOR void segment_write(WORD addr, WORD data, WORD len) UPDATA_SEGMENT
void segment_write(WORD addr, WORD data, WORD len)
{
  FLASH_PageProgram(addr, (BYTE *)(UINT16)(data&0xffff), len);
}

//MONITOR void segment_read(WORD addr, WORD buf, WORD len) UPDATA_SEGMENT
void segment_read(WORD addr, WORD buf, WORD len) 
{
  FLASH_Read(addr, (BYTE *)(UINT16)(buf&0xffff), len);
}




//MONITOR void restore_ext_spi_flashio(void) UPDATA_SEGMENT
void restore_ext_spi_flashio(void) 
{
/*
  FLASH_SPI_IO; // SF_MOSI SF_MISO SF_CK 
  FLASH_SPI_MODE;

*/
}


void FLASH_deep_close_miso(void)
{
/*
  FLASH_MISO_IN;
  FLASH_MISO_IO_MODE;
  FLASH_MISO_REN_OPEN;
  FLASH_MISO_0;
*/
}

//MONITOR void flash_spi_init(BOOL on) UPDATA_SEGMENT
void flash_spi_init(BOOL on) 
{
/*
  if(on) 
  {
    FLASH_CS_1;
    restore_ext_spi_flashio();   
    UCA0CTL1 |= UCSWRST;                    	 		// **Initialize USCI state machine**
    //SPI MODE3
    UCA0CTL0 |= UCCKPL + UCMSB + UCMST + UCSYNC;  	// 3-pin, 8-bit SPI master
    UCA0CTL1 |= UCSSEL_2;                     		// SMCLK, 默认值为8M
    UCA0BR0 = 2;                          				// /2
    UCA0BR1 = 0;                              		//
    UCA0CTL1 &= ~UCSWRST;                    	 		// **Initialize USCI state machine**
    FLASH_DeepPowerDown(FALSE);
    FLASH_StateRegisterWR();
  } 
  else
  {
    //并不能省电
    FLASH_DeepPowerDown(TRUE);
    FLASH_deep_close_miso();	
  }
*/
}



void extern_flash_cs_init(void)
{
/*
  FLASH_CS_OUT;
  FLASH_CS_IO_MODE;
  FLASH_CS_REN_CLOSE;
*/
}
void disable_extern_flash(void)
{
/*
  extern_flash_cs_init();
  FLASH_CS_1;
*/
}
