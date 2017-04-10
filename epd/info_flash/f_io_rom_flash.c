#include "datatype.h"
#include "f_io_rom_flash.h"
#include "sys_init.h"
#include "adc10.h"

#define INFO_SEGMENT_SIZE   64


#define FLASH_ERASER_FN	(24)	//320k

//MONITOR void rom_segment_erase(WORD seg_addr) UPDATA_SEGMENT
void rom_segment_erase(WORD seg_addr)
{
/*
  char *Flash_ptr;                          // Flash pointer
  
  if(seg_addr  >  INFO_ADDR_3 || seg_addr  < INFO_ADDR_1)
    return ;

  WathcDogStop();
  
  Flash_ptr = (char *)(UINT16)seg_addr;              // Initialize Flash pointer
  FCTL2 = FWKEY + FSSEL_1 + FLASH_ERASER_FN;                        //时钟频率范围257k~476K ，目前8M /（24+1）
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY + ERASE;                    // Set Erase bit   
  *Flash_ptr = 0;                           // Dummy write to erase Flash segment
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
  
  SetWathcDog();
*/
}

//MONITOR void rom_segment_write(WORD addr, WORD data, WORD len) UPDATA_SEGMENT 
void rom_segment_write(WORD addr, WORD data, WORD len)
{
/*
  char *pdata = (char *)(UINT16)data;  // Segment C pointer
  char *pdst = (char *)(UINT16)addr;  // Segment D pointer  
  UINT16 i;
  
  if(addr  >  INFO_ADDR_3 || addr  < INFO_ADDR_1)
    return ;
  
  if(len >= INFO_SEGMENT_SIZE )
    len = INFO_SEGMENT_SIZE;

  WathcDogStop();
  
  FCTL2 = FWKEY + FSSEL_1 + FLASH_ERASER_FN;                        //时钟频率范围257k~476K ，目前8M /（24+1）
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY + WRT;                    // Set Erase bit  
  
  for (i = 0; i < len; i++)
    *pdst++ = *pdata++;          // copy value segment C to segment D
  
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
  SetWathcDog();
  */
}

//需要移植的函数，实现将数据从FLASH拷贝到内存中
//MONITOR  void rom_segment_read(WORD addr,WORD buf, WORD len) UPDATA_SEGMENT
void rom_segment_read(WORD addr,WORD buf, WORD len) 
{
/*
  char *pdata = (char *)(UINT16)addr;  // Segment C pointer
  char *pdst = (char *)(UINT16)buf;  // Segment D pointer  
  UINT8 i = 0;
  
  if(addr  >  INFO_ADDR_3 || addr  < INFO_ADDR_1)
    return ;
  
  if(len >= INFO_SEGMENT_SIZE )
    len = INFO_SEGMENT_SIZE; 
  for (i = 0; i < len; i++)
    *pdst++ = *pdata++;          // copy value segment C to segment D  
*/
}
