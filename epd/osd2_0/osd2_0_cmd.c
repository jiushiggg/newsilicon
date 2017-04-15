#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include "datatype.h"
#include "mini_fs_conf.h"
#include "mini_fs.h"
#include "osd2_0_cmd.h"
#include "bit_display.h"
#include "global_variable.h"
#include "delay.h"
#include "crc16.h"
#include "three_event.h"
#include "reed.h"
#include "adc10.h"
#include "global_variable.h"




STATIC void change_page_into_file(void)
{
  if(gSys_tp.use_page_info_fileid ==  F_PAGE_INFO1)
  {
    gSys_tp.use_page_info_fileid = F_PAGE_INFO2;
  }
  else
  {
    gSys_tp.use_page_info_fileid = F_PAGE_INFO1;
  }
}
STATIC void eraser_page_info_file(void)
{ 
  
  if(gSys_tp.use_page_info_fileid ==  F_PAGE_INFO1)
  {
    f_erase(F_PAGE_INFO2);
  }
  else
  {
    f_erase(F_PAGE_INFO1);
  }
  
}

STATIC void change_page_number_file(void)
{
  if(gSys_tp.use_page_num_fileid ==  F_PAGE_NUM_1)
  {
    gSys_tp.use_page_num_fileid = F_PAGE_NUM_2;
  }
  else
  {
    gSys_tp.use_page_num_fileid = F_PAGE_NUM_1;
  }
}
STATIC void eraser_page_number_file(void)
{
  if(gSys_tp.use_page_num_fileid ==  F_PAGE_NUM_1)
  {
    f_erase(F_PAGE_NUM_2);
  }
  else
  {
    f_erase(F_PAGE_NUM_1);
  }
}

const UINT8 g_page_flag[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

BOOL change_page_makesure_fun(void)
{
  if((gSys_tp.present_page_id == gSys_tp.gpage_nowid) && ((gSys_tp.change_map & g_page_flag[gSys_tp.present_page_id]) == 0)) //��ǰ��ʾ��ҳ==��Ļ��ʾ��ҳ�������ҳ������Ϊ�ı�ʱ�Ų��������²���    
  {
    return FALSE;
  }
  return TRUE;
}

BOOL number_store_fun(file_id_t src_id, UINT16 len,UINT32 offset,UINT8 page_id,UINT8 *src_buf,UINT8 *dst_buf)
{
  UINT16 j,k;
  UINT8 flag=0;
  
  if(len > PAGE_LEN)
    return FALSE;
  
  f_read(F_BMP_DATA, offset, (UINT8 *)src_buf,len);//��ȡ��Ӧҳ����������
  f_read(src_id,(WORD)((page_id) * PAGE_LEN) , (UINT8 *)dst_buf,PAGE_LEN);//��ȡ��Ӧҳ֮ǰ������flash�е�����
  k=0;  //�˴�k=0.����ʾÿҳ��ʼʱ����ͷ��ʼ��
  /*
  k=0���������Ŀ�ģ���С��ѯ�Ļ��ᣬ���ֵķ��ջ��ǣ��������ݵ�ʱ����ִ���δ�ҵ���Ҫ������ʱ��k���ӵ����ֵ���´����ұ�ҳ������ʱ��Խ�����Ҫ�Ӹ���־λ�ж��Ƿ��ҵ���������
  ��û���ҵ�������Ҫ����һ������ֵ��osd��ÿ������Ĵ���ֵ��Ҫ��Ӧ�ã�Ŀ���Ǳ�֤����Ҫ�Ĳ�������
  */
  for(j=0;j<len ;j+= sizeof(number_t))//ԭʼ����
  {
    //tp_id=src_buf[j];                          //��ȡҪ�ı����ֶ�Ӧ����ʾ����id
    //�˴�k!=0.����ʾÿҳ��ʼʱ����layerid�Ѿ�����
    for(;k < PAGE_LEN ; k+= sizeof(number_t))//Ŀ������
    {
      // if(tp_id == dst_buf[k] )//�ж��Ƿ������ͬ��id
      if((0 == memcmp(src_buf + j, dst_buf + k, sizeof(UINT16))))
      {
        flag = 1;
        memcpy(dst_buf+k,src_buf+j,sizeof(number_t));//�������滻��ֵ
        break;
      }
    }
    if(flag == 0)
    {
      k=0;
      return FALSE;//�ڱ�ҳ��δ�ҵ���֮����Ե�����
    }
    else
    {
      flag =0;
    } 
  }
  f_write_direct(gSys_tp.use_page_num_fileid ,(WORD)((page_id) * PAGE_LEN), (UINT8 *)dst_buf,PAGE_LEN);//��ҳ�����滻��ɺ���д�뵽�µ�ҳ��
  
  
  return TRUE;
  
}
void stor_number_unchange_fun(UINT8 tp_page_map,file_id_t src_id,UINT8 *dst_temp)
{
  // UINT8 tp_info_dst[PAGE_LEN];
  UINT8 i=0;
  for(i=0;i<= ALL_PAGE_SUM;i++)
  {
    if( (tp_page_map & g_page_flag[i]) == 0)//��ʾ��ҳû��д���
    {
      f_read(src_id,(WORD)(i * PAGE_LEN) , (UINT8 *)dst_temp,PAGE_LEN);//��ȡ��Ӧҳ֮ǰ������flash�е�����
      f_write_direct(gSys_tp.use_page_num_fileid ,(WORD)(i * PAGE_LEN), (UINT8 *)dst_temp,PAGE_LEN);//��д�뵽�µ�ҳ��
    }
  }
}

BOOL sys_page_display_store_fun(UINT8 default_page_id,UINT8 present_page_id,UINT8 now_dispaly_id,UINT16 stay_time,UINT16 stay_time_cont )
{
  gSys_tp.default_page_id = default_page_id;
  gSys_tp.present_page_id = present_page_id;
  gSys_tp.gpage_nowid = now_dispaly_id;
  gSys_tp.stay_time = stay_time;
  gSys_tp.stay_time_cont = stay_time_cont;
  // gSys_tp.gflag_dispage_state = gflag_dispage_state ;
  if(FALSE == save_sys_load_page_info())
  {
    return FALSE;
  }
  return TRUE;
}

STATIC int32_t cmd1_fun(file_id_t cmdfd, UINT32 offset )
{
  cmd1_t tp;
  file_id_t src_id;
  UINT8 tp_info_src[PAGE_LEN],tp_info_dst[PAGE_LEN]; 
  UINT8 tp_page_map = 0;//��ʼ��������
  
  src_id = gSys_tp.use_page_num_fileid;
  change_page_number_file();//�л��������ֵ��ļ���
  f_read(cmdfd, offset, (UINT8 *)&tp, sizeof(cmd1_t));
  offset += sizeof(cmd1_t);
  tp_page_map |= g_page_flag[tp.default_page_id];    
  if(TRUE != number_store_fun(src_id,tp.len-sizeof(tp.default_page_id),offset,tp.default_page_id,tp_info_src,tp_info_dst))
    return -1;
  
  stor_number_unchange_fun(tp_page_map,src_id,tp_info_dst);//copyΪ�ı��ҳ�ϵ�����
  eraser_page_number_file(); 
  
  gpage.flag = FALSE;//ֹ֮ͣǰ�ļ���
  if(FALSE == sys_page_display_store_fun(tp.default_page_id,tp.default_page_id,gSys_tp.gpage_nowid,0,0))//����ҳ��ʾ����
    return -1;
  
  gEventFlag |= EVENT_FALG_DISPLAY_PAGE;
  
  gSys_tp.change_map = 0xff;            //ȫ��ҳ���ó��Ѿ��ı�
  
  return tp.len + offsetof(cmd1_t,default_page_id);
}

STATIC int32_t cmd2_fun(file_id_t cmdfd, UINT32 offset )
{
#define CHANGE_ALL_NUMBER       (0x01)
#define CHANGE_FORMER_NUMBER    (0x00)
  
  
  alter_num_t tp;
  UINT16 i;
  UINT8 tp_info_src[PAGE_LEN],tp_info_dst[PAGE_LEN];
  UINT8 tp_page_map = 0;//��ʼ��������
  page_num_sum_t np; 
  file_id_t src_id;
  
  f_read(cmdfd, offset, (UINT8 *)&tp, sizeof(alter_num_t));
  offset += sizeof(tp);
  src_id = gSys_tp.use_page_num_fileid;
  change_page_number_file();//�л��������ֵ��ļ���
  if(tp.flag != CHANGE_ALL_NUMBER)//ֻ���޸�����
  {   
    for(i=0;i<tp.len - sizeof(tp.flag) ;)//2����������峤��
    {
      f_read(cmdfd, offset, (UINT8 *)&np, sizeof(page_num_sum_t));//��ȡҳ�ź�ÿ��ҳ��������ֳ���
      tp_page_map |= g_page_flag[np.page_id];                    //����д�����ҳ
      
      offset += sizeof(page_num_sum_t);
      
      if(TRUE != number_store_fun(src_id,np.page_num_len,offset,np.page_id,tp_info_src,tp_info_dst))//�޸ĸı������
        return -1;
      
      offset += np.page_num_len;
      i+= ( sizeof(page_num_sum_t) + np.page_num_len);
    }
    
    stor_number_unchange_fun(tp_page_map,src_id,tp_info_dst);//copyΪ�ı��ҳ�ϵ�����
    eraser_page_number_file();  
    goto loop;
    
  }
  //����
  
  eraser_page_number_file();
  for(i=0;i<tp.len - sizeof(tp.flag) ;)
  {
    f_read(cmdfd, offset, (UINT8 *)&np, sizeof(page_num_sum_t));
    offset += sizeof(page_num_sum_t);
    f_read(cmdfd, offset, (UINT8 *)tp_info_src,np.page_num_len);
    f_write_direct(gSys_tp.use_page_num_fileid ,(WORD)((np.page_id) * PAGE_LEN), (UINT8 *)tp_info_src, np.page_num_len);
    offset += np.page_num_len;
    i+= ( sizeof(page_num_sum_t) + np.page_num_len);
  }
loop:
  
  gSys_tp.change_map = 0xff;                         //���߼�������124����ȫ����Ϊ��ҳ�ı䣬��Ҫ���¸���
  return (tp.len + 3);//3=cmd+len
  
}

STATIC int32_t cmd3_fun(file_id_t cmdfd, UINT32 offset )
{ 
#define  LEN_NULL (0xffffffff)
#define  FILE_ERR  (0xff)
  write_layer_t tp;
  wrt_layer_arrt_t lay_tp,lay_tp_temp;
  
  
  f_read(cmdfd, offset, (UINT8 *)&tp, sizeof(write_layer_t));
  lay_tp.offset = offset + offsetof(write_layer_t,layerid);//ƫ�Ƶ�ַ�Ǵ�layer_id��ʼ
  lay_tp.len = tp.len;
  lay_tp.src_file = cmdfd;
  f_read(F_LAY_MAP, (tp.layerid - 1) * sizeof(wrt_layer_arrt_t), (UINT8 *)&lay_tp_temp, sizeof(wrt_layer_arrt_t));
  
  if( ((lay_tp_temp.len == LEN_NULL) && (lay_tp_temp.offset == LEN_NULL) && (lay_tp_temp.src_file == FILE_ERR)) || ( (lay_tp_temp.len == lay_tp.len) && (lay_tp_temp.offset == lay_tp.offset) && (lay_tp_temp.src_file == lay_tp.src_file ) ))
  {
    f_write_direct(FILE_LAYER_MAP, (tp.layerid - 1) * sizeof(wrt_layer_arrt_t), (UINT8 *)&lay_tp, sizeof(wrt_layer_arrt_t));//����3������������ͼ������
  }
  else
  {
    return -1;//��ͬͼ���д��һ����ַ������
  }
  
  return  tp.len + (UINT32)offsetof(write_layer_t,layerid);
  
}



STATIC int32_t cmd4_fun(file_id_t cmdfd, UINT32 offset )
{
  
  
  UINT8 i=0; 
  UINT8 tp_info[PAGE_LEN];
  UINT16  num_len;
  store_page_cmd_t tp;
  page_into_t page_tp;
  
  f_read(cmdfd, offset, (UINT8 *)&tp, sizeof(store_page_cmd_t));
  if( (tp.page_sum > ALL_PAGE_SUM+1) )//ҳ������7�Ǵ����
    return -1;
  offset += sizeof(store_page_cmd_t);
  
  change_page_into_file();
  eraser_page_info_file();
  change_page_number_file();
  eraser_page_number_file();
  memset(gSys_tp.page_map,0xff,8);
  gSys_tp.change_map = 0xff;            //ȫ��ҳ���ó��Ѿ��ı�
  for(i = 0 ; i < tp.page_sum ; i++ )
  {
    f_read(cmdfd, offset, (UINT8 *)&page_tp, sizeof(page_into_t));
    memset(tp_info,0xff,PAGE_LEN);
    offset += sizeof(page_into_t);//��ַ�ӵ�page_info
    //----������ͼ��-----
    if(page_tp.layer_len > PAGE_LEN )
      return -1;
    gSys_tp.page_map[page_tp.page_id] = page_tp.page_id;//��������ҳ�룬ҳ���±��Ӧ��Ӧ��ҳ��
    
    if(page_tp.layer_len != 0)//������ͼ�㳤�Ȳ�Ϊ0
    {
      f_read(cmdfd, offset, (UINT8 *)&tp_info, page_tp.layer_len);//������ͼ��
      f_write_direct(gSys_tp.use_page_info_fileid, (page_tp.page_id) * PAGE_LEN, (UINT8 *)tp_info, page_tp.layer_len );//д�������ͼ��
      offset += page_tp.layer_len;  //���Ϸ����ֵ�id����  
    }
    //----����ͼ��-----
    f_read(cmdfd, offset, (UINT8 *)&num_len, sizeof(num_len));
    offset += sizeof(num_len);//���˵�ַ�ӵ���һ����layerid��
    
    if(num_len == 0)//û������
      continue;
    
    if(num_len > PAGE_LEN )
      return -1;
    memset(tp_info,0xff,PAGE_LEN);
    f_read(cmdfd, offset, (UINT8 *)&tp_info, num_len);//��ȡ����ͼ��ĳ���
    f_write_direct(gSys_tp.use_page_num_fileid, (page_tp.page_id) * PAGE_LEN, (UINT8 *)tp_info, num_len);//д�������ͼ��
    offset += (num_len);//��������Ԫ�صĳ���  
  }
  
  if(FALSE == sys_page_display_store_fun(gSys_tp.default_page_id,gSys_tp.present_page_id,gSys_tp.gpage_nowid,gSys_tp.stay_time,0))//����ҳ��ʾ���� Ĭ��ҳ��Ҫ��ʾ��ҳ����Ļ��ʾ��ҳ��ͣ��ʱ�䣬�Ѿ��߹���ʱ�䡢״̬��־
    return -1;
  
  return tp.len + (UINT32)offsetof(store_page_cmd_t,page_sum);
  
}


STATIC int32_t cmd5_fun(file_id_t cmdfd, UINT32 offset )
{
  dis_page_cmd_t tp;
  
  f_read(cmdfd, offset, (UINT8 *)&tp, sizeof(dis_page_cmd_t));
  
  gpage.flag = FALSE;//ֹ֮ͣǰ�ļ���
  if(FALSE == sys_page_display_store_fun(tp.default_page_id,tp.default_page_id,gSys_tp.gpage_nowid,0,0))//����ҳ��ʾ���� Ĭ��ҳ��Ҫ��ʾ��ҳ����Ļ��ʾ��ҳ��ͣ��ʱ�䣬�Ѿ��߹���ʱ�䡢״̬��־
    return -1;
  gEventFlag |= EVENT_FALG_DISPLAY_PAGE;
  
  return sizeof(dis_page_cmd_t);
}

STATIC int32_t cmd6_fun(file_id_t cmdfd, UINT32 offset)
{
#define UPDATE_ROM_SIZE ((UINT16)0x7000)
  UINT16 rom_len =0;
  UINT16 len = 0;
  UINT8 temp_buf[512];
  const UINT8 temp[]="hanshowsromupdata";
  
  memset(temp_buf,0x00,512);
  memset((UINT8 *)&updata_info,0x00,sizeof(ROM_UP_T));
  
  
  offset += 1;
  f_read(cmdfd, offset, (UINT8 *)&rom_len, sizeof(rom_len));
  
  if(rom_len != UPDATE_ROM_SIZE)
  {
    gerr_info = TR3_OSD_CMD6_ERR;
    goto loop;
  }
  
  updata_info.crc = 0;
  updata_info.flag = GROM_UPDATA_FLAG;
  gEventFlag |= EVENT_FLAG_UPDATA_ROM;
  updata_info.addr  = f_addr(cmdfd);
  updata_info.addr += offset;
  
  
  updata_info.crc = my_cal_crc16(updata_info.crc,(UINT8 *)temp,strlen((char const *)temp));
  for(len = 0;len < rom_len;)
  {
    f_read(cmdfd, offset+sizeof(rom_len)+len, temp_buf, 512);
    updata_info.crc = my_cal_crc16(updata_info.crc,temp_buf,512);
    len += 512;
  }
loop:
  return (1 + rom_len + sizeof(rom_len));
  
}

STATIC int32_t cmd7_fun(file_id_t cmdfd, UINT32 offset )
{
#define RC_DATA_START_ADDR 4
  
  UINT16 len = 0;
  UINT8 temp_buf[32];
  
  f_read(cmdfd, offset, temp_buf, 30);
  offset += 1;
  memcpy((UINT8 *)&len,temp_buf+1,sizeof(len));
  memcpy((UINT8 *)rc_attr_info.secur_code,temp_buf+1+sizeof(len),20);
  
  gpage.flag = FALSE;//ֹ֮ͣǰ�ļ���
  //ң����Ҳ���ı�defaultҳ
  if(rc_attr_info.display_time ==0 )//ͣ��ʱ����0sʱ��Ĭ����ʾĬ��ҳ
    rc_attr_info.page_num = gSys_tp.default_page_id;
  if(FALSE == sys_page_display_store_fun( gSys_tp.default_page_id,rc_attr_info.page_num,gSys_tp.gpage_nowid ,rc_attr_info.display_time,0))//����ҳ��ʾ���� Ĭ��ҳ��Ҫ��ʾ��ҳ����ǰ��ʾ��ҳ��ͣ��ʱ�䣬�Ѿ��߹���ʱ�䡢״̬��־
    return -1;
  
  gEventFlag |= EVENT_FALG_DISPLAY_PAGE;
  
/*
  if((rc_attr_info.t0_time ==0)&&(rc_attr_info.t1_time ==0)&&(rc_attr_info.t2_time ==0)&&(rc_attr_info.period ==0) ||(rc_attr_info.period>0))
  {
    led_clear_fun();
    led_tp_init_fun(0);
    rc_led_choose();
    led_tp_info.timer_enable = TRUE;
    gEventFlag |= EVENT_FLAG_INTER_LED;
    led_all_off();
    magent4_clear_fun(TRUE);//���magent==2 ���ɻɹ���һ������g�ص� 
  }
  */
   rc_led_init();
  return (1 + len + sizeof(len));
  
}
STATIC int32_t cmd8_fun(file_id_t cmdfd, UINT32 offset )
{
  UINT16 len = 0,crc= 0;
  UINT8 temp_buf[32];
  
  f_read(cmdfd, offset, temp_buf, 30);
  offset += 1;
  memcpy((UINT8 *)&len,temp_buf+1,sizeof(len));
  memcpy((UINT8 *)&epd_attr_info,temp_buf+1+sizeof(len),sizeof(epd_attr_info));
  
  epd_attr_info.global_crc = my_cal_crc16(crc,(UINT8 *)&epd_attr_info,sizeof(epd_attr_info)-sizeof(epd_attr_info.global_crc));
  save_state_info_fun();
  
  return (1 + len + sizeof(len));
}
int32_t cmd_err_fun(file_id_t cmdfd, UINT32 offset )
{
  return -1;
}

typedef int32_t (*cmd_cb_t) (file_id_t cmdfd, UINT32 offset);
static const cmd_cb_t cmd_cb[] = {cmd_err_fun,cmd1_fun,cmd2_fun,cmd3_fun,cmd4_fun,cmd5_fun,cmd6_fun,cmd7_fun,cmd8_fun};

STATIC int32_t cmd_order(file_id_t cmdfd, UINT32 offset)
{
  UINT8 cmd = 0;
  
  f_read(cmdfd,offset, &cmd, 1);
  if (cmd == OSD_END_CMD)
  {
    gcmd_tp.len = offset - gcmd_tp.start_addr + 1;//cmd0x76
    f_read(cmdfd, offset+1, (UINT8 *)&g_crc , sizeof(osd_crc_t)); 
    return 0;
  }
  if((cmd <= OSD_CMD_SUN) && (cmd != 0))
    return cmd_cb[cmd](cmdfd, offset);	
  else
    gerr_info = TR3_OSD_CMDERR;
  return -1;
}

//extern UINT8 cmd2_buf[];
extern UINT32 cmd_before_offset;
BOOL process_cmd(file_id_t cmdfd)
{
  UINT32 i = 0;
  int32_t n = 1;
  gcmd_tp.start_addr = cmd_before_offset;
  for (i = 0; n > 0; i += n)
  {
    //	//fprintf(stderr,"[%s]: i = %d,",__FUNCTION__,i);
    n = cmd_order(cmdfd, i + cmd_before_offset);
    //	//fprintf(stderr,"[%s]:cmd size %d\n***************\n",__FUNCTION__, n);
  }
  
  if(n < 0)
    return FALSE;
  return TRUE;
}
