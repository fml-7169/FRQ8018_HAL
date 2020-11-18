/*
 * =====================================================================================
 *
 *       Filename:  config.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/16/2020 02:29:51 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef  config_INC
#define  config_INC


//define HAL_KEYS_CONFIG_
#ifdef HAL_KEYS_CONFIG_
#endif
struct keys_config_stuct{
    unsigned int gpio;
    int debounce_interval; //ms
    unsigned int short_timeout_ms; //ms
};
#endif   /* ----- #ifndef config_INC  ----- */
