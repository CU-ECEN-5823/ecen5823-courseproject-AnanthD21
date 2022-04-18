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
#include "lcd.h"
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

// The advertising set handle allocated from Bluetooth stack.
//static uint8_t advertising_set_handle = 0xff;
/*bool connect_status = false;

static uint8_t client_config_flag = 0;
static uint8_t status_flags = 0;
static uint8_t openedConnection = 0;*/

static bool isVOCAvail = 0;
static bool isHumidityAvail = 0;
static bool in_flight = false;

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
   //bd_addr address;
   //uint8_t address_type;
   uint8_t sys_id[8];

   ble_data_struct_t* data = provideBleDataPtr();

   // Handle stack events
   switch (SL_BT_MSG_ID(evt->header))
   {
      // ******************************************************
      // Events common to both Servers and Clients
      // ******************************************************

      // system_boot Event
      case sl_bt_evt_system_boot_id:

        displayInit();
        // obtain unique ID from bluetooth Address.
        sc = sl_bt_system_get_identity_address(&data->myAddress, &data->myAddressType);
        app_assert_status(sc);

        displayPrintf(DISPLAY_ROW_NAME, "Server");
        displayPrintf(DISPLAY_ROW_BTADDR, "%02X:%02X:%02X:%02X:%02X:%02X",
                                           data->myAddress.addr[5],
                                           data->myAddress.addr[4],
                                           data->myAddress.addr[3],
                                           data->myAddress.addr[2],
                                           data->myAddress.addr[1],
                                           data->myAddress.addr[0]);
        displayPrintf(DISPLAY_ROW_ASSIGNMENT, "A6");

        //reverse unique ID to get System ID.
        sys_id[0] = data->myAddress.addr[5];
        sys_id[1] = data->myAddress.addr[4];
        sys_id[2] = data->myAddress.addr[3];
        sys_id[3] = 0xFF;
        sys_id[4] = 0xFE;
        sys_id[5] = data->myAddress.addr[2];
        sys_id[6] = data->myAddress.addr[1];
        sys_id[7] = data->myAddress.addr[0];

        sc = sl_bt_gatt_server_write_attribute_value(gattdb_system_id,
                                                     0,
                                                     sizeof(sys_id),
                                                     sys_id);
        app_assert_status(sc);

        // Create advertising set.
        sc = sl_bt_advertiser_create_set(&data->advertisingSetHandle);
        app_assert_status(sc);

        // Set advertising interval to 250ms as mentioned in A5 documents
        sc = sl_bt_advertiser_set_timing(
                                         data->advertisingSetHandle, // advertising set handle
                                         400, // minimum advertising interval (milliseconds * 1.6)
                                         400, // maximum advertising interval (milliseconds * 1.6)
                                           0, // advertising duration
                                           0);
        app_assert_status(sc);

        // Start general advertising and enable connections.
        sc = sl_bt_advertiser_start(
                                    data->advertisingSetHandle,
                                    sl_bt_advertiser_general_discoverable,
                                    sl_bt_advertiser_connectable_scannable);
        app_assert_status(sc);

        LOG_INFO("advertising Started\n");

        displayPrintf(DISPLAY_ROW_CONNECTION, "Adverstising");
        break;

      // This event indicates that a new connection was opened.
      case sl_bt_evt_connection_opened_id:

        LOG_INFO("Connection has been opened\n");
        displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");
        data->connection_status = true;

        sc = sl_bt_advertiser_stop(data->advertisingSetHandle);

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

        displayPrintf(DISPLAY_ROW_TEMPVALUE,"");
        LOG_INFO("Connection has closed\n");
        data->connection_status = false;

        /* Clear the flag bits */
        data->client_config_flag = 0;
        data->status_flags = 0;

        // start advertising
        sc = sl_bt_advertiser_start(
                                    data->advertisingSetHandle,
                                    sl_bt_advertiser_general_discoverable,
                                    sl_bt_advertiser_connectable_scannable);

        app_assert_status(sc);
        LOG_INFO("Started advertising\n");
        displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");
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
        data->client_config_flag = evt->data.evt_gatt_server_characteristic_status.client_config_flags;
        data->status_flags       = evt->data.evt_gatt_server_characteristic_status.status_flags;
        data->openedConnection   = evt->data.evt_gatt_server_characteristic_status.connection;

        if(evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_VOC_index)
        {
           if ((data->client_config_flag == sl_bt_gatt_indication) &&
               (data->openedConnection != sl_bt_gatt_disable))
           {
               isVOCAvail = 1;
           }
           else
           {
               isVOCAvail = 0;
           }
        }
        else if(evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_Relative_Humidity)
        {
            if ((data->client_config_flag == sl_bt_gatt_indication) &&
                (data->openedConnection != sl_bt_gatt_disable))
            {
                isHumidityAvail = 1;
            }
            else
            {
                isHumidityAvail = 0;
            }
        }
        break;

      case sl_bt_evt_system_soft_timer_id:
        displayUpdate();
        break;

      // Default event handler.
      default:
        break;


    }
}

void report_data_ble(int temp_c)
{
   sl_status_t sc;

   if((isVOCAvail == 1))
   {
      LOG_INFO("inside new report data ble\n\r");

      /*Temperature transfer using Gatt server write attribute*/
      /*uint8_t htm_temperature_buffer[5];
      uint8_t *p = htm_temperature_buffer;
      uint32_t htm_temperature_flt;
      uint8_t flags = 0x00;

      UINT8_TO_BITSTREAM(p, flags);
      htm_temperature_flt = UINT32_TO_FLOAT(temp_c*1000, -3);

      // Convert temperature to bitstream and place it in the htm_temperature_buffer
      UINT32_TO_BITSTREAM(p, htm_temperature_flt);*/
      sc = sl_bt_gatt_server_write_attribute_value (gattdb_VOC_index, // Attribute from gatt_db.h
                                                    0,                              // Offset set to 0
                                                    4,                              // Size of the data transmitted = array length of 5.
                                                    &temp_c);        // Passing the address of the value
      app_assert_status(sc);

      sc = sl_bt_gatt_server_send_indication(ble_data.openedConnection,                  // characteristic
                                             gattdb_VOC_index,    // Attribute from gatt_db.h
                                             4,                                 // value_len
                                             &temp_c);           // value to be sent

      printf("VOC: opened connection : %d\t client_config_flag = %d\t status_flags = %d\n",
             ble_data.openedConnection, ble_data.client_config_flag, ble_data.status_flags);
      app_assert_status(sc);
      displayPrintf(DISPLAY_ROW_TEMPVALUE, "VOC = %d", (int)temp_c);

      isVOCAvail = 0;
      in_flight = true;
   }
   else
   {
      displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
   }
}

void report_data_ble_humidity(uint16_t temp_c)
{
   sl_status_t sc;

   if((isHumidityAvail == 1))
   {
      LOG_INFO("inside new report data ble\n\r");

      /*Temperature transfer using Gatt server write attribute*/
      /*uint8_t htm_temperature_buffer[5];
      uint8_t *p = htm_temperature_buffer;
      uint32_t htm_temperature_flt;
      uint8_t flags = 0x00;

      UINT8_TO_BITSTREAM(p, flags);
      htm_temperature_flt = UINT32_TO_FLOAT(temp_c*1000, -3);

      // Convert temperature to bitstream and place it in the htm_temperature_buffer
      UINT32_TO_BITSTREAM(p, htm_temperature_flt);*/
      //temp_c = 1;
      sc = sl_bt_gatt_server_write_attribute_value (gattdb_Relative_Humidity, // Attribute from gatt_db.h
                                                    0,                              // Offset set to 0
                                                    2,                              // Size of the data transmitted = array length of 5.
                                                    &temp_c);        // Passing the address of the value
      app_assert_status(sc);

      sc = sl_bt_gatt_server_send_indication(ble_data.openedConnection,                  // characteristic
                                             gattdb_Relative_Humidity,    // Attribute from gatt_db.h
                                             2,                                 // value_len
                                             &temp_c);           // value to be sent

      printf("VOC: opened connection : %d\t client_config_flag = %d\t status_flags = %d\n",
             ble_data.openedConnection, ble_data.client_config_flag, ble_data.status_flags);
      app_assert_status(sc);
      displayPrintf(DISPLAY_ROW_TEMPVALUE, "humidity = %d", (int)temp_c);

      isHumidityAvail = 0;
      in_flight = true;
   }
   else
   {
      displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
   }
}

/*********************************end of file*************************************/
