#ifndef __HOLTEK_MSP430_EPDIO_H__
#define __HOLTEK_MSP430_EPDIO_H__

//#include "msp430g2755.h"
#include "..\sys_init\datatype.h"
#include "em_gpio.h"
/*
*EPD_IO_INT
*/

//#define __NOP()  UDELAY_Delay(1)
#define EPD_SPI_CK_PORT		gpioPortD
#define EPD_SPI_SDA_PORT	gpioPortD
#define EPD_SPI_CS_PORT		gpioPortC
#define EPD_BUSY_PORT		gpioPortF
#define EPD_RST_PORT		gpioPortA
#define EPD_CD_PORT			gpioPortA
#define EPD_SPI_CK_PIN		13
#define EPD_SPI_SDA_PIN		14
#define EPD_SPI_CS_PIN		9
#define EPD_BUSY_PIN		3
#define EPD_RST_PIN			2
#define EPD_CD_PIN			3
#define EPD_ENVDD_PORT		gpioPortC
#define EPD_ENVDD_PIN		10

#define CONTROL_SCREEN_PORT			gpioPortD
#define CONTROL_SCREEN_PIN			15
#define CONTROL_COM_PORT			gpioPortA
#define CONTROL_COM_PIN				5

//-------------------------------FMSDO-----------------
#define EPD_WF_FMSDO_OUT               
#define EPD_WF_FMSDO_IN              
#define EPD_WF_FMSDO_IO                 
#define EPD_WF_FMSDO_REN_CLOSE        
#define EPD_WF_FMSDO_REN_OPEN         
#define EPD_WF_FMSDO_0                 
#define EPD_WF_FMSDO_1                

//------------------------------EPD_POWER--------------
//设置输入输出模式
#define EPD_WF_POWER_OUT		GPIO_PinModeSet(EPD_ENVDD_PORT, EPD_ENVDD_PIN, gpioModePushPull, 1)
#define EPD_WF_POWER_IN			GPIO_PinModeSet(EPD_ENVDD_PORT, EPD_ENVDD_PIN, gpioModeInput, 0)
#define EPD_WF_POWER_IO

//是否打开上下拉电阻
#define EPD_WF_POWER_REN_CLOSE
#define EPD_WF_POWERT_REN_OPEN

//供电、断电
#define EPD_W21_POWER_ON		GPIO_PinModeSet(EPD_ENVDD_PORT, EPD_ENVDD_PIN, gpioModePushPull, 0)
#define EPD_W21_POWER_OFF		GPIO_PinModeSet(EPD_ENVDD_PORT, EPD_ENVDD_PIN, gpioModePushPull, 1)

//------------------------------EPD_Rst----------------  
//设置输入输出模式
#define EPD_WF_RST_OUT
#define EPD_WF_RST_IN			GPIO_PinModeSet(EPD_RST_PORT, EPD_RST_PIN, gpioModeInput, 0)
#define EPD_WF_RST_IO

//是否打开上下拉电阻
#define EPD_WF_RST_REN_CLOSE
#define EPD_WF_RST_REN_OPEN

//输出高低或者是上拉下拉
#define EPD_W21_RST_0		GPIO_PinModeSet(EPD_RST_PORT, EPD_RST_PIN, gpioModePushPull, 0)
#define EPD_W21_RST_1		GPIO_PinModeSet(EPD_RST_PORT, EPD_RST_PIN, gpioModePushPull, 1)

//------------------------------cs----------------------
//设置输入输出模式
#define EPD_WF_CS_OUT
#define EPD_WF_CS_IN
#define EPD_WF_CS_IO

//是否打开上下拉电阻
#define EPD_WF_CS_REN_CLOSE
#define EPD_WF_CS_REN_OPEN

//输出高低或者是上拉下拉
#define EPD_W21_CS_0		GPIO_PinModeSet(EPD_SPI_CS_PORT, EPD_SPI_CS_PIN, gpioModePushPull, 0)
#define EPD_W21_CS_1		GPIO_PinModeSet(EPD_SPI_CS_PORT, EPD_SPI_CS_PIN, gpioModePushPull, 1)

//------------------------------dc----------------------
//设置输入输出模式
#define EPD_WF_DC_OUT
#define EPD_WF_DC_IN
#define EPD_WF_DC_IO

//是否打开上下拉电阻
#define EPD_WF_DC_REN_CLOSE
#define EPD_WF_DC_REN_OPEN

//输出高低或者是上拉下拉
#define EPD_W21_DC_0		GPIO_PinModeSet(EPD_CD_PORT, EPD_CD_PIN, gpioModePushPull, 0)
#define EPD_W21_DC_1		GPIO_PinModeSet(EPD_CD_PORT, EPD_CD_PIN, gpioModePushPull, 1)

//------------------------------bs----------------------
//设置输入输出模式
#define EPD_WF_BS_OUT
#define EPD_WF_BS_IN
#define EPD_WF_BS_IO

//是否打开上下拉电阻
#define EPD_WF_BS_REN_CLOSE
#define EPD_WF_BS_REN_OPEN

//输出高低或者是上拉下拉
#define EPD_W21_BS_0
#define EPD_W21_BS_1

//-----------------------------cs1----------------------
//设置输入输出模式
#define EPD_WF_CS1_OUT
#define EPD_WF_CS1_IN
#define EPD_WF_CS1_IO

//是否打开上下拉电阻
#define EPD_WF_CS1_REN_CLOSE
#define EPD_WF_CS1_REN_OPEN

//输出高低或者是上拉下拉
#define EPD_W21_CS1_0		GPIO_PinModeSet(EPD_SPI_CS_PORT, EPD_SPI_CS_PIN, gpioModePushPull, 0)
#define EPD_W21_CS1_1		GPIO_PinModeSet(EPD_SPI_CS_PORT, EPD_SPI_CS_PIN, gpioModePushPull, 1)

//---------------------------EPD_CK---------------------   
//设置输入输出模式
#define EPD_WF_CLK_OUT
#define EPD_WF_CLK_IN
#define EPD_WF_CLK_IO

//是否打开上下拉电阻
#define EPD_WF_CLK_REN_CLOSE
#define EPD_WF_CLK_REN_OPEN

//输出高低或者是上拉下拉
#define EPD_W21_CLK_0		GPIO_PinModeSet(EPD_SPI_CK_PORT, EPD_SPI_CK_PIN, gpioModePushPull, 0)
#define EPD_W21_CLK_1		GPIO_PinModeSet(EPD_SPI_CK_PORT, EPD_SPI_CK_PIN, gpioModePushPull, 1)

//---------------------------EPD_SDA-------------------   

//设置输入输出模式
#define EPD_WF_SDA_OUT		GPIO_PinModeSet(EPD_SPI_SDA_PORT, EPD_SPI_SDA_PIN, gpioModePushPull, 0)
#define EPD_WF_SDA_IN		GPIO_PinModeSet(EPD_SPI_SDA_PORT, EPD_SPI_SDA_PIN, gpioModeInput, 0)
#define EPD_WF_SDA_IO

//是否打开上下拉电阻
#define EPD_WF_SDA_REN_CLOSE
#define EPD_WF_SDA_REN_OPEN

//输出高低或者是上拉下拉
#define EPD_W21_MOSI_0		GPIO_PinModeSet(EPD_SPI_SDA_PORT, EPD_SPI_SDA_PIN, gpioModePushPull, 0)
#define EPD_W21_MOSI_1		GPIO_PinModeSet(EPD_SPI_SDA_PORT, EPD_SPI_SDA_PIN, gpioModePushPull, 1)
#define EPD_READ_SDA		(1 == GPIO_PinOutGet(EPD_SPI_SDA_PORT, EPD_SPI_SDA_PIN))

//---------------------------EPD_Busy-------------------
//设置输入输出模式
#define EPD_WF_BUSY_OUT
#define EPD_WF_BUSY_IN		GPIO_PinModeSet(EPD_BUSY_PORT, EPD_BUSY_PIN, gpioModeInput, 0)
#define EPD_WF_BUSY_IO

//是否打开上下拉电阻
#define EPD_WF_BUSY_REN_CLOSE	GPIO_PinOutClear(EPD_BUSY_PORT, EPD_BUSY_PIN);
#define EPD_WF_BUSY_REN_OPEN	GPIO_PinOutSet(EPD_BUSY_PORT, EPD_BUSY_PIN);

//输出高低或者是上拉下拉
#define EPD_W21_BUSY_0		GPIO_PinModeSet(EPD_BUSY_PORT, EPD_BUSY_PIN, gpioModePushPull, 0)
#define EPD_W21_BUSY_1		GPIO_PinModeSet(EPD_BUSY_PORT, EPD_BUSY_PIN, gpioModePushPull, 1)
#define isEPD_W21_BUSY 		(1 == GPIO_PinOutGet(EPD_BUSY_PORT, EPD_BUSY_PIN))



/*-------------------函数声明-----------------------*/
void epd_powerIO_init(void);
void epd_spi_init(void);
void close_epd(void);
void open_epd(void);
void EPD_IC_Rst(void);
BOOL screen_busy_fun(void);
void epd_deep_sleep_io(void);
void epd_power_deep_sleep_fun(void);
#endif



























