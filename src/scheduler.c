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
#define INCLUDE_LOG_DEBUG 0

static State_I2C_t currState = idle;

static State_Disovery_t DiscState;

// Temperature Measurement characteristic UUID
static const uint8_t thermo_char[2] = { 0x1c, 0x2a };

// Health Thermometer service UUID
static const uint8_t thermo_service[2] = { 0x09, 0x18 };

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

/**********************************************************************
 * discovery state machine
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void discovery_state_machine(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  ble_data_struct_t* ble_data = provideBleDataPtr();

  if(Client_Closed_Event == ble_data->ble_client_evt)
  {
      DiscState = ConnectionState;
  }

  switch(DiscState)
  {
     case ConnectionState:
     {
       DiscState = ConnectionState;
        if(Client_Connected_Event == ble_data->ble_client_evt)
        {
           // Discover Health Thermometer service on the responder device
           sc = sl_bt_gatt_discover_primary_services_by_uuid(ble_data->connectionHandle,
                                                             sizeof(thermo_service),
                                                             (const uint8_t*)thermo_service);
           //app_assert_status(sc);
           if(sc != SL_STATUS_OK)
           {
              LOG_ERROR("sl_bt_gatt_discover_primary_services_by_uuid failed\n");
           }
           DiscState = ServiceState;
        }
     }
     break;

     case ServiceState:
     {
       DiscState = ServiceState;

        if( Client_Procedure_Complete_Event == ble_data->ble_client_evt)
        {
           // Discover thermometer characteristic on the responder device
           sc = sl_bt_gatt_discover_characteristics_by_uuid(ble_data->connectionHandle,
                                                            ble_data->thermometer_service_handle,
                                                            sizeof(thermo_char),
                                                            (const uint8_t*)thermo_char);
           //app_assert_status(sc);
           if(sc != SL_STATUS_OK)
           {
              LOG_ERROR("sl_bt_gatt_discover_characteristics_by_uuid failed\n");
           }

           DiscState = CharacteristicState;
        }
     }
     break;

     case CharacteristicState:
     {
       DiscState = CharacteristicState;

        if(Client_Procedure_Complete_Event == ble_data->ble_client_evt)
        {
           // enable indications
           sc = sl_bt_gatt_set_characteristic_notification(ble_data->connectionHandle,
                                                           ble_data->thermometer_characteristic_handle,
                                                           sl_bt_gatt_indication);
           //app_assert_status(sc);
           if(sc != SL_STATUS_OK)
           {
              LOG_ERROR("sl_bt_gatt_set_characteristic_notification failed\n");
           }

           DiscState = IndicationState;
        }
     }
     break;

    case IndicationState:
    {
      DiscState = IndicationState;

       if( Client_Indicate_Event == ble_data->ble_client_evt)
       {
          // enable indications
          sc = sl_bt_gatt_send_characteristic_confirmation(evt->data.evt_gatt_characteristic_value.connection);
          //app_assert_status(sc);
          if(sc != SL_STATUS_OK)
          {
             LOG_ERROR("sl_bt_gatt_send_characteristic_confirmation failed\n");
          }
          DiscState = IndicationState;
       }
    }
    break;
  }

  // Clearing every time before state chart execution.
  ble_data->ble_client_evt = Client_default;
}


/*********************end of file**************************/
