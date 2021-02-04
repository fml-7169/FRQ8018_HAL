/*
 * app_bridge.h
 *
 *  Created on: 2020-9-11
 *      Author: Weili.Hu
 */

#ifndef APP_BRIDGE_H_
#define APP_BRIDGE_H_

#include "hal_types.h"
#include "driver_gpio.h"
#include "middle_ble.h"
#include "middle_audio.h"


extern int32 co_printf(const char *fmt, ...);
extern void uart_send (unsigned char *buff, unsigned int len);

#define app_bridge_printf(fmt, ...)    co_printf(fmt, ##__VA_ARGS__)


uint32 app_bridge_os_tick_get(void);
void app_bridge_os_delay_ms(uint32 conut);
int32 app_bridge_gatt_write_data(uint8* p_data, uint32 length);
int32 app_bridge_ble_connect_check(void);
int32 app_bridge_uart_send_data(uint8* p_data, uint32 size);

void app_bridge_pwm_init(uint8 PWMindex, uint8 enableWork, uint16 PWMperiod, uint16 dutyCycle);
void app_bridge_pwm_enable(uint8 PWMindex, uint8 enableWork);
void app_bridge_pwm_perid_set(uint8 PWMindex, uint16 PWMperiod);
void app_bridge_pwm_duty_cycle_set(uint8 PWMindex, uint16 dutyCycle);
void app_bridge_system_reboot(void);
void app_bridge_flash_read_data(void* buffer, uint32 size, uint32 address);
int32 app_bridge_flash_write_data(void* buffer, uint32 size, uint32 address);
void app_bridge_flash_erase_sector(uint32 address);
uint32 app_bridge_flash_uuid_address(void);
uint32 app_bridge_rand_number_get(uint32 seed);
int32 app_bridge_ble_mac_get(uint8* ble_mac);
void app_bridge_gpio_config(int index, int dir);
void app_bridge_gpio_output(int index, uint32 val);
uint32 app_bridge_gpio_input(int index);

void app_bridge_wdt_init(uint8 wdt_time);
void app_bridge_wdt_feed(void);
void app_bridge_wdt_control(uint8 start);
uint8_t app_bridge_getotas_status(void);

#endif /* APP_BRIDGE_H_ */
