/*
 * middle_time.c
 *
 *  Created on: 2020-10-26
 *      Author: Weili.Hu
 */

#include <string.h>
#include "govee_log.h"
#include "configs.h"
#include "middle_time.h"


typedef struct
{
    timer_callback callback;
    uint8 precision;
    uint8 times;
    uint8 flag;
    void* args;
    uint32 interval;
    uint64 start;
} timer_config_t;


static uint64 g_count_10ms = 0;
static uint64 g_count_96us = 0;

static timeval_t g_time_value;
static uint8 g_time_set_flag = 0;
static timer_config_t gt_timers[GOVEE_TIMER_COUNTS];


static void time_update(void* args)
{
    if (g_time_set_flag)
    {
        g_time_value.sec++;
        if(g_time_value.sec == 60)
        {
            g_time_value.sec = 0;
            g_time_value.min++;
            if(g_time_value.min == 60)
            {
                g_time_value.min = 0;
                g_time_value.hour++;
                if(g_time_value.hour == 24)
                {
                    g_time_value.hour = 0;
                    g_time_value.week++;
                    if(g_time_value.week > 7)
                    {
                        g_time_value.week = 1;
                    }
                }
            }
        }
    }
}

static void time_timer_callback(uint8 type)
{
    int i = 0;
    uint64 count = 0;

    if (type == TIMER_10MS)
    {
        g_count_10ms++;
    }
    else
    {
        g_count_96us++;
    }

    for (i = 0; i < GOVEE_TIMER_COUNTS; i++)
    {
        if (gt_timers[i].callback != NULL && gt_timers[i].flag && gt_timers[i].times > 0)
        {
            if ((type == TIMER_10MS && gt_timers[i].precision == TIMER_10MS) || (type == TIMER_96US && gt_timers[i].precision == TIMER_96US))
            {
                count = (type == TIMER_10MS) ? g_count_10ms : g_count_96us;

                if ((count != gt_timers[i].start) && (count - gt_timers[i].start) % (gt_timers[i].interval) == 0)
                {
                    gt_timers[i].callback(gt_timers[i].args);
                    if (gt_timers[i].times != 0xff)
                    {
                        gt_timers[i].times = 0;
                    }
                }
            }
        }
    }
}

uint64 mid_time_tick_get(void)
{
    return (96*g_count_96us) / 1000;
}

void mid_time_set(timeval_t* pt_time)
{
    if (pt_time)
    {
        memcpy(&g_time_value, pt_time, sizeof(timeval_t));
        g_time_set_flag = 1;
    }
}

int32 mid_time_get(timeval_t* pt_time)
{
    if (!g_time_set_flag)
    {
        GOVEE_PRINT(LOG_ERROR, "Time have not been set.\r\n");
        return -1;
    }

    if (pt_time)
    {
        memcpy(pt_time, &g_time_value, sizeof(timeval_t));
    }

    return 0;
}

int32 mid_time_state(void)
{
    return g_time_set_flag;
}

uint32 mid_time_timer_create(timer_callback cb, uint8 precision, uint8 b_dumplicate, uint32 counts, void* args)
{
    int i = 0;
    uint32 handle = 0;

    for (i = 0; i < GOVEE_TIMER_COUNTS; i++)
    {
        if (gt_timers[i].callback == NULL)
        {
            gt_timers[i].callback = cb;
            gt_timers[i].precision = precision;
            gt_timers[i].times = (b_dumplicate) ? 0xff : 0x01;
            gt_timers[i].interval = counts;

            handle = (uint32)&gt_timers[i];
            break;
        }
    }

    return handle;
}

void mid_time_timer_start(uint32 handle)
{
    int i = 0;

    for (i = 0; i < GOVEE_TIMER_COUNTS; i++)
    {
        if ((uint32)&gt_timers[i] == handle)
        {
            gt_timers[i].flag = 1;
            gt_timers[i].start = (gt_timers[i].precision == TIMER_10MS) ? g_count_10ms : g_count_96us;
            break;
        }
    }
}

void mid_time_timer_stop(uint32 handle)
{
    int i = 0;

    for (i = 0; i < GOVEE_TIMER_COUNTS; i++)
    {
        if ((uint32)&gt_timers[i] == handle)
        {
            gt_timers[i].flag = 0;
            break;
        }
    }
}

void mid_time_timer_destroy(uint32 handle)
{
    int i = 0;

    for (i = 0; i < GOVEE_TIMER_COUNTS; i++)
    {
        if ((uint32)&gt_timers[i] == handle)
        {
            memset(&gt_timers[i], 0, sizeof(timer_config_t));
            break;
        }
    }
}

void mid_time_init(time_config_t* pt_time)
{
    uint32 handle = 0;

    memset(gt_timers, 0 , sizeof(timer_config_t)*GOVEE_TIMER_COUNTS);
    handle = mid_time_timer_create(time_update, TIMER_10MS, 1, 100, NULL);
    if (handle > 0)
    {
        mid_time_timer_start(handle);
    }
    pt_time->timer = time_timer_callback;
}

