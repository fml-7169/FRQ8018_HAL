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

#ifndef __HAL_BUZZER_H
#define __HAL_BUZZER_H __FILE__
#define BUZZER_HARDWARE_MODULE_ID "buzzer"
#include "hardware.h"   
typedef struct _bz_portPin_map {
  uint32_t GPIOx;
  uint32_t GPIO_Pin_x;
} bz_portPin_map;

typedef struct bz_device_t {
    struct hw_device_t common; 
    bz_portPin_map bz_portPin_map;
    int (*start)(struct bz_device_t *dev,\
                        int channel,\
                        unsigned int frequency,\
                        unsigned int high_duty);
    void (*stop)(struct bz_device_t* dev,
                int channel);
    int (*init)(struct bz_device_t *dev,\
                        bz_portPin_map portPin_map);
}bz_device_t;

#endif /* __BUZZER_H */
