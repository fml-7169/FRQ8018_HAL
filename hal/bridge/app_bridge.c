/*
 * app_bridge.c
 *
 *  Created on: 2020-9-11
 *      Author: Weili.Hu
 */
#include <string.h>
#include "timer.h"
#include "le_att.h"
#include "LE_gatt_server.h"
#include "includes.h"
#include "tra_hcit.h"
#include "app_debug.h"
#include "driver_pwm.h"
#include "BK3000_reg.h"
#include "driver_audio.h"
#include "sys_rand_num_gen.h"
#include "le_connection.h"

#include "app_bridge.h"


#define APP_BRIDGE_UUID_OFFSET_ADDR     0x400


extern u_int8 user_define_notify_handle_tx;
extern volatile uint32 FLASH_CHANGED_BT_ADDR;



bridge_param_t gt_bridge_param;

uint32_t app_bridge_free_mem_get(void)
{
    return memory_usage_show();
}

uint64 app_bridge_os_tick_get(void)
{
    return os_get_tick_counter();
}

void app_bridge_os_delay_ms(uint32 count)
{
    os_delay_ms(count);
}

int32 app_bridge_gatt_write_data(u_int8* p_data, uint32 length)
{
    if (LEconnection_LE_Connected())
    {
        return GATTserver_Characteristic_Write_Local_Value(user_define_notify_handle_tx, length, p_data);
    }

    return 0;
}

int32 app_bridge_ble_connect_check(void)
{
    return (int32)LEconnection_LE_Connected();
}

int32 app_bridge_param_set(govee_sys_t* pt_config)
{
    memset(&gt_bridge_param, 0, sizeof(bridge_param_t));

    gt_bridge_param.p_ble_adv = pt_config->ble_config.p_ble_adv;
    gt_bridge_param.p_ble_resp = pt_config->ble_config.p_ble_resp;
    gt_bridge_param.ble_adv_len = pt_config->ble_config.ble_adv_len;
    gt_bridge_param.ble_resp_len = pt_config->ble_config.ble_resp_len;
    gt_bridge_param.p_gatt_uuid = pt_config->ble_config.p_gatt_uuid;
    gt_bridge_param.p_rx_uuid = pt_config->ble_config.p_rx_uuid;
    gt_bridge_param.p_tx_uuid = pt_config->ble_config.p_tx_uuid;
    gt_bridge_param.uuid_bit_size = pt_config->ble_config.uuid_bit_size;
    gt_bridge_param.p_ble_recv = pt_config->ble_config.ble_recv;
    gt_bridge_param.p_ble_event = pt_config->ble_config.ble_event;

    gt_bridge_param.p_uart_read = pt_config->uart_config.uart_read;

    gt_bridge_param.p_audio_read = pt_config->audio_config.audio_read;

    gt_bridge_param.p_infrared_recv = pt_config->infrared_config.infrared_recv;

    gt_bridge_param.p_timer = pt_config->time_config.timer;

    gt_bridge_param.p_button_map = pt_config->button_config.pt_map;
    gt_bridge_param.button_count = pt_config->button_config.button_count;
    gt_bridge_param.p_button_value = pt_config->button_config.value;
    gt_bridge_param.p_button_event = pt_config->button_config.button_down;

    gt_bridge_param.p_env_get = pt_config->app_config.p_env_get;
    gt_bridge_param.p_app_start = pt_config->app_config.p_app_start;
    gt_bridge_param.p_app_loop = pt_config->app_config.p_app_loop;
    gt_bridge_param.pwm_perid = pt_config->app_config.pwm_perid;

    return 0;
}

int32 app_bridge_spi_send_data(uint8* p_data, uint32 size)
{
#ifdef APP_BRIDGE_SPI_ENABLE
    SPI_WriteBuffer_API(p_data, size);
#endif
    return 0;
}

int32 app_bridge_uart_send_data(uint8* p_data, uint32 size)
{
    uart_send(p_data, size);
    return 0;
}

void app_bridge_pwm_init(uint8 PWMindex, uint8 enableWork, uint16 PWMperiod, uint16 dutyCycle)
{
    PWMxinit(PWMindex, enableWork, PWMperiod, dutyCycle);
}

void app_bridge_pwm_enable(uint8 PWMindex, uint8 enableWork)
{
    PWMxEnableWork(PWMindex, enableWork);
}

void app_bridge_pwm_perid_set(uint8 PWMindex, uint16 PWMperiod)
{
    PWMxPeriodSet(PWMindex, PWMperiod);
}

void app_bridge_pwm_duty_cycle_set(uint8 PWMindex, uint16 dutyCycle)
{
#ifdef APP_BRIDGE_PWM_ENABLE
    PWMxdutyCycleSet(PWMindex, dutyCycle);
#endif
}

void* app_bridge_env_config_get(void)
{
    app_env_t* p_env = app_env_get_handle();

    return (void*)p_env->env_data.govee_data;
}

void app_bridge_env_flash_write(void)
{
    app_flash_write_env_data();
}

void app_bridge_system_reboot(void)
{
    BK3000_WDT_CONFIG = 0x5A0001;
    BK3000_WDT_CONFIG = 0xA50001;
}

void app_bridge_volume_set(uint8 vol)
{
    aud_mic_volume_set(vol);
}

void app_bridge_flash_read_data(void* buffer, uint32 size, uint32 address)
{
    flash_read_data((uint8 *)buffer, address, size);
}

int32 app_bridge_flash_write_data(void* buffer, uint32 size, uint32 address)
{
    uint8* data = NULL;
    int i = 0;

    data = jmalloc(size, M_ZERO);
    if (NULL == data)
    {
        app_bridge_printf("malloc buffer size %d failed.\r\n", size);
        return -1;
    }

    flash_read_data(data, address, size);
    for (i = 0; i < size; i++)
    {
        if (0xff != data[i])
        {
            app_bridge_printf("Write flash failed, select address have been writed.\r\n");
            return -1;
        }
    }
    jfree(data);

    flash_write_data(buffer, address, size);

    return 0;
}

void app_bridge_flash_erase_sector(uint32 address)
{
    flash_erase_sector(address);
}

uint32 app_bridge_flash_uuid_address(void)
{
    if (FLASH_CHANGED_BT_ADDR == 0x0)
    {
        return 0x0;
    }

    return FLASH_CHANGED_BT_ADDR + APP_BRIDGE_UUID_OFFSET_ADDR;
}

int32 app_bridge_timer_start(void)
{
	timer_pt1_start(3);

    return 0;
}

uint32 app_bridge_rand_number_get(uint32 seed)
{
    SYSrand_Seed_Rand(seed);
    return SYSrand_Get_Rand();
}

int32 app_bridge_ble_mac_get(uint8* ble_mac)
{
    app_env_handle_t env_h = app_env_get_handle();
    if (!ble_mac)
    {
        os_printf("Invalid parameters.\r\n");
        return -1;
    }
    memcpy(ble_mac, env_h->env_cfg.bt_para.device_addr.b, 6);

    return 0;
}

void app_bridge_gpio_config(int index, int dir)
{
    gpio_config(index, dir);
}

void app_bridge_gpio_output(int index, uint32 val)
{
    gpio_output(index, val);
}

uint32 app_bridge_gpio_input(int index)
{
    return gpio_input(index);
}
