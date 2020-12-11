/*
 * middle_uart.h
 *
 *  Created on: 2020-9-17
 *      Author: Weili.Hu
 */

#ifndef MIDDLE_UART_H_
#define MIDDLE_UART_H_

#include "hal_types.h"


#define UART_COMMAND_LINE_ENABLE        1


#if UART_COMMAND_LINE_ENABLE
#define UART_COMMAND_MAX_LEN            32
#define UART_COMMAND_MAX_PARAM          6
#define UART_RX_BUFFER_SIZE             128
#else
#define UART_RX_BUFFER_SIZE             512
#define UART_DATA_TEST                  0
#endif

typedef void (*uart_callback)(uint8* p_data, uint32 size);

typedef struct
{
    int8 baud_rate;
    uart_callback uart_read;
} uart_config_t;


int32 mid_uart_command_get(uint8* buffer);
int32 mid_uart_data_size(void);
int mid_uart_data_get(uint8* buffer, uint32 size);
int32 mid_uart_data_send(uint8* p_data, uint32 size);
int32 mid_uart_init(int8 baud_rate);

#endif /* MIDDLE_UART_H_ */
