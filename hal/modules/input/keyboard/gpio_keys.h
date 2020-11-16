#ifndef _GPIO_KEYS_H
#define _GPIO_KEYS_H
#include "hardware.h"   


#define KEYS_HARDWARE_MODULE_ID "keys"
struct work_struct;
typedef void (*work_func_t)(struct work_struct *work);
struct work_struct {
	long data;
	work_func_t func;
};
typedef void (*keys_report_func_t)(unsigned int gpio,int type,int count);
struct gpio_keys_button {
	/* Configuration parameters */
	unsigned int gpio;    
	int type;		/* configure the button as a wake-up source */
	int debounce_interval;	/* debounce ticks interval in msecs */        
    unsigned int short_timeout;   /* short press ticks interval in msecs */
};
typedef struct gpio_keys_report{
    keys_report_func_t report_func_cb;
}gpio_keys_report;

typedef struct key_device_t {
    struct hw_device_t common;    
	unsigned int n_buttons;
	struct gpio_keys_button *button;    
    void *timer_list;  //timer_list from n_buttons
	struct work_struct work;    
	unsigned int anti_shake_mask;
    int (*key_report_init)(keys_report_func_t keys_report_func);  //get calllback from native
    gpio_keys_report keys_report;
}key_device_t;

#endif
