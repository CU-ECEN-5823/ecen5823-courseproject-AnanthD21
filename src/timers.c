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
  LETIMER_CompareSet(LETIMER0, 1, VALUE_TO_COMP1);

  //set letimer counter value
  LETIMER_CounterSet(LETIMER0, VALUE_TO_COMP0);

  //Interrupt enabled at Underflow and COMP1
  LETIMER_IntEnable(LETIMER0, (LETIMER_IEN_UF | LETIMER_IEN_COMP1));

  //clear any pending interrupts
  NVIC_ClearPendingIRQ(LETIMER0_IRQn);

  //set LETIMER0 interrupt
  NVIC_EnableIRQ(LETIMER0_IRQn);

  //enable LETIMER0
  LETIMER_Enable(LETIMER0, true);
}

/**************************end of file**********************************/
