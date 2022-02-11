/***********************************************************************
* @file timer.c
* @version 0.0.1
* @brief Function header file.
*
* @author Ananth Deshpande, ande9392@Colorado.edu
* @date Jan 28, 2022
*
* @institution University of Colorado Boulder (UCB)
* @course ECEN 5823-001: IoT Embedded Firmware (Fall 2020)
* @instructor David Sluiter
*
* @assignment ecen5823-assignment3-AwesomeStudent
* @due Jan 28, 2022
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
#include "timers.h"
#include "app.h"

#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

/*letimer init structure object*/
LETIMER_Init_TypeDef letimerInit;

/**********************************************************************
 * initialise letimer
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void letimerinit()
{
  /*initialize structure object for LETIMER_Init_TypeDef*/
  letimerInit.comp0Top = true;
  letimerInit.bufTop   = false;
  letimerInit.enable   = false;
  letimerInit.debugRun = false;
  letimerInit.out0Pol  = 0;
  letimerInit.out1Pol  = 0;
  letimerInit.ufoa0    = letimerUFOANone;
  letimerInit.ufoa1    = letimerUFOANone;
  letimerInit.repMode  = letimerRepeatFree;

  LETIMER_Init(LETIMER0, &letimerInit);

  // Counter value updated to COMP0
  LETIMER_CompareSet(LETIMER0, 0, VALUE_TO_COMP0);

  // Interrupt count updated to COMP1
  //LETIMER_CompareSet(LETIMER0, 1, VALUE_TO_COMP1);

  //set letimer counter value
  LETIMER_CounterSet(LETIMER0, VALUE_TO_COMP0);

  //Interrupt enabled at Underflow and COMP1
  LETIMER_IntEnable(LETIMER0, (LETIMER_IEN_UF/* | LETIMER_IEN_COMP1*/));

  //clear any pending interrupts
  NVIC_ClearPendingIRQ(LETIMER0_IRQn);

  //set LETIMER0 interrupt
  NVIC_EnableIRQ(LETIMER0_IRQn);

  //enable LETIMER0
  LETIMER_Enable(LETIMER0, true);
}


/**********************************************************************
 * wait for provided time value
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void timerWaitUs_polled(uint32_t waitMicroSeconds)
{

    uint32_t curCounterVal;

    // Condition to check if the delay time can be handled
    if(waitMicroSeconds > (LETIMER_PERIOD_MS*1000))
    {
       LOG_ERROR("input counter value is beyond the max possible value\n");
       waitMicroSeconds = 3000000;  // Maximum possible delay
    }

    uint32_t waitMilliseconds = waitMicroSeconds / 1000;
    uint32_t delayCount       = (waitMilliseconds * ACTUAL_CLK_FREQ) / 1000;

    curCounterVal = LETIMER_CounterGet(LETIMER0);

    // decrement the count wrt Letimer
    while (delayCount)
    {
       if (curCounterVal != LETIMER_CounterGet(LETIMER0) )
       {
          delayCount--;
          curCounterVal = LETIMER_CounterGet(LETIMER0);
       }
    } // while
}

/**********************************************************************
 * wait for provided time value
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void timerWaitUs_irq(uint32_t waitMicroSeconds)
{
   volatile uint32_t temp;

   if((waitMicroSeconds > (LETIMER_PERIOD_MS*1000)))
   {
      LOG_ERROR("input counter value is beyond the max possible value\n");
      waitMicroSeconds = 3000000;
   }
   else if(waitMicroSeconds == 0)
   {
      LOG_ERROR("input value is zero!!!\r\n");
   }

   //convert to milliseconds
   uint32_t waitMilliseconds = waitMicroSeconds / 1000;
   uint32_t us_wait_count    = (waitMilliseconds * ACTUAL_CLK_FREQ) / 1000;

   uint32_t delayCount       = ((LETIMER_PERIOD_MS * ACTUAL_CLK_FREQ) / 1000);

   if((LETIMER_CounterGet(LETIMER0) - us_wait_count) >= 0)
   {
      temp = (LETIMER_CounterGet(LETIMER0) - us_wait_count);
   }
   else
   {
      temp = (delayCount) -(us_wait_count - LETIMER_CounterGet(LETIMER0));
   }

   //clear comp1 Interrupt flag
   LETIMER_IntClear(LETIMER0, LETIMER_IFC_COMP1);

   //load comp1 value
   LETIMER_CompareSet(LETIMER0, 1, temp);

   //enable comp1 interrupt
   LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);
}

/**************************end of file**********************************/
