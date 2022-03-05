/***********************************************************************
* @file ble.c
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
#include "ble.h"

#define INCLUDE_LOG_DEBUG 0
#include "src/log.h"

#include "ble_device_type.h"

// BLE private data
ble_data_struct_t ble_data; // this is the declaration

#if DEVICE_IS_BLE_SERVER
// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;
#else
// Health Thermometer service UUID
static const uint8_t thermo_service[2] = { 0x09, 0x18 };
// Temperature Measurement characteristic UUID
static const uint8_t thermo_char[2] = { 0x1c, 0x2a };
#endif

/**********************************************************************
 * provides BLE data
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
ble_data_struct_t* provideBleDataPtr()
{
   return (&ble_data);
}

/**********************************************************************
 * handles ble state machine
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/

void handle_ble_event(sl_bt_msg_t *evt)
{
   sl_status_t sc;

#if DEVICE_IS_BLE_SERVER
    //uint8_t system_id[8];
#else
    uint8_t * temperature;
    int i=0;
    int flag_addr = 0;
#endif

    // Handle stack events
    switch (SL_BT_MSG_ID(evt->header))
    {
      // ******************************************************
      // Events common to both Servers and Clients
      // ******************************************************

      // Boot Event
      case sl_bt_evt_system_boot_id:

        //obtain unique ID from BT Address.
        sc = sl_bt_system_get_identity_address(&(ble_data.myAddress), &(ble_data.myAddressType));
        //app_assert_status(sc);
#if DEVICE_IS_BLE_SERVER
        displayInit();

        displayPrintf(DISPLAY_ROW_NAME, "Server");
        displayPrintf(DISPLAY_ROW_BTADDR, "%02X:%02X:%02X:%02X:%02X:%02X",
                      ble_data.myAddress.addr[5],
                      ble_data.myAddress.addr[4],
                      ble_data.myAddress.addr[3],
                      ble_data.myAddress.addr[2],
                      ble_data.myAddress.addr[1],
                      ble_data.myAddress.addr[0]);
        displayPrintf(DISPLAY_ROW_ASSIGNMENT, "A7");

        app_assert_status(sc);

        if(sc != SL_STATUS_OK)
        {
           LOG_ERROR("sl_bt_gatt_server_write_attribute_value failed\n");
        }

        LOG_INFO("Bluetooth %s address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                 ble_data.myAddressType ? "static random" : "public device",
                     ble_data.myAddress.addr[5],
                     ble_data.myAddress.addr[4],
                     ble_data.myAddress.addr[3],
                     ble_data.myAddress.addr[2],
                     ble_data.myAddress.addr[1],
                     ble_data.myAddress.addr[0]);

        // Create an advertising set.
        sc = sl_bt_advertiser_create_set(&advertising_set_handle);
        //app_assert_status(sc);
        if(sc != SL_STATUS_OK)
        {
           LOG_ERROR("sl_bt_advertiser_create_set failed\n");
        }

        // Set advertising interval to 250ms.
        sc = sl_bt_advertiser_set_timing(
                                         advertising_set_handle, // advertising set handle
                                         400,                    // min. adv. interval (milliseconds * 1.6)
                                         400,                    // max. adv. interval (milliseconds * 1.6)
                                           0,                    // adv. duration
                                           0);                   // max. num. adv. events
        //app_assert_status(sc);
        if(sc != SL_STATUS_OK)
        {
           LOG_ERROR("sl_bt_advertiser_set_timing failed\n");
        }
        // Start general advertising and enable connections.
        sc = sl_bt_advertiser_start(
          advertising_set_handle,
          sl_bt_advertiser_general_discoverable,
          sl_bt_advertiser_connectable_scannable);
        //app_assert_status(sc);

        if(sc != SL_STATUS_OK)
        {
           LOG_ERROR("sl_bt_advertiser_start failed\n");
        }

        LOG_INFO("advertising started\n");

        displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");

#else
/*CLIENT Boot ID*/
        // Set passive scanning on 1Mb PHY
        sc = sl_bt_scanner_set_mode(sl_bt_gap_1m_phy, 0); // Set to 1m phy and passive scanning
        //app_assert_status(sc);

        // Set scan interval and scan window
        sc = sl_bt_scanner_set_timing(sl_bt_gap_1m_phy, SCAN_INTERVAL, SCAN_WINDOW);
        //app_assert_status(sc);

        // Set the default connection parameters for subsequent connections
        sc = sl_bt_connection_set_default_parameters(60,  //Time = Value x 1.25 ms time : 20ms(default); value = 16 default interval min
                                                     60,  //Time = Value x 1.25 ms time : 50ms(default); value = 40 default interval max
                                                      3,
                                                     75, //Timeout default 1000ms; value = time/10ms = 100
                                                     0x0000, // min value for connection event length 0x0000(default)
                                                     0xffff); // max value for connection event length 0xffff(default)
        //app_assert_status(sc);
        // Start scanning
        sc = sl_bt_scanner_start(sl_bt_gap_1m_phy, sl_bt_scanner_discover_observation);
        app_assert_status_f(sc, "Failed to start discovery #1\n");

        LOG_INFO("Started scanning\r\n");
        displayInit();
        displayPrintf(DISPLAY_ROW_NAME, "Client");

        displayPrintf(DISPLAY_ROW_ASSIGNMENT, "A7");
        displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
        displayPrintf(DISPLAY_ROW_BTADDR, "%02X:%02X:%02X:%02X:%02X:%02X",
                                    ble_data.myAddress.addr[5],
                                    ble_data.myAddress.addr[4],
                                    ble_data.myAddress.addr[3],
                                    ble_data.myAddress.addr[2],
                                    ble_data.myAddress.addr[1],
                                    ble_data.myAddress.addr[0]);

              //conn_state = scanning;
#endif

        break;

#if DEVICE_IS_BLE_SERVER
          // No Action
#else

     case sl_bt_evt_scanner_scan_report_id:
     {
        bd_addr server_address  = SERVER_BT_ADDRESS;
        ble_data.servAddress = server_address;

        for(i = 0; i < 6; i++)
        {
           LOG_INFO("evt->data.evt_scanner_scan_report.address.addr[%d] = %x\r\n", i, evt->data.evt_scanner_scan_report.address.addr[i]);
           LOG_INFO("ble_data.servAddress.addr[%d] = %x\r\n", i, ble_data.servAddress.addr[i]);
        }

        for(i = 0; i < 6; i++)
        {
           if(evt->data.evt_scanner_scan_report.address.addr[i] == server_address.addr[i])
           {
              flag_addr++;
              LOG_INFO("evt->data.evt_scanner_scan_report.address.addr[%d] = %x\r\n",i, evt->data.evt_scanner_scan_report.address.addr[i]);
              LOG_INFO("ble_data.servAddress.addr[%d] = %x\r\n",i, ble_data.servAddress.addr[i]);
           }
        }

        if(flag_addr == 6)
        {
           //stop scanning for sometime
           sc = sl_bt_scanner_stop();
           //app_assert_status(sc);
           // and connect to that device
           sc = sl_bt_connection_open(evt->data.evt_scanner_scan_report.address,
                                      evt->data.evt_scanner_scan_report.address_type,
                                      sl_bt_gap_1m_phy,
                                      NULL);
           //app_assert_status(sc);
        }

        break;
     }
#endif

     case sl_bt_evt_connection_opened_id:
#if DEVICE_IS_BLE_SERVER
     LOG_INFO("Connection opened\n");
     displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");
     ble_data.connection_status = true;

     sc = sl_bt_advertiser_stop(advertising_set_handle);

     ble_data.openedConnection = evt->data.evt_connection_opened.connection;
     //app_assert_status(sc);
     if(sc != SL_STATUS_OK)
     {
        LOG_ERROR("sl_bt_advertiser_stop failed\n");
     }

     // Connection Set parameters
     sl_bt_connection_set_parameters(evt->data.evt_connection_opened.connection,
                                        60,  /*Time = Value x 1.25 ms  Required time = 75ms*/
                                        60,  /*Time = Value x 1.25 ms  Required time = 75ms*/
                                        3,
                                        75,
                                        0,
                                        0xffff);
#else

        ble_data.connectionHandle = evt->data.evt_connection_opened.connection;
        ble_data.ble_client_evt = Client_Connected_Event;
        LOG_INFO("ProcedureComplete : sending to state Client_EvtConnected\n");

        displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");
        displayPrintf(DISPLAY_ROW_BTADDR2, "%02X:%02X:%02X:%02X:%02X:%02X",
                      ble_data.servAddress.addr[5],
                      ble_data.servAddress.addr[4],
                      ble_data.servAddress.addr[3],
                      ble_data.servAddress.addr[2],
                      ble_data.servAddress.addr[1],
                      ble_data.servAddress.addr[0]);

#endif

        break;

      // Connection Closed event
      case sl_bt_evt_connection_closed_id:
#if DEVICE_IS_BLE_SERVER
        LOG_INFO("Connection closed\n");
        displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
        ble_data.connection_status = false;

        /* Clear the flag bits */
        ble_data.client_config_flag = 0;
        ble_data.status_flags = 0;

        // Begin Advertising
        sc = sl_bt_advertiser_start(
                                    advertising_set_handle,
                                    sl_bt_advertiser_general_discoverable,
                                    sl_bt_advertiser_connectable_scannable);
        //app_assert_status(sc);
        if(sc != SL_STATUS_OK)
        {
           LOG_ERROR("sl_bt_advertiser_start failed\n");
        }

        LOG_INFO("Started advertising\n");
        displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");
#else
        // start scanning again to find new devices
        ble_data.ble_client_evt = Client_Closed_Event;
        LOG_INFO("ProcedureComplete : sending to state Client_Closed\n");
        sc = sl_bt_scanner_start(sl_bt_gap_1m_phy, sl_bt_scanner_discover_observation);
        displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
        displayPrintf(DISPLAY_ROW_BTADDR2, " ");
        displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
#endif
        break;

      case sl_bt_evt_system_soft_timer_id:
      {
         displayUpdate();
      }
      break;

        // Connection Parameters event
      case sl_bt_evt_connection_parameters_id:
        /* Informational. Triggered whenever the connection parameters are changed and at any time a connection is established */
        //LOG_INFO("Connection Parameter -> Interval = %d\r\n",(int)((evt->data.evt_connection_parameters.interval)*1.25));
        //LOG_INFO("Connection Parameter -> Latency = %d\r\n",evt->data.evt_connection_parameters.latency);
        //LOG_INFO("Connection Parameter -> timeout = %d\r\n",(evt->data.evt_connection_parameters.timeout)*10);

        break;
#if DEVICE_IS_BLE_SERVER

      case sl_bt_evt_gatt_server_characteristic_status_id:
          {
              ble_data.client_config_flag = evt->data.evt_gatt_server_characteristic_status.client_config_flags;
              ble_data.status_flags = evt->data.evt_gatt_server_characteristic_status.status_flags;
          }


        break;


#else
        // ******************************************************
        // Events for Clients/Masters
        // ******************************************************
        // -------------------------------
        // This event is generated when a new service is discovered
       case sl_bt_evt_gatt_service_id:
       {
          // saving service handle for implementation in state chart
          if(evt->data.evt_gatt_service.uuid.data[0] == thermo_service[0] && evt->data.evt_gatt_service.uuid.data[1] == thermo_service[1])
          {
             ble_data.thermometer_service_handle = evt->data.evt_gatt_service.service;

             LOG_INFO("ProcedureComplete : sending to state Client_EvtService\r\n");
          }

          break;
       }

            // This event is generated when a new characteristic is discovered
       case sl_bt_evt_gatt_characteristic_id:

       {
          // saving characteristic handle for implementation in state chart
          if(evt->data.evt_gatt_characteristic.uuid.data[0] == thermo_char[0] && evt->data.evt_gatt_characteristic.uuid.data[1] == thermo_char[1])
          {
             ble_data.thermometer_characteristic_handle = evt->data.evt_gatt_characteristic.characteristic;
             LOG_INFO("ProcedureComplete : sending to state Client_EvtCharacteristic\n");
          }
          break;
       }

       case sl_bt_evt_gatt_procedure_completed_id:
       {
          if(evt->data.evt_gatt_procedure_completed.result == 0)
          {
             ble_data.ble_client_evt = Client_Procedure_Complete_Event;
             ble_data.procedure_completed = evt->data.evt_gatt_procedure_completed.connection;
             LOG_INFO("ProcedureComplete : sending to state Client_EvtProcedure_Complete\n");
          }
          break;
       }

       case sl_bt_evt_gatt_characteristic_value_id:
       {
          if((evt->data.evt_gatt_characteristic_value.att_opcode == sl_bt_gatt_handle_value_indication) &&
             (evt->data.evt_gatt_characteristic_value.characteristic == ble_data.thermometer_characteristic_handle))
          {
             ble_data.ble_client_evt = Client_Indicate_Event;

             temperature = &(evt->data.evt_gatt_characteristic_value.value.data[0]);
             int temp = FLOAT_TO_INT32(temperature);
             LOG_INFO("int_temp = %ld", temp);
             displayPrintf(DISPLAY_ROW_CONNECTION, "Handling Indications");
             displayPrintf(DISPLAY_ROW_TEMPVALUE, "Temp=%d", temp);
          }

          break;
       }
#endif
      // Default event handler.
      default:
        break;


    }
}

/**********************************************************************
 * report_data_ble provides temperature value
 *
 * Parameters:
 *   void
 *
 * Returns:
 *   void
 *********************************************************************/
void report_data_ble(float temp_c)
{
   sl_status_t sc;
   int temp;

   if ((ble_data.openedConnection != 0) && (ble_data.client_config_flag == 2) && (ble_data.status_flags != 0))
   {
      temp = (int)temp_c;

      uint8_t htm_temperature_buffer[5];
      uint8_t *p = htm_temperature_buffer;
      uint32_t htm_temperature_flt;
      uint8_t flags = 0x00;

      UINT8_TO_BITSTREAM(p, flags);
      htm_temperature_flt = UINT32_TO_FLOAT(temp_c*1000, -3);

      // Convert temperature to bitstream and place it in the htm_temperature_buffer
      UINT32_TO_BITSTREAM(p, htm_temperature_flt);

      sc = sl_bt_gatt_server_write_attribute_value(gattdb_temperature_measurement, // Attribute from gatt_db.h
                                                     0,                             // Offset set to 0
                                                     5,                             // Size of the data transmitted = array length of 5.
                                                     htm_temperature_buffer);       // Passing the address of the value
      //app_assert_status(sc);

      if(sc != SL_STATUS_OK)
      {
         LOG_ERROR("sl_bt_gatt_server_write_attribute_value failed\n");
      }

      sc = sl_bt_gatt_server_send_indication(ble_data.openedConnection,                  // characteristic
                                         gattdb_temperature_measurement,            // Attribute from gatt_db.h
                                                5,                                  // value_len
                                                htm_temperature_buffer);            // value to be sent

      //app_assert_status(sc);
      if(sc != SL_STATUS_OK)
      {
         LOG_ERROR("sl_bt_gatt_server_send_indication failed\n");
      }

      displayPrintf(DISPLAY_ROW_TEMPVALUE, "Temp = %d", temp);
   }
   else
   {
#if DEVICE_IS_BLE_SERVER
      displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
#else

#endif
   }
}

// Original code from Dan Walkes. I (Sluiter) fixed a sign extension bug with the mantissa.
// convert IEEE-11073 32-bit float to integer
int32_t FLOAT_TO_INT32(const uint8_t *value_start_little_endian)
{
   uint8_t signByte = 0;
   int32_t mantissa;

   // data format pointed at by value_start_little_endian is:
   // [0] = contains the flags byte
   // [3][2][1] = mantissa (24-bit 2’s complement)
   // [4] = exponent (8-bit 2’s complement)
   int8_t exponent = (int8_t)value_start_little_endian[4];

   // sign extend the mantissa value if the mantissa is negative
   if (value_start_little_endian[3] & 0x80)
   { // msb of [3] is the sign of the mantissa
      signByte = 0xFF;
   }

   mantissa = (int32_t) (value_start_little_endian[1] << 0) |
                        (value_start_little_endian[2] << 8) |
                        (value_start_little_endian[3] << 16) |
                        (signByte << 24) ;

   // value = 10^exponent * mantissa, pow() returns a double type
   return (int32_t) (pow(10, exponent) * mantissa);
} // gattFloat32ToInt
