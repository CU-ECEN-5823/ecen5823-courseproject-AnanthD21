/***********************************************************************
* @file oscillator.c
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
*
*
* @copyright All rights reserved. Distribution allowed only for the
* use of assignment grading. Use of code excerpts allowed at the
* discretion of author. Contact for permission.
*/

#include "oscillator.h"

/**********************************************************************
 * initialise oscillator
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void oscillatorInit()
{
  // for energy mode 3, we shall utilize Ultra low frequency
  // oscillator
  if (ENERGY_MODE == EM3)
  {
      // enable ultra low frequency oscillator
      CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);

      //select low frequency clock A
      CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);

      //set prescalar to 1
      CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_1);
  }
  // for rest of the energy modes, we shall utilise
  // low frequency oscillator
  else
  {
      //enable low frequency oscillator
      CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

      //select low frequency clock A
      CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

      //set prescalar to 4
      CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_4);
  }

  //enable letimer0 clock
  CMU_ClockEnable(cmuClock_LETIMER0, true);
}

/**************************end of file**********************************/
