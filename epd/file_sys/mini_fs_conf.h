#ifndef __MINI_FS_CONF__
#define __MINI_FS_CONF__
#include "..\epd_screen\epd_choose.h"
/**< 文件名，使用ID的方式替代字符串 */
typedef enum {
  FILE1,
  F_BMP_BW = FILE1,			//屏幕显示黑白颜色区域内容4k对齐
  F_BMP_RED,                            //屏幕显示红色区域内容4k对齐
  F_SB,                                 //要显示的结构体属性大小4k
  F_LAY_MAP,                            //元素偏移地址存放区大小4k
  F_BMP_PKG_1,                          //所有数据包包号bitmap大小4k
  F_BMP_PKG_2,                          //所有数据包包号bitmap大小4k 
  F_PAGE_INFO1,                         //8页数据的非number类型存储区，每页512个字节，256个元素，大小4K
  F_PAGE_INFO2,                         //8页数据的非number类型存储区，交换使用区，大小4K
  F_PAGE_NUM_1,                         //8页数据要显示的数字，每个页上要显示的数字512个字节，85个数字，大小4K
  F_PAGE_NUM_2,                         //8页数据要显示的数字，每个页上要显示的数字512个字节，85个数字，交换使用区，大小4K
  F_BMP_DATA,                           //元素bitmap固定存放区域  
  FILE_ID_END,
} file_id_t;


/*
* 文件系统定义
*/

#define FILE_LAYER              F_BMP_DATA
#define FILE_LAYER_MAP          F_LAY_MAP
#define FILE_SB                 F_SB


/**< 定义每个文件的大小，使用结构体的方式方便计算文件的起始地址 */


//#define FS_DISK_ROM_FLASH					/**< 使用MCU内部的ROM FLASH，主要区别在读方式上 */
#define FS_DISK_SPI_FLASH				/**< 使用外部的SPI FLASH */
//#define FS_DISK_RAM_FLASH				/**< 使用内存作为文件系统, 主要用于测试 */

//#define F_COPY_USE_EXT_MEM      			/**< f_copy使用外部缓存 */
#define F_COPY_CACHE_SIZE	(WORD)16

#ifdef  F_COPY_USE_EXT_MEM
extern BYTE *F_COPY_CACHE;
#endif

#ifdef FS_DISK_ROM_FLASH					/**< 内部FLASH的读写参数 */
#define SEGMENT_SIZE		(WORD)512				/**< FLASH的最小擦除单元大小 */
#define MAX_WRITE_UNIT	        (WORD)32					/**< FLASH的最大写入单元 */
#endif

#ifdef FS_DISK_SPI_FLASH					/**< 外部FLASH的读写参数 */
#define SEGMENT_SIZE		(WORD)(4096)				/**< FLASH的最小擦除单元大小 */
#define MAX_WRITE_UNIT	        (WORD)256				/**< FLASH的最大写入单元 */
#endif

#ifdef FS_DISK_RAM_FLASH					/**< 内部FLASH的读写参数 */
#define SEGMENT_SIZE		(WORD)64				/**< FLASH的最小擦除单元大小 */
#define MAX_WRITE_UNIT	        (WORD)32					/**< FLASH的最大连续写入单元 */
#endif


#define UP_ALIGN(x) (((x) + SEGMENT_SIZE -1)/ SEGMENT_SIZE * SEGMENT_SIZE)
#define PIC_SIZE_ZERO(x) (UP_ALIGN(x) - (x)) 


#define PIC_FILE_SIZE   (WORD)(UP_ALIGN(SCREEN_WIDTH * SCREEN_HEIGHT / 8))      //屏幕显示区  


//总大小12k
#define FLASH_SB          (WORD)(8*1024)		                        //SB结构体区
#define FLASH_LAYER_MAP   (WORD)(4*1024)                                        //图层映射区
#define FLASH_NUM_ATTR    (WORD)(4*1024)		                        //存储数字格式的文件
#define PKG_NUM_BITMAP    (WORD)(4*1024)                                        //每个数据包包号bitmap大小 1.5k
#define F_PAGE_INFO       (WORD)(4*1024)                                        //
#define F_PAGE_NUM        (WORD)(4*1024)   

#define CMD_ELE_DATA       (WORD)(FLASH_ALL_SIZE - (PIC_FILE_SIZE + PIC_FILE_SIZE + FLASH_SB + \
                            FLASH_LAYER_MAP +  PKG_NUM_BITMAP + PKG_NUM_BITMAP + F_PAGE_INFO + F_PAGE_INFO+ F_PAGE_NUM + F_PAGE_NUM)) //等于172k

#define ELEMENT_DATA_BITMAP (WORD)((ELEMENT_SIZE_K) * (WORD)1024)               //实际元素区大小
 


#define FILE_TABLE_SIZE  (WORD)(PIC_FILE_SIZE + PIC_FILE_SIZE + FLASH_SB + FLASH_LAYER_MAP + \
PKG_NUM_BITMAP + PKG_NUM_BITMAP + F_PAGE_INFO + F_PAGE_INFO + F_PAGE_NUM +F_PAGE_NUM + CMD_ELE_DATA)


#endif
