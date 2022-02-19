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

#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;
bool connect_status = false;

static uint8_t client_config_flag = 0;
static uint8_t status_flags = 0;
static uint8_t openedConnection = 0;

static bool isTempAvail = 0;

// BLE private data
ble_data_struct_t ble_data;

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
   bd_addr address;
   uint8_t address_type;
   uint8_t sys_id[8];

   // Handle stack events
   switch (SL_BT_MSG_ID(evt->header))
   {
      // ******************************************************
      // Events common to both Servers and Clients
      // ******************************************************

      // system_boot Event
      case sl_bt_evt_system_boot_id:

        // obtain unique ID from bluetooth Address.
        sc = sl_bt_system_get_identity_address(&address, &address_type);
        app_assert_status(sc);

        //reverse unique ID to get System ID.
        sys_id[0] = address.addr[5];
        sys_id[1] = address.addr[4];
        sys_id[2] = address.addr[3];
        sys_id[3] = 0xFF;
        sys_id[4] = 0xFE;
        sys_id[5] = address.addr[2];
        sys_id[6] = address.addr[1];
        sys_id[7] = address.addr[0];

        sc = sl_bt_gatt_server_write_attribute_value(gattdb_system_id,
                                                     0,
                                                     sizeof(sys_id),
                                                     sys_id);
        app_assert_status(sc);

        // Create advertising set.
        sc = sl_bt_advertiser_create_set(&advertising_set_handle);
        app_assert_status(sc);

        // Set advertising interval to 250ms as mentioned in A5 documents
        sc = sl_bt_advertiser_set_timing(
                                         advertising_set_handle, // advertising set handle
                                         400, // minimum advertising interval (milliseconds * 1.6)
                                         400, // maximum advertising interval (milliseconds * 1.6)
                                           0, // advertising duration
                                           0);
        app_assert_status(sc);

        // Start general advertising and enable connections.
        sc = sl_bt_advertiser_start(
                                    advertising_set_handle,
                                    sl_bt_advertiser_general_discoverable,
                                    sl_bt_advertiser_connectable_scannable);
        app_assert_status(sc);

        LOG_INFO("advertising Started\n");
        break;

      // This event indicates that a new connection was opened.
      case sl_bt_evt_connection_opened_id:

        LOG_INFO("Connection has been opened\n");
        connect_status = true;

        sc = sl_bt_advertiser_stop(advertising_set_handle);

        app_assert_status(sc);

        sl_bt_connection_set_parameters(evt->data.evt_connection_opened.connection,
                                        60,  /*Time = Value x 1.25 ms  Required time = 75ms*/
                                        60,  /*Time = Value x 1.25 ms  Required time = 75ms*/
                                        3,   /*latency = Total latency time/ connection interval - 1 : 300ms/75ms - 1 = 3*/
                                        75,  /*timeout = (1+latency) * (max_connection_interval)*2 : (1+4)*75*2 = 750ms; value = 750/10 = 75ms */
                                        0,
                                        0xffff);
        break;

      // Connection Closed event
      case sl_bt_evt_connection_closed_id:

        LOG_INFO("Connection has closed\n");
        connect_status = false;

        /* Clear the flag bits */
        client_config_flag = 0;
        status_flags = 0;

        // start advertising
        sc = sl_bt_advertiser_start(
                                    advertising_set_handle,
                                    sl_bt_advertiser_general_discoverable,
                                    sl_bt_advertiser_connectable_scannable);

        app_assert_status(sc);
        LOG_INFO("Started advertising\n");
        break;

      // Connection Parameters event
      case sl_bt_evt_connection_parameters_id:

        /* Informational. Triggered whenever the connection parameters are changed and at any time a connection is established */
        LOG_INFO("Connection Parameter -> Interval = %d\r\n",(int)((evt->data.evt_connection_parameters.interval)*1.25));
        LOG_INFO("Connection Parameter -> Latency = %d\r\n",evt->data.evt_connection_parameters.latency);
        LOG_INFO("Connection Parameter -> timeout = %d\r\n",(evt->data.evt_connection_parameters.timeout)*10);

        break;

        // ******************************************************
        // Events for Servers/Slaves
        // ******************************************************
        // GATT server characteristic status event
      case sl_bt_evt_gatt_server_characteristic_status_id:
        /* Indicates either that a local Client Characteristic Configuration descriptor (CCCD)
         * was changed by the remote GATT client,
         * or that a confirmation from the remote GATT client was received
         * upon a successful reception of the indication
         */
        client_config_flag = evt->data.evt_gatt_server_characteristic_status.client_config_flags;
        status_flags       = evt->data.evt_gatt_server_characteristic_status.status_flags;
        openedConnection   = evt->data.evt_gatt_server_characteristic_status.connection;

        if ((client_config_flag == 2) && (openedConnection != 0))
        {
            isTempAvail = 1;
        }
        else
        {
            isTempAvail = 0;
        }
        break;

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

   if(isTempAvail == 1)
   {
      LOG_INFO("inside isTempAvail\n\r");

      /*Temperature transfer using Gatt server write attribute*/
      uint8_t htm_temperature_buffer[5];
      uint8_t *p = htm_temperature_buffer;
      uint32_t htm_temperature_flt;
      uint8_t flags = 0x00;

      UINT8_TO_BITSTREAM(p, flags);
      htm_temperature_flt = UINT32_TO_FLOAT(temp_c*1000, -3);

      // Convert temperature to bitstream and place it in the htm_temperature_buffer
      UINT32_TO_BITSTREAM(p, htm_temperature_flt);
      sc = sl_bt_gatt_server_write_attribute_value (gattdb_temperature_measurement, // Attribute from gatt_db.h
                                                    0,                              // Offset set to 0
                                                    5,                              // Size of the data transmitted = array length of 5.
                                                    htm_temperature_buffer);        // Passing the address of the value
      app_assert_status(sc);

      sc = sl_bt_gatt_server_send_indication(openedConnection,                  // characteristic
                                             gattdb_temperature_measurement,    // Attribute from gatt_db.h
                                             5,                                 // value_len
                                             htm_temperature_buffer);           // value to be sent

      printf("opened connection : %d\t client_config_flag = %d\t status_flags = %d",
              openedConnection, client_config_flag, status_flags);
      app_assert_status(sc);
   }
}

/*********************************end of file*************************************/
