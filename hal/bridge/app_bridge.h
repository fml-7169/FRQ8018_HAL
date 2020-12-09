/*
 * app_bridge.h
 *
 *  Created on: 2020-9-11
 *      Author: Weili.Hu
 */

#ifndef APP_BRIDGE_H_
#define APP_BRIDGE_H_

#include "types.h"
#include "configs.h"
#include "driver_gpio.h"
#include "app_button.h"
#include "middle_ble.h"
#include "middle_uart.h"
#include "middle_led.h"
#include "middle_audio.h"
#include "middle_infrared.h"
#include "middle_config.h"
#include "middle_button.h"
#include "middle_time.h"
#include "govee_app.h"



#define APP_BRIDGE_CONFIG_LENGTH    GOVEE_CONFIG_DATA_MAX_LENGTH
#define APP_BRIDGE_BUTTON_COUNTS    GOVEE_BUTTON_COUNT

#if (GOVEE_DEVICE_TYPE == DEVICE_TYPE_RGB)
#define APP_BRIDGE_PWM_ENABLE
#elif (GOVEE_DEVICE_TYPE == DEVICE_TYPE_RGBWW)
#define APP_BRIDGE_PWM_ENABLE
#define APP_BRIDGE_PWM_WW_ENABLE
#elif (GOVEE_DEVICE_TYPE == DEVICE_TYPE_RGBIC)
#define APP_BRIDGE_SPI_ENABLE
#else
#error "GOVEE_DEVICE_TYPE is invalid."
#endif

#if GOVEE_DEVICE_WITH_INFRARED
#define APP_BRIDGE_IRDA_ENABLE
#endif

#if GOVEE_DEVICE_WITH_AUDIO_ADC
#define APP_BRIDGE_AUDIO_ADC_ENABLE
#endif


#if GOVEE_LOG_PRINT_ENABLE
#define APP_BRIDGE_PRINT_ENABLE
#endif

typedef struct
{
    uint32  pwm_perid;
    u_int8* p_ble_adv;
    u_int8* p_ble_resp;
    int32 ble_adv_len;
    int32 ble_resp_len;
    u_int8* p_gatt_uuid;
    u_int8* p_rx_uuid;
    u_int8* p_tx_uuid;
    u_int8 uuid_bit_size;
    read_callback p_ble_recv;
    event_callback p_ble_event;
    uart_callback p_uart_read;
    audio_callback p_audio_read;
    infrared_callback p_infrared_recv;
    timer_system p_timer;
    button_map_t* p_button_map;
    uint8 button_count;
    button_value p_button_value;
    button_callback p_button_event;
    get_env_callback p_env_get;
    start_callback p_app_start;
    loop_callback p_app_loop;
} bridge_param_t;

extern bridge_param_t gt_bridge_param;
extern int32 os_printf(const char *fmt, ...);
extern void uart_send (unsigned char *buff, unsigned int len);



#define app_bridge_printf(fmt, ...)    os_printf(fmt, ##__VA_ARGS__)

uint32 app_bridge_free_mem_get(void);
uint64 app_bridge_os_tick_get(void);
void app_bridge_os_delay_ms(uint32 conut);
int32 app_bridge_gatt_write_data(u_int8* p_data, uint32 length);
int32 app_bridge_ble_connect_check(void);
int32 app_bridge_param_set(govee_sys_t* pt_config);
int32 app_bridge_spi_send_data(uint8* p_data, uint32 size);
int32 app_bridge_uart_send_data(uint8* p_data, uint32 size);
void app_bridge_pwm_init(uint8 PWMindex, uint8 enableWork, uint16 PWMperiod, uint16 dutyCycle);
void app_bridge_pwm_enable(uint8 PWMindex, uint8 enableWork);
void app_bridge_pwm_perid_set(uint8 PWMindex, uint16 PWMperiod);
void app_bridge_pwm_duty_cycle_set(uint8 PWMindex, uint16 dutyCycle);
void* app_bridge_env_config_get(void);
void app_bridge_env_flash_write(void);
void app_bridge_system_reboot(void);
void app_bridge_volume_set(uint8 vol);
void app_bridge_flash_read_data(void* buffer, uint32 size, uint32 address);
int32 app_bridge_flash_write_data(void* buffer, uint32 size, uint32 address);
void app_bridge_flash_erase_sector(uint32 address);
uint32 app_bridge_flash_uuid_address(void);
int32 app_bridge_timer_start(void);
uint32 app_bridge_rand_number_get(uint32 seed);
int32 app_bridge_ble_mac_get(uint8* ble_mac);
void app_bridge_gpio_config(int index, int dir);
void app_bridge_gpio_output(int index, uint32 val);
uint32 app_bridge_gpio_input(int index);



#endif /* APP_BRIDGE_H_ */
