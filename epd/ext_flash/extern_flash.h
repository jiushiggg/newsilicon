#ifndef __HOLTEK_MSP430_EXTERNFLASH_H__
#define __HOLTEK_MSP430_EXTERNFLASH_H__

//#include "msp430g2755.h"
#include "..\sys_init\datatype.h"
#include "..\epd_screen\epd_choose.h"
#include "em_device.h"
#include "em_gpio.h"
#include "em_cmu.h"

/*------------�ⲿflash IO����----------*/
#define MX25_PORT_MOSI         gpioPortC
#define MX25_PIN_MOSI          6
#define MX25_PORT_MISO         gpioPortC
#define MX25_PIN_MISO          7
#define MX25_PORT_SCLK         gpioPortC
#define MX25_PIN_SCLK          8
#define MX25_PORT_CS           gpioPortA
#define MX25_PIN_CS            4

#define MX25_USART             USART1
#define MX25_USART_CLK         cmuClock_USART1
//------------------------------cs--------------
//�����������ģʽ
#define FLASH_CS_OUT			GPIO_PinModeSet( MX25_PORT_CS, MX25_PIN_CS, gpioModePushPull, 1);
#define FLASH_CS_IN
#define FLASH_CS_IO_MODE
//�Ƿ������������
#define FLASH_CS_REN_CLOSE
#define FLASH_CS_REN_OPEN
#define FLASH_CS_0				GPIO_PinOutClear( MX25_PORT_CS, MX25_PIN_CS )
#define FLASH_CS_1				GPIO_PinOutSet( MX25_PORT_CS, MX25_PIN_CS )

#define FLASH_SPI_IO
#define FLASH_SPI_MODE

#define  FLASH_MISO_IN			GPIO_PinModeSet( MX25_PORT_MISO, MX25_PIN_MISO, gpioModeInput, 0 );
#define  FLASH_MISO_IO_MODE
#define  FLASH_MISO_REN_CLOSE	GPIO_PinModeSet( MX25_PORT_MISO, MX25_PIN_MISO, gpioModeInput, 0 );
#define  FLASH_MISO_REN_OPEN	GPIO_PinModeSet( MX25_PORT_MISO, MX25_PIN_MISO, gpioModeInputPull, 0 );
#define  FLASH_MISO_0			GPIO_PinOutClear( MX25_PORT_MISO, MX25_PIN_MISO )
    



/*------------------��������---------------*/

#define  EF_STAA_DDR        ((WORD)0x5000)//20k
#define  EF_FS_USED      ((WORD)0x2000)//8k

#ifdef  EXTERN_FALSH_4M
    #define  EF_EADDR        ((WORD)0x80000)
#else
    #define  EF_EADDR        ((WORD)0x40000)//256k == 0x40000  128k =0x20000 //512k=0x80000
#endif

#define  FLASH_ALL_SIZE   (WORD)(EF_EADDR - EF_FS_USED - EF_STAA_DDR)        //8k���ļ�ϵͳ��20k��ϵͳ����
#define  EF_BLOCK_SIZE   ((WORD)0x1000)   //4k


//��д�����
#define FLASH_FASTREAD 			0x0B
#define FLASH_WRITE_BYTE 		0x02     // Flashд

//дʹ�ܼ�дʹ�ܽ�ֹ
#define FLASH_WRITE_ENABLE  	0x06
#define FLASH_WRITE_DISABLE 	0x04

//Flash��������
#define FLASH_EREASE_SECTOR 	0x20
#define FLASH_EREASE_BLOCK 	0x52
#define FLASH_EREASE_CHIP  	0x60

//��ID��������
#define FLASH_READ_ID  		 	0x90    //��Manufacture ID/ Device ID (REMS)
#define FLASH_RDID   			0x9F

//д�Ĵ��������Ĵ�������
#define FLASH_WRSR   			0x01       //д״̬�Ĵ���
#define FLASH_RDSR_H  			0x35      //��״̬�Ĵ���
#define FLASH_RDSR_L  			0x05

//˯��״̬����˯������
#define FLASH_SLEEP 0xB9
#define FLASH_WAKEUP 0xAB

//״̬�Ĵ�����־λ����
#define State_REG_BIT_WIP  (1 << 0)

//����SPI��ʱ�˳�����
//16Mclock��ÿ3��whileѭ������1us��
//ѭ��3M�ι�������1�롣

#define FLASH_TIMEOUT	(3*1000*1000L)    //4.87s
/*------------��������----------------*/
void disable_extern_flash(void);
void flash_spi_init(BOOL on);
void restore_ext_spi_flashio(void);
void FLASH_DeepPowerDown(BOOL bOnOff);
void fs_erase_all(void);
#endif
