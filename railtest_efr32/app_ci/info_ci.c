/***************************************************************************//**
 * @file info_ci.c
 * @brief This file implements informational commands for RAIL test applications.
 * @copyright Copyright 2015 Silicon Laboratories, Inc. http://www.silabs.com
 ******************************************************************************/
#include <string.h>
#include "command_interpreter.h"
#include "response_print.h"

#include "rail.h"
#include "rail_types.h"
#include "app_common.h"
#include "config.h"

const char *rfStates[] = { "Idle", "Rx", "Tx" };

void getStatus(int argc, char **argv)
{
  responsePrint(argv[0],
                "TxCount:%u"
                ",RxCount:%u"
                ",SyncDetect:%u"
                ",FrameErrors:%u"
                ",RxOverflow:%u"
                ",AddrFilt:%u"
                ",Calibrations:%u"
                ",TxAbort:%u"
                ",TxChannelBusy:%u"
                ",NoRxBuffer:%u"
                ",RfSensed:%u"
                ",ackTimeout:%u"
                ",Channel:%u"
                ",AppMode:%s"
                ",RfState:%s"
                ,counters.transmit
                ,counters.receive
                ,counters.syncDetect
                ,counters.frameError
                ,counters.rxOfEvent
                ,counters.addrFilterEvent
                ,counters.calibrations
                ,counters.txAbort
                ,counters.txChannelBusy
                ,counters.noRxBuffer
                ,counters.rfSensedEvent
                ,counters.ackTimeout
                ,channel
                ,appModeNames(currentAppMode())
                ,rfStates[RAIL_RfStateGet()]
               );
}

void getVersion(int argc, char **argv)
{
  RAIL_Version_t rail_ver;
  RAIL_VersionGet(&rail_ver, false);
  responsePrint(argv[0], "App:%d.%d.%d,RAIL:%d.%d.%d",
                EMBER_MAJOR_VERSION, EMBER_MINOR_VERSION, EMBER_PATCH_VERSION,
                rail_ver.major, rail_ver.minor, rail_ver.rev);
}

void getVersionVerbose(int argc, char **argv)
{
  RAIL_Version_t rail_ver;
  RAIL_VersionGet(&rail_ver, true);
  responsePrint(argv[0], "App:%d.%d.%d,RAIL:%d.%d.%d.%d",
                EMBER_MAJOR_VERSION, EMBER_MINOR_VERSION, EMBER_PATCH_VERSION,
                rail_ver.major, rail_ver.minor, rail_ver.rev, rail_ver.build);
  responsePrint(argv[0], "hash:0x%.8X,flags:0x%.2X",
                rail_ver.hash, rail_ver.flags);
}

void getRssi(int argc, char **argv)
{
  int16_t rssi = RAIL_RxGetRSSI();

  // The lowest negative value is used to indicate an error reading the RSSI
  if (rssi == RAIL_RSSI_INVALID)
  {
    responsePrintError(argv[0], 0x08, "Could not read RSSI. Ensure Rx is enabled");
    return;
  }
  responsePrint(argv[0], "rssi:%.2f", ((float) rssi / 4));
}

void resetCounters(int argc, char **argv)
{
  memset(&counters, 0, sizeof(counters));
  getStatus(1, argv);
}

void getTime(int argc, char **argv)
{
  responsePrint(argv[0], "Time:%u", RAIL_GetTime());
}

void setTime(int argc, char **argv)
{
  uint32_t timeUs = ciGetUnsigned(argv[1]);
  if(RAIL_STATUS_NO_ERROR == RAIL_SetTime(timeUs))
  {
    responsePrint(argv[0], "Status:Success,CurrentTime:%u", RAIL_GetTime());
  }
  else
  {
    responsePrint(argv[0], "Status:Error,CurrentTime:%u", RAIL_GetTime());
  }
}
