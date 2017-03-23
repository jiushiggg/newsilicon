/***************************************************************************//**
 * @file error_rate_ci.c
 * @brief This file provides functionality to test RAIL error rates.
 * @copyright Copyright 2015 Silicon Laboratories, Inc. http://www.silabs.com
 ******************************************************************************/
#include "command_interpreter.h"
#include "response_print.h"

#include "rail.h"
#include "app_common.h"
#include "app_ci.h"

uint32_t perCount;
uint32_t perDelay;

void startPerMode(int argc, char **argv)
{
  uint32_t packets = ciGetUnsigned(argv[1]);
  uint32_t delayUs = ciGetUnsigned(argv[2]);

  if (!enableAppModeSync(PER, packets != 0, argv[0]))
  {
    return;
  }
  resetCounters(argc, argv);

  perCount = packets;
  perDelay = delayUs / 2;
  if (packets > 0)
  {
    RAIL_TimerSet(perDelay, RAIL_TIME_DELAY);
  }
}

void updateStats(int32_t newValue, Stats_t *stats)
{
  stats->samples++;
  if (stats->samples == 1)
  {
    stats->min = newValue;
    stats->max = newValue;
    stats->mean = newValue;
    stats->varianceTimesSamples = 0;
  }
  else
  {
    stats->min = (newValue < stats->min) ? newValue : stats->min;
    stats->max = (newValue > stats->max) ? newValue : stats->max;

    float delta = newValue - stats->mean;
    stats->mean += delta / stats->samples;
    // wikipedia.org/wiki/Algorithms_for_calculating_variance#On-line_algorithm
    // Update by adding (newValue - oldMean) * (newValue - newMean)
    stats->varianceTimesSamples += delta * (newValue - stats->mean);
  }
}

float variance(const Stats_t stats)
{
  return stats.varianceTimesSamples / (stats.samples - 1);
}

void getPerStats(int argc, char **argv)
{
  responsePrint(argv[0],
                "PerTriggers:%u"
                ",RssiMean:%f"
                ",RssiMin:%.2f"
                ",RssiMax:%.2f"
                ",RssiVariance:%f"
                ,counters.perTriggers
                ,counters.rssi.mean / 4
                ,((float) counters.rssi.min) / 4
                ,((float) counters.rssi.max) / 4
                ,variance(counters.rssi) / 16
               );
}

void berConfigSet(int argc, char **argv)
{
  RAIL_BerConfig_t berConfig;
  berConfig.bytesToTest = ciGetUnsigned(argv[1]);
  RAIL_BerConfigSet(&berConfig);
}

void berRx(int argc, char **argv)
{
  bool enable = !!ciGetUnsigned(argv[1]);

  if (!enableAppModeSync(BER, enable, argv[0]))
  {
    return;
  }
  resetCounters(argc, argv);

  if(enable)
  {
    RAIL_BerRxStart();
  }
  else
  {
    RAIL_BerRxStop();
  }
}

void berStatusGet(int argc, char **argv)
{
  RAIL_BerStatus_t berStats;
  float percentDone;
  float percentBitError;

  RAIL_BerStatusGet(&berStats);
  // don't divide by 0
  if(0 != berStats.bitsTotal)
  {
    percentDone = (float)((((double)berStats.bitsTested) / berStats.bitsTotal) * 100);
  }
  else
  {
    percentDone = 0.0;
  }
  // don't divide by 0
  if(0 != berStats.bitsTested)
  {
    percentBitError = (float)((((double)berStats.bitErrors) / berStats.bitsTested) * 100);
  }
  else
  {
    percentBitError = 0.0;
  }

  responsePrint(argv[0],
                "BitsToTest:%u"
                ",BitsTested:%u"
                ",PercentDone:%0.2f"
                ",RSSI:%d"
                ",BitErrors:%u"
                ",PercentBitError:%0.2f"
                ,berStats.bitsTotal
                ,berStats.bitsTested
                ,percentDone
                ,berStats.rssi
                ,berStats.bitErrors
                ,percentBitError
               );
}
