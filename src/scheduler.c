/***********************************************************************
* @file scheduler.c
* @version 0.0.1
* @brief Function header file.
*
* @author Ananth Deshpande, ande9392@Colorado.edu
*          Abhishek suryawanshi, absu8154@colorado.edu
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

static State_Disovery_t DiscState;

//Humidity service UUID
static const uint8_t hum_service[16] = {0x52, 0x87, 0xE6, 0x69,0x6E, 0x23, 0xd3, 0xb2, 0x47, 0x40, 0x6d, 0x5c, 0x42, 0xec, 0x5d, 0xb0};
static const uint8_t hum_char[16] = {0x17, 0x1E, 0x67, 0xE4, 0x39, 0xCE, 0xB4, 0x9C, 0xAE, 0x4D, 0x63, 0x35, 0x96, 0x8B, 0x14, 0xE4};

//VOC service UUID
static const uint8_t voc_service[16] = {0xd7, 0x38, 0xd9, 0xfe, 0x19, 0x95, 0x08, 0xA2, 0x47, 0x4E, 0xD9, 0x30, 0x3C, 0xF0, 0xD0, 0xCE};
static const uint8_t voc_char[16] = {0x02, 0xEB, 0x12, 0xE4, 0x4E, 0xE9, 0x37, 0x93, 0xC4, 0x42, 0x85, 0x52, 0x14, 0xCA, 0xD1, 0x6D};

#define TIME_TO_TRANSFER 11000

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
                 report_data_ble_sgp40_values(retVal);
             }
             else
             {
                 int retVal = obtainHumidityValues();
                 report_data_ble_shtc3_values(retVal);
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

void  schedulerSetEventSensorRead()
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

  LOG_ERROR("DiscState = %d\n", DiscState);

  switch(DiscState)
  {
     case ConnectionState:
     {
       DiscState = ConnectionState;
        if(Client_Connected_Event == ble_data->ble_client_evt)
        {
           // Discover Health Thermometer service on the responder device
           sc = sl_bt_gatt_discover_primary_services_by_uuid(ble_data->connectionHandle,
                                                             sizeof(voc_service),
                                                             (const uint8_t*)voc_service);
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
                                                            ble_data->voc_service_handle,
                                                            sizeof(voc_char),
                                                            (const uint8_t*)voc_char);
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
                                                           ble_data->voc_characteristic_handle,
                                                           sl_bt_gatt_indication);
           //app_assert_status(sc);
           if(sc != SL_STATUS_OK)
           {
              LOG_ERROR("sl_bt_gatt_set_characteristic_notification failed\n");
           }

           DiscState = humConnectionState;
        }
     }
     break;

     case humConnectionState:
     {
       DiscState = humConnectionState;
       if(Client_Procedure_Complete_Event == ble_data->ble_client_evt)
       {
          // Discover Health Thermometer service on the responder device
          sc = sl_bt_gatt_discover_primary_services_by_uuid(ble_data->connectionHandle,
                                                            sizeof(hum_service),
                                                            (const uint8_t*)hum_service);
          //app_assert_status(sc);
          if(sc != SL_STATUS_OK)
          {
             LOG_ERROR("sl_bt_gatt_discover_primary_services_by_uuid failed\n");
          }
          DiscState = humServiceState;
       }
     }
     break;

     case humServiceState:
     {
       DiscState = humServiceState;

        if( Client_Procedure_Complete_Event == ble_data->ble_client_evt)
        {
           // Discover thermometer characteristic on the responder device
           sc = sl_bt_gatt_discover_characteristics_by_uuid(ble_data->connectionHandle,
                                                            ble_data->hum_service_handle,
                                                            sizeof(hum_char),
                                                            (const uint8_t*)hum_char);
           //app_assert_status(sc);
           if(sc != SL_STATUS_OK)
           {
              LOG_ERROR("sl_bt_gatt_discover_characteristics_by_uuid failed\n");
           }

           DiscState = humCharacteristicState;
        }
     }
     break;

     case humCharacteristicState:
     {
       DiscState = humCharacteristicState;

        if(Client_Procedure_Complete_Event == ble_data->ble_client_evt)
        {
           // enable indications
           sc = sl_bt_gatt_set_characteristic_notification(ble_data->connectionHandle,
                                                           ble_data->hum_characteristic_handle,
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
    }
    break;
  }

  // Clearing every time before state chart execution.
  ble_data->ble_client_evt = Client_default;
}


/*********************end of file**************************/
