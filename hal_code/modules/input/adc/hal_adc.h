/* arch/arm/plat-samsung/include/plat/adc.h
 *
 * Copyright (c) 2008 Simtec Electronics
 *	http://armlinux.simtec.co.uk/	
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * S3C ADC driver information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_PLAT_ADC_H
#define __ASM_PLAT_ADC_H __FILE__
#define ADC_HARDWARE_MODULE_ID "adc"
#include "hardware.h"   

struct adc_client;
struct adc_client {
	unsigned int		nr_samples;
	int			        result;    
	unsigned int		ref;
	unsigned char		channel;
	void	(*select_cb)(struct adc_client *c, unsigned selected);
	void	(*convert_cb)(struct adc_client *c,
			      unsigned val1, unsigned val2,
			      unsigned *samples_left);    
    void (*pin_config)(void *pin_map,
                    int pin_map_len,
                    void* device);    
    int (*read)(void *pdev, unsigned int ch);    
    int (*stop)(void *pdev);
};

typedef struct adc_device_t {    
    struct hw_device_t common;    
	struct adc_client	cli;    
}adc_device_t;

#endif /* __ASM_PLAT_ADC_H */
