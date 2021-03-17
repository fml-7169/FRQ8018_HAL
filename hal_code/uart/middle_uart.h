/*
 * middle_uart.h
 *
 *  Created on: 2020-9-17
 *      Author: Weili.Hu
 */

#ifndef MIDDLE_UART_H_
#define MIDDLE_UART_H_

#include "hal_types.h"

#define UART_RX_BUFFER_SIZE             512

typedef void (*uart_callback)(uint8* p_data, uint32 size);

enum _mid_uart_id
{
    UART_ID_0,
    UART_ID_1,
    UART_MAX
};

typedef struct
{
    int8 baud_rate;
    uart_callback uart_read;
} uart_config_t;

int32 mid_uart_data_size(uint8 u_id);
int mid_uart_data_get(uint8 u_id,uint8* buffer, uint32 size);
int32 mid_uart_data_send(uint8 u_id,uint8* p_data, uint32 size);
int32 mid_uart_init(uint8 u_id,int8 baud_rate);
void mid_uart0_port_set(Pin_Map tx_pin,Pin_Map rx_pin);

#endif /* MIDDLE_UART_H_ */
