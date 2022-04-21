/***********************************************************************
* @file ble.h
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

#ifndef _BLE_H

#define _BLE_H

#include "sl_bt_api.h"

#include "gatt_db.h"
#include "app_log.h"
#include "app_assert.h"

#include "stdbool.h"
#include "lcd.h"

#include "math.h"

void handle_ble_event(sl_bt_msg_t *evt);
void report_data_ble_sgp40_values(int );
void report_data_ble_shtc3_values(int );

#define UINT8_TO_BITSTREAM(p, n) { *(p)++ = (uint8_t)(n); }
#define UINT32_TO_BITSTREAM(p, n) { *(p)++ = (uint8_t)(n); *(p)++ = (uint8_t)((n) >> 8); \
*(p)++ = (uint8_t)((n) >> 16); *(p)++ = (uint8_t)((n) >> 24); }
#define UINT32_TO_FLOAT(m, e) (((uint32_t)(m) & 0x00FFFFFFU) | (uint32_t)((int32_t)(e) << 24))

#define SCAN_INTERVAL  80//Time : 50ms
#define SCAN_WINDOW  40//Time : 25ms

int32_t FLOAT_TO_INT32(const uint8_t *value_start_little_endian);

typedef enum {
  Client_default = 0,
  Client_Connected_Event,
  Client_Procedure_Complete_Event,
  Client_Indicate_Event,
  Client_Closed_Event
}Client_Evt;


typedef struct
{
  bd_addr myAddress;
  uint8_t myAddressType;
  bool connection_status;
  uint8_t connectionHandle;
  uint8_t openedConnection;
  uint8_t client_config_flag;
  uint8_t status_flags;

  bd_addr servAddress;

  uint32_t thermometer_service_handle;
  uint16_t thermometer_characteristic_handle;

  uint32_t voc_service_handle;
  uint16_t voc_characteristic_handle;

  uint32_t hum_service_handle;
  uint16_t hum_characteristic_handle;

  bool inflight_indication;

  Client_Evt ble_client_evt;
  uint8_t procedure_completed;
} ble_data_struct_t;

// function prototypes
ble_data_struct_t* provideBleDataPtr(void);

#endif
