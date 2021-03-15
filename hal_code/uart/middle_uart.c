/*
 * middle_uart.c
 *
 *  Created on: 2020-9-17
 *      Author: Weili.Hu
 */


#include <string.h>
#include <stdlib.h>
#include "Lite-Rbuffer.h"
#include "middle_uart.h"
#include "govee_log.h"
#include "app_bridge.h"
#include "govee_utils.h"
#include "middle_uart.h"
#include "driver_uart.h"
#include "driver_system.h"
#include "os_mem.h"
#include "hci_test.h"


static LR_handler gt_uart_lr = NULL;


static void uart_data_callback(uint8* p_data, uint32 size)
{
    Lite_ring_buffer_write_data(gt_uart_lr, (uint8*)p_data, size);
}


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

int32 mid_uart_data_send(uint8* p_data, uint32 size)
{
    uart_write(UART0,p_data,size);
    return 0;
}

/*********************************************************************
 * @fn      uart0_isr_ram
 *
 * @brief   UART0 interruption, when uart0 FIFO received charaters, this ISR will be called
 *
 *
 * @param   None
 *
 *
 * @return  None
 */
__attribute__((section("ram_code"))) void uart0_isr_ram(void)
{
    uint8_t int_id;
    volatile struct uart_reg_t * const uart_reg_ram = (volatile struct uart_reg_t *)UART0_BASE;
    int_id = uart_reg_ram->u3.iir.int_id;

    if(int_id == 0x04 || int_id == 0x0c )   /* Receiver data available or Character time-out indication */
    {
        if(hci_test())
            return;
        else{
            while(uart_reg_ram->lsr & 0x01)
            {
                Lite_ring_buffer_write_data(gt_uart_lr, (uint8*)&(uart_reg_ram->u1.data), 1);
                //uart_putc_noint_no_wait(UART1,uart_reg_ram->u1.data);
            }  
        }

    }
    else if(int_id == 0x06)
    {
        uart_reg_ram->lsr = uart_reg_ram->lsr;
    }
}

#if 0
void mid_uart_isr_handle(uint8* p_data)
{
    Lite_ring_buffer_write_data(gt_uart_lr, p_data, 1);
}
#endif

int32 mid_uart_init(int8 baud_rate)
{

    gt_uart_lr = Lite_ring_buffer_init(UART_RX_BUFFER_SIZE);
    if (gt_uart_lr == NULL)
    {
        GOVEE_PRINT(LOG_ERROR, "Uart ring buffer init failed.\r\n");
        return -1;
    }
    system_set_port_pull(GPIO_PD4, true);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_4, PORTD4_FUNC_UART0_RXD);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_5, PORTD5_FUNC_UART0_TXD);
    uart_init(UART0, baud_rate);
    //uart_init_x(gAT_buff_env.uart_param);
    NVIC_EnableIRQ(UART0_IRQn);
    GOVEE_PRINT(LOG_DEBUG,"mid_uart_init %d\r\n",baud_rate);
    uart_write(UART0,"hello,world",11);
    return 0;
}