

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
#include "driver_adc.h"
#include "os_mem.h"
#include "co_printf.h"

#include "hal_adc.h"
#include "hal_config.h"
#include "gpio.h"

#define ACQ_RANGE   (1<<10)
#define DEV_ERR(format,...) do { \
co_printf("[ADC] error:"); \
co_printf(format,##__VA_ARGS__); \
} while(0)
#define DEV_DB(format,...) do { \
    co_printf("[ADC] debug:"); \
    co_printf(format,##__VA_ARGS__); \
    } while(0)

//static inline void adc_convert(struct adc_client *adc)
//{

//}

//static inline void adc_select(struct adc_client *adc,
//				  struct adc_client *client)
//{

//}
//static void convert_done(struct adc_client *client,
//               unsigned v, unsigned u, unsigned *left)
//{

//}
//static void adc_default_select(struct adc_client *client,
//				   unsigned select)
//{
//}

int adc_start(struct adc_client *client)
{
    if(client == NULL){
        DEV_ERR("input is null\n");
        return -1;
    }
    DEV_DB("sample is %d,channel is %d\r\n",client->nr_samples,client->channel);
	struct adc_cfg_t cfg;	 
    memset((void*)&cfg, 0, sizeof(cfg));
    cfg.src = ADC_TRANS_SOURCE_PAD;
    cfg.ref_sel = ADC_REFERENCE_AVDD;
    cfg.channels = client->channel;
    cfg.route.pad_to_sample = 1;
    cfg.clk_sel = client->nr_samples;
    cfg.clk_div = 0x3f;
    adc_init(&cfg);
    adc_enable(NULL, NULL, 0);
    client->ref = adc_get_ref_voltage(ADC_REFERENCE_AVDD);
	return 0;
}



static void adc_stop(struct adc_client *client){
    adc_disable();
}
static int adc_read(struct adc_client *client, unsigned int ch)
{
	int ret=-1;
    unsigned short adc_value;
    if(client == NULL){
        DEV_ERR("input dev is null\r\n");
        return -1;
    }
	//ret = adc_start(client);
	if(client->channel &ch){
		
	    adc_get_result(ADC_TRANS_SOURCE_PAD, ch, &adc_value);
        client->result=adc_value*client->ref/ACQ_RANGE;
        DEV_DB("adc result %d,ref %d,acq range %d\r\n",client->result,client->ref,ACQ_RANGE);
		
	}
    else{        
        DEV_ERR("no channel had be match\r\n");
        return 0;
    }

	return client->result;
}



static int  adc_remove(adc_device_t *pdev)
{
    if(pdev){
        os_free(pdev);
    }
	return 0;
}
void adc_pin_config(void *pin_map,int pin_map_len,void *device)
{
    if(pin_map == NULL || pin_map_len<=0 || device == NULL){
        DEV_ERR("input is error\r\n");
        return;
    }    
    struct adc_device_t *dev = (struct adc_device_t *)device;
    struct adc_config_stuct* adc_map=(struct adc_config_stuct*)pin_map;
    int pin_count=pin_map_len/sizeof(struct adc_config_stuct);    
    DEV_ERR("start pin count is %d\r\n",pin_count);
    int i=0;
    for(i=0;i<pin_count;i++){        
        DEV_DB("adc[%d] index %d,channel %d,sample %d\r\n",i,adc_map[i].adc_index,adc_map[i].channel,adc_map[i].sample);
        gpio_adc(adc_map[i].adc_index);        
        dev->cli.channel |=adc_map[i].channel;        
        dev->cli.nr_samples=adc_map[i].sample;
    }
    adc_start(&dev->cli);
    return;
}


static int adc_open(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device)
{
    //malloc dev
    DEV_DB("adc open\r\n");
	adc_device_t *dev = os_malloc(sizeof(adc_device_t));
    memset(dev, 0, sizeof(*dev));
    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
    dev->common.close = (int (*)(struct hw_device_t*))adc_remove;
    dev->cli.read=(int (*)(void *,unsigned int))adc_read;
    dev->cli.stop=(int (*)(void *))adc_stop;
    dev->cli.pin_config=adc_pin_config;
    *device = (struct hw_device_t*)dev;
    return 0;
}


static struct hw_module_methods_t adc_module_methods = {
    .open =  adc_open,
};


const struct hw_module_t hal_module_info_adc = {
    .tag = HARDWARE_MODULE_TAG,       // 规定的tag
    .version_major = 1,
    .version_minor = 0,
    .id = ADC_HARDWARE_MODULE_ID,  // 模块id
    .name = "Govee adc Module",     // 名称
    .author = "Govee",
    .methods = &adc_module_methods,// 方法
};

