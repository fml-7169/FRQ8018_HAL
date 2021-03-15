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
typedef struct _I2cPin_Map {
  uint32_t GPIOx;
  uint32_t GPIO_Pin_x;
  uint32_t GPIO_Func;
} I2cPin_Map;


void mid_i2c_port_set(I2cPin_Map scl_pin,I2cPin_Map sda_pin);
int32 mid_i2c_init(uint16 rate);
void mid_i2c_write(uint8 addr, uint8 reg, uint8* buffer, uint32 len);

#endif /* MIDDLE_UART_H_ */
