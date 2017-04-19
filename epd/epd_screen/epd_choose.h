#ifndef __HOLTEK_MSP430_EPD_H__
#define __HOLTEK_MSP430_EPD_H__


//---------------------------------------------START------------------------------

/*
*下面的宏是新屏幕带温度传感器
*/
//#define EPD_WF_154_TEP
//#define EPD_WF_154_RED_TEP
//#define EPD_WF_213_TEP
//#define EPD_WF_213_RED_TEP
//#define LTP_EPD_WF_213
//#define WF_JX_29_WB_TEP
//#define  WF_JX_29_RED_TEP
//#define WF_JX_75_BW
//#define WF_JX_75_RED        
//#define WF_420_RED_SINGLE
//#define WF_420_BW_SINGLE
//#define WF_420_RED_DOUBLE


#define EPD_HLT_209_WB
//#define EPD_HLT_213_BW
//#define EPD_HLT_154_WB_2450
//#define EPD_HLT_154_WB_2477
//#define EPD_HLT_420_BW
//#define EPD_HLT_213_BW_LOW
//#define EPD_HLT_213_RED_TEP
//#define EPD_HLT_209_RED_TEP


#ifdef EPD_HLT_209_RED_TEP
#define EPD_POWER_OFTEN_OPEN
//#define EPD_NEW_ENCLOSURE
#define SCREEN_RELOAD_OPEN
#define SCREEN_DSIPLAY_RELOAD_CONT  ((UINT16)39600)  //22小时

#define DIRECTION_0
#define DIRECTION_90
#define DIRECTION_180
#define DIRECTION_270

#define VERSION_OP5       (105)  
#define SCREEN_WIDTH    (WORD)(296)
#define SCREEN_HEIGHT   (WORD)(128)                    
#define ELEMENT_SIZE_K (WORD)(172)             //存放元素data区
#endif

#ifdef EPD_HLT_209_WB
#define EPD_POWER_OFTEN_OPEN
//#define EPD_NEW_ENCLOSURE
#define DIRECTION_0
#define DIRECTION_90
#define DIRECTION_180
#define DIRECTION_270

#define VERSION_OP5      (100)
#define SCREEN_WIDTH    (WORD)(296)
#define SCREEN_HEIGHT   (WORD)(128)                    
#define ELEMENT_SIZE_K (WORD)(172)             //存放元素data区
#endif

#ifdef WF_JX_29_RED_TEP
//#define EPD_NEW_ENCLOSURE
#define DIRECTION_0
#define DIRECTION_90
#define DIRECTION_180
#define DIRECTION_270

#define VERSION_OP5      ((UINT16)(81))  
#define SCREEN_WIDTH    (WORD)(296)
#define SCREEN_HEIGHT   (WORD)(128)                            
#define ELEMENT_SIZE_K (WORD)(172)             
#endif

#ifdef EPD_HLT_213_BW
#define EPD_POWER_OFTEN_OPEN
#define DIRECTION_0
#define DIRECTION_90
#define DIRECTION_180
#define DIRECTION_270

#define VERSION_OP5       (101)  
#define SCREEN_WIDTH    (WORD)(250)
#define SCREEN_HEIGHT   (WORD)(128)               
#define ELEMENT_SIZE_K (WORD)(172)           
#endif

#ifdef  EPD_HLT_213_RED_TEP
#define EPD_POWER_OFTEN_OPEN
#define SCREEN_RELOAD_OPEN
#define SCREEN_DSIPLAY_RELOAD_CONT  ((UINT16)39600)  //22小时
#define DIRECTION_0
#define DIRECTION_90
#define DIRECTION_180
#define DIRECTION_270

#define VERSION_OP5       (104)  
#define SCREEN_WIDTH    (WORD)(212)
#define SCREEN_HEIGHT   (WORD)(104)               
#define ELEMENT_SIZE_K (WORD)(172)           
#endif

#ifdef EPD_HLT_154_WB_2450
#define EPD_POWER_OFTEN_OPEN
#define EPD_HLT_154_WB
#define DIRECTION_0
#define DIRECTION_90
#define DIRECTION_180
#define DIRECTION_270

#define VERSION_OP5       (102)  
#define SCREEN_WIDTH    (WORD)(200)
#define SCREEN_HEIGHT   (WORD)(200)                                                
#define ELEMENT_SIZE_K (WORD)(172)            
#endif

#ifdef EPD_HLT_154_WB_2477
#define EPD_POWER_OFTEN_OPEN
#define EPD_HLT_154_WB
#define DIRECTION_0
#define DIRECTION_90
#define DIRECTION_180
#define DIRECTION_270

#define VERSION_OP5       (103)  
#define SCREEN_WIDTH    (WORD)(200)
#define SCREEN_HEIGHT   (WORD)(200)                                                
#define ELEMENT_SIZE_K (WORD)(172)            
#endif

#ifdef EPD_WF_154_TEP

#define DIRECTION_0
#define DIRECTION_90
#define DIRECTION_180
#define DIRECTION_270

#define VERSION_OP5       (84)  
#define SCREEN_WIDTH    (WORD)(152)
#define SCREEN_HEIGHT   (WORD)(152)
#define ELEMENT_SIZE_K (WORD)(172)             //存放元素data区
#endif

#ifdef EPD_WF_154_RED_TEP

#define DIRECTION_0
#define DIRECTION_90
#define DIRECTION_180
#define DIRECTION_270

#define VERSION_OP5       (85)  
#define SCREEN_WIDTH    (WORD)(152)
#define SCREEN_HEIGHT   (WORD)(152)
#define ELEMENT_SIZE_K (WORD)(172)             //存放元素data区
#endif


#ifdef WF_JX_29_WB_TEP
//#define EPD_NEW_ENCLOSURE
#define DIRECTION_0
#define DIRECTION_90
#define DIRECTION_180
#define DIRECTION_270

#define VERSION_OP5      ((UINT16)(86))  
#define SCREEN_WIDTH    (WORD)(296)
#define SCREEN_HEIGHT   (WORD)(128)                            
#define ELEMENT_SIZE_K (WORD)(172)             
#endif


#ifdef EPD_WF_213_TEP
//#define EPD_NEW_ENCLOSURE
#define DIRECTION_0
#define DIRECTION_90
#define DIRECTION_180
#define DIRECTION_270 

#define VERSION_OP5       (87)  
#define SCREEN_WIDTH    (WORD)(212)
#define SCREEN_HEIGHT   (WORD)(104)
#define ELEMENT_SIZE_K (WORD)(172)             //存放元素data区
#endif

#ifdef EPD_WF_213_RED_TEP
//#define EPD_NEW_ENCLOSURE
#define DIRECTION_0
#define DIRECTION_90
#define DIRECTION_180
#define DIRECTION_270 

#define VERSION_OP5       (88)  
#define SCREEN_WIDTH    (WORD)(212)
#define SCREEN_HEIGHT   (WORD)(104)
#define ELEMENT_SIZE_K (WORD)(172)             //存放元素data区
#endif



#ifdef EPD_HLT_420_BW
#define EPD_POWER_OFTEN_OPEN
#define DIRECTION_0
#define DIRECTION_90
#define DIRECTION_180
#define DIRECTION_270 

#define VERSION_OP5       (104)  
#define SCREEN_WIDTH    (WORD)(300)
#define SCREEN_HEIGHT   (WORD)(400)
#define ELEMENT_SIZE_K (WORD)(256)             //存放元素data区
#define EXTERN_FALSH_4M
#endif


#ifdef WF_420_RED_SINGLE

#define DIRECTION_0
#define DIRECTION_90
#define DIRECTION_180
#define DIRECTION_270 

#define VERSION_OP5       (90)  
#define SCREEN_WIDTH    (WORD)(300)
#define SCREEN_HEIGHT   (WORD)(400)
#define ELEMENT_SIZE_K (WORD)(256)             //存放元素data区
#define EXTERN_FALSH_4M
#endif



#ifdef WF_420_BW_SINGLE

#define DIRECTION_0
#define DIRECTION_90
#define DIRECTION_180
#define DIRECTION_270 

#define VERSION_OP5       (91)  
#define SCREEN_WIDTH    (WORD)(300)
#define SCREEN_HEIGHT   (WORD)(400)
#define ELEMENT_SIZE_K (WORD)(256)             //存放元素data区
#define EXTERN_FALSH_4M
#endif

#ifdef WF_420_RED_DOUBLE

#define DIRECTION_0
#define DIRECTION_90
#define DIRECTION_180
#define DIRECTION_270 

#define VERSION_OP5       (92)  
#define SCREEN_WIDTH    (WORD)(300)
#define SCREEN_HEIGHT   (WORD)(400)
#define ELEMENT_SIZE_K (WORD)(256)             //存放元素data区
#define EXTERN_FALSH_4M
#endif

#ifdef EPD_HLT_213_BW_LOW
#define EPD_POWER_OFTEN_OPEN
#define DIRECTION_0
#define DIRECTION_90
#define DIRECTION_180
#define DIRECTION_270

#define VERSION_OP5       (93)  
#define SCREEN_WIDTH    (WORD)(250)
#define SCREEN_HEIGHT   (WORD)(128)               
#define ELEMENT_SIZE_K (WORD)(172)           
#endif


#ifdef WF_JX_75_BW
//#define NFC_DISABLE
//#define WRITE_ID_NEW_LCD   
#define DIRECTION_0
#define DIRECTION_90
#define DIRECTION_180
#define DIRECTION_270

#define VERSION_OP5       (4+90)  
#define SCREEN_WIDTH    (WORD)(384)
#define SCREEN_HEIGHT   (WORD)(640)                           //屏幕显示区 
#define PKG_NUM    (WORD)(2000)                                     //接收数据包data缓冲区大小168个数据包
#define ELEMENT_SIZE_K (WORD)(256)             //存放元素data区
//此处占用flash大小为108k
#define SCREEN_POWER_DOWN()  
#endif

#ifdef WF_JX_75_RED
//#define NFC_DISABLE
//#define WRITE_ID_NEW_LCD   
#define DIRECTION_0
#define DIRECTION_90
#define DIRECTION_180
#define DIRECTION_270

#define VERSION_OP5       (5+90)  
#define SCREEN_WIDTH    (WORD)(384)
#define SCREEN_HEIGHT   (WORD)(640)                           //屏幕显示区 
#define PKG_NUM    (WORD)(2000)                                     //接收数据包data缓冲区大小168个数据包
#define ELEMENT_SIZE_K (WORD)(256)             //存放元素data区
//此处占用flash大小为108k
#define SCREEN_POWER_DOWN()  
#endif

void Init_EPD(void);
#endif
