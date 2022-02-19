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

/*header files*/
#include "scheduler.h"
#include "log.h"
#include "i2c.h"
#define INCLUDE_LOG_DEBUG 1

static State_I2C_t currState = idle;
#define TIME_TO_TRANSFER 11000

/**********************************************************************
 * temperature state machine
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void state_machine(sl_bt_msg_t *evt)
{
    switch(currState)
    {
       case idle:
       {
          currState = idle;

          if(evt->data.evt_system_external_signal.extsignals == evtLETIMER0_UF)
          {
             currState = poweron;
             enable_si7021();
          }

          break;
       }

      case poweron:
      {
         currState = poweron;

         if(evt->data.evt_system_external_signal.extsignals == evtLETIMER0_COMP1)
         {
            currState = waitforwritecompletion;

            sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
            write_to_si7021();
         }

         break;
      }

      case waitforwritecompletion:
      {
         currState = waitforwritecompletion;

         if(evt->data.evt_system_external_signal.extsignals == evt_I2C)
         {
            currState = intiateread;
            NVIC_DisableIRQ(I2C0_IRQn);
            sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);

            timerWaitUs_irq(TIME_TO_TRANSFER);
         }

         break;
      }

      case intiateread:
      {
         currState = intiateread;

         if(evt->data.evt_system_external_signal.extsignals == evtLETIMER0_COMP1)
         {
            currState = readcomplete;
            sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
            read_from_si7021();
         }

         break;
      }

      case readcomplete:
      {
         currState = readcomplete;

         if(evt->data.evt_system_external_signal.extsignals == evt_I2C)
         {
            NVIC_DisableIRQ(I2C0_IRQn);
            currState = idle;
            sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
            provide_temperature();
         }
         break;
      }
  }
}

/**********************************************************************
 * set the underflow event
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/

void  schedulerSetEventTemperatureRead()
{
   CORE_CRITICAL_SECTION(sl_bt_external_signal(evtLETIMER0_UF););
}

/**********************************************************************
 * set the comp1 event
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void schedulerSetEventSetComp1()
{
   CORE_CRITICAL_SECTION(sl_bt_external_signal(evtLETIMER0_COMP1););
}

/**********************************************************************
 * set the i2c event
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void schedulerSetI2CEvent()
{
   CORE_CRITICAL_SECTION(sl_bt_external_signal(evt_I2C););
}

/*********************end of file**************************/
