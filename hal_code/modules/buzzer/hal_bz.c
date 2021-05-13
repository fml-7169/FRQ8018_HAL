

/* This driver is designed to control the usage of the ADC block between
 * the touchscreen and any other drivers that may need to use it, such as
 * the hwmon driver.
 *
 * Priority will be given to the touchscreen driver, but as this itself is
 * rate limited it should not starve other requests which are processed in
 * order that they are received.
 *
 * Each user registers to get a client block which uniquely identifies it
 * and stores information such as the necessary functions to callback when
 * action is required.
 */
 
#include <string.h>
#include <stdint.h>         // standard definition
#include <stdbool.h>        // boolean definition
#include <stddef.h>         // for NULL and size_t

#include "driver_pwm.h"
#include "driver_gpio.h"
#include "driver_system.h"

#include "os_mem.h"
#include "co_printf.h"

#include "hal_bz.h"
#include "hal_config.h"
#include "gpio.h"
#if 0
#define DEV_ERR(format,...) do { \
co_printf("[BUZZER] error:"); \
co_printf(format,##__VA_ARGS__); \
} while(0)
#define DEV_DB(format,...) do { \
    co_printf("[BUZZER] debug:"); \
    co_printf(format,##__VA_ARGS__); \
    } while(0)

#else
#define DEV_ERR(format,...)
#define DEV_DB(format,...)
#endif

int buzzer_start(struct bz_device_t *dev,\
                        int channel,\
                        unsigned int frequency,\
                        unsigned int high_duty)
{
    if(dev == NULL)
        return -1;
	system_set_port_mux(dev->bz_portPin_map.GPIOx,\
	                    dev->bz_portPin_map.GPIO_Pin_x,\
	                    PORTD0_FUNC_PWM0);//init buzzer port
	gpio_set_dir(dev->bz_portPin_map.GPIOx,\
                 dev->bz_portPin_map.GPIO_Pin_x,true);//set output
	pwm_init(channel,frequency,high_duty);//1000 80
	pwm_start(channel);
	return 0;
}



static void buzzer_stop(struct bz_device_t *dev,int channel){
    pwm_stop(channel);
    return;
}

static int buzzer_init(struct bz_device_t *dev,bz_portPin_map pportPin_map)
    {
        int i=0;
        if(dev ==NULL ){
            return -1;
        }
        DEV_DB("gpiox is 0x%x,gpio_pin is 0x%x\r\n",pportPin_map.GPIOx,\
                                                  pportPin_map.GPIO_Pin_x);
        dev->bz_portPin_map=pportPin_map;
    }


static int  buzzer_remove(bz_device_t *pdev)
{
    if(pdev){
        os_free(pdev);
    }
	return 0;
}

static int buzzer_open(const struct hw_module_t* module,\
                            char const* name,
                            struct hw_device_t** device)
{
    //malloc dev
    DEV_DB("buzzer open\r\n");
	bz_device_t *dev = os_malloc(sizeof(bz_device_t));
    memset(dev, 0, sizeof(*dev));
    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
    dev->common.close = (int (*)(struct hw_device_t*))buzzer_remove;
    dev->start=buzzer_start;
    dev->init=buzzer_init;
    dev->stop=buzzer_stop;
    *device = (struct hw_device_t*)dev;
    return 0;
}


static struct hw_module_methods_t bz_module_methods = {
    .open =  buzzer_open,
};


const struct hw_module_t hal_module_info_bz = {
    .tag = HARDWARE_MODULE_TAG,       // 规定的tag
    .version_major = 1,
    .version_minor = 0,
    .id = BUZZER_HARDWARE_MODULE_ID,  // 模块id
    .name = "Govee buzzer Module",     // 名称
    .author = "Govee",
    .methods = &bz_module_methods,// 方法
};

