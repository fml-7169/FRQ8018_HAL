/*
 * middle_time.h
 *
 *  Created on: 2020-10-26
 *      Author: Weili.Hu
 */

#ifndef MIDDLE_TIME_H_
#define MIDDLE_TIME_H_

#include "types.h"

typedef enum
{
    TIMER_10MS = 0,
    TIMER_96US,
} TIMER_TYPE;

typedef void (*timer_callback)(void* args);
typedef void (*timer_system)(uint8 type);


typedef struct
{
    uint8 hour;
    uint8 min;
    uint8 sec;
    uint8 week; //1-7
} timeval_t;

typedef struct
{
    timer_system timer;
} time_config_t;


uint64 mid_time_tick_get(void);
void mid_time_set(timeval_t* pt_time);
int32 mid_time_get(timeval_t* pt_time);
int32 mid_time_state(void);
uint32 mid_time_timer_create(timer_callback cb, uint8 precision, uint8 b_dumplicate, uint32 counts, void* args);
void mid_time_timer_start(uint32 handle);
void mid_time_timer_stop(uint32 handle);
void mid_time_timer_destroy(uint32 handle);
void mid_time_init(time_config_t* pt_time);

#endif /* MIDDLE_TIME_H_ */
