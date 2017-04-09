/***************************************************************************//**
 * @file app_main.c
 * @brief This is the base test application. It handles basic RAIL configuration
 * as well as transmit, receive, and various debug modes.
 * @copyright Copyright 2015 Silicon Laboratories, Inc. http://www.silabs.com
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rail.h"
#include "rail_ieee802154.h"
#include "rail_types.h"
#include "rail_chip_specific.h"

#include "em_chip.h"
#include "em_rmu.h"

#include "retargetserial.h"
#include "gpiointerrupt.h"
#include "graphics.h"
#include "command_interpreter.h"
#include "response_print.h"
#include "buffer_pool_allocator.h"
#include "circular_queue.h"

#include "rail_config.h"

#include "app_ci.h"
#include "app_common.h"

#ifdef CONFIGURATION_HEADER
#include CONFIGURATION_HEADER
#endif

// Configuration defines
#ifndef APP_MAX_PACKET_LENGTH
#define APP_MAX_PACKET_LENGTH  (MAX_BUFFER_SIZE - sizeof(RAIL_RxPacketInfo_t))
#endif
#ifndef APP_CONTINUOUS_TRANSFER_PERIOD
#define APP_CONTINUOUS_TRANSFER_PERIOD 250UL
#endif
#ifndef APP_COMMAND_INTERFACE_BUFFER_SIZE
#define APP_COMMAND_INTERFACE_BUFFER_SIZE 256
#endif

// External control and status variables
Counters_t counters = { 0 };
bool receiveModeEnabled = true;
uint8_t logLevel = PERIPHERAL_ENABLE | ASYNC_RESPONSE;
int32_t txCount = 0;
uint32_t continuousTransferPeriod = APP_CONTINUOUS_TRANSFER_PERIOD;
uint32_t txAfterRxDelay = 0;
int32_t txCancelDelay = -1;
int currentConfig = 0; //default is first in list
bool skipCalibrations = false;
bool afterRxCancelAck = false;
bool afterRxUseTxBufferForAck = false;
bool schRxStopOnRxEvent = false;

// Internal app state variables
static uint32_t startTransmitCounter = 0;
static uint32_t internalTransmitCounter = 0;
static uint32_t failPackets = 0;
static uint8_t  lastTxStatus = 0;
static bool packetTx = false;
static bool finishTxSequence = false;
static RAIL_ScheduleTxConfig_t nextPacketTxTime = { 0, RAIL_TIME_ABSOLUTE, };
static uint32_t previousTxTime = 0;
static Queue_t  rxPacketQueue;
static uint32_t railTimerExpireTime = 0;
static uint32_t railTimerConfigExpireTime = 0;
static bool     railTimerExpired = false;
static bool     calibrateRadio = false;
static bool     newTxError = false;
static bool     receivingPacket = false;
static bool     rxAckTimeout = false;

// Allow local echo to be turned on/off for the command prompt
#ifdef DISABLE_LOCAL_ECHO
  #define localEcho 0
#else
  #define localEcho 1
#endif

//Command line variables
static CommandEntry_t commands[] = {
  APP_CI_COMMANDS,
  COMMAND_ENTRY(NULL, NULL, NULL, NULL)
};
static CommandState_t state;
static char ciBuffer[APP_COMMAND_INTERFACE_BUFFER_SIZE];


const uint32_t generated_100kbps[] = {
  0x01010FFCUL, 0x0003C008UL,
  0x00014140UL, 0x008800E2UL,
  0x00014144UL, 0x1153E6C1UL,
  0x0001601CUL, 0x2002C01FUL,
  0x00016024UL, 0x000CD000UL,
  0x00016030UL, 0x00FF2FD0UL,
  0x00016034UL, 0x00000B2EUL,
  0x00016038UL, 0x01880020UL,
  0x00016048UL, 0x11200714UL,
  0x00016050UL, 0x003503B4UL,
  0x0001701CUL, 0x82710060UL,
  0x00017074UL, 0x00000213UL,
  0xFFFFFFFFUL,
};

const uint32_t generated_500kbps[] = {
  0x01010FFCUL, 0x0003C00BUL,
  0x00014140UL, 0x00880067UL,
  0x00014144UL, 0x1153E6C0UL,
  0x0001601CUL, 0x1802C01FUL,
  0x00016024UL, 0x000C5000UL,
  0x00016030UL, 0x00FF0990UL,
  0x00016034UL, 0x000008A2UL,
  0x00016038UL, 0x0102000AUL,
  0x00016048UL, 0x1DE00714UL,
  0x00016050UL, 0x002B03D1UL,
  0x0001701CUL, 0x82720060UL,
  0x00017074UL, 0x00000223UL,
  0xFFFFFFFFUL,
};

// Channel Variables
uint16_t channel = 0;

// Generic
uint8_t txData[APP_MAX_PACKET_LENGTH] = {
  0x1A, 0x0E, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
  0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE,
};
RAIL_TxData_t transmitPayload = { &txData[0], 0x1A };

uint8_t ackData[RAIL_AUTOACK_MAX_LENGTH] = {
  0x1a, 0x0E, 0xF1, 0xE2, 0xD3, 0xC4, 0xB5, 0xA6,
  0x97, 0x88, 0x79, 0x6A, 0x5B, 0x4C, 0x3D, 0x2E,
};
RAIL_AutoAckData_t ackPayload = { &ackData[0], 0X1A };

// Helper buffer for printing received packet data
static char packetPrintBuffer[(APP_MAX_PACKET_LENGTH+1)*5];

static const RAIL_Init_t railInitParams = {
  APP_MAX_PACKET_LENGTH,
  RADIO_CONFIG_XTAL_FREQUENCY,
  RAIL_CAL_ALL
};

static const RAIL_CalInit_t railCalInitParams = {
  RAIL_CAL_ALL,
  irCalConfig
};

// Structure that holds txOptions
RAIL_TxOptions_t txOptions;
// If this pointer is not NULL, call RAIL_TxStartWithOptions
RAIL_TxOptions_t *txOptionsPtr = NULL;

// Prototypes
void changeRadioConfig(int newConfig);
void changeChannelConfig(int newConfig);
// Called during main loop
void processInputCharacters(void);
void sendPacketIfPending(void);
void finishTxSequenceIfPending(void);
void changeAppModeIfPending(void);
void printReceivedPacket(void);
void printNewTxError(void);
void checkTimerExpiration(void);
void updateDisplay(void);
void processPendingCalibrations(void);
void printAckTimeout(void);

///******syncword config************/////
uint32_t changesyncword;
//uint32_t syncwordcnf[3] = {0x00016040UL, 0xCA1E6A4AUL,0xFFFFFFFFUL,};
uint32_t syncwordcnf[3] = {0x00016040UL, 0x0UL,0xFFFFFFFFUL};
uint32_t lencnf[] = {0x00010018UL, 0x0UL, 0xFFFFFFFFUL};
uint32_t change_syncword(uint32_t souce)
{
	uint32_t t = 0;
    uint32_t i = 0;
	for( i = 0;i<4;i++)
	{
		t>>=1;
		if(souce & 0x80000000)
		{
			t|=0x80000000;
		}
		if(souce & 0x08000000)
		{
			t|=0x08000000;
		}
		if(souce & 0x00800000)
		{
			t|=0x00800000;
		}
        if(souce & 0x00080000)
		{
			t|=0x00080000;
		}
         if(souce & 0x00008000)
		{
			t|=0x00008000;
		}
         if(souce & 0x00000800)
		{
			t|=0x00000800;
		}
        if(souce & 0x00000080)
		{
			t|=0x00000080;
		}
        if(souce & 0x00000008)
		{
			t|=0x00000008;
		}
		souce<<=1;
	}
	return ((t&0x0000000f)<<28) + ((t&0x000000f0)<<20) + ((t&0x00000f00)<<12) +  ((t&0x0000f000)<<4) + ((t&0x000f0000)>>4)+((t&0x00f00000)>>12)+((t&0x0f000000)>>20)+((t&0xf0000000)>>28) ;
}

void print_RAIL_status(char* mydebug)
{
	char serial_debug = 0;
	serial_debug = (uint8_t)RAIL_RfStateGet();
	responsePrint(mydebug, "RAIL state:%d", serial_debug);
}
void set_iodebug(void)
{
	char *para1[] =  {"mytx", "PC10", "TXACTIVE"};
	char *para2[] =  {"myrx", "PC11", "RXACTIVE"};
	char *para3[] =  {"myrx", "PF3", "PTIDATA"};
	setDebugSignal(3, para1);
	setDebugSignal(3, para2);
	setDebugSignal(3, para3);
}
void myChangeRadioConfig(uint8_t channel, bps_enum bps, uint8_t* ID, uint8_t len)
{
#define BASE_FREQ			2400000000
#define CHANNEL_SPACING		500000
	const uint32_t * p=NULL;

	RAIL_RfIdle();
	if (0xff == channel){
		goto bpsconf;
	}
	//channel
	if (0 == channel%2)
	{
		generated_channels[0].baseFrequency = BASE_FREQ + channel*CHANNEL_SPACING;
	}
	if (1 == channel%2)
	{
		generated_channels[0].baseFrequency += CHANNEL_SPACING;
	}
	RAIL_ChannelConfig(channelConfigs[0]); //channel

bpsconf:
	//bps
	switch(bps){
		case BPS100: p = configList[1];
			break;
		case BPS500: p = configList[2];
			break;
		default:
			break;
	}
	if (RAIL_RadioConfig((void*)p)) { while(1); }

	//ID
	if (NULL != ID)
	{
		syncwordcnf[1] = change_syncword((uint32_t)ID[0]<<24|(uint32_t)ID[1]<<16|(uint32_t)ID[2]<<8 | (uint32_t)ID[3]);
		if (RAIL_RadioConfig((void*)configList[3])) { while(1); }
	}

	//len
	lencnf[1] = len-1;
	if (RAIL_RadioConfig((void*)configList[4])) { while(1); }
}
uint8_t ID[4] = {0X53, 0X78,0X00,0X66};

int main(void)
{
  // Initialize the chip
  CHIP_Init();

  // Grab the reset cause
  uint32_t resetCause = RMU_ResetCauseGet();
  RMU_ResetCauseClear(); // So resetCause is rational and not an accmulated mess
  // Release GPIOs that were held by EM4h
  // This is reportedly a workaround that I've found needs to be done
  // *before* appHalInit() tries to start oscillators, otherwise we'll
  // hang indefinitely waiting for the oscillator to become ready.
  if (resetCause & RMU_RSTCAUSE_EM4RST)
  {
    EMU->CMD = EMU_CMD_EM4UNLATCH;
  }

  // Initialize hardware for application
  appHalInit();

  // Initialize Radio
  RAIL_RfInit(&railInitParams);

  // Initialize Radio Calibrations
  RAIL_CalInit(&railCalInitParams);

  // Configure modem, packet handler
  changeRadioConfig(currentConfig);
  myChangeRadioConfig(150, BPS500, ID, 6);
  // Configure RAIL callbacks with no appended info
  RAIL_RxConfig((  RAIL_RX_CONFIG_FRAME_ERROR
                 | RAIL_RX_CONFIG_SYNC1_DETECT
                 | RAIL_RX_CONFIG_ADDRESS_FILTERED
                 | RAIL_RX_CONFIG_BUFFER_OVERFLOW
                 | RAIL_RX_CONFIG_SCHEDULED_RX_END
				 | RAIL_RX_CONFIG_TIMEOUT),
                false);

  RAIL_SetRxTransitions(RAIL_RF_STATE_RX, RAIL_RF_STATE_RX,
                        RAIL_IGNORE_NO_ERRORS);
  RAIL_SetTxTransitions(RAIL_RF_STATE_RX, RAIL_RF_STATE_RX);

  // Initialize the queue we use for tracking packets
  if (!queueInit(&rxPacketQueue, MAX_QUEUE_LENGTH)) { while(1); }

  updateDisplay();

  printf("\n"APP_DEMO_STRING_INIT"\n");
  if (resetCause & RMU_RSTCAUSE_EM4RST)
  {
    responsePrint("sleepWoke", "EM:4%c,SerialWakeup:No,RfSensed:%s",
                  (((EMU->EM4CTRL & EMU_EM4CTRL_EM4STATE)
                    == EMU_EM4CTRL_EM4STATE_EM4S) ? 's' : 'h'),
                  RAIL_RfSensed() ? "Yes" : "No");
  }
  printf("> ");
  ciInitState(&state, ciBuffer, sizeof(ciBuffer), commands);

  // Initialize autoack data
  RAIL_AutoAckLoadBuffer(&ackPayload);
  set_iodebug();
//todo
  while(1)
  {
    processInputCharacters();

    rfSensedCheck();

    sendPacketIfPending();

    finishTxSequenceIfPending();

    changeAppModeIfPending();

    printReceivedPacket();

    printNewTxError();

    checkTimerExpiration();

    updateDisplay();

    processPendingCalibrations();

    printAckTimeout();

  }
} //main()


/*
int main(void)
{
  // Initialize the chip
  CHIP_Init();

  // Initialize the system clocks and other HAL components
  halInit();
  // Initialize Radio
  RAIL_RfInit(&railInitParams);

  // Configure modem, packet handler
  changeRadioConfig(currentConfig);

  // Configure RAIL callbacks with no appended info
  RAIL_RxConfig((  RAIL_RX_CONFIG_INVALID_CRC
                 | RAIL_RX_CONFIG_SYNC1_DETECT
                 | RAIL_RX_CONFIG_ADDRESS_FILTERED
                 | RAIL_RX_CONFIG_BUFFER_OVERFLOW  ),
                false);

    while(1)
    {
    	unsigned int i;
        RAIL_RfIdle();
        //RAIL_TxDataLoad(&transmitPayload);
        RAIL_TxDataLoad(NULL);
        RAIL_TxStart(channel, NULL, NULL);
        while(1);
        for (i=0;i <100; i++)
      	  UDELAY_Delay(1000);
    }
}
*/
/******************************************************************************
 * RAIL Callback Implementation
 *****************************************************************************/
void RAILCb_RfReady(void)
{
  LedSet(0);
  LedSet(1);
}

void RAILCb_CalNeeded()
{
  calibrateRadio = true;
}

void RAILCb_RadioStateChanged(uint8_t state)
{
}

void RAILCb_TxPacketSent(RAIL_TxPacketInfo_t *txPacketInfo)
{
  counters.transmit++;
  internalTransmitCounter++;
  LedToggle(1);
  redrawDisplay = true;

  // Store the previous tx time for printing later
  previousTxTime = txPacketInfo->timeUs;

  scheduleNextTx();
}
//todo
void RAILCb_RxPacketReceived(void *rxPacketHandle)
{
  // Always load, in case we have to transition to TX
  RAIL_TxDataLoad(&transmitPayload);

  receivingPacket = false;
  RAIL_RxPacketInfo_t *rxPacketInfo;

  counters.receive++;

  //todo
//  print_RAIL_status("my_received");
  // Count packets that we received but had no memory to store
  rxPacketInfo = (RAIL_RxPacketInfo_t*)memoryPtrFromHandle(rxPacketHandle);
  if (rxPacketInfo == NULL)
  {
    counters.noRxBuffer++;
  }
  else
  {
    // If we have just received an ACK, don't respond with an ACK
    if (rxPacketInfo->dataPtr[2] == 0xF1)
    {
      RAIL_AutoAckCancelAck();
    }

    // Cancel ack if user requested
    if (afterRxCancelAck)
    {
      afterRxCancelAck = false;
      RAIL_AutoAckCancelAck();
    }

    // Use Tx Buffer for Ack if user requested
    if (afterRxUseTxBufferForAck)
    {
      afterRxUseTxBufferForAck = false;
      RAIL_AutoAckUseTxBuffer();
    }

    if (currentAppMode() == SCHTX_AFTER_RX)
    {
      // Schedule the next transmit after this receive
      setNextPacketTime(rxPacketInfo->appendedInfo.timeUs + txAfterRxDelay);
      txCount = 1;
      pendPacketTx();
    }
  }

  LedToggle(0);
  if (logLevel & ASYNC_RESPONSE)
  {
    redrawDisplay = true;

    // If we ran out of buffers then we can't queue this up
    if (rxPacketInfo != NULL)
    {
      // Take an extra reference to this rx packet pointer so it's not released
      memoryTakeReference(rxPacketHandle);

      // Copy this received packet into our circular queue
      queueAdd(&rxPacketQueue, rxPacketHandle);
    }
  }

  // Track the state of scheduled Rx to figure out when it ends
  if (inAppMode(RX_SCHEDULED, NULL) && schRxStopOnRxEvent)
  {
    enableAppMode(RX_SCHEDULED, false, NULL);
  }

  // In Rx overflow test mode hang in this ISR to prevent processing new
  // packets to force an overflow
  if ((currentAppMode() == RX_OVERFLOW))
  {
    enableAppMode(RX_OVERFLOW, false, NULL); // Switch back after the overflow
    changeAppModeIfPending();
    // 10 seconds should be enough to trigger an overflow
    usDelay(10 * 1000000);
  }
  updateStats(rxPacketInfo->appendedInfo.rssiLatch, &counters.rssi);
}

void RAILCb_RxRadioStatusExt(uint32_t status)
{
  if (status & RAIL_RX_CONFIG_FRAME_ERROR)
  {
    receivingPacket = false;
    counters.frameError++;
    LedToggle(1);
  }
  if (status & RAIL_RX_CONFIG_SYNC1_DETECT)
  {
    receivingPacket = true;
    counters.syncDetect++;
    printf("rx_sync");
	//todo
//	print_RAIL_status("my_rec_status");
  }
  if (status & RAIL_RX_CONFIG_PREAMBLE_DETECT)
  {
	printf("rx_preamble");
    counters.preambleDetect++;
  }
  if (status & RAIL_RX_CONFIG_BUFFER_OVERFLOW)
  {
    receivingPacket = false;
    counters.rxOfEvent++;
  }
  if (status & RAIL_RX_CONFIG_ADDRESS_FILTERED)
  {
    receivingPacket = false;
    counters.addrFilterEvent++;
  }
  if (status & RAIL_RX_CONFIG_RF_SENSED)
  {
    counters.rfSensedEvent++;
    if (counters.rfSensedEvent == 0) // Wrap it to 1 not 0
    {
      counters.rfSensedEvent = 1;
    }
  }
  if (status & RAIL_RX_CONFIG_TIMEOUT)
  {
	  printf("--timeout--");
  }
  // End scheduled receive mode if an appropriate end or error event is received
  if ((status & RAIL_RX_CONFIG_SCHEDULED_RX_END)
      || ((schRxStopOnRxEvent && inAppMode(RX_SCHEDULED, NULL))
          && ((status & RAIL_RX_CONFIG_ADDRESS_FILTERED)
              || (status & RAIL_RX_CONFIG_BUFFER_OVERFLOW)
              || (status & RAIL_RX_CONFIG_FRAME_ERROR))))
  {
    enableAppMode(RX_SCHEDULED, false, NULL);
  }
}

void RAILCb_TxRadioStatus(uint8_t status)
{
  lastTxStatus = status;
  newTxError = true;
  failPackets++;
  scheduleNextTx();
	//todo
	print_RAIL_status("send_status");
}

void RAILCb_TimerExpired(void)
{
  if (inAppMode(NONE, NULL))
  {
    railTimerExpireTime = RAIL_GetTime();
    railTimerConfigExpireTime = RAIL_TimerGet();
    railTimerExpired = true;
  }
  else if (currentAppMode() == PER)
  {
    GPIO_PinOutToggle(PER_PORT, PER_PIN);
    counters.perTriggers += GPIO_PinOutGet(PER_PORT, PER_PIN);
    perCount -= GPIO_PinOutGet(PER_PORT, PER_PIN);
    if (perCount < 1)
    {
      GPIO_PinOutClear(PER_PORT, PER_PIN);
      enableAppMode(PER, false, NULL);
    }
    else
    {
      RAIL_TimerSet(perDelay, RAIL_TIME_DELAY);
    }
  }
  else
  {
    pendPacketTx();
  }
}

void RAILCb_RxAckTimeout(void)
{
  counters.ackTimeout++;
  rxAckTimeout = true;
}

void RAILCb_IEEE802154_DataRequestCommand(RAIL_IEEE802154_Address_t *data)
{
  // Assume that every node has data pending if using 802.15.4 acceleration
  RAIL_IEEE802154_SetFramePending();
}

/******************************************************************************
 * Application Helper Functions
 *****************************************************************************/


void processPendingCalibrations()
{
  // Only calibrate the radio when not currently transmitting or in a
  // transmit mode. Also don't try to calibrate while receiving a packet
  bool calsInMode = inAppMode(NONE, NULL);
  if (calibrateRadio && calsInMode && !skipCalibrations && !receivingPacket)
  {
    RAIL_CalMask_t pendingCals = RAIL_CalPendingGet();

    // Disable the radio if we have to do an offline calibration
    if (pendingCals & RAIL_CAL_OFFLINE)
    {
      RAIL_RfIdle();
    }

    // Perform the necessary calibrations and don't save the results
    counters.calibrations++;
    calibrateRadio = false;
    RAIL_CalStart(NULL, pendingCals, false);

    // Turn the radio back on if we disabled it above
    if ((pendingCals & RAIL_CAL_OFFLINE))
    {
      // Wait for RxStart to succeed
      while (receiveModeEnabled && RAIL_RxStart(channel))
      {
        RAIL_RfIdle();
      }
    }
  }
}

void checkTimerExpiration()
{
  if (railTimerExpired)
  {
    railTimerExpired = false;
    responsePrint("timerCb",
                  "TimerExpiredCallback:%u,ConfiguredExpireTime:%u",
                  railTimerExpireTime,
                  railTimerConfigExpireTime);
  }
}

void printNewTxError()
{
  if (newTxError)
  {
    newTxError = false;
    if (lastTxStatus & RAIL_TX_CONFIG_BUFFER_UNDERFLOW)
    {
      if (logLevel & ASYNC_RESPONSE)
      {
        responsePrint("txPacket",
                      "txStatus:Error,"
                      "errorReason:Tx underflow or abort,"
                      "errorCode:%u",
                      lastTxStatus);
      }
      counters.txAbort++;
    }
    if (lastTxStatus & RAIL_TX_CONFIG_CHANNEL_BUSY)
    {
      if (logLevel & ASYNC_RESPONSE)
      {
        responsePrint("txPacket",
                      "txStatus:Error,"
                      "errorReason:Tx channel busy,"
                      "errorCode:%u",
                      lastTxStatus);
      }
      counters.txChannelBusy++;
    }
  }
}

void printAckTimeout()
{
  if (rxAckTimeout)
  {
    rxAckTimeout = false;
    responsePrintError("rxAckTimeout", 0x2F,
                 "Searching for RX Ack has timed out.");
  }
}


void changeChannel(uint32_t i)
{
  channel = i;
  redrawDisplay = true;
}

void changeRadioConfig(int newConfig)
{
  // Turn off the radio before reconfiguring it
  RAIL_RfIdle();

  // Reconfigure the radio parameters

  RAIL_PacketLengthConfigFrameType(frameTypeConfigList[newConfig]);
  if (RAIL_RadioConfig((void*)configList[newConfig])) { while(1); }

  // Set us to a valid channel for this config and force an update in the main
  // loop to restart whatever action was going on
//  selfsyncword[1] = change_syncword(0x52567854);

//  if (RAIL_RadioConfig((void*)configList[2])) { while(1); }

//  if (RAIL_RadioConfig((void*)configList[3])) { while(1); }

  changeChannelConfig(newConfig);
  currentConfig = newConfig;
}

void changeChannelConfig(int newConfig)
{
  channel = RAIL_ChannelConfig(channelConfigs[newConfig]);
  changeChannel(channel);
}

void pendPacketTx()
{
  packetTx = true;
}

void sendPacketIfPending()
{
  if (packetTx)
  {
    packetTx = false;
    uint8_t txStatus;
    uint32_t storedTransmitCounter = internalTransmitCounter;

    // Don't decrement in continuous mode
    if (currentAppMode() != TX_CONTINUOUS)
    {
      txCount--;
    }

    RAIL_RfIdle();
    // Generate the payload and start transmitting
    if (currentAppMode() != TX_UNDERFLOW) // Force underflows in this mode
    {
      RAIL_TxDataLoad(&transmitPayload);
    }
    if (currentAppMode() == TX_SCHEDULED || currentAppMode() == SCHTX_AFTER_RX)
    {
      if (txOptionsPtr == NULL)
      {
        txStatus = RAIL_TxStart(channel, &RAIL_ScheduleTx,
                                    &nextPacketTxTime);
      }
      else
      {
        txStatus = RAIL_TxStartWithOptions(channel, txOptionsPtr,
                                            &RAIL_ScheduleTx,
                                            &nextPacketTxTime);
      }
    }
    else if ((startTransmitCounter == storedTransmitCounter)
             && (failPackets == 0))
    {
      if (txOptionsPtr == NULL)
      {
        txStatus = RAIL_TxStart(channel, txPreTxOp, txPreTxOpArgs);
      }
      else
      {
        txStatus = RAIL_TxStartWithOptions(channel, txOptionsPtr,
                                            txPreTxOp, txPreTxOpArgs);
      }
    }
    else
    {
      // Sending NULL for txPreTxOpArgs reuses previous arguments, which
      // saves computation. Reuse arguments for all but the first
      // packet in a sequence
      if (txOptionsPtr == NULL)
      {
        txStatus = RAIL_TxStart(channel, txPreTxOp, NULL);
      }
      else
      {
        txStatus = RAIL_TxStartWithOptions(channel, txOptionsPtr,
                                            txPreTxOp, NULL);
      }
    }

    if (txStatus != 0)
    {
      lastTxStatus = txStatus;
      failPackets++;
      pendPacketTx(); // No callback will fire, so fake it
    }
    else if (currentAppMode() == TX_CANCEL)
    {
      usDelay(txCancelDelay);
      RAIL_RfIdle();
    }
  }
}

void pendFinishTxSequence()
{
  finishTxSequence = true;
}

void finishTxSequenceIfPending()
{
  if (finishTxSequence)
  {
    finishTxSequence = false;

    if (logLevel & ASYNC_RESPONSE)
    {
      // Compute the number of packets sent
      uint32_t sentPackets = internalTransmitCounter - startTransmitCounter;
      // Print the number of sent and failed packets
      responsePrint("txEnd",
                    "txStatus:%s,"
                    "transmitted:%u,"
                    "lastTxTime:%u,"
                    "failed:%u,"
                    "lastTxStatus:0x%x",
                    failPackets == 0 ?
                      "Complete" :
                      (sentPackets == 0 ? "Error" : "Partial"),
                    sentPackets,
                    previousTxTime,
                    failPackets,
                    lastTxStatus);
    }
    startTransmitCounter = internalTransmitCounter;
    failPackets = 0;
    lastTxStatus = 0;
  }
}

void setNextPacketTime(uint32_t absTime)
{
  nextPacketTxTime.when = absTime;
}
//
void printReceivedPacket()
{
  // Print any newly received packets
  if (!queueIsEmpty(&rxPacketQueue))
  {
    void *rxPacketHandle = queueRemove(&rxPacketQueue);
    RAIL_RxPacketInfo_t *rxPacketInfo =
      (RAIL_RxPacketInfo_t*) memoryPtrFromHandle(rxPacketHandle);

    // Print the received packet and appended info
    printPacket("rxPacket",
                rxPacketInfo->dataPtr,
                rxPacketInfo->dataLength,
                rxPacketInfo);

    // Free the memory allocated for this packet since we're now done with it
    memoryFree(rxPacketHandle);
  }
}

void printPacket(char *cmdName,
                 uint8_t *data,
                 uint16_t dataLength,
                 RAIL_RxPacketInfo_t *packetInfo)
{
  uint32_t offset = 0;
  int i;

  // Print out a length 0 packet message if no packet was given
  if (data == NULL)
  {
    responsePrint(cmdName, "len:0");
    return;
  }

  for(i = 0; i < dataLength; i++)
  {
    int n = snprintf(packetPrintBuffer + offset,
                     sizeof(packetPrintBuffer) - offset,
                     " 0x%.2x",
                     data[i]);
    if (n >= 0)
    {
      offset += n;
    }
    else
    {
      snprintf(packetPrintBuffer, sizeof(packetPrintBuffer), "Invalid Packet");
      break;
    }

    // If we've filled up the packet buffer make sure we stop trying to print
    if (offset >= sizeof(packetPrintBuffer))
    {
      snprintf(packetPrintBuffer,
               sizeof(packetPrintBuffer),
               "Packet too large!");
      break;
    }
  }

  // If this is an Rx packet print the appended info
  if (packetInfo != NULL)
  {
    responsePrint(cmdName,
                  "len:%d,timeUs:%u,crc:%s,coding:%s,rssi:%.2f,isAck:%s,payload:%s",
                  packetInfo->dataLength,
                  packetInfo->appendedInfo.timeUs,
                  (packetInfo->appendedInfo.crcStatus) ? "Pass":"Fail",
                  (packetInfo->appendedInfo.frameCodingStatus) ? "Pass":"Fail",
                  ((float) packetInfo->appendedInfo.rssiLatch),
                  packetInfo->appendedInfo.isAck ? "True":"False",
                  packetPrintBuffer);
  }
  else
  {
    responsePrint(cmdName, "len:%d,payload:%s", dataLength, packetPrintBuffer);
  }
}

void processInputCharacters()
{
  char input = getchar();
  while(input != '\0' && input != 0xFF)
  {
    if (localEcho)
    {
      if (input != '\n')
      {
        printf("%c", input);
      }
    }
    if (ciProcessInput(&state, &input, 1) > 0)
    {
      printf("> ");
    }
    input = getchar();
  }
}

/******************************************************************************
 * Command Interpreter Override Functions
 *****************************************************************************/

// Error callback function using responsePrint
void ciErrorCallback(char* command, CommandError_t error)
{
  if (error == CI_UNKNOWN_COMMAND)
  {
    responsePrintError("ci",
                       CI_UNKNOWN_COMMAND,
                       "Unknown command '%s'",
                       command);
  }
  else if (error == CI_MAX_ARGUMENTS)
  {
    responsePrintError("ci",
                       CI_MAX_ARGUMENTS,
                       "CI_MAX_ARGUMENTS '%s'",
                       command);
  }
  else if (error == CI_INVALID_ARGUMENTS)
  {
    responsePrintError("ci",
                       CI_INVALID_ARGUMENTS,
                       "Invalid arguments for '%s'",
                       command);
  }
}

#ifdef USE_RESPONSE_HELP
// Help callback function using responsePrint
bool ciPrintHelp(CommandEntry_t *commands)
{
  responsePrintHeader("help", "command:%s,args:%s,help:%s");
  while(commands != NULL && commands->command != NULL)
  {
    char *args = commands->arguments;
    char *helpStr = commands->helpStr;

    if (commands->callback != NULL)
    {
      // Make sure that we print something for NULL pointers
      if (args == NULL)
      {
        args = "";
      }
      if (helpStr == NULL)
      {
        helpStr = "";
      }

      // Print the multiline response for this entry
      responsePrintMulti("command:%s,args:%s,help:%s",
                          commands->command,
                          args,
                          helpStr);
    }
    commands++;
  }
  return true;
}
#endif //USE_RESPONSE_HELP
