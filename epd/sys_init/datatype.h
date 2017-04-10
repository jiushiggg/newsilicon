#ifndef __HOLTEK_MSP430_DATATYPE_H__
#define __HOLTEK_MSP430_DATATYPE_H__


/** @defgroup datatype
*  数据类型定义
*  @{
*/

#include <stdint.h>
typedef int16_t INT16;
typedef uint16_t UINT16;
typedef int32_t INT32;
typedef uint32_t UINT32;
typedef int64_t INT64;
typedef uint64_t UINT64;

#ifndef OSDTEST
typedef uint32_t WORD;
#define STATIC static

#else

#define STATIC 
#ifdef __x86_64__
typedef uint64_t WORD;
#endif

#ifdef __i386__
typedef uint32_t WORD;
#endif

#endif

typedef char INT8;
typedef unsigned char UINT8, UCHAR, BOOL, BYTE;
typedef float FLOAT;
typedef double DOUBLE;

//通用定义
#define TRUE 1	
#define FALSE 0

typedef void VOID;
#ifndef NULL
#define NULL ((void*)0)
#endif

#define MAX_TEMP_BUF	64
#define MAX_TEMP_CARRY  (sizeof(TEMP.osd_struct_temp.osd_data))

/**@}*/ //datatype

#endif
