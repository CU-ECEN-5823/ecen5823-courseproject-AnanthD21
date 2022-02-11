/***********************************************************************
* @file timer.h
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

#ifndef SRC_TIMERS_H_
#define SRC_TIMERS_H_

#include "em_letimer.h"

/**********************************************************************
 * initialise letimer
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void letimerinit();

void timerWaitUs_polled(uint32_t waitMicroSeconds);

void timerWaitUs_irq(uint32_t us_wait);

#endif /* SRC_TIMERS_H_ */

/**************************end of file**********************************/
