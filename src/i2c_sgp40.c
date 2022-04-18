/***********************************************************************
* @file i2c.c
* @version 0.0.1
* @brief Function header file.
*
* @author Ananth Deshpande, ande9392@Colorado.edu
* @date Feb 2, 2022
*
* @institution University of Colorado Boulder (UCB)
* @course ECEN 5823-001: IoT Embedded Firmware (Fall 2020)
* @instructor David Sluiter
*
* @assignment ecen5823-assignment3-AwesomeStudent
* @due Feb 2, 2022
*
* @resources Utilized Silicon Labs' EMLIB peripheral libraries to
* implement functionality.
*
*
* @copyright All rights reserved. Distribution allowed only for the
* use of assignment grading. Use of code excerpts allowed at the
* discretion of author. Contact for permission.
*/
#include <src/i2c_sgp40.h>
#include "scheduler.h"

#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#define I2C0_SCL_PORT  gpioPortC
#define I2C0_SCL_PIN   10
#define I2C0_SDA_PORT  gpioPortC
#define I2C0_SDA_PIN   11
#define I2C0_PORT_LOC_SCL 14
#define I2C0_PORT_LOC_SDA 16
#define SGP40_SENSOR_ADDR 0x59

//sequence of bytes to be sent to sgp40 to initiate a read
uint8_t sgp_write[8] = {0x26, 0x0F, 0x80, 0x00, 0xA2, 0x66, 0x66, 0x93};

static I2C_TransferSeq_TypeDef transferSequence;
uint8_t tempData[3];
/**********************************************************************
 * to write to sgp40 via I2C
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void write_to_sgp40(void)
{
   I2C_TransferReturn_TypeDef transferStatus;

   /*initialise i2c*/
   I2Cinit();

   transferSequence.addr        = (SGP40_SENSOR_ADDR << 1);
   transferSequence.flags       = I2C_FLAG_WRITE;
   transferSequence.buf[0].data = &sgp_write[0];
   transferSequence.buf[0].len  = sizeof(sgp_write);

   NVIC_EnableIRQ(I2C0_IRQn);

   transferStatus = I2C_TransferInit(I2C0, &transferSequence);

   if (transferStatus < 0)
   {
      LOG_ERROR("I2C_TransferInit() failed = %d", transferStatus);
   }
}

/**********************************************************************
 * read VOC value from sgp40
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void read_from_sgp40()
{
   I2C_TransferReturn_TypeDef transferStatus;

   /*initialise i2c*/
   I2Cinit();

   transferSequence.addr = (SGP40_SENSOR_ADDR << 1);
   transferSequence.flags = I2C_FLAG_READ;
   transferSequence.buf[0].data = &tempData[0];
   transferSequence.buf[0].len = sizeof(tempData);

   NVIC_EnableIRQ(I2C0_IRQn);

   transferStatus = I2C_TransferInit (I2C0, &transferSequence);

   if (transferStatus < 0)
   {
      LOG_ERROR("I2C_TransferInit() failed = %d", transferStatus);
   }
}

/**********************************************************************
 * to read VOC values
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
int obtainVOCRawValues()
{
   uint16_t dataRead = (tempData[0] << 8) + tempData[1];

   LOG_INFO("raw VOC value is %d\n\r",dataRead);

   return dataRead;
}

/**********************************************************************
 * initialise i2c peripheral
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void I2Cinit()
{
  I2CSPM_Init_TypeDef i2cSpmInit;

  i2cSpmInit.port            = I2C0;
  i2cSpmInit.sclPin          = I2C0_SCL_PIN;
  i2cSpmInit.sclPort         = I2C0_SCL_PORT;
  i2cSpmInit.sdaPin          = I2C0_SDA_PIN;
  i2cSpmInit.sdaPort         = I2C0_SDA_PORT;
  i2cSpmInit.portLocationScl = I2C0_PORT_LOC_SCL;
  i2cSpmInit.portLocationSda = I2C0_PORT_LOC_SDA;

  i2cSpmInit.i2cRefFreq      = 0;
  i2cSpmInit.i2cMaxFreq      = I2C_FREQ_STANDARD_MAX;
  i2cSpmInit.i2cClhr         = i2cClockHLRStandard;

  //configure I2C0
  I2CSPM_Init(&i2cSpmInit);
  //i2cInit.
}

/**************************end of file**********************************/
