#ifndef __MINI_FS_CONF__
#define __MINI_FS_CONF__
#include "..\epd_screen\epd_choose.h"
/**< �ļ�����ʹ��ID�ķ�ʽ����ַ��� */
typedef enum {
  FILE1,
  F_BMP_BW = FILE1,			//��Ļ��ʾ�ڰ���ɫ��������4k����
  F_BMP_RED,                            //��Ļ��ʾ��ɫ��������4k����
  F_SB,                                 //Ҫ��ʾ�Ľṹ�����Դ�С4k
  F_LAY_MAP,                            //Ԫ��ƫ�Ƶ�ַ�������С4k
  F_BMP_PKG_1,                          //�������ݰ�����bitmap��С4k
  F_BMP_PKG_2,                          //�������ݰ�����bitmap��С4k 
  F_PAGE_INFO1,                         //8ҳ���ݵķ�number���ʹ洢����ÿҳ512���ֽڣ�256��Ԫ�أ���С4K
  F_PAGE_INFO2,                         //8ҳ���ݵķ�number���ʹ洢��������ʹ��������С4K
  F_PAGE_NUM_1,                         //8ҳ����Ҫ��ʾ�����֣�ÿ��ҳ��Ҫ��ʾ������512���ֽڣ�85�����֣���С4K
  F_PAGE_NUM_2,                         //8ҳ����Ҫ��ʾ�����֣�ÿ��ҳ��Ҫ��ʾ������512���ֽڣ�85�����֣�����ʹ��������С4K
  F_BMP_DATA,                           //Ԫ��bitmap�̶��������  
  FILE_ID_END,
} file_id_t;


/*
* �ļ�ϵͳ����
*/

#define FILE_LAYER              F_BMP_DATA
#define FILE_LAYER_MAP          F_LAY_MAP
#define FILE_SB                 F_SB


/**< ����ÿ���ļ��Ĵ�С��ʹ�ýṹ��ķ�ʽ��������ļ�����ʼ��ַ */


//#define FS_DISK_ROM_FLASH					/**< ʹ��MCU�ڲ���ROM FLASH����Ҫ�����ڶ���ʽ�� */
#define FS_DISK_SPI_FLASH				/**< ʹ���ⲿ��SPI FLASH */
//#define FS_DISK_RAM_FLASH				/**< ʹ���ڴ���Ϊ�ļ�ϵͳ, ��Ҫ���ڲ��� */

//#define F_COPY_USE_EXT_MEM      			/**< f_copyʹ���ⲿ���� */
#define F_COPY_CACHE_SIZE	(WORD)16

#ifdef  F_COPY_USE_EXT_MEM
extern BYTE *F_COPY_CACHE;
#endif

#ifdef FS_DISK_ROM_FLASH					/**< �ڲ�FLASH�Ķ�д���� */
#define SEGMENT_SIZE		(WORD)512				/**< FLASH����С������Ԫ��С */
#define MAX_WRITE_UNIT	        (WORD)32					/**< FLASH�����д�뵥Ԫ */
#endif

#ifdef FS_DISK_SPI_FLASH					/**< �ⲿFLASH�Ķ�д���� */
#define SEGMENT_SIZE		(WORD)(4096)				/**< FLASH����С������Ԫ��С */
#define MAX_WRITE_UNIT	        (WORD)256				/**< FLASH�����д�뵥Ԫ */
#endif

#ifdef FS_DISK_RAM_FLASH					/**< �ڲ�FLASH�Ķ�д���� */
#define SEGMENT_SIZE		(WORD)64				/**< FLASH����С������Ԫ��С */
#define MAX_WRITE_UNIT	        (WORD)32					/**< FLASH���������д�뵥Ԫ */
#endif


#define UP_ALIGN(x) (((x) + SEGMENT_SIZE -1)/ SEGMENT_SIZE * SEGMENT_SIZE)
#define PIC_SIZE_ZERO(x) (UP_ALIGN(x) - (x)) 


#define PIC_FILE_SIZE   (WORD)(UP_ALIGN(SCREEN_WIDTH * SCREEN_HEIGHT / 8))      //��Ļ��ʾ��  


//�ܴ�С12k
#define FLASH_SB          (WORD)(8*1024)		                        //SB�ṹ����
#define FLASH_LAYER_MAP   (WORD)(4*1024)                                        //ͼ��ӳ����
#define FLASH_NUM_ATTR    (WORD)(4*1024)		                        //�洢���ָ�ʽ���ļ�
#define PKG_NUM_BITMAP    (WORD)(4*1024)                                        //ÿ�����ݰ�����bitmap��С 1.5k
#define F_PAGE_INFO       (WORD)(4*1024)                                        //
#define F_PAGE_NUM        (WORD)(4*1024)   

#define CMD_ELE_DATA       (WORD)(FLASH_ALL_SIZE - (PIC_FILE_SIZE + PIC_FILE_SIZE + FLASH_SB + \
                            FLASH_LAYER_MAP +  PKG_NUM_BITMAP + PKG_NUM_BITMAP + F_PAGE_INFO + F_PAGE_INFO+ F_PAGE_NUM + F_PAGE_NUM)) //����172k

#define ELEMENT_DATA_BITMAP (WORD)((ELEMENT_SIZE_K) * (WORD)1024)               //ʵ��Ԫ������С
 


#define FILE_TABLE_SIZE  (WORD)(PIC_FILE_SIZE + PIC_FILE_SIZE + FLASH_SB + FLASH_LAYER_MAP + \
PKG_NUM_BITMAP + PKG_NUM_BITMAP + F_PAGE_INFO + F_PAGE_INFO + F_PAGE_NUM +F_PAGE_NUM + CMD_ELE_DATA)


#endif
