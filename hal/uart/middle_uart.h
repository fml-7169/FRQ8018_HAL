/*
 * middle_uart.h
 *
 *  Created on: 2020-9-17
 *      Author: Weili.Hu
 */

#ifndef MIDDLE_UART_H_
#define MIDDLE_UART_H_

#include "types.h"


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

 uint32_t uart_baud_map[12] = {1200,2400,4800,9600,14400,19200,38400,57600,115200,230400,460800,921600};

typedef enum
{
    BAUD_RATE_1200 = 0,
    BAUD_RATE_2400,
    BAUD_RATE_4800,
    BAUD_RATE_9600,
    BAUD_RATE_14400,
    BAUD_RATE_19200,
    BAUD_RATE_38400,
    BAUD_RATE_57600,
    BAUD_RATE_115200,
    BAUD_RATE_230400,
    BAUD_RATE_460800,
    BAUD_RATE_921600,
    BAUD_RATE_MAX
} msg_source_e;


int32 mid_uart_command_get(uint8* buffer);
int32 mid_uart_data_size(void);
int mid_uart_data_get(uint8* buffer, uint32 size);
int32 mid_uart_data_send(uint8* p_data, uint32 size);
int32 mid_uart_init(uart_config_t* pt_uart);


#endif /* MIDDLE_UART_H_ */
