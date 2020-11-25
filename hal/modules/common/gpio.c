/*
 * =====================================================================================
 *
 *       Filename:  gpio.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/17/2020 10:53:47 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include "gpio.h"
#include "driver_pmu_regs.h"
#include "driver_pmu.h"
#if 0
#define GPIO_PA0              (1<<0)
#define GPIO_PA1              (1<<1)
#define GPIO_PA2              (1<<2)
#define GPIO_PA3              (1<<3)
#define GPIO_PA4              (1<<4)
#define GPIO_PA5              (1<<5)
#define GPIO_PA6              (1<<6)
#define GPIO_PA7              (1<<7)

#define GPIO_PB0              (1<<8)
#define GPIO_PB1              (1<<9)
#define GPIO_PB2              (1<<10)
#define GPIO_PB3              (1<<11)
#define GPIO_PB4              (1<<12)
#define GPIO_PB5              (1<<13)
#define GPIO_PB6              (1<<14)
#define GPIO_PB7              (1<<15)

#define GPIO_PC0              (1<<16)
#define GPIO_PC1              (1<<17)
#define GPIO_PC2              (1<<18)
#define GPIO_PC3              (1<<19)
#define GPIO_PC4              (1<<20)
#define GPIO_PC5              (1<<21)
#define GPIO_PC6              (1<<22)
#define GPIO_PC7              (1<<23)

#define GPIO_PD0              (1<<24)
#define GPIO_PD1              (1<<25)
#define GPIO_PD2              (1<<26)
#define GPIO_PD3              (1<<27)
#define GPIO_PD4              (1<<28)
#define GPIO_PD5              (1<<29)
#define GPIO_PD6              (1<<30)
#define GPIO_PD7              ((uint32_t)1<<31)
#endif

#define GPOUP_COUNT             8
#define GPIO_NAME(gp,index)     GPIO_##gp##index
void gpio_pmu_wakeupsrc(unsigned int        gpios){
    int i=0;    
    uint32_t gp_port=0;     
    uint32_t port_bit=0;

    
    //select group A
    gp_port =GPIO_NAME(PA,0) | GPIO_NAME(PA,1)| GPIO_NAME(PA,2)| GPIO_NAME(PA,3)| GPIO_NAME(PA,4)\
            | GPIO_NAME(PA,5)| GPIO_NAME(PA,6)| GPIO_NAME(PA,7);
    port_bit=gp_port &gpios;
    if(port_bit ){
        pmu_set_pin_pull(GPIO_PORT_A,port_bit, true);
    }

    //select group B
    gp_port =GPIO_NAME(PB,0) | GPIO_NAME(PB,1)| GPIO_NAME(PB,2)| GPIO_NAME(PB,3)| GPIO_NAME(PB,4)\
            | GPIO_NAME(PB,5)| GPIO_NAME(PB,6)| GPIO_NAME(PB,7);    
    port_bit=gp_port &gpios;
    if(port_bit ){
        pmu_set_pin_pull(GPIO_PORT_B,port_bit>>GPOUP_COUNT, true);
    }

    //select group C
    gp_port =GPIO_NAME(PC,0) | GPIO_NAME(PC,1)| GPIO_NAME(PC,2)| GPIO_NAME(PC,3)| GPIO_NAME(PC,4)\
            | GPIO_NAME(PC,5)| GPIO_NAME(PC,6)| GPIO_NAME(PC,7);    
    port_bit=gp_port &gpios;
    if(port_bit ){
        pmu_set_pin_pull(GPIO_PORT_C,port_bit>>(GPOUP_COUNT*2), true);
    }

    //select group D
    gp_port =GPIO_NAME(PD,0) | GPIO_NAME(PD,1)| GPIO_NAME(PD,2)| GPIO_NAME(PD,3)| GPIO_NAME(PD,4)\
            | GPIO_NAME(PD,5)| GPIO_NAME(PD,6)| GPIO_NAME(PD,7);    
    port_bit=gp_port &gpios;
    if(port_bit ){
        pmu_set_pin_pull(GPIO_PORT_D,port_bit>>(GPOUP_COUNT*3), true);
    }
    pmu_port_wakeup_func_set(gpios);
}
