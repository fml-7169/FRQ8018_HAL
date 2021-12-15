/*
 * govee_log.h
 *
 *  Created on: 2020-9-11
 *      Author: Weili.Hu
 */

#ifndef GOVEE_LOG_H_
#define GOVEE_LOG_H_

#include "co_printf.h"

#define LOG_ERROR           0           //error
#define LOG_WARNING         1           //warning
#define LOG_INFOR           2           //information
#define LOG_DEBUG           3           //debug
#define LOG_CLOSE           0x0f        //close

#define LOG_LEVEL           LOG_CLOSE


#define GOVEE_PRINT(level, fmt, args...) \
    do \
    { \
        if (level <= LOG_LEVEL && LOG_LEVEL != LOG_CLOSE) \
        { \
            switch(level) \
            { \
                case LOG_DEBUG: \
                    co_printf("[debug] "fmt, ##args); \
                    break; \
                case LOG_INFOR: \
                    co_printf("[info] "fmt, ##args); \
                    break;\
                case LOG_WARNING: \
                    co_printf("[warning] "fmt, ##args); \
                    break;\
                case LOG_ERROR:\
                    co_printf("[error] "fmt, ##args); \
                    break;\
                default: \
                    break; \
            } \
        } \
    } \
    while (0)

#endif /* GOVEE_LOG_H_ */
