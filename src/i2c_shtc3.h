/*
 * i2c_shtc3.h
 *
 *  Created on: Apr 11, 2022
 *      Author: Abhishek suryawanshi, absu8154@colorado.edu
 */

#ifndef SRC_I2C_SHTC3_H_
#define SRC_I2C_SHTC3_H_

#include "sl_i2cspm.h"

#include "app.h"
#include "timers.h"

void write_to_shtc3(void);

void read_from_shtc3();

float obtainHumidityValues();

void read_humidity_from_shtc3();

#endif /* SRC_I2C_SHTC3_H_ */
