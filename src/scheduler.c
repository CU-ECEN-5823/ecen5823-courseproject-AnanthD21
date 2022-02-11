/***********************************************************************
* @file scheduler.c
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
#include "scheduler.h"

static evt_t event;

/**********************************************************************
 * set underflow event
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void schedulerSetEventTemperatureRead()
{
  CORE_DECLARE_IRQ_STATE;

  // enter critical section
  CORE_ENTER_CRITICAL();

  event = evtLETIMER0_UF;

  // exit critical section
  CORE_EXIT_CRITICAL();

} // schedulerSetEventTemperatureRead()

/**********************************************************************
 * Set comp1 event
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void schedulerSetEventSetComp1()
{
  CORE_DECLARE_IRQ_STATE;

  // enter critical section
  CORE_ENTER_CRITICAL();

  event = evtLETIMER0_COMP1;

  // exit critical section
  CORE_EXIT_CRITICAL();

} // schedulerSetEventComp()

/**********************************************************************
 * set I2C event
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void schedulerSetI2CEvent()
{
  CORE_DECLARE_IRQ_STATE;

  // enter critical section
  CORE_ENTER_CRITICAL();

  event = evt_I2C;

  // exit critical section
  CORE_EXIT_CRITICAL();

} // schedulerSetEventComp()

/**********************************************************************
 * return event
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
uint32_t getNextEvent()
{
  uint32_t theEvent;

  CORE_DECLARE_IRQ_STATE;

  // return an event to main
  theEvent = event;

  // enter critical section
  CORE_ENTER_CRITICAL();

  // clear the event, this is a read-modify-write
  // and thus protected via critical section
  event = clear;

  // exit critical section
  CORE_EXIT_CRITICAL();

  return (theEvent);
} // getNextEvent()

/**************************end of file**********************************/
