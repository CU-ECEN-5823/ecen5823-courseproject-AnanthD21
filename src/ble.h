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


void handle_ble_event(sl_bt_msg_t *evt);
void report_data_ble_humidity(uint16_t temp_c);
void report_data_ble(int temp_c);

#define UINT8_TO_BITSTREAM(p, n) { *(p)++ = (uint8_t)(n); }
#define UINT32_TO_BITSTREAM(p, n) { *(p)++ = (uint8_t)(n); *(p)++ = (uint8_t)((n) >> 8); \
*(p)++ = (uint8_t)((n) >> 16); *(p)++ = (uint8_t)((n) >> 24); }
#define UINT32_TO_FLOAT(m, e) (((uint32_t)(m) & 0x00FFFFFFU) | (uint32_t)((int32_t)(e) << 24))

typedef struct
{
   // values that are common to servers and clients
   bd_addr myAddress;
   uint8_t myAddressType;
   // values unique for server
   uint8_t advertisingSetHandle;
   bool connection_status;
   bool indication_flag;
   uint8_t connectionHandle;
   uint8_t openedConnection;
   uint8_t client_config_flag;
   uint8_t status_flags;
   // values unique for client
} ble_data_struct_t;

// function prototypes
ble_data_struct_t* provideBleDataPtr(void);

#endif
