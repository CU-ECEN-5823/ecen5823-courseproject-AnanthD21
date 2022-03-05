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
#include "i2c.h"
#include "ble.h"
#include "gpio.h"
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

// 80 millisec obtained from datasheet
#define TIME_TO_WAIT 80000

#define I2C0_SCL_PORT  gpioPortC
#define I2C0_SCL_PIN   10
#define I2C0_SDA_PORT  gpioPortC
#define I2C0_SDA_PIN   11
#define I2C0_PORT_LOC_SCL 14
#define I2C0_PORT_LOC_SDA 16
#define SI7021_TEMP_SENSOR_ADDR 0x40


I2C_TransferSeq_TypeDef transferSequence;
uint8_t cmd_data;
uint8_t tempData[2];

/**********************************************************************
 * enable or disable sensor
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void enableSensor(bool status)
{
   if(status == true)
   {
      GPIO_PinOutSet(gpioPortD, 15);
   }
   else
   {
      GPIO_PinOutClear(gpioPortD, 15);
   }
}

/**********************************************************************
 * enable the si7021 temperature sensor
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
int enable_si7021()
{
   // Enable the sensor
   //enableSensor(true);
   gpioSensorEnSetOn();
   // wait for I2C transfer
   timerWaitUs_irq(TIME_TO_WAIT);

   return 0;
}

/**********************************************************************
 * to write to si7021 via I2C
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void write_to_si7021(void)
{
   I2C_TransferReturn_TypeDef transferStatus;

   /*initialise i2c*/
   I2Cinit();

   cmd_data = 0xF3;

   transferSequence.addr        = (SI7021_TEMP_SENSOR_ADDR << 1);
   transferSequence.flags       = I2C_FLAG_WRITE;
   transferSequence.buf[0].data = &cmd_data;
   transferSequence.buf[0].len  = sizeof(cmd_data);

   NVIC_EnableIRQ(I2C0_IRQn);

   transferStatus = I2C_TransferInit(I2C0, &transferSequence);

   if (transferStatus < 0)
   {
      //LOG_ERROR("I2C_TransferInit() failed = %d", transferStatus);
   }
}

/**********************************************************************
 * read temperature value from si7021
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void read_from_si7021()
{
   I2C_TransferReturn_TypeDef transferStatus;

   /*initialise i2c*/
   I2Cinit();

   transferSequence.addr = (SI7021_TEMP_SENSOR_ADDR << 1);
   transferSequence.flags = I2C_FLAG_READ;
   transferSequence.buf[0].data = tempData;
   transferSequence.buf[0].len = sizeof(tempData);

   NVIC_EnableIRQ(I2C0_IRQn);

   transferStatus = I2C_TransferInit (I2C0, &transferSequence);

   if (transferStatus < 0)
   {
      //LOG_ERROR("I2C_TransferInit() failed = %d", transferStatus);
   }
}

/**********************************************************************
 * provides temperature value from si7021
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void provide_temperature()
{
   float tempInDegCelcius = 0;

   uint16_t dataRead = (tempData[0] << 8) + tempData[1];

   tempInDegCelcius = (((dataRead * 175.72) / 65536) - 46.85);

   //LOG_INFO("temperature is %d\n\r",tempInDegCelcius);

   //enableSensor(false);

   report_data_ble(tempInDegCelcius);
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
