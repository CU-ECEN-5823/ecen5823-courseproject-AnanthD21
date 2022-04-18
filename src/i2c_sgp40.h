/***********************************************************************
* @file i2c.h
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
*
*
* @copyright All rights reserved. Distribution allowed only for the
* use of assignment grading. Use of code excerpts allowed at the
* discretion of author. Contact for permission.
*/

#ifndef SRC_I2C_SGP40_H_
#define SRC_I2C_SGP40_H_

#include "sl_i2cspm.h"

#include "app.h"
#include "timers.h"

void I2Cinit();

void write_to_sgp40(void);

void read_from_sgp40();

int obtainVOCRawValues();


#endif /* SRC_I2C_SGP40_H_ */
