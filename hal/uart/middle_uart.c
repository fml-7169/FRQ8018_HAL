/*
 * middle_uart.c
 *
 *  Created on: 2020-9-17
 *      Author: Weili.Hu
 */


#include <string.h>
#include <stdlib.h>
#include "timer.h"
#include "Lite-Rbuffer.h"
#include "configs.h"
#include "middle_uart.h"
#include "govee_log.h"
#include "app_bridge.h"
#include "middle_led.h"
#include "govee_utils.h"


#if UART_COMMAND_LINE_ENABLE
static uint8 g_uart_read_data[UART_RX_BUFFER_SIZE] = {0};
static uint8 g_uart_data_flag = 0;
static int32 g_uart_data_offset = 0;
#else
static LR_handler gt_uart_lr = NULL;
#endif


#if UART_COMMAND_LINE_ENABLE
static void uart_data_callback(uint8* p_data, uint32 size)
{
    if (g_uart_data_flag || g_uart_data_offset >= UART_RX_BUFFER_SIZE)
    {
        return;
    }

    if (size == 3 && p_data[0] == 0x1b && p_data[1] == 0x5b) // for direction key
    {
        return;
    }

    if (g_uart_data_offset + size > UART_RX_BUFFER_SIZE)
    {
        memcpy(g_uart_read_data + g_uart_data_offset, p_data, UART_RX_BUFFER_SIZE - g_uart_data_offset);
        g_uart_data_offset = UART_RX_BUFFER_SIZE;
    }
    else
    {
        memcpy(g_uart_read_data + g_uart_data_offset, p_data, size);
        g_uart_data_offset += size;
    }

    if (app_bridge_os_tick_get() >= 300)
    {
        p_data[size] = '\0';
        app_bridge_printf("%s", p_data);
    }

    if (g_uart_data_offset >= UART_RX_BUFFER_SIZE || p_data[size - 1] == 0x0d)
    {
        g_uart_data_flag = 1;
    }
}
#else
static void uart_data_callback(uint8* p_data, uint32 size)
{
    Lite_ring_buffer_write_data(gt_uart_lr, (uint8*)p_data, size);
}
#endif


#if UART_COMMAND_LINE_ENABLE
int32 mid_uart_command_get(uint8* buffer)
{
    int ret = -1;

    if (g_uart_data_flag && g_uart_data_offset > 0)
    {
        ret = g_uart_data_offset;
        memcpy(buffer, g_uart_read_data, g_uart_data_offset);
        memset(g_uart_read_data, 0, UART_RX_BUFFER_SIZE);
        g_uart_data_offset = 0;
        g_uart_data_flag = 0;
    }

    return ret;
}
#else
int32 mid_uart_data_size(void)
{
    return Lite_ring_buffer_size_get(gt_uart_lr);
}

int32 mid_uart_data_get(uint8* buffer, uint32 size)
{
    int left = Lite_ring_buffer_size_get(gt_uart_lr);

    if (size > left)
    {
        GOVEE_PRINT(LOG_ERROR, "uart data is not enough.\r\n");
        return -1;
    }

    Lite_ring_buffer_read_data(gt_uart_lr, (uint8*)buffer, size);

    return 0;
}
#endif

int32 mid_uart_data_send(uint8* p_data, uint32 size)
{
    return app_bridge_uart_send_data(p_data, size);
}

int32 mid_uart_init(uart_config_t* pt_uart)
{
#if (!UART_COMMAND_LINE_ENABLE)
    gt_uart_lr = Lite_ring_buffer_init(UART_RX_BUFFER_SIZE);
    if (gt_uart_lr == NULL)
    {
        GOVEE_PRINT(LOG_ERROR, "Uart ring buffer init failed.\r\n");
        return -1;
    }
#endif
    pt_uart->uart_read = uart_data_callback;

    return 0;
}

