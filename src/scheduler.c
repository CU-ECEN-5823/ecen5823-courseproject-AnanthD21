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
#include "src/i2c_sgp40.h"
#include <src/i2c_shtc3.h>

#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

static State_I2C_t currState = idle;

#define TIME_TO_WAIT_SGP40 1000
#define TIME_TO_COMPUTE_SGP40 250000
#define TIME_TO_WAIT_SHTC3 300
#define TIME_TO_COMPUTE_SHTC3 13000

//bool flag to alternate between sgp40 and shtc3 sensors reading alternatively
bool switchSensor = false;

/**********************************************************************
 * state Machine for reading sensors alternatively
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void sensorsStateMachine(sl_bt_msg_t *evt)
{
    switch(currState)
    {
       case idle:
       {
          currState = idle;

          if(evt->data.evt_system_external_signal.extsignals == evtLETIMER0_UF)
          {
              if(true == switchSensor)
              {
                 timerWaitUs_irq(TIME_TO_WAIT_SGP40);
              }
              else
              {
                  timerWaitUs_irq(TIME_TO_WAIT_SHTC3);
              }
              currState = initiatewrite;
          }

          break;
       }

      case initiatewrite:
      {
         currState = initiatewrite;

         if(evt->data.evt_system_external_signal.extsignals == evtLETIMER0_COMP1)
         {
             // Disable the LETIMER0 COMP1
            LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);
            // EM1 Sleepmode at I2C transfer
            sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);

            if(true == switchSensor)
            {
               write_to_sgp40();
            }
            else
            {
                write_to_shtc3();
            }

            currState = waitforwritecompletion;
         }

         break;
      }

      case waitforwritecompletion:
      {
         currState = waitforwritecompletion;

         if(evt->data.evt_system_external_signal.extsignals == evt_I2C)
         {
             // Disable IRQ I2C
            NVIC_DisableIRQ(I2C0_IRQn);

            sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);

            if(true == switchSensor)
            {
               timerWaitUs_irq(TIME_TO_COMPUTE_SGP40);
            }
            else
            {
                timerWaitUs_irq(TIME_TO_COMPUTE_SHTC3);
            }

            currState = intiateread;
         }

         break;
      }

      case intiateread:
      {
         currState = intiateread;

         if(evt->data.evt_system_external_signal.extsignals == evtLETIMER0_COMP1)
         {
             // Disable the LETIMER0 COMP1
            LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);

            sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);

            if(true == switchSensor)
            {
                read_from_sgp40();
            }
            else
            {
                read_from_shtc3();
            }

            currState = readcomplete;
         }

         break;
      }

      case readcomplete:
      {
         currState = readcomplete;

         if(evt->data.evt_system_external_signal.extsignals == evt_I2C)
         {
             NVIC_DisableIRQ(I2C0_IRQn);

             if(true == switchSensor)
             {
                 int retVal = obtainVOCRawValues();
                 report_data_ble(retVal);
             }
             else
             {
                 int retVal = obtainHumidityValues();
                 report_data_ble_humidity(retVal);
             }

             sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);

             currState = idle;

             /*toggle the flag in order to read each sensor data alternatively*/
             switchSensor = !switchSensor;
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
