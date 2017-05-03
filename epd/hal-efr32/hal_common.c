/***************************************************************************//**
 * @file hal_common.c
 * @brief This file holds generic setup functions for the test app to help
 * abstract between the different hardware platforms.
 * @copyright Copyright 2015 Silicon Laboratories, Inc. http://www.silabs.com
 ******************************************************************************/

#include "hal_common.h"
#include "rail_types.h"

#include "em_cmu.h"
#include "rtcdriver.h"
#include "ustimer.h"

volatile uint32_t msTicks; /* counts 1ms timeTicks */

// Define a WEAK macro to work across different compilers
#ifdef __ICCARM__
  #define WEAK __weak
#elif defined(__GNUC__)
  #define WEAK __attribute__ ((weak))
#else
  #define WEAK
#endif

WEAK void halInitChipSpecific(void)
{
  // Do nothing, but allow each chip to override this if necessary
}

void halInit(void)
{
  // Call the chip specific hal init
  halInitChipSpecific();

  // Initialize the microsecond timer
  USTIMER_Init();

  //RTC Init -- for system timekeeping and other useful things
  RTCDRV_Init();
}

uint32_t halCommonGetInt32uMillisecondTick(void)
{
  return RTCDRV_TicksToMsec(RTCDRV_GetWallClockTicks64());
}

void halCommonDelayMicroseconds(uint16_t us)
{
  USTIMER_Init();
  USTIMER_DelayIntSafe(us);
}

void halCommonMemMove(void *dest, const void *src, uint16_t bytes)
{
  uint8_t *d = (uint8_t *)dest;
  uint8_t *s = (uint8_t *)src;

  if (d > s) {
    d += bytes - 1;
    s += bytes - 1;
    #ifndef _HAL_SMALL_MEMUTILS_
      while(bytes >= 4) {
        bytes -= 4;
        *d-- = *s--;
        *d-- = *s--;
        *d-- = *s--;
        *d-- = *s--;
      }
    #endif // _HAL_SMALL_MEMUTILS_
    for(; bytes; bytes--) {
      *d-- = *s--;
    }
  } else {
    #ifndef _HAL_SMALL_MEMUTILS_
      while(bytes >= 4) {
        bytes -= 4;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
      }
    #endif // _HAL_SMALL_MEMUTILS_
    for(; bytes; bytes--) {
      *d++ = *s++;
    }
  }
}
