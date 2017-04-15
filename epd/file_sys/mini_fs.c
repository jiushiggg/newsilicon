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
  FS_FLAG_SWAP_DIRE	= 0x04,	//����segment_to_segment������1ʱ��ʾ�����ݴ�SWAP������DISK
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
***	�û��ӿڲ����
***	read����ֱ�ӷ���FLASH��ַ����
*** 	write������Ҫ����FLASH��д�������
***	erase��������ָ�����ļ�
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
  //ÿ��д�ļ�ʱ���������Ҫ�޸ĵĲ��ֺ���Ҫ׷�ӵĲ���
  //��Ҫ�޸ĵĲ�����Ƶ�����FLASH����׷�ӵĲ��������ļ�����ʱ�Ѿ���д����
  file_addr = f_addr(id);
  file_len = f_len(id);
  
  if (offset >= file_len) {
    //��Ҫ���������ֻ��Ҫ׷��
    fs.fs_len[id] = offset + len;
  } else if (offset < file_len && offset + len > file_len) {
    //��Ҫ���������һ����λ�����������ڲ�������һ������Ҫ׷��
    n = file_len - offset;
    if (write_flag != DIRECT_WRITE) 
      addr_split_opera(VIRT2PHY(file_addr + offset), (WORD)data, n, segment_clean);
    fs.fs_len[id] = offset + len;
  } else {
    //��Ҫ�����������ȫλ�����������ڲ�
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
  //���ļ����ȵ���0ʱ��Ҳ������segment_erase����
  if ( id >= FILE_ID_END )
    return;
  addr_split_opera(VIRT2PHY(f_addr(id)), (WORD)NULL, f_size(id), segment_clean);
  fs.fs_len[id] = 0;
  
  fs.flag |= FS_FLAG_CHANGED;
}
void f_erase_offset(file_id_t id, WORD offset, WORD len) {
  //���ļ����ȵ���0ʱ��Ҳ������segment_erase����
#define NOUSED 0x2200  //�˵�ַֻΪ�����㺯���ӿڣ�����ʵ�ʲ���
  
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
    //�ļ�ϵͳ��ֵʹ��rom�е�
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
***	�������������
***	��Ҫ�漰�������Ϳ����ʱ���������������Ĳ���
*******************************************************/

//���������
static void addr_split_opera(WORD addr, WORD data, WORD len, op_fun_t op) {
  WORD i, temp_off, temp_len = len;
  WORD split_unit = (op == segment_clean) ? SEGMENT_SIZE : MAX_WRITE_UNIT;
  //��һ��������δ����Ĳ���
  if ((temp_off = addr % split_unit) != 0) {
    if (temp_off + len > split_unit) 
      temp_len = split_unit - temp_off;
    op(addr, data, temp_len);
    addr += temp_len; data += temp_len; len -= temp_len;
  }
  //�ڶ���, ����պö���Ĳ���
  for (i = len / split_unit; i > 0; i--, addr += split_unit, data += split_unit, len -= split_unit)
    op(addr, data, split_unit);
  //������,�������ʣ�µ�
  if (len != 0)
    op(addr, data, len);
}


#define is_contain(a, b, c,d ) ((d) >= (a) && (c) < (b))
#define MIN(a, b) (a) < (b) ? (a) : (b)
#define MAX(a, b) (a) > (b) ? (a) : (b)

/************************************************
*********���segment��ָ�����������*************
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
  if (!(fs.flag & FS_FLAG_SWAP_CLEAN)) {	//˵���ղ��и������ݵĶ���
    fs.flag |= FS_FLAG_SWAP_DIRE;	//�������ݸ��Ʒ���
    __segment_op(addr, addr + len);
  }
}
#else
static void segment_clean(WORD addr, WORD noused, WORD len){
  segment_erase(addr);
}
#endif

/************************************************
*****************��临�ƺ���********************
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
BYTE buf[F_COPY_CACHE_SIZE];	//SPI FLASH��Ҫһ����ʱ�ս����������
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
***	�ײ�IO����
***	��Ҫʵ��Flash�����������ڴ浽Flash�ĸ��ƣ�
***  flash��flash�ĸ��ƹ�3������
*******************************************************/

#define DISK_SPACE   SEGMENT_SIZE*DISK_BLOCK

#define SIZEOF(s,m) ((size_t) sizeof(((s *)0)->m))

WORD DISK = 0;

/*******************************************
 FILE1,
  F_BMP_BW = FILE1,			//��Ļ��ʾ�ڰ���ɫ��������4k����
  F_BMP_RED,                            //��Ļ��ʾ��ɫ��������4k����
  F_SB,                                 //Ҫ��ʾ�Ľṹ�����Դ�С4k
  F_LAY_MAP,                            //Ԫ��ƫ�Ƶ�ַ�������С4k
  F_NUM_ATTR,                           //�洢���ָ�ʽ���ļ���С4k
  F_BMP_PKG_1,                          //�������ݰ�����bitmap��С4k
  F_BMP_PKG_2,                          //�������ݰ�����bitmap��С4k 
  F_PAGE_INFO1,                         //8ҳ���ݵķ�number���ʹ洢����ÿҳ512���ֽڣ�256��Ԫ�أ���С4K
  F_PAGE_INFO2,                         //8ҳ���ݵķ�number���ʹ洢��������ʹ��������С4K
  F_PAGE_NUM_1,                         //8ҳ����Ҫ��ʾ�����֣�ÿ��ҳ��Ҫ��ʾ������512���ֽڣ�85�����֣���С4K
  F_PAGE_NUM_2,                         //8ҳ����Ҫ��ʾ�����֣�ÿ��ҳ��Ҫ��ʾ������512���ֽڣ�85�����֣�����ʹ��������С4K
  F_BMP_DATA,                           //Ԫ��bitmap�̶��������  
  FILE_ID_END,

//�ܴ�С12k
#define FLASH_SB          (WORD)(4*1024)		                        //SB�ṹ����
#define FLASH_LAYER_MAP   (WORD)(4*1024)                                        //ͼ��ӳ����
#define FLASH_NUM_ATTR    (WORD)(4*1024)		                        //�洢���ָ�ʽ���ļ�
#define PKG_NUM_BITMAP    (WORD)(4*1024)                                        //ÿ�����ݰ�����bitmap��С 1.5k
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

