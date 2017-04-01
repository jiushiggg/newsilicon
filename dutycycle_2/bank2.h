#ifndef _BANK2_H_
#define _BANK2_H_

#include "datatype.h"

typedef struct {
	unsigned char b0 : 1;
	unsigned char b1 : 1;
	unsigned char b2 : 1;
	unsigned char b3 : 1;
	unsigned char b4 : 1;
	unsigned char b5 : 1;
	unsigned char b6 : 1;
	unsigned char b7 : 1;
} _stbits;
typedef union{
	_stbits bvar;
	UINT8   var;
}bits;

bits var_0;
#define gRxOverflow 	var_0.bvar.b0
#define bmulti_en 		var_0.bvar.b1
#define bglitter_dsp 	var_0.bvar.b2
#define bglitter_en   	var_0.bvar.b3
#define gEEPROM 		var_0.bvar.b4
#define blow_vol_flg 	var_0.bvar.b5
#define bled_glitter 	var_0.bvar.b6
#define grx3ms_overflow var_0.bvar.b7

bits var_1;
#define boverflow 	var_1.bvar.b0
#define bnetlink	var_1.bvar.b1
#define bquery		var_1.bvar.b2
#define bheartbeat_en		var_1.bvar.b5
#define brf_set_flg		var_1.bvar.b6
#define bremote_flg		var_1.bvar.b7
bits var_2;
#define hbreq_en		var_2.bvar.b1
#define blast_pkg_flg   var_2.bvar.b2
#define bscroll		var_2.bvar.b3
#define bwtd		var_2.bvar.b4
#define gRFInitData grf.info

//bank0�б���

UINT8 gRFError;
UINT8 gRFPowerFlag;
stHBReq hbreq;
UINT8 gpkg_cnt;		//���ż���
UINT8 * tmp_p;
eRF_STATE gmode_backup;			//RFģʽ
UINT8 screen_num;
stRF_ST gRF_mode;				//7
stRFInit grf;					//*	
UINT16 RF_DataCRC;
UINT8 spidata;
UINT16 gbt_trigger;	//bt�����ж�ֵ
UINT16 gEventFlag;	//�¼�������־

UINT16 	glvd_cnt;	//
eLCD_STATE gdsp_mode;	//��ʾģʽ



 stRecvBuff gRFbuf;		//26���ֽ�
 UINT8 gdisplay_cnf ;	//����ʾλ
 stDisplayCnf gcnf;	//��ʾ����
 UINT16 gbit_map;		//�յ����ݰ���λͼ
 UINT8 swor_cnt;
 UINT8 gpkg_cnt_bak;
 UINT8 gvol_value;

UINT16 stb1_cnt;		//��ҳ����	
UINT16 gsave_flg;	//�洢��־
UINT8 gled_bit;	//�洢��־
 UINT16 gsolt;	//solt˯��ʱ��
UINT8 ghb_request;	//
UINT8 gLEDstate;	//
UINT16 glcd_config;	//LCD����
UINT16 gall_crc;			//set wor ���� *

UINT16 gwor_cnt;			//wor��֡����   
UINT16 remote_display_cnt;		

UINT8 gpage_num;				//ң����������ʾҳ
UINT16 gdisplay_time;			//ң����������ʾʱ��


stDisplayBuff gdisplay;//��ʾ����84BYTE *
UINT8 gdisplay_buf[2][20];//��ʾ����

stDisplayBuff gdisplay1;//��ʾ���� *
UINT8 gglitter_seg[20]; //��˸λ*
stLEDSet gled;          //LED���� *
stSysCnf gsyscnf;       //ϵͳ���� *
UINT8 work_time;       //ϵͳ���� *
UINT8 grp_wor_period;       //ϵͳ���� *

 RFINIT bak; 
st_tx_buff gtx;	//26�ֽ�
UINT16 crc_buff[9];	//18�ֽ�
UINT8 eeprom_buff[22];	//22�ֽ�

#endif
