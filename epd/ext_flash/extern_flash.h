#ifndef __HOLTEK_MSP430_EXTERNFLASH_H__
#define __HOLTEK_MSP430_EXTERNFLASH_H__

//#include "msp430g2755.h"
#include "..\sys_init\datatype.h"
#include "..\epd_screen\epd_choose.h"


/*------------外部flash IO定义----------*/
//------------------------------cs--------------
//设置输入输出模式
#define FLASH_CS_OUT
#define FLASH_CS_IN
#define FLASH_CS_IO_MODE
//是否打开上下拉电阻
#define FLASH_CS_REN_CLOSE
#define FLASH_CS_REN_OPEN
#define FLASH_CS_0
#define FLASH_CS_1

#define FLASH_SPI_IO
#define FLASH_SPI_MODE

#define  FLASH_MISO_IN
#define  FLASH_MISO_IO_MODE
#define  FLASH_MISO_REN_CLOSE
#define  FLASH_MISO_REN_OPEN
#define  FLASH_MISO_0
    



/*------------------参数定义---------------*/

#define  EF_STAA_DDR        ((WORD)0x5000)//20k
#define  EF_FS_USED      ((WORD)0x2000)//8k

#ifdef  EXTERN_FALSH_4M
    #define  EF_EADDR        ((WORD)0x80000)
#else
    #define  EF_EADDR        ((WORD)0x40000)//256k == 0x40000  128k =0x20000 //512k=0x80000
#endif

#define  FLASH_ALL_SIZE   (WORD)(EF_EADDR - EF_FS_USED - EF_STAA_DDR)        //8k的文件系统，20k的系统保留
#define  EF_BLOCK_SIZE   ((WORD)0x1000)   //4k


//读写命令定义
#define FLASH_FASTREAD 			0x0B
#define FLASH_WRITE_BYTE 		0x02     // Flash写

//写使能及写使能禁止
#define FLASH_WRITE_ENABLE  	0x06
#define FLASH_WRITE_DISABLE 	0x04

//Flash擦除命令
#define FLASH_EREASE_SECTOR 	0x20
#define FLASH_EREASE_BLOCK 	0x52
#define FLASH_EREASE_CHIP  	0x60

//读ID操作命令
#define FLASH_READ_ID  		 	0x90    //读Manufacture ID/ Device ID (REMS)
#define FLASH_RDID   			0x9F

//写寄存器及读寄存器命令
#define FLASH_WRSR   			0x01       //写状态寄存器
#define FLASH_RDSR_H  			0x35      //读状态寄存器
#define FLASH_RDSR_L  			0x05

//睡眠状态及解睡眠命令
#define FLASH_SLEEP 0xB9
#define FLASH_WAKEUP 0xAB

//状态寄存器标志位定义
#define State_REG_BIT_WIP  (1 << 0)

//加入SPI超时退出函数
//16Mclock，每3个while循环大致1us。
//循环3M次估计消耗1秒。

#define FLASH_TIMEOUT	(3*1000*1000L)    //4.87s
/*------------函数声明----------------*/
void disable_extern_flash(void);
void flash_spi_init(BOOL on);
void restore_ext_spi_flashio(void);
void FLASH_DeepPowerDown(BOOL bOnOff);
void fs_erase_all(void);
#endif
