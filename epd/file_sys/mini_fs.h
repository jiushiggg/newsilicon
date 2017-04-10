#ifndef __MINI_FS__
#define __MINI_FS__

/**< 文件系统配置信息，由平台自定义 */
#include "..\sys_init\datatype.h"
#include "mini_fs_conf.h"
#include "..\ext_flash\extern_flash.h"


/**<  虚拟地址到物理地址转换 */
extern WORD DISK;

/**< 文件系统结构体 */
typedef struct fs_t {
  BYTE valid;
  BYTE flag;
  WORD fs_len[FILE_ID_END];		
  struct fs_info_ext_t {
    WORD start_addr;
    WORD file_size;
  } const *fs_ext;
} fs_t;
extern fs_t fs;

/**< 以下4个宏不需要改动 */

#define FILE_START_ADDR   (EF_STAA_DDR)
#define FS_BLOCK     ((FILE_TABLE_SIZE + SEGMENT_SIZE -1) / SEGMENT_SIZE) 
#define SUPER_BLOCK  ((sizeof(fs) + SEGMENT_SIZE -1) / SEGMENT_SIZE)       
#define SWAP_BLOCK   1  
#define DISK_BLOCK   (FS_BLOCK + SUPER_BLOCK + SWAP_BLOCK)

/**< 系统启动时加载文件系统 */
void 	f_init(void);

/**< 系统关机时或需要时保存文件系统 */
void 	f_sync(void);
#ifdef FS_DISK_ROM_FLASH
const BYTE* f_rom_read(file_id_t id, WORD offset);	/**< 快速读函数，直接返回FLASH地址。只能在ROM FLASH使用 */
#endif

/**
* 标准读函数 调用者需确认buf空间足够容纳len个字节
* @param[in] id 文件ID   
* @param[in] offset 从文件的偏移地址offset开始读   
* @param[out] buf 将数据读入到buf中
* @param[in] len 读入的字节数
* @return 返回成功读入的字节数
*/
WORD	f_read(file_id_t id, WORD offset,	BYTE *buf, WORD len);

/**
* 标准写函数
* @param[in] id 文件ID   
* @param[in] offset 从文件的偏移地址offset开始写   
* @param[in] data 待写入的数据
* @param[in] len 需要写入的字节数
* @return 返回写入成功的字节数
*/
WORD 	f_write(file_id_t id, WORD offset,	const BYTE *data, WORD len);

/**
* 直接写函数
* 直接往文件的指定偏移量写入指定长度的内容。调用者需要确保此区域被擦除过。
* 如果此区域已经有内容，而且和要写入的数据内容不一致，则调用此函数后，此区域的数据内容未知
* @param[in] id 文件ID   
* @param[in] offset 从文件的偏移地址offset开始写   
* @param[in] data 待写入的数据
* @param[in] len 需要写入的字节数
* @return 返回写入成功的字节数
*/
WORD 	f_write_direct(file_id_t id, WORD offset,	const BYTE *data, WORD len);

/**< 返回文件长度 */
WORD	f_len(file_id_t id);

/**< 返回文件大小 */
WORD	f_size(file_id_t id);

/**< 返回文件地址 */
WORD	f_addr(file_id_t id);

/**< 清空文件 */
void	f_erase(file_id_t id);
/**< 清空制定大小> */
void f_erase_offset(file_id_t id, WORD offset, WORD len);
WORD f_copy_direct( file_id_t src, WORD src_off,file_id_t dst, WORD dst_off, WORD len) ;

/**< 擦除一个最小的块，调用则保证地址addr按 SEGMENT_SIZE 对齐 */
extern  void segment_erase(WORD seg_addr);

/**
* IO块读函数
* @param[in] addr 需要读的地址
* @param[in] buf 写入的内存区，参数应该为指针，但传参时需要强转为WORD类型
* @param[in] len 需要读取的字节数, 调用者保证不会超出buf空间
* @return 无
*/
extern  void segment_read(WORD addr, WORD buf, WORD len);

/**
* IO块写函数
* @param[in] addr 需要写的地址
* @param[in] buf 为数据源，参数应该为指针，但传参时需要强转为WORD类型
* @param[in] len 需要写入的字节数，调用者保证不会跨MAX_WRITE_UNIT写入
* @return 无
*/
extern  void segment_write(WORD addr, WORD data, WORD len);
extern  void FLASH_Erase(UINT8 erasetype, WORD start_addr);
/**@}*/ // mini_fs

#endif
