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

#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

// 80 millisec obtained from datasheet
#define TIME_TO_WAIT 80000

// 11 millisec transfer time
#define TIME_TO_TRANSFER 11000


#define I2C0_SCL_PORT  gpioPortC
#define I2C0_SCL_PIN   10
#define I2C0_SDA_PORT  gpioPortC
#define I2C0_SDA_PIN   11
#define I2C0_PORT_LOC_SCL 14
#define I2C0_PORT_LOC_SDA 16
#define SI7021_TEMP_SENSOR_ADDR 0x40

uint8_t tempData[2];

/**********************************************************************
 * read temperature value from si7021
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
int readTemperatureValue()
{
  I2C_TransferReturn_TypeDef transferStatus;
  int tempInDegCelcius = 0;
  uint16_t dataRead = 0;
  I2C_TransferSeq_TypeDef readData;

  /*fill the transfer buffer*/
  readData.addr = SI7021_TEMP_SENSOR_ADDR << 1;
  readData.flags = I2C_FLAG_READ;
  readData.buf[0].data = tempData;
  readData.buf[0].len = sizeof(tempData);

  transferStatus = I2CSPM_Transfer(I2C0,&readData);

  if(i2cTransferDone == transferStatus)
  {
     dataRead = (tempData[0] << 8)+ tempData[1];

     tempInDegCelcius = (((dataRead * 175.72)/65536)-46.85);

     LOG_INFO("temperature read from si7021 is %d deg celcius\n\r",tempInDegCelcius);

     return tempInDegCelcius;
  }
  else
  {
      LOG_ERROR("I2CSPM_Transfer failed with status %d",transferStatus);
     return 0;
  }
}

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
 * read temperature from si7021 sensor
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void read_temp_from_si7021()
{
  I2C_TransferReturn_TypeDef transferStatus;
  I2C_TransferSeq_TypeDef writeData;

  // Enable the sensor
  enableSensor(true);

  //wait for 80ms to get sensor up
  timerWaitUs(TIME_TO_WAIT);

  uint8_t cmd_data = 0xF3;
  writeData.addr         = (SI7021_TEMP_SENSOR_ADDR << 1);
  writeData.flags        = I2C_FLAG_WRITE;
  writeData.buf[0].data  = &cmd_data;
  writeData.buf[0].len   = sizeof(cmd_data);

  // I2CSPM_Transfer returns the transfer state
  transferStatus = I2CSPM_Transfer(I2C0, &writeData);

  // on successful I2CSPM_Transfer
  if(i2cTransferDone == transferStatus)
  {
     /*wait for transfer to complete*/
     timerWaitUs(TIME_TO_TRANSFER);
     readTemperatureValue();
  }
  else
  {
     LOG_ERROR("I2CSPM_Transfer failed with status %d",transferStatus);
  }

  //disable the sensor
  enableSensor(false);
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
