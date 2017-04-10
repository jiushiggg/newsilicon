#include <stdio.h>
#include <string.h>
#include "..\epd_screen\epd_choose.h"
#include "..\file_sys\mini_fs.h"
#include "osd_cmd.h"
#include "sys_init.h"
#include "bit_display.h"
#include "global_variable.h"


#define is_not_chaced(C, d, n) (((d) != C->fd) || ((n) < C->index - CACHE_SIZE) || ((n) >= C->index))


//0=>left, 1=>right, 2=>middle, 3=>left

static UINT16 get_space(UINT16 space, UINT8 i)
{
  UINT16 s[] = {0, space, space/2};
  return s[i];
}
static UINT16 get_space_num( const UINT16 end, const UINT16 start, const UINT16 width)
{
  return ( (width > ( end - start +1 )) ? 0 : (end - start +1 - width));
}

static UINT16 get_space_high_0 ( UINT8 i,UINT16 font_h,UINT16 start,UINT16 end)
{
  UINT16 s[] = {start, start + (end-start-font_h+1)/2 , end-font_h+1};
  return s[i];
}
static UINT16 get_space_high_180 ( UINT8 i,UINT16 font_h,UINT16 start,UINT16 end)
{
  UINT16 s[] = {end - font_h , (end - font_h ) - ((end-start-font_h+1)/2) , start};
  return s[i];
}
static UINT16 get_space_width_270 ( UINT8 i,UINT16 font_w,UINT16 start,UINT16 end)//顶端对齐，垂直居中，低端对齐
{
  UINT16 s[] = { start , start + ((end-start-font_w+1)/2), (end - font_w) };
  return s[i];
}

static UINT16 get_space_high_270 ( UINT8 i,UINT16 num_len,UINT16 start,UINT16 end,UINT16 font_w)//左对齐，右对齐，居中
{
   UINT16 s[] = {end-font_w, start + num_len- font_w ,end - (end-start-num_len+1)/2 - font_w};
  return s[i];
}
static UINT16 get_space_width_90 ( UINT8 i,UINT16 font_w,UINT16 start,UINT16 end)//顶端对齐，垂直居中，低端对齐
{
  UINT16 s[] = { (end - font_w ) , start + ((end-start+1-font_w)/2),start };
  return s[i];
}
static UINT16 get_space_high_90 ( UINT8 i,UINT16 num_len,UINT16 start,UINT16 end)//左对齐，右对齐，居中
{
  UINT16 s[] = {start ,end - num_len, start + ((end-start-num_len+1)/2)};
  return s[i];
}



UINT32 get_dst_start_bit(const font_arrt_t *p, const num_ft *ft, UINT16 width, UINT8 dire,UINT8 dh,UINT8 dw,dis_format_t *format)
{

  UINT16 space;
  UINT32 offset = 0;
  UINT16 ah = p->ey - p->sy +1;
  UINT16 aw = p->ex - p->sx +1;
  
  if(format->horizontal_alignment >2)
    format->horizontal_alignment = 0;
  if(format->vertical_alignment >2)
    format->vertical_alignment = 0;
  if(aw < width)//宽度不够
    format->horizontal_alignment = 0;
  if(ah<ft->font_h)//高度不够
    format->vertical_alignment = 0;
  switch (dire)
  {
#ifdef  DIRECTION_0 
  case 0: 
    space = get_space_num(p->ex, p->sx, width);
    offset = (p->sx + get_space(space, format->horizontal_alignment)) * SCREEN_HEIGHT ;//水平对齐方式
    offset += get_space_high_0(format->vertical_alignment,ft->font_h,p->sy,p->ey); //垂直对齐方式
    break;
#endif
#ifdef  DIRECTION_90
  case 1:
    offset = get_space_width_90(format->vertical_alignment,ft->font_w,p->sx,p->ex) * SCREEN_HEIGHT ;//垂直对齐方式
    offset += get_space_high_90(format->horizontal_alignment,width,p->sy,p->ey);//水平对齐方式
    break;
#endif
#ifdef  DIRECTION_180     
  case 2:
    space = get_space_num(p->ex , p->sx , width);
    offset = (p->ex - ft->font_w +1 - get_space(space, format->horizontal_alignment)) * SCREEN_HEIGHT ;//水平对齐方式
    offset += get_space_high_180(format->vertical_alignment,ft->font_h,p->sy,p->ey); //垂直对齐方式
    break;
#endif    
#ifdef  DIRECTION_270
  case 3:
    offset = get_space_width_270(format->vertical_alignment,ft->font_w,p->sx,p->ex) * SCREEN_HEIGHT ;//垂直对齐方式
    offset += get_space_high_270(format->horizontal_alignment,width,p->sy,p->ey,ft->font_h);//水平对齐方式
    break;
#endif   
  }
  
  return offset;
}

void alignment_dignum(const UINT8 dire,const UINT8 h ,const UINT8 w , const   UINT8 dis_format,UINT8 num)
{
  //UINT8 i = 0,k= 0;
  esl_screen_block_t *sb =  &TEMP.td.sb;
  
  if(dis_format > 1 )
    return ;
  
 // for(i = 0 ; i < 2; i++,k++)
  {
    f_read(F_SB,num * sizeof(esl_screen_block_t),(UINT8 *)sb,sizeof(esl_screen_block_t));//读取要补充数字的sb属性
    if(dis_format ==0)
    {
#ifdef  DIRECTION_90
      if(dire == 1)
      {
        sb->dst_startbit -= (w-sb->w)*SCREEN_HEIGHT;
        sb->w = w -sb->w;           
      } 
#endif
#ifdef  DIRECTION_270  
      if(dire == 3)
      {
        sb->dst_startbit += (sb->w * SCREEN_HEIGHT);
        sb->w = w -sb->w;            
      } 
#endif
#ifdef  DIRECTION_0      
      if(dire == 0)
      {
        sb->dst_startbit += (sb->h);
        sb->h = h -sb->h;           
      } 
#endif
#ifdef  DIRECTION_180       
      if(dire == 2)
      {
        sb->dst_startbit -= (h-sb->h);
        sb->h = h -sb->h;            
      }
#endif
      
    }
    if(dis_format ==1)
    {
#ifdef  DIRECTION_90     
      if(dire == 1)
      {
        sb->dst_startbit += (sb->w * SCREEN_HEIGHT);
        sb->w = w -sb->w;              
      } 
#endif
#ifdef  DIRECTION_270    
      if(dire == 3)
      {
        sb->dst_startbit -= (w-sb->w)*SCREEN_HEIGHT;
        sb->w = w -sb->w;            
      }  
#endif
#ifdef  DIRECTION_0 
      if(dire == 0)
      {
        sb->dst_startbit -= (h-sb->h);
        sb->h = h -sb->h; 
      } 
#endif
#ifdef  DIRECTION_180     
      if(dire == 2)
      {
        sb->dst_startbit += (sb->h);
        sb->h = h -sb->h; 
      }   
#endif
    }
    
    sb->attr.ispadd = 1;  
    sb->attr.paddinfo =0; 
    f_write_direct(FILE_SB,(gdis_id_num)*sizeof(esl_screen_block_t),(UINT8 *)sb,sizeof(esl_screen_block_t));
    gdis_id_num++;
  }
}

UINT32 get_dst_bit(UINT32 provbit, UINT8 pw, UINT8 ph, const font_arrt_t *p, const num_ft *ft,property_t *pro_tp,dis_format_t *attr_tp)
{
  UINT8 dir = pro_tp->direction;
  
  
  //fprintf(stderr, "dir %d provbit %d, %d, %d, %d, %d, isdown? %d, ", dir, provbit, w, h, ft->w, ft->h, ft->isdown);
  switch (dir) {
#ifdef  DIRECTION_0 
  case 0:
    provbit += pw * SCREEN_HEIGHT;
    if (attr_tp->dis_decimals_mode)
      provbit += (ph - ft->font_h);
    break;
#endif
#ifdef  DIRECTION_90
  case 1:
    provbit += ph;
    //fprintf(stderr,"w=%d,pw=%d,ftw=%d,",w,pw,ft->w);
    if (!attr_tp->dis_decimals_mode)
      provbit += (pw - ft->font_w) * SCREEN_HEIGHT;
    break;
#endif
#ifdef  DIRECTION_180      
  case 2:
    //fprintf(stderr,"h=%d,ph=%d,fth=%d,",h,ph,ft->h);
    provbit -= ft->font_w * SCREEN_HEIGHT;
    if (!attr_tp->dis_decimals_mode)
      provbit += (ph - ft->font_h);
    break;
#endif  
#ifdef  DIRECTION_270
  case 3:
#endif
  default:
    provbit -= ft->font_h;
    if (attr_tp->dis_decimals_mode)
      provbit += (pw - ft->font_w) * SCREEN_HEIGHT;
    break;
    
  }
  //fprintf(stderr, "out bit %d\n", provbit);
  return provbit;
}


static void refresh_cache(bit_cache_t *c_temp, file_id_t fd, UINT32 n) {
  if (c_temp->index == CACHE_NOT_CLEAN || is_not_chaced(c_temp, fd, n)) { 
    //fprintf(stderr, "refresh cache for %d, index %u, n %u\n", fd, *cache_index, n);
    c_temp->fd = fd;
    c_temp->index = n / CACHE_SIZE * CACHE_SIZE; 
    f_read(c_temp->fd, c_temp->index, c_temp->cache, CACHE_SIZE); 
    c_temp->index += CACHE_SIZE; 
  } 
}

#if 0
static UINT8 get_bit(file_id_t src, UINT32 bitn, bit_cache_t *C) { 
  UINT32 n = bitn /8; 
  refresh_cache(C, src, n);
  return (C->cache[n % CACHE_SIZE] & (1 << (bitn % 8))) ? 1 : 0; 
} 

static void set_bit(file_id_t dst, UINT32 bitn, UINT8 val, bit_cache_t *C) { 
  UINT32 n = bitn /8;
  
  if (C->index != CACHE_NOT_CLEAN && (C->fd != dst || (n < C->index - CACHE_SIZE) || (n >= C->index)))
    f_write(C->fd, C->index - CACHE_SIZE, C->cache, CACHE_SIZE);
  refresh_cache(C, dst, n);
  C->cache[n % CACHE_SIZE] |= (val << (bitn % 8));
} 

/*
* 带缓存的bit搬移函数，
* 从文件src的第src_bitn个bit开始，连续拷贝bit_len个bit，到dst的第dst_bitn开始的位置
* isrevert控制是否需要反转bit
*/
static void bit_move(file_id_t dst, UINT32 dst_bitn, file_id_t src, \
  UINT32 src_bitn, UINT32 bit_len, UINT8 isrevert, bit_cache_t *get_bit_cache, bit_cache_t *set_bit_cache)
{
  UINT32 i;
  UINT8 bit;
  
  for (i = 0; i < bit_len; i++) {
    bit = get_bit(src, src_bitn + i, get_bit_cache);
    if (isrevert)
      bit = bit ? 0 : 1;
      set_bit(dst, dst_bitn + i, bit, set_bit_cache); 
  }
}

static UINT8 move_one_block(const screen_block_t *sb, bit_cache_t *get_bit_cache, bit_cache_t *set_bit_cache) {
  int i;
  
  
  for (i = 0; i < sb->w; i++)
    bit_move(sb->dstfd, sb->dst_startbit + i * SCREEN_HEIGHT, \
      sb->srcfd, sb->src_startbit + i * sb->h, sb->h, sb->attr.isrevert, get_bit_cache, set_bit_cache);
  f_write(set_bit_cache->fd, set_bit_cache->index - CACHE_SIZE, set_bit_cache->cache, CACHE_SIZE);
  return 0;
}

static void dump_sb(screen_block_t *sb) {
  fprintf(stderr, "sb->src:%d, sb->dst:%d\n", sb->srcfd, sb->dstfd);
}
#endif





static UINT8 dis_check_sb(esl_screen_block_t *sb)
{
  if(sb->src_startbit >= ((f_addr(sb->srcfd) +f_size(sb->srcfd))* 8))
  {
    gerr_info = TR3_OSD_SB_START_ERR;
    return 0;
  }
  if((sb->dst_startbit / SCREEN_HEIGHT) > SCREEN_WIDTH ) 
  {
    gerr_info = TR3_OSD_SB_END_ADDR_ERR;
    return 0;
  }
  if( (sb->src_startbit +(sb->w * sb->h *8)) >=  ((f_addr(sb->srcfd) +f_size(sb->srcfd))* 8) )
  {
    gerr_info = TR3_OSD_SB_ERR;
    return 0;
  }
  if ((sb->dst_startbit / SCREEN_HEIGHT  + sb->w) > (SCREEN_WIDTH  ))
    sb->w = SCREEN_WIDTH  - (sb->dst_startbit / SCREEN_HEIGHT); 
  if ((sb->dst_startbit % SCREEN_HEIGHT  + sb->h) > (SCREEN_HEIGHT  ))
    sb->h = SCREEN_HEIGHT  - (sb->dst_startbit % SCREEN_HEIGHT) ; 
  return 1;
}

UINT8 dis2screen(file_id_t sbfd, UINT8 sbn) 
{
  UINT16 j, k;
  UINT8 i, bit;
  UINT32 sbit, dbit, byten;
  // UINT8 tp[64] = {0},tp1[64] = {0},tp2[64] = {0};
  esl_screen_block_t *sb = (void *)&TEMP.BYTE64[44];
  bit_cache_t *C1 = (void *)&TEMP.BYTE64[0], *C2 = (void *)&TEMP.BYTE64[24];
  //esl_screen_block_t *sb = (void *)tp;
  //bit_cache_t *C1 = (void *)tp1, *C2 = (void *)tp2;
  
  //清理bit缓存
  C1->index = CACHE_NOT_CLEAN, C2->index = CACHE_NOT_CLEAN;
  
  for (i = 0; i < sbn; i++) {
    //读取单个图层属性
    SetWathcDog();
    f_read(sbfd, sizeof(esl_screen_block_t) * i, (UINT8 *)sb, sizeof(esl_screen_block_t));
    if(0 == dis_check_sb(sb))
      return 0;
    //搬移单个图层到屏幕映射区
    //move_one_block(&sb, &get_bit_cache, &set_bit_cache);
    sbit = sb->src_startbit; dbit = sb->dst_startbit;
    for (j = 0; j < sb->w; j++, sbit += (sb->h+7)/8*8, dbit += SCREEN_HEIGHT) {
      SetWathcDog();  //喂狗 
      for (k = 0; k < sb->h; k++) {
        
        byten = (sbit + k) /8;
        if (C1->index == CACHE_NOT_CLEAN || is_not_chaced(C1, sb->srcfd, byten))	
          refresh_cache(C1, sb->srcfd, byten);
        bit = (C1->cache[byten % CACHE_SIZE] & (1 << (7-(sbit + k) % 8))) ? 1 : 0;  
        if (sb->attr.reverse_flag)//反显示
          bit = bit ? 0 : 1;
          if (sb->attr.ispadd)//数字还是图片
            bit = sb->attr.paddinfo;//填充的颜色
          
          //set_bit(sb.dstfd, sb.dst_startbit + j * SCREEN_HEIGHT + k, bit, &set_bit_cache); 
          byten = (dbit+k) /8;
          if (C2->index != CACHE_NOT_CLEAN && is_not_chaced(C2, sb->dstfd, byten))
            f_write_direct(C2->fd, C2->index - CACHE_SIZE, C2->cache, CACHE_SIZE);
          if (C2->index == CACHE_NOT_CLEAN || is_not_chaced(C2, sb->dstfd, byten))
            refresh_cache(C2, sb->dstfd, byten);
          
          C2->cache[byten % CACHE_SIZE] &= ~(1 << (7 - (dbit + k) % 8));
          C2->cache[byten % CACHE_SIZE] |= (bit << (7- (dbit + k) % 8));
      }
    }    
  }
  
  //flush 最后一帧bit流
  f_write_direct(C2->fd, C2->index - CACHE_SIZE, C2->cache, CACHE_SIZE);
  
  return 1;
}
