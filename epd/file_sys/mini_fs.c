#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "..\sys_init\datatype.h"
#include "mini_fs.h"
#include "mini_fs_conf.h"
#include "..\epd_screen\epd_choose.h"

#define SWAP_ADDR	((WORD)(SEGMENT_SIZE * (FS_BLOCK + SUPER_BLOCK)))
#define SUPER_ADDR	((WORD)(SEGMENT_SIZE * FS_BLOCK + EF_STAA_DDR))

extern const fs_t fs_table;
fs_t fs;

enum {
  FS_FLAG_CHANGED	= 0x01,
  FS_FLAG_SWAP_CLEAN	= 0x02,
  FS_FLAG_SWAP_DIRE	= 0x04,	//用于segment_to_segment，等于1时表示将数据从SWAP拷贝回DISK
};

enum {
  NORMAL_WRITE,
  DIRECT_WRITE,
};

//static void data_to_swap(WORD swap_addr, WORD data_addr, WORD len);
static void segment_clean(WORD addr, WORD noused, WORD len);
typedef void (*op_fun_t)(WORD addr, WORD data, WORD len);
static void addr_split_opera(WORD addr, WORD data, WORD len, op_fun_t op);

#define VIRT2PHY(virt) (DISK+virt)

/*******************************************************
***	用户接口层代码
***	read函数直接返回FLASH地址即可
*** 	write函数需要进行FLASH擦写函数配合
***	erase函数擦除指定的文件
*******************************************************/

WORD f_len(file_id_t id) {
  return id >= FILE_ID_END ? 0 : fs.fs_len[id];
}

WORD f_size(file_id_t id) {
  return id >= FILE_ID_END ? 0 : fs.fs_ext[id].file_size;
}

WORD f_addr(file_id_t id) {
  return id >= FILE_ID_END ? 0 : fs.fs_ext[id].start_addr;
}

#ifdef FS_DISK_ROM_FLASH
const BYTE* f_rom_read(file_id_t id, WORD offset) {
  if (id >= FILE_ID_END || offset >= f_size(id))
    return NULL;
  return (const BYTE *)(VIRT2PHY(f_addr(id) + offset));
}
#endif


static int check_args(file_id_t id, WORD offset, const BYTE *buf, WORD len) {
  WORD size;
  if (id >= FILE_ID_END || len == 0 || buf == NULL || fs.valid != 0x76)
    return 0;
  
  size = f_size(id);
  if (len > size || offset >= size || offset + len < len || offset + len < offset)
    return 0;
  
  if (offset + len > size)
    len = size - offset;
  return len;
}

WORD f_read(file_id_t id, WORD offset,	BYTE *buf, WORD len) {
  if ((len = check_args(id, offset, buf, len)) == 0)
    return 0;
  //addr_split_opera(VIRT2PHY(f_addr(id) + offset), (WORD)buf, len, (op_fun_t)segment_read);
  segment_read(VIRT2PHY(f_addr(id) + offset), (WORD)buf, len);
  return len;
}

WORD f_copy_direct( file_id_t src, WORD src_off,file_id_t dst, WORD dst_off, WORD len) {
  UINT8 ret_r, ret_w, cache[F_COPY_CACHE_SIZE];
  WORD n = 0, ret = len;
  
  while(len > 0) {
    ret_r = f_read(src, src_off + n, cache, len > F_COPY_CACHE_SIZE ? F_COPY_CACHE_SIZE : len);
    if ((ret_w = f_write_direct(dst, dst_off + n, cache, ret_r)) < ret_r) {
      ret =  n + ret_w;
      break; //copy overide of dst 
    }
    n += ret_r;
    len -= ret_r;
    
    if (ret_r < F_COPY_CACHE_SIZE) {
      ret = n;
      break; //copy overide of src
    }
    
  }
  
  return ret;
}

static WORD _f_write(file_id_t id,	WORD offset, const BYTE *data, WORD len, BYTE write_flag) {
  WORD n, file_addr, file_len;
  
  if ((len = check_args(id, offset, data, len)) == 0)
    return 0;
  //每次写文件时，计算出需要修改的部分和需要追加的部分
  //需要修改的部分设计到擦除FLASH，而追加的部分则在文件创建时已经擦写过了
  file_addr = f_addr(id);
  file_len = f_len(id);
  
  if (offset >= file_len) {
    //需要保存的数据只需要追加
    fs.fs_len[id] = offset + len;
  } else if (offset < file_len && offset + len > file_len) {
    //需要保存的数据一部分位于已有数据内部，另外一部分需要追加
    n = file_len - offset;
    if (write_flag != DIRECT_WRITE) 
      addr_split_opera(VIRT2PHY(file_addr + offset), (WORD)data, n, segment_clean);
    fs.fs_len[id] = offset + len;
  } else {
    //需要保存的数据完全位于已有数据内部
    if (write_flag != DIRECT_WRITE) 
      addr_split_opera(VIRT2PHY(file_addr + offset), (WORD)data, len, segment_clean);
  }
  addr_split_opera(VIRT2PHY(file_addr + offset), (WORD)data, len, (op_fun_t)segment_write);
  fs.flag |= FS_FLAG_CHANGED;
  
  return len;
}

WORD f_write(file_id_t id,	WORD offset, const BYTE *data, WORD len) {
  return _f_write(id, offset, data, len, NORMAL_WRITE);
}

WORD 	f_write_direct(file_id_t id, WORD offset,	const BYTE *data, WORD len) {
  // return _f_write(id, offset, data, len, DIRECT_WRITE);
  WORD file_addr, file_len;
  if ((len = check_args(id, offset, data, len)) == 0)
    return 0;
  
  file_addr = f_addr(id);
  file_len = f_len(id);
  
  addr_split_opera(VIRT2PHY(file_addr + offset), (WORD)data, len, (op_fun_t)segment_write);
  
  if (offset + len > file_len)
    fs.fs_len[id] = offset + len;
  
  fs.flag |= FS_FLAG_CHANGED;
  
  return len;
}		

void f_erase(file_id_t id) {
  //当文件长度等于0时，也会引发segment_erase操作
  if ( id >= FILE_ID_END )
    return;
  addr_split_opera(VIRT2PHY(f_addr(id)), (WORD)NULL, f_size(id), segment_clean);
  fs.fs_len[id] = 0;
  
  fs.flag |= FS_FLAG_CHANGED;
}
void f_erase_offset(file_id_t id, WORD offset, WORD len) {
  //当文件长度等于0时，也会引发segment_erase操作
#define NOUSED 0x2200  //此地址只为了满足函数接口，不会实际操作
  
  if ((len = check_args(id, offset, (void *)NOUSED, len)) == 0)
    return ;
  addr_split_opera(VIRT2PHY(f_addr(id) + offset), (WORD)NULL, len, segment_clean);
}

void f_sync(void) {
  if (fs.flag & FS_FLAG_CHANGED) {
    fs.flag &= ~FS_FLAG_CHANGED;
    addr_split_opera(VIRT2PHY(SUPER_ADDR), (WORD)&fs, sizeof(fs), segment_clean);
    addr_split_opera(VIRT2PHY(SUPER_ADDR), (WORD)&fs, sizeof(fs), (op_fun_t)segment_write);
  }
}

void f_init(void) {
  file_id_t id;
  
  segment_read(VIRT2PHY(SUPER_ADDR), (WORD)&fs, sizeof(fs));
  if (fs.valid != 0x76) {
    memcpy(&fs, &fs_table, sizeof(fs));
#if 0
    addr_split_opera(VIRT2PHY(0), (WORD)NULL, FILE_TABLE_SIZE, segment_clean);
#else
    fs_erase_all();
#endif
    //文件系统的值使用rom中的
    fs.valid = 0x76;
    fs.flag |= FS_FLAG_CHANGED;
  } else { 
    for (id = FILE1; id < FILE_ID_END; id++) {
      if (f_len(id) > f_size(id))
        f_erase(id);
    }
  }
  f_sync();
}

/*******************************************************
***	块层对齐操作代码
***	主要涉及到块对齐和块擦除时复制已有数据区的操作
*******************************************************/

//对齐操作集
static void addr_split_opera(WORD addr, WORD data, WORD len, op_fun_t op) {
  WORD i, temp_off, temp_len = len;
  WORD split_unit = (op == segment_clean) ? SEGMENT_SIZE : MAX_WRITE_UNIT;
  //第一步，处理未对其的部分
  if ((temp_off = addr % split_unit) != 0) {
    if (temp_off + len > split_unit) 
      temp_len = split_unit - temp_off;
    op(addr, data, temp_len);
    addr += temp_len; data += temp_len; len -= temp_len;
  }
  //第二步, 处理刚好对齐的部分
  for (i = len / split_unit; i > 0; i--, addr += split_unit, data += split_unit, len -= split_unit)
    op(addr, data, split_unit);
  //第三步,处理最后剩下的
  if (len != 0)
    op(addr, data, len);
}


#define is_contain(a, b, c,d ) ((d) >= (a) && (c) < (b))
#define MIN(a, b) (a) < (b) ? (a) : (b)
#define MAX(a, b) (a) > (b) ? (a) : (b)

/************************************************
*********清除segment中指定区域的数据*************
************************************************/
#if 0
static void segment_clean(WORD addr, WORD noused, WORD len) {
  //fprintf(stderr, "%s(%d,%d,,%d)\n", __FUNCTION__, seg_addr, offset, len);
  if (!(fs.flag & FS_FLAG_SWAP_CLEAN)) {
    segment_erase(VIRT2PHY(SWAP_ADDR));
    fs.flag |= FS_FLAG_SWAP_CLEAN;
  }
  fs.flag &= ~FS_FLAG_SWAP_DIRE;
  __segment_op(addr, addr + len);
  segment_erase((addr / SEGMENT_SIZE) * SEGMENT_SIZE);
  if (!(fs.flag & FS_FLAG_SWAP_CLEAN)) {	//说明刚才有复制数据的动作
    fs.flag |= FS_FLAG_SWAP_DIRE;	//更改数据复制方向
    __segment_op(addr, addr + len);
  }
}
#else
static void segment_clean(WORD addr, WORD noused, WORD len){
  segment_erase(addr);
}
#endif

/************************************************
*****************块间复制函数********************
************************************************/

#define SWAP(a, b) do {\
WORD temp; \
  if (fs.flag & FS_FLAG_SWAP_DIRE) {\
    temp = a; a = b; b = temp;\
  } \
} while (0);

#if defined(FS_DISK_ROM_FLASH) || defined(FS_DISK_RAM_FLASH)
static void data_to_swap(WORD swap_addr, WORD data_addr, WORD len) {
  SWAP(swap_addr, data_addr);
  segment_write(swap_addr, data_addr, len);
}
#endif

#ifdef FS_DISK_SPI_FLASH
/*
static void data_to_swap(WORD swap_addr, WORD data_addr, WORD len) {
BYTE buf[F_COPY_CACHE_SIZE];	//SPI FLASH需要一个临时空交换块间数据
UINT32 i;

SWAP(swap_addr, data_addr);

for (i = len / F_COPY_CACHE_SIZE; i > 0; i--, swap_addr += F_COPY_CACHE_SIZE, \
data_addr += F_COPY_CACHE_SIZE, len -= F_COPY_CACHE_SIZE) 
{
segment_read(data_addr, (WORD)buf, F_COPY_CACHE_SIZE);
segment_write(swap_addr, (WORD)buf, F_COPY_CACHE_SIZE);
  }

if (len != 0) {
segment_read(data_addr, (WORD)buf, len);
segment_write(swap_addr, (WORD)buf, len);
  }
}*/
#endif
/*******************************************************
***	底层IO函数
***	需要实现Flash擦除函数，内存到Flash的复制，
***  flash到flash的复制共3个函数
*******************************************************/

#define DISK_SPACE   SEGMENT_SIZE*DISK_BLOCK

#define SIZEOF(s,m) ((size_t) sizeof(((s *)0)->m))

WORD DISK = 0;

/*******************************************
 FILE1,
  F_BMP_BW = FILE1,			//屏幕显示黑白颜色区域内容4k对齐
  F_BMP_RED,                            //屏幕显示红色区域内容4k对齐
  F_SB,                                 //要显示的结构体属性大小4k
  F_LAY_MAP,                            //元素偏移地址存放区大小4k
  F_NUM_ATTR,                           //存储数字格式的文件大小4k
  F_BMP_PKG_1,                          //所有数据包包号bitmap大小4k
  F_BMP_PKG_2,                          //所有数据包包号bitmap大小4k 
  F_PAGE_INFO1,                         //8页数据的非number类型存储区，每页512个字节，256个元素，大小4K
  F_PAGE_INFO2,                         //8页数据的非number类型存储区，交换使用区，大小4K
  F_PAGE_NUM_1,                         //8页数据要显示的数字，每个页上要显示的数字512个字节，85个数字，大小4K
  F_PAGE_NUM_2,                         //8页数据要显示的数字，每个页上要显示的数字512个字节，85个数字，交换使用区，大小4K
  F_BMP_DATA,                           //元素bitmap固定存放区域  
  FILE_ID_END,

//总大小12k
#define FLASH_SB          (WORD)(4*1024)		                        //SB结构体区
#define FLASH_LAYER_MAP   (WORD)(4*1024)                                        //图层映射区
#define FLASH_NUM_ATTR    (WORD)(4*1024)		                        //存储数字格式的文件
#define PKG_NUM_BITMAP    (WORD)(4*1024)                                        //每个数据包包号bitmap大小 1.5k
#define F_PAGE_INFO       (WORD)(4*1024)                                        //
#define F_PAGE_NUM        (WORD)(4*1024)   

*******************************************/
const struct fs_info_ext_t fs_info_ext[FILE_ID_END] = {
  {FILE_START_ADDR , PIC_FILE_SIZE },
  {FILE_START_ADDR + PIC_FILE_SIZE , PIC_FILE_SIZE },
  {FILE_START_ADDR + PIC_FILE_SIZE + PIC_FILE_SIZE , FLASH_SB },
  {FILE_START_ADDR + PIC_FILE_SIZE + PIC_FILE_SIZE + FLASH_SB , FLASH_LAYER_MAP},
  
  {FILE_START_ADDR + PIC_FILE_SIZE + PIC_FILE_SIZE + FLASH_SB + FLASH_LAYER_MAP  , PKG_NUM_BITMAP},
  {FILE_START_ADDR + PIC_FILE_SIZE + PIC_FILE_SIZE + FLASH_SB + FLASH_LAYER_MAP  + PKG_NUM_BITMAP , PKG_NUM_BITMAP },
  {FILE_START_ADDR + PIC_FILE_SIZE + PIC_FILE_SIZE + FLASH_SB + FLASH_LAYER_MAP  + PKG_NUM_BITMAP + PKG_NUM_BITMAP , F_PAGE_INFO },
  
  {FILE_START_ADDR + PIC_FILE_SIZE + PIC_FILE_SIZE + FLASH_SB + FLASH_LAYER_MAP + PKG_NUM_BITMAP + PKG_NUM_BITMAP + F_PAGE_INFO , F_PAGE_INFO},
  {FILE_START_ADDR + PIC_FILE_SIZE + PIC_FILE_SIZE + FLASH_SB + FLASH_LAYER_MAP + PKG_NUM_BITMAP + PKG_NUM_BITMAP + F_PAGE_INFO + F_PAGE_INFO , F_PAGE_NUM},
  {FILE_START_ADDR + PIC_FILE_SIZE + PIC_FILE_SIZE + FLASH_SB + FLASH_LAYER_MAP + PKG_NUM_BITMAP + PKG_NUM_BITMAP + F_PAGE_INFO + F_PAGE_INFO + F_PAGE_NUM , F_PAGE_NUM},
  {FILE_START_ADDR + PIC_FILE_SIZE + PIC_FILE_SIZE + FLASH_SB + FLASH_LAYER_MAP + PKG_NUM_BITMAP + PKG_NUM_BITMAP + F_PAGE_INFO + F_PAGE_INFO + F_PAGE_NUM + F_PAGE_NUM , CMD_ELE_DATA},
};


const fs_t fs_table = { 
  .flag = 0,
  .fs_len = {0,0,0,0,0,0,0,0,0,0,0},
  .fs_ext = fs_info_ext,
};

