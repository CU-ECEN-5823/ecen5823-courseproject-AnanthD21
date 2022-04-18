/***********************************************************************
* @file i2c.c
* @version 0.0.1
* @brief Function header file.
*
* @author Abhishek suryawanshi, absu8154@colorado.edu
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

#include <src/i2c_shtc3.h>
#include "scheduler.h"
#include <src/i2c_sgp40.h>

#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#define TIME_TO_WAIT_SHTC3 300
#define TIME_TO_COMPUTE_SHTC3 13000

#define I2C0_SCL_PORT  gpioPortC
#define I2C0_SCL_PIN   10
#define I2C0_SDA_PORT  gpioPortC
#define I2C0_SDA_PIN   11
#define I2C0_PORT_LOC_SCL 14
#define I2C0_PORT_LOC_SDA 16
#define SHTC3_HUMIDITY_SENSOR_ADDR 0x70

static I2C_TransferSeq_TypeDef transferSequence;
static uint8_t cmd_data[2] = {0x5C, 0x24};
static uint8_t tempData[2];

/**********************************************************************
 * to write to shtc3 via I2C
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void write_to_shtc3(void)
{
   I2C_TransferReturn_TypeDef transferStatus;

   /*initialise i2c*/
   //I2Cinit();

   transferSequence.addr        = (SHTC3_HUMIDITY_SENSOR_ADDR << 1);
   transferSequence.flags       = I2C_FLAG_WRITE;
   transferSequence.buf[0].data = &cmd_data[0];
   transferSequence.buf[0].len  = sizeof(cmd_data);

   NVIC_EnableIRQ(I2C0_IRQn);

   transferStatus = I2C_TransferInit(I2C0, &transferSequence);

   if (transferStatus < 0)
   {
      LOG_ERROR("I2C_TransferInit() failed = %d", transferStatus);
   }
}

/**********************************************************************
 * read temperature value from shtc3
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void read_from_shtc3()
{
   I2C_TransferReturn_TypeDef transferStatus;

   /*initialise i2c*/
   I2Cinit();

   transferSequence.addr = (SHTC3_HUMIDITY_SENSOR_ADDR << 1);
   transferSequence.flags = I2C_FLAG_READ;
   transferSequence.buf[0].data = &tempData[0];
   transferSequence.buf[0].len = sizeof(tempData);

   NVIC_EnableIRQ(I2C0_IRQn);

   transferStatus = I2C_TransferInit(I2C0, &transferSequence);

   if (transferStatus < 0)
   {
      LOG_ERROR("I2C_TransferInit() failed = %d", transferStatus);
   }
}

/**********************************************************************
 * to read humidity values
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
float obtainHumidityValues()
{
  uint16_t dataRead = (tempData[0] << 8)+ tempData[1];
  float relHumidity = (100 * dataRead) / 65536;

  LOG_INFO("humidity raw value: %f\n\r",relHumidity);

  return relHumidity;
}
/**************************end of file**********************************/
