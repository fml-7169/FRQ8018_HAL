#include <string.h>
#include "hardware.h"
#include "gpio_keys.h"
#include "os_timer.h"
#include "co_printf.h"
#include "os_mem.h"
#include "os_msg_q.h"
#include "hal_config.h"
#include "gpio.h"
#include "driver_pmu.h"

#define DEV_ERR(format,...) do { \
co_printf("[KEYS] error:"); \
co_printf(format,##__VA_ARGS__); \
} while(0)
#define DEV_DEBUG(format,...) do { \
    co_printf("[KEYS] debug:"); \
    co_printf(format,##__VA_ARGS__); \
} while(0)

#define KEYS_SET_FAIL -1
#define KEYS_SUC 0

static void *key_drvdata=NULL;
static os_timer_t anti_shake_timer;


enum keys_button_type_t {
    KEYS_BUTTON_PRESSED,        
    KEYS_BUTTON_RELEASED,
    KEYS_BUTTON_SHORT_RELEASED,  //press to release,short end
    KEYS_BUTTON_LONG_PRESSED,
    KEYS_BUTTON_LONG_RELEASED,
    
    KEYS_BUTTON_COMB_PRESSED,
    KEYS_BUTTON_COMB_RELEASED,
    KEYS_BUTTON_COMB_SHORT_PRESSED,
    KEYS_BUTTON_COMB_LONG_PRESSED,
    KEYS_BUTTON_COMB_LONG_PRESSING,
    KEYS_BUTTON_COMB_LONG_RELEASED,
    KEYS_BUTTON_COMB_LONG_LONG_PRESSED,
    KEYS_BUTTON_COMB_LONG_LONG_RELEASED,
};


static void  keys_set_drvdata(void *pdrvdata){
        key_drvdata=pdrvdata;
        return ;
}


static void*  keys_get_drvdata(void ){
       return  key_drvdata;
}

#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})
#define PREPARE_WORK(_work, _func)				\
        do {                            \
            (_work)->func = (_func);            \
        } while (0)

/**
 * gpio_keys_disable_button() - disables given GPIO button
 * @bdata: button data for button to be disabled
 *
 * Disables button pointed by @bdata. This is done by masking
 * IRQ line. After this function is called, button won't generate
 * input events anymore. Note that one can only disable buttons
 * that don't share IRQs.
 *
 * Make sure that @bdata->disable_lock is locked when entering
 * this function to avoid races when concurrent threads are
 * disabling buttons at the same time.
 */
static void gpio_keys_disable_button(void *bdata)
{

}

/**
 * gpio_keys_enable_button() - enables given GPIO button
 * @bdata: button data for button to be disabled
 *
 * Enables given button pointed by @bdata.
 *
 * Make sure that @bdata->disable_lock is locked when entering
 * this function to avoid races with concurrent threads trying
 * to enable the same button at the same time.
 */
static void gpio_keys_enable_button(void *bdata)
{

}


static void gpio_keys_report_event(struct gpio_keys_button *bdata)
{
    if(bdata == NULL){
        DEV_ERR("gpio_keys_report_event input is invaild\r\n");
        return;
    }    
    
    key_device_t * keys_data=(key_device_t *)keys_get_drvdata();
    if(keys_data !=NULL){
        if(keys_data->keys_report.report_func_cb){
            keys_data->keys_report.report_func_cb(bdata->event,bdata->type,0);
        }
    }
    DEV_DEBUG("0x%x report key value 0x%x\r\n",bdata->event,bdata->type);
}

static void gpio_keys_work_func(struct work_struct *work)
{
	struct key_device_t *bdata =
		container_of(work, struct key_device_t, work);
    DEV_DEBUG("report long key\r\n");

//	gpio_keys_report_event(bdata);
}
static void gpio_keys_long_timer(void *_pdata)  //long keys
{
	struct gpio_keys_button *data = (struct gpio_keys_button *)_pdata;
    data->type=KEYS_BUTTON_LONG_PRESSED; //for long    
	gpio_keys_report_event(data);
}

int gpio_keys_report_init(keys_report_func_t keys_report_func)
{
    key_device_t * keys_data=(key_device_t *)keys_get_drvdata();
    if(keys_data == NULL)
        return KEYS_SET_FAIL;
    keys_data->keys_report.report_func_cb=keys_report_func;
}

static void gpio_keys_state(void *_pdata,uint32_t gpio_value){
	struct key_device_t *data = (struct key_device_t *)_pdata;
    int i=0;
    if(data == NULL)
        return;
    
    os_timer_t* timer_handle=((os_timer_t*)data->timer_list);
    for(i=0;i<data->n_buttons;i++){
        uint32_t gpio_value_pin =gpio_value & data->button[i].gpio;
        DEV_DEBUG("button index %d,type %d,%s\r\n",i,data->button[i].type,\
                    (gpio_value_pin ^ data->button[i].gpio)?"press":"release");
        int last_type=data->button[i].type;
        switch(data->button[i].type){
            case KEYS_BUTTON_PRESSED:{
                    if(!(gpio_value_pin ^ data->button[i].gpio)){ //release
                        data->button[i].type=KEYS_BUTTON_SHORT_RELEASED;
                        os_timer_stop(&timer_handle[i]);                    
                        gpio_keys_report_event(&data->button[i]);
                        //report to service
                    }
                }break;
            case KEYS_BUTTON_RELEASED:{
                if((gpio_value_pin ^ data->button[i].gpio)){ //press
                    data->button[i].type=KEYS_BUTTON_PRESSED;                    
                    gpio_keys_report_event(&data->button[i]);
                    os_timer_start(&timer_handle[i],\
                        data->button[i].short_timeout,false);                
                }
            }break;
            case KEYS_BUTTON_SHORT_RELEASED:{  //release to press
                    if((gpio_value_pin ^ data->button[i].gpio)){
                        data->button[i].type=KEYS_BUTTON_PRESSED;                        
                        gpio_keys_report_event(&data->button[i]);
                        os_timer_start(&timer_handle[i],\
                        data->button[i].short_timeout,false);
                    }
                }break;
            case KEYS_BUTTON_LONG_PRESSED:{
                if(!(gpio_value_pin ^ data->button[i].gpio)){ //release
                    data->button[i].type=KEYS_BUTTON_LONG_RELEASED;                    
                    gpio_keys_report_event(&data->button[i]);
                    os_timer_stop(&timer_handle[i]);
                }
            }break;
            case KEYS_BUTTON_LONG_RELEASED:{
                if((gpio_value_pin ^ data->button[i].gpio)){ //press
                    data->button[i].type=KEYS_BUTTON_PRESSED;                    
                    gpio_keys_report_event(&data->button[i]);
                    os_timer_start(&timer_handle[i],\
                        data->button[i].short_timeout,false);
                }
            }break;
            default:{
                DEV_ERR("keys index %d,type %d,%s\r\n",i,data->button[i].type,\
                (gpio_value_pin ^ data->button[i].gpio)?"press":"release");
            }break;
        }
    }
}
static void gpio_keys_shake_timer(void *_pdata)
{
	struct key_device_t *data = (struct key_device_t *)_pdata;    
//    DEV_DEBUG("dev shake address is 0x%x\r\n",data);
    int i=0;    
    unsigned int key_dev_mask=0;
    if(data == NULL){
        DEV_ERR("keys shake timer is null\r\n");
        return;
    }
    for(i=0;i<data->n_buttons;i++){
        key_dev_mask|=data->button[i].gpio;
    }
    unsigned int keys_gpio_value = ool_read32(PMU_REG_GPIOA_V);    
    keys_gpio_value &= key_dev_mask;        
    DEV_DEBUG("keys_gpio_value is 0x%x,anti_shake_mask is 0x%x\r\n",\
                    keys_gpio_value,data->anti_shake_mask);
    if(data->anti_shake_mask == keys_gpio_value){  //keys change
        gpio_keys_state(data,keys_gpio_value);
          //press or release 
    }
    return;
}

void button_toggle_detected(uint32_t curr_button)
{
//    DEV_DEBUG("curr_button is 0x%x\r\n",curr_button);
    struct key_device_t *dev =(struct key_device_t *)keys_get_drvdata();
    unsigned int key_dev_mask=0;
    int i=0;
    if(dev) {
        for(i=0;i<dev->n_buttons;i++){
            key_dev_mask|=dev->button[i].gpio;
        }        
        dev->anti_shake_mask = key_dev_mask & curr_button;
        os_timer_start(&anti_shake_timer, dev->button[0].debounce_interval, false);
    }
}

static int  gpio_keys_setup_key(struct key_device_t *dev,void *pin_map,int pin_len)
{
	int irq, error=KEYS_SET_FAIL;    
    int i=0;
    if(dev == NULL){
        return error;
    }    
    struct keys_config_stuct *keys_config_pin=(struct keys_config_stuct *)pin_map;
    dev->n_buttons=pin_len/sizeof(struct keys_config_stuct);
    //malloc and init button from gpio's map
    dev->button=os_malloc(dev->n_buttons*sizeof(struct gpio_keys_button));
    dev->timer_list=(os_timer_t*)os_malloc(dev->n_buttons*sizeof(os_timer_t));    
    os_timer_t* timer_handle=((os_timer_t*)dev->timer_list);
    for(i=0;i<dev->n_buttons;i++){
        os_timer_init(&timer_handle[i], gpio_keys_long_timer, &dev->button[i]);
        //for gpio's config
        dev->button[i].debounce_interval=keys_config_pin[i].debounce_interval;   //3ms
        dev->button[i].type=KEYS_BUTTON_RELEASED;
        dev->button[i].gpio=keys_config_pin[i].gpio;
        dev->button[i].event=keys_config_pin[i].event;
        dev->button[i].short_timeout=keys_config_pin[i].short_timeout_ms;
        dev->anti_shake_mask |=dev->button[i].gpio;
    }
    gpio_pmu_wakeupsrc(dev->anti_shake_mask);
    DEV_DEBUG("dev setup address is 0x%x\r\n",dev);
    os_timer_init(&anti_shake_timer, gpio_keys_shake_timer, dev);
	error= KEYS_SUC;
	return error;
}

static int gpio_keys_close(struct key_device_t *dev)
{
     int i;
     if (dev) {
         if(dev->button){
             os_free(dev->button);
             dev->button=NULL;
         }
         if(dev->timer_list){
             os_free(dev->timer_list);
             dev->timer_list=NULL;
         }
         os_free(dev);
         key_drvdata=NULL;
     }
     
     return 0;
}
void gpio_keys_pin_config(void *pin_map,int pin_map_len,void *device)
{
    if(pin_map == NULL || pin_map_len<=0 || device == NULL){
        DEV_ERR("input is error\r\n");
        return;
    }
    struct key_device_t*dev=(struct key_device_t*)device;
    gpio_keys_setup_key(dev,pin_map,pin_map_len);
    return;
}

static int gpio_keys_open(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device)
{
    //malloc dev
	key_device_t *dev = os_malloc(sizeof(key_device_t));
    memset(dev, 0, sizeof(*dev));
    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
    dev->common.close = (int (*)(struct hw_device_t*))gpio_keys_close;
    dev->key_pin_config=gpio_keys_pin_config;
    dev->key_report_init=gpio_keys_report_init;
    keys_set_drvdata(dev);
    *device = (struct hw_device_t*)dev;
    return 0;
}



static struct hw_module_methods_t keys_module_methods = {
    .open =  gpio_keys_open,
};


const struct hw_module_t hal_module_info_key = {
    .tag = HARDWARE_MODULE_TAG,       // 规定的tag
    .version_major = 1,
    .version_minor = 0,
    .id = KEYS_HARDWARE_MODULE_ID,  // 模块id
    .name = "Govee keys Module",     // 名称
    .author = "Govee",
    .methods = &keys_module_methods,// 方法
};




