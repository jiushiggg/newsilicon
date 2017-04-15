#ifndef __HOLTEK_MSP430_EPDIO_H__
#define __HOLTEK_MSP430_EPDIO_H__

//#include "msp430g2755.h"
#include "..\sys_init\datatype.h"
#include <udelay.h>

/*
*EPD_IO_INT
*/

//#define __NOP()  UDELAY_Delay(1)

//-------------------------------FMSDO-----------------
#define EPD_WF_FMSDO_OUT               
#define EPD_WF_FMSDO_IN              
#define EPD_WF_FMSDO_IO                 
#define EPD_WF_FMSDO_REN_CLOSE        
#define EPD_WF_FMSDO_REN_OPEN         
#define EPD_WF_FMSDO_0                 
#define EPD_WF_FMSDO_1                

//------------------------------EPD_POWER--------------
//�����������ģʽ
#define EPD_WF_POWER_OUT
#define EPD_WF_POWER_IN
#define EPD_WF_POWER_IO

//�Ƿ������������
#define EPD_WF_POWER_REN_CLOSE
#define EPD_WF_POWERT_REN_OPEN

//���硢�ϵ�
#define EPD_W21_POWER_ON
#define EPD_W21_POWER_OFF

//------------------------------EPD_Rst----------------  
//�����������ģʽ
#define EPD_WF_RST_OUT
#define EPD_WF_RST_IN
#define EPD_WF_RST_IO

//�Ƿ������������
#define EPD_WF_RST_REN_CLOSE
#define EPD_WF_RST_REN_OPEN

//����ߵͻ�������������
#define EPD_W21_RST_0
#define EPD_W21_RST_1

//------------------------------cs----------------------
//�����������ģʽ
#define EPD_WF_CS_OUT
#define EPD_WF_CS_IN
#define EPD_WF_CS_IO

//�Ƿ������������
#define EPD_WF_CS_REN_CLOSE
#define EPD_WF_CS_REN_OPEN

//����ߵͻ�������������
#define EPD_W21_CS_0
#define EPD_W21_CS_1

//------------------------------dc----------------------
//�����������ģʽ
#define EPD_WF_DC_OUT
#define EPD_WF_DC_IN
#define EPD_WF_DC_IO

//�Ƿ������������
#define EPD_WF_DC_REN_CLOSE
#define EPD_WF_DC_REN_OPEN

//����ߵͻ�������������
#define EPD_W21_DC_0
#define EPD_W21_DC_1

//------------------------------bs----------------------
//�����������ģʽ
#define EPD_WF_BS_OUT
#define EPD_WF_BS_IN
#define EPD_WF_BS_IO

//�Ƿ������������
#define EPD_WF_BS_REN_CLOSE
#define EPD_WF_BS_REN_OPEN

//����ߵͻ�������������
#define EPD_W21_BS_0
#define EPD_W21_BS_1

//-----------------------------cs1----------------------
//�����������ģʽ
#define EPD_WF_CS1_OUT
#define EPD_WF_CS1_IN
#define EPD_WF_CS1_IO

//�Ƿ������������
#define EPD_WF_CS1_REN_CLOSE
#define EPD_WF_CS1_REN_OPEN

//����ߵͻ�������������
#define EPD_W21_CS1_0
#define EPD_W21_CS1_1

//---------------------------EPD_CK---------------------   
//�����������ģʽ
#define EPD_WF_CLK_OUT
#define EPD_WF_CLK_IN
#define EPD_WF_CLK_IO

//�Ƿ������������
#define EPD_WF_CLK_REN_CLOSE
#define EPD_WF_CLK_REN_OPEN

//����ߵͻ�������������
#define EPD_W21_CLK_0
#define EPD_W21_CLK_1

//---------------------------EPD_SDA-------------------   

//�����������ģʽ
#define EPD_WF_SDA_OUT
#define EPD_WF_SDA_IN
#define EPD_WF_SDA_IO

//�Ƿ������������
#define EPD_WF_SDA_REN_CLOSE
#define EPD_WF_SDA_REN_OPEN

//����ߵͻ�������������
#define EPD_W21_MOSI_0
#define EPD_W21_MOSI_1
#define EPD_READ_SDA

//---------------------------EPD_Busy-------------------
//�����������ģʽ
#define EPD_WF_BUSY_OUT
#define EPD_WF_BUSY_IN
#define EPD_WF_BUSY_IO

//�Ƿ������������
#define EPD_WF_BUSY_REN_CLOSE
#define EPD_WF_BUSY_REN_OPEN

//����ߵͻ�������������
#define EPD_W21_BUSY_0
#define EPD_W21_BUSY_1
#define isEPD_W21_BUSY 0



/*-------------------��������-----------------------*/
void epd_powerIO_init(void);
void epd_spi_init(void);
void close_epd(void);
void open_epd(void);
void EPD_IC_Rst(void);
BOOL screen_busy_fun(void);
void epd_deep_sleep_io(void);
void epd_power_deep_sleep_fun(void);
#endif



























