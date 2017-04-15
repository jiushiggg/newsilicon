#ifndef __MINI_FS__
#define __MINI_FS__

/**< �ļ�ϵͳ������Ϣ����ƽ̨�Զ��� */
#include "..\sys_init\datatype.h"
#include "mini_fs_conf.h"
#include "..\ext_flash\extern_flash.h"


/**<  �����ַ�������ַת�� */
extern WORD DISK;

/**< �ļ�ϵͳ�ṹ�� */
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

/**< ����4���겻��Ҫ�Ķ� */

#define FILE_START_ADDR   (EF_STAA_DDR)
#define FS_BLOCK     ((FILE_TABLE_SIZE + SEGMENT_SIZE -1) / SEGMENT_SIZE) 
#define SUPER_BLOCK  ((sizeof(fs) + SEGMENT_SIZE -1) / SEGMENT_SIZE)       
#define SWAP_BLOCK   1  
#define DISK_BLOCK   (FS_BLOCK + SUPER_BLOCK + SWAP_BLOCK)

/**< ϵͳ����ʱ�����ļ�ϵͳ */
void 	f_init(void);

/**< ϵͳ�ػ�ʱ����Ҫʱ�����ļ�ϵͳ */
void 	f_sync(void);
#ifdef FS_DISK_ROM_FLASH
const BYTE* f_rom_read(file_id_t id, WORD offset);	/**< ���ٶ�������ֱ�ӷ���FLASH��ַ��ֻ����ROM FLASHʹ�� */
#endif

/**
* ��׼������ ��������ȷ��buf�ռ��㹻����len���ֽ�
* @param[in] id �ļ�ID   
* @param[in] offset ���ļ���ƫ�Ƶ�ַoffset��ʼ��   
* @param[out] buf �����ݶ��뵽buf��
* @param[in] len ������ֽ���
* @return ���سɹ�������ֽ���
*/
WORD	f_read(file_id_t id, WORD offset,	BYTE *buf, WORD len);

/**
* ��׼д����
* @param[in] id �ļ�ID   
* @param[in] offset ���ļ���ƫ�Ƶ�ַoffset��ʼд   
* @param[in] data ��д�������
* @param[in] len ��Ҫд����ֽ���
* @return ����д��ɹ����ֽ���
*/
WORD 	f_write(file_id_t id, WORD offset,	const BYTE *data, WORD len);

/**
* ֱ��д����
* ֱ�����ļ���ָ��ƫ����д��ָ�����ȵ����ݡ���������Ҫȷ�������򱻲�������
* ����������Ѿ������ݣ����Һ�Ҫд����������ݲ�һ�£�����ô˺����󣬴��������������δ֪
* @param[in] id �ļ�ID   
* @param[in] offset ���ļ���ƫ�Ƶ�ַoffset��ʼд   
* @param[in] data ��д�������
* @param[in] len ��Ҫд����ֽ���
* @return ����д��ɹ����ֽ���
*/
WORD 	f_write_direct(file_id_t id, WORD offset,	const BYTE *data, WORD len);

/**< �����ļ����� */
WORD	f_len(file_id_t id);

/**< �����ļ���С */
WORD	f_size(file_id_t id);

/**< �����ļ���ַ */
WORD	f_addr(file_id_t id);

/**< ����ļ� */
void	f_erase(file_id_t id);
/**< ����ƶ���С> */
void f_erase_offset(file_id_t id, WORD offset, WORD len);
WORD f_copy_direct( file_id_t src, WORD src_off,file_id_t dst, WORD dst_off, WORD len) ;

/**< ����һ����С�Ŀ飬������֤��ַaddr�� SEGMENT_SIZE ���� */
extern  void segment_erase(WORD seg_addr);

/**
* IO�������
* @param[in] addr ��Ҫ���ĵ�ַ
* @param[in] buf д����ڴ���������Ӧ��Ϊָ�룬������ʱ��ҪǿתΪWORD����
* @param[in] len ��Ҫ��ȡ���ֽ���, �����߱�֤���ᳬ��buf�ռ�
* @return ��
*/
extern  void segment_read(WORD addr, WORD buf, WORD len);

/**
* IO��д����
* @param[in] addr ��Ҫд�ĵ�ַ
* @param[in] buf Ϊ����Դ������Ӧ��Ϊָ�룬������ʱ��ҪǿתΪWORD����
* @param[in] len ��Ҫд����ֽ����������߱�֤�����MAX_WRITE_UNITд��
* @return ��
*/
extern  void segment_write(WORD addr, WORD data, WORD len);
extern  void FLASH_Erase(UINT8 erasetype, WORD start_addr);
/**@}*/ // mini_fs

#endif
