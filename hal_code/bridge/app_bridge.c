/*
 * app_bridge.c
 *
 *  Created on: 2020-9-11
 *      Author: Weili.Hu
 */
#include <stdio.h>
#include <string.h>

#include "gap_api.h"
#include "gatt_api.h"
#include "ota.h"

#include "os_timer.h"
#include "os_mem.h"
#include "sys_utils.h"
//#include "button.h"
#include "jump_table.h"

//#include "user_task.h"
#include "driver_pwm.h"
#include "app_bridge.h"
#include "driver_gpio.h"
#include "co_math.h"

#include "driver_flash.h"
#include "driver_wdt.h"


#define APP_BRIDGE_UUID_OFFSET_ADDR     0x400

uint32 app_bridge_free_mem_get(void)
{

}

uint32 app_bridge_os_tick_get(void)
{
    return system_get_curr_time();
}

void app_bridge_os_delay_ms(uint32 count)
{
    co_delay_100us(count*10);
}

int32 app_bridge_gatt_write_data(uint8* p_data, uint32 length)
{
    return 0;
}

int32 app_bridge_ble_connect_check(void)
{
    return (int32)gap_get_connect_status(0);
}


int32 app_bridge_uart_send_data(uint8* p_data, uint32 size)
{
    uart_send(p_data, size);
    return 0;
}

void app_bridge_pwm_init(uint8 PWMindex, uint8 enableWork, uint16 PWMperiod, uint16 dutyCycle)
{
    system_set_port_mux(GPIO_PORT_A,GPIO_BIT_4,PORTA4_FUNC_PWM4);       //R
    gpio_set_dir(GPIO_PORT_A,GPIO_BIT_4,true);
    pwm_init(PWM_CHANNEL_4,20000,80);
    //pwm_update
    pwm_start(PWM_CHANNEL_4);
    pwm_update(PWM_CHANNEL_4,20000,99);     // R G B
}

void app_bridge_pwm_enable(uint8 PWMindex, uint8 enableWork)
{

}

void app_bridge_pwm_perid_set(uint8 PWMindex, uint16 PWMperiod)
{

}

void app_bridge_pwm_duty_cycle_set(uint8 PWMindex, uint16 dutyCycle)
{

}

void app_bridge_system_reboot(void)
{
    //wdt_init(WDT_ACT_RST_CHIP, 1);
    //wdt_start();
    //while(1);
    platform_reset_patch(0);
}

void app_bridge_flash_read_data(void* buffer, uint32 size, uint32 address)
{
    flash_read( address, size, (uint8 *) buffer);
}

int32 app_bridge_flash_write_data(void* buffer, uint32 size, uint32 address)
{

    flash_write(address, size,buffer);
    return 0;
}

void app_bridge_flash_erase_sector(uint32 address)
{
    flash_page_erase(address);
}


uint32 app_bridge_rand_number_get(uint32 seed)
{
    co_random_init(seed);
    return co_rand_word();
}

int32 app_bridge_ble_mac_get(uint8* ble_mac)
{
    if(ble_mac==NULL){
        return -1;
    }
    mac_addr_t addr;
    gap_address_get(&addr);
    memcpy(ble_mac,&addr.addr[0],6);
    return 0;
}

void app_bridge_gpio_config(int index, int dir)      // 0-7 bit          8-11 port
{
    gpio_set_dir(((index>>8)&0x03),(index&0xff),dir);
}

void app_bridge_gpio_output(int index, uint32 val)      //
{
    gpio_set_pin_value(((index>>8)&0x03),(index&0xff),val);
}

uint32 app_bridge_gpio_input(int index)         // 0-7 bit          8-11 port
{
    return (uint32)gpio_get_pin_value(((index>>8)&0x03),(index&0xff));
}

typedef os_timer_func_t timer_callback ;


uint32 app_bridge_timer_create(timer_callback cb, void* args)
{
    os_timer_t* ptimer = (os_timer_t*)os_malloc(sizeof(os_timer_t));

    if (NULL == ptimer)
    {
        co_printf("Malloc timer buffer failed.\r\n");
        return 0;
    }

    os_timer_init(ptimer, cb, args);

    return (uint32)ptimer;
}

void app_bridge_timer_start(uint32 handle, uint32 ms, uint8 b_repeat)
{
    os_timer_t* ptimer = (os_timer_t*)handle;

    os_timer_start(ptimer, ms, b_repeat);
}

void app_bridge_timer_stop(uint32 handle)
{
    os_timer_t* ptimer = (os_timer_t*)handle;

    os_timer_stop(ptimer);
}

void app_bridge_timer_destroy(uint32 handle)
{
    os_timer_t* ptimer = (os_timer_t*)handle;

    if (NULL != ptimer)
    {
        os_timer_destroy(ptimer);
        os_free(ptimer);
    }
}

void app_bridge_wdt_init(uint8 wdt_time)
{
    wdt_init(WDT_ACT_CALL_IRQ, wdt_time);
    wdt_start();
    wdt_feed();
}

void app_bridge_wdt_feed(void)
{
    wdt_feed();
}

void app_bridge_wdt_control(uint8 start)
{
    if(!start){     // 0 stop
        wdt_stop();
    }
    else{           //start
        wdt_start();
    }
}

uint8_t app_bridge_getotas_status(void)
{
    return app_otas_get_status();
}
