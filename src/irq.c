/***********************************************************************
* @file irq.c
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
#include "irq.h"
#include "gpio.h"
#include "em_core.h"

/*global variables*/
uint32_t irqState;

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

  //disable all interrupts
  CORE_ENTER_CRITICAL();
  if(flags == LETIMER_IEN_UF)
  {
      // LED is Switched ON at end of count
     gpioLed0SetOn();
  }
  else if (flags == LETIMER_IEN_COMP1)
  {
     // LED is switched OFF at 175ms
     gpioLed0SetOff();
  }
  CORE_EXIT_CRITICAL();
  //enable all interrupts

} // LETIMER0_IRQHandler()

/**************************end of file**********************************/
