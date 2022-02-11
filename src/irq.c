/***********************************************************************
* @file irq.c
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
* -
*
* @copyright All rights reserved. Distribution allowed only for the
* use of assignment grading. Use of code excerpts allowed at the
* discretion of author. Contact for permission.
*/

/*header files*/
#include "irq.h"
#include "gpio.h"
#include "em_core.h"
#include "scheduler.h"
#include "em_i2c.h"
#include "log.h"

/*global variables*/
uint32_t irqState;
static int underflowCount = 0;

/**********************************************************************
 * letimer interrupt handler
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void LETIMER0_IRQHandler (void)
{
  int flags = 0;

  //to obtain all current interrupts into a local flag
  flags = LETIMER_IntGetEnabled(LETIMER0);

  // clear all the interrupts
  LETIMER_IntClear(LETIMER0, flags);

  if(flags == LETIMER_IEN_UF)
  {
      schedulerSetEventTemperatureRead();
      underflowCount++;
  }
  if (flags == LETIMER_IEN_COMP1)
  {
      schedulerSetEventSetComp1();
  }
} // LETIMER0_IRQHandler()

/**********************************************************************
 * provide milliseconds using Letimer
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
int letimerMilliseconds()
{
   int temp = (LETIMER_CounterGet(LETIMER0) * 1000)/ACTUAL_CLK_FREQ;

   return((underflowCount * LETIMER_PERIOD_MS) + temp);
}

/**********************************************************************
 * I2C interrupt handler
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void I2C0_IRQHandler(void)
{
   I2C_TransferReturn_TypeDef transferStatus;
   transferStatus = I2C_Transfer(I2C0);

   if (transferStatus == i2cTransferDone)
   {
       schedulerSetI2CEvent();
   }

   if (transferStatus < 0)
   {
      LOG_ERROR("I2C_Transfer failed with %d", transferStatus);
   }
} // I2C0_IRQHandler()

/**************************end of file**********************************/
