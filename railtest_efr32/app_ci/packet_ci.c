/***************************************************************************//**
 * @file packet_ci.c
 * @brief This file implements the packet commands for RAIL test applications.
 * @copyright Copyright 2015 Silicon Laboratories, Inc. http://www.silabs.com
 ******************************************************************************/
#include <stdio.h>

#include "command_interpreter.h"
#include "response_print.h"

#include "rail.h"
#include "app_common.h"

void printTxPacket(int argc, char **argv)
{
  // Use the packet print helper to print out the transmit payload
  printPacket(argv[0],
              transmitPayload.dataPtr,
              transmitPayload.dataLength,
              NULL);
}

void setTxPayload(int argc, char **argv)
{
  uint16_t offset = ciGetUnsigned(argv[1]);

  // Read as many bytes as have been supplied and set them
  for (int i = 2; i < argc; i++)
  {
    uint32_t index = offset + i - 2;
    uint8_t value = ciGetUnsigned(argv[i]);

    // Make sure this fits in the txData buffer
    if (index >= sizeof(txData))
    {
      responsePrintError(argv[0], 5, "Data overflows txData buffer");
      return;
    }
    txData[index] = value;
  }

  transmitPayload.dataPtr = &txData[0];
  RAIL_TxDataLoad(&transmitPayload);
  printTxPacket(1, argv);
}

void setTxLength(int argc, char **argv)
{
  uint32_t length = ciGetUnsigned(argv[1]);

  if (length > sizeof(txData))
  {
    responsePrintError(argv[0], 6, "Invalid length %d", length);
    return;
  }

  // Make sure we're using the txData array and set the length
  transmitPayload.dataPtr = &txData[0];
  transmitPayload.dataLength = length;
  RAIL_TxDataLoad(&transmitPayload);
  responsePrint(argv[0], "TxLength:%d", transmitPayload.dataLength);
}

void printAckPacket(int argc, char **argv)
{
  // Use the packet print helper to print out the transmit payload
  printPacket(argv[0],
              ackPayload.dataPtr,
              ackPayload.dataLength,
              NULL);
}

void setAckPayload(int argc, char **argv)
{
  uint16_t offset = ciGetUnsigned(argv[1]);

  // Read as many bytes as have been supplied and set them
  for (int i = 2; i < argc; i++)
  {
    uint32_t index = offset + i - 2;
    uint8_t value = ciGetUnsigned(argv[i]);

    // Make sure this fits in the txData buffer
    if (index >= sizeof(ackData))
    {
      responsePrintError(argv[0], 5, "Data overflows ackData buffer");
      return;
    }
    ackData[index] = value;
  }

  ackPayload.dataPtr = &ackData[0];
  RAIL_AutoAckLoadBuffer(&ackPayload);
  printAckPacket(1, argv);
}

void setAckLength(int argc, char **argv)
{
  uint32_t length = ciGetUnsigned(argv[1]);

  if (length > sizeof(ackData))
  {
    responsePrintError(argv[0], 6, "Invalid length %d", length);
    return;
  }

  // Make sure we're using the txData array and set the length
  ackPayload.dataPtr = &ackData[0];
  ackPayload.dataLength = length;
  RAIL_AutoAckLoadBuffer(&ackPayload);
  responsePrint(argv[0], "TxLength:%d", ackPayload.dataLength);
}
