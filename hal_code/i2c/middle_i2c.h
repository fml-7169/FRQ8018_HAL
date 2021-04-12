/*
 * middle_uart.h
 *
 *  Created on: 2020-9-17
 *      Author: Weili.Hu
 */

#ifndef MIDDLE_I2C_H_
#define MIDDLE_I2C_H_

#include "hal_types.h"

#define     _I2C_USE_HARD_      0

void mid_i2c_port_set(Pin_Map scl_pin,Pin_Map sda_pin);
int32 mid_i2c_init(uint16 rate);
void mid_i2c_write(uint8 addr, uint8 reg, uint8* buffer, uint32 len);

#endif /* MIDDLE_UART_H_ */
