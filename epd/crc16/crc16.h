#ifndef __HOLTEK_MSP430_CRC16_H__
#define __HOLTEK_MSP430_CRC16_H__

UINT16 my_cal_crc16(UINT16 crc,UINT8 *ptr,UINT16 len);
BOOL rf_pgk_crc(UINT8 *ptr,UINT16 len,RFID rfid);
UINT16 ack_crc_fun(UINT8 *ptr,UINT16 len);
#endif