/***********************************************************************
* @file scheduler.h
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


#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "em_core.h"
#include "app.h"
#include "ble.h"

void scheduler_init();

void  schedulerSetEventSensorRead();
void schedulerSetEventSetComp1();
void schedulerSetI2CEvent();
uint32_t getNextEvent();
void sensorsStateMachine(sl_bt_msg_t *evt);



typedef enum
{
  clear = 0,
  evtLETIMER0_UF = 1,
  evtLETIMER0_COMP1,
  evt_I2C
}evt_t;

typedef enum
{
  idle,
  initiatewrite,
  waitforwritecompletion,
  intiateread,
  readcomplete
}State_I2C_t;

typedef enum
{
   ConnectionState,
   ServiceState,
   CharacteristicState,
   humConnectionState,
   humServiceState,
   humCharacteristicState,
   IndicationState
}State_Disovery_t;

void discovery_state_machine(sl_bt_msg_t *evt);

#endif
