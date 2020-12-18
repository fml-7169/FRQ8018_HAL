/*
 * =====================================================================================
 *
 *       Filename:  light.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/03/2020 02:31:15 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include "hardware.h"

#include <string.h>
#include <errno.h>
#include <limits.h>

#include "driver_pmu.h"
#include "driver_iomux.h"
#include "sys_utils.h"
#include "lights.h"
#include "os_mem.h"
#include "hal_config.h"
#if 0
#define LOGD(...)
#define LOGE(...)
#define LOGI(...)
#define LOGV(...)
#else
#define LOGD(format,...) do { \
    co_printf("[LIGHTS] debug:"); \
    co_printf(format,##__VA_ARGS__); \
} while(0)

#define LOGE(format,...) do { \
    co_printf("[LIGHTS] error:"); \
    co_printf(format,##__VA_ARGS__); \
} while(0)

#endif
struct led_prop {
    const char *filename;
    int fd;
};
// 3个操作的集合
struct led {
    struct led_prop brightness;
    struct led_prop flash_on_ms;
    struct led_prop flash_off_ms;
};
// 系统拥有的light
enum {
    RED_LED,
    GREEN_LED,
    BLUE_LED,
    LCD_BACKLIGHT,
    BUTTONS_LED,
    NUM_LEDS,
};

static int is_battery_light_on = 0;
/**
 * device methods
 */
// 初始化单个节点

// 关闭节点
static void close_prop(struct led_prop *prop)
{
    #if 0
    int fd;
 
    if (prop->fd > 0)
        close(prop->fd);
    #endif
    return;
}
static int write_int(struct led_prop *prop, int value)
{
    char buffer[20];
    int bytes;
    int amt;
 #if 0
    if (prop->fd < 0)
        return 0;
 
    LOGV("%s %s: 0x%x\n", __FUNCTION__, prop->filename, value);
 
    bytes = snprintf(buffer, sizeof(buffer), "%d\n", value);
    while (bytes > 0) {
        amt = write(prop->fd, buffer, bytes);
        if (amt < 0) {
            if (errno == EINTR)
                continue;
            return -errno;
        }
        bytes -= amt;
    }
 #endif
    return 0;
}
// 用于呼吸灯控制
static int set_speaker_light(struct light_device_t* dev,
        struct light_state_t const* state)
{
    int len;
    int value;
    unsigned int colorRGB;
    unsigned int colorR;
    unsigned int colorG;
    unsigned int colorB;
 
    colorRGB = state->color & 0xFFFFFF;
    if (colorRGB == 0xFFFFFF) { /* white */
        colorRGB = 0x0;
    }
    colorR = (colorRGB >> 16) & 0x00ff;
    colorG = (colorRGB >> 8) & 0x00ff;
    colorB = colorRGB & 0x00ff;
 
    switch (state->flashMode) {
        case LIGHT_FLASH_TIMED:
        case LIGHT_FLASH_HARDWARE:
            break;
        case LIGHT_FLASH_NONE:
            break;
        default:
            LOGE("set_led_state colorRGB=%08X, unknown mode %d\n",
                  colorRGB, state->flashMode);
    }
    return 0;
}
// rgb数据转换成背光值
static int rgb_to_brightness(struct light_state_t const* state)
{
    int color = state->color & 0x00ffffff;
    return ((77*((color>>16)&0x00ff))            
            + (150*((color>>8)&0x00ff)) + (29*(color&0x00ff))) >> 8;
}
// 设置按键灯亮度
static int set_light_buttons(struct light_device_t* dev,
        struct light_state_t const* state)
{
	return 0;
}
// 设置lcd背光亮度
static int set_light_backlight(struct light_device_t* dev,
        struct light_state_t const* state)
{
    if(dev == NULL || state == NULL)
        return -1;
    if(!state->color){ //off
        pmu_set_gpio_value(dev->plight_portPin_map->GPIOx,\
                        BIT(dev->plight_portPin_map->GPIO_Pin_x), 0);
    }else{ //on
        pmu_set_gpio_value(dev->plight_portPin_map->GPIOx,\
                        BIT(dev->plight_portPin_map->GPIO_Pin_x), 1);
    }
    return 0;
}

// 设置lcd背光亮度
static int set_light_rgbww(struct light_device_t* dev,
        struct light_state_t const* state)
{
    if(dev == NULL || state == NULL)
        return -1;
    if(!state->color){ //off
        pmu_set_gpio_value(dev->plight_portPin_map->GPIOx,\
                        BIT(dev->plight_portPin_map->GPIO_Pin_x), 0);
    }else{ //on
        pmu_set_gpio_value(dev->plight_portPin_map->GPIOx,\
                        BIT(dev->plight_portPin_map->GPIO_Pin_x), 1);
    }
    return 0;
}

// 设置低电light
static int set_light_battery(struct light_device_t* dev,
        struct light_state_t const* state)
{
    int colorRGB = state->color & 0xFFFFFF;
    LOGD("%s flashMode %d, flashOnMs %d, flashOffMs %d, color=0x%08x\n",
            __FUNCTION__, state->flashMode, state->flashOnMS, state->flashOffMS, state->color);
    if (colorRGB != 0x0 && colorRGB != 0xFFFFFF) {
        is_battery_light_on = 1;
    } else  {
        is_battery_light_on = 0;
    }
    return set_speaker_light(dev, state);
}
// 设置通知light
static int set_light_notifications(struct light_device_t* dev,
        struct light_state_t const* state)
{
    LOGD("%s flashMode %d, flashOnMs %d, flashOffMs %d, color=0x%08x\n",
            __FUNCTION__, state->flashMode, state->flashOnMS, state->flashOffMS, state->color);
    if (!is_battery_light_on) {
        set_speaker_light(dev, state);
    }
    return 0;
}
// 设置提醒light
static int set_light_attention(struct light_device_t* dev,
        struct light_state_t const* state)
{
    LOGD("%s flashMode %d, flashOnMs %d, flashOffMs %d, color=0x%08x\n",
            __FUNCTION__, state->flashMode, state->flashOnMS, state->flashOffMS, state->color);
    if (!is_battery_light_on) {
        set_speaker_light(dev, state);
    }
    return 0;
}
static int init_light_backlight(struct light_device_t *dev,light_portPin_map *pportPin_map,int map_count)
{
    int i=0;
    if(dev ==NULL || pportPin_map == NULL){
        return -1;
    }
    LOGD("name is %s,gpiox is 0x%x,gpio_pin is 0x%x\r\n",pportPin_map[0].name,\
                                                        pportPin_map[0].GPIOx,\
                                                        pportPin_map[0].GPIO_Pin_x);
    dev->plight_portPin_map=os_calloc(1, sizeof(light_portPin_map));
    for(i=0;i<map_count;i++){
        if (0 == strcmp(LIGHT_ID_BACKLIGHT, pportPin_map[i].name)) {
            dev->plight_portPin_map->GPIOx=pportPin_map[i].GPIOx;            
            dev->plight_portPin_map->GPIO_Pin_x=pportPin_map[i].GPIO_Pin_x;
            pmu_set_port_mux(dev->plight_portPin_map->GPIOx,\
                            dev->plight_portPin_map->GPIO_Pin_x,\
                            PMU_PORT_MUX_GPIO);
            pmu_set_pin_to_PMU(dev->plight_portPin_map->GPIOx,\
                            BIT(dev->plight_portPin_map->GPIO_Pin_x) );
            pmu_set_pin_dir(dev->plight_portPin_map->GPIOx,\
                            BIT(dev->plight_portPin_map->GPIO_Pin_x), \
                            GPIO_DIR_OUT);  
            
            LOGD("gpiox is 0x%x,gpio_pin is 0x%x\r\n",dev->plight_portPin_map->GPIOx,\
                                                    dev->plight_portPin_map->GPIO_Pin_x);
        }
    }
}

static int init_light_rgbww(struct light_device_t *dev,light_portPin_map *pportPin_map,int map_count)
{
    int i=0;
    if(dev ==NULL || pportPin_map == NULL){
        return -1;
    }
    LOGD("name is %s,gpiox is 0x%x,gpio_pin is 0x%x\r\n",pportPin_map[0].name,\
                                                        pportPin_map[0].GPIOx,\
                                                        pportPin_map[0].GPIO_Pin_x);
    dev->plight_portPin_map=os_calloc(1, sizeof(light_portPin_map));
    for(i=0;i<map_count;i++){
        if (0 == strcmp(LIGHT_ID_BACKLIGHT, pportPin_map[i].name)) {
            dev->plight_portPin_map->GPIOx=pportPin_map[i].GPIOx;            
            dev->plight_portPin_map->GPIO_Pin_x=pportPin_map[i].GPIO_Pin_x;
            pmu_set_port_mux(dev->plight_portPin_map->GPIOx,\
                            dev->plight_portPin_map->GPIO_Pin_x,\
                            PMU_PORT_MUX_GPIO);
            pmu_set_pin_to_PMU(dev->plight_portPin_map->GPIOx,\
                            BIT(dev->plight_portPin_map->GPIO_Pin_x) );
            pmu_set_pin_dir(dev->plight_portPin_map->GPIOx,\
                            BIT(dev->plight_portPin_map->GPIO_Pin_x), \
                            GPIO_DIR_OUT);  
            
            LOGD("gpiox is 0x%x,gpio_pin is 0x%x\r\n",dev->plight_portPin_map->GPIOx,\
                                                    dev->plight_portPin_map->GPIO_Pin_x);
        }
    }
}
// 关闭light
static int close_lights(struct light_device_t *dev)
{
    int i; 
    if (dev) {
        os_free(dev);
    }
    return 0;
}
/** Open a new instance of a lights device using name */
static int open_lights(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device)
{
    int (*set_light)(struct light_device_t* dev,
            struct light_state_t const* state);
    int (*init_light)(struct light_device_t* dev,
        light_portPin_map *pportPin_map,int map_count);
    if (0 == strcmp(LIGHT_ID_BACKLIGHT, name)) {  // 根据名称设置相应的接口
        set_light = set_light_backlight;
        init_light= init_light_backlight;
    }
    else if (0 == strcmp(LIGHT_ID_BUTTONS, name)) {
        set_light = set_light_buttons;
    }
    else if (0 == strcmp(LIGHT_ID_BATTERY, name)) {
        set_light = set_light_battery;
    }
    else if (0 == strcmp(LIGHT_ID_NOTIFICATIONS, name)) {
        set_light = set_light_notifications;
    }
    else if (0 == strcmp(LIGHT_ID_ATTENTION, name)) {
        set_light = set_light_attention;
    }
    else if (0 == strcmp(LIGHT_ID_RGBWW, name)) {
        set_light = set_light_rgbww;
        init_light= init_light_rgbww;
    }
    else {
        LOGE("name %s\n", name);
        return -1;
    }
    LOGD("name is %s\r\n",name);
    // 创建light_device_t
    light_device_t *dev = os_malloc(sizeof(light_device_t));
    memset(dev, 0, sizeof(*dev));
 
    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
    // 关闭接口
    dev->common.close = (int (*)(struct hw_device_t*))close_lights;
    dev->set_light = set_light;
    dev->init_light=init_light;
    *device = (struct hw_device_t*)dev;
    return 0;
}
 
 
static struct hw_module_methods_t lights_module_methods = {
    .open =  open_lights,
};
 
/*
 * The lights Module
 */
const struct hw_module_t hal_module_info_light = {
    .tag = HARDWARE_MODULE_TAG,       // 规定的tag
    .version_major = 1,
    .version_minor = 0,
    .id = LIGHTS_HARDWARE_MODULE_ID,  // 模块id
    .name = "Govee lights Module",     // 名称
    .author = "Govee",
    .methods = &lights_module_methods,// 方法
};


