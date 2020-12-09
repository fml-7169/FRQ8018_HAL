/*
 * =====================================================================================
 *
 *       Filename:  gpio.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/17/2020 10:54:13 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef  gpio_INC
#define  gpio_INC
#define ADC_CH0 0
#define ADC_CH1 1
#define ADC_CH2 2
#define ADC_CH3 3

void gpio_pmu_wakeupsrc(unsigned int gpios);
void gpio_adc(int adc_index);

#endif   /* ----- #ifndef gpio_INC  ----- */
