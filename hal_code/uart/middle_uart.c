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

typedef struct _mid_uart0_TypeDef {
  
  Pin_Map TX; // 
  Pin_Map RX;  //
} mid_uart0_TypeDef;

typedef struct _mid_uart_config {
   uint32  uart_port;
   uint32  uart_irqn;
   LR_handler uart_lr;
}mid_uart_config;

static mid_uart_config uart_config[2] = {
    {UART0,UART0_IRQn,NULL},
    {UART1,UART1_IRQn,NULL}
};


mid_uart0_TypeDef g_mid_uart0={\
                            {GPIO_PORT_D,GPIO_BIT_4,PORTD4_FUNC_UART0_RXD},\
                            {GPIO_PORT_D,GPIO_BIT_5,PORTD5_FUNC_UART0_TXD}}; //default port and 


void mid_uart0_port_set(Pin_Map tx_pin,Pin_Map rx_pin){
    g_mid_uart0.TX=tx_pin;
    g_mid_uart0.RX=rx_pin;
    return;
}

static void uart_data_callback(uint8 u_id,uint8* p_data, uint32 size)
{
    if(u_id>=UART_MAX||uart_config[u_id].uart_lr==NULL){
        GOVEE_PRINT(LOG_ERROR, "Uart %d ring id error.\r\n",u_id);
        return ;  
    }
    Lite_ring_buffer_write_data(uart_config[u_id].uart_lr, (uint8*)p_data, size);
}


int32 mid_uart_data_size(uint8 u_id)
{
    if(u_id>=UART_MAX||uart_config[u_id].uart_lr==NULL){
        GOVEE_PRINT(LOG_ERROR, "Uart %d ring id error.\r\n",u_id);
        return -1;  
    }
    return Lite_ring_buffer_size_get(uart_config[u_id].uart_lr);
}

int32 mid_uart_data_get(uint8 u_id,uint8* buffer, uint32 size)
{
    if(u_id>=UART_MAX||uart_config[u_id].uart_lr==NULL){
        GOVEE_PRINT(LOG_ERROR, "Uart %d ring id error.\r\n",u_id);
        return -1;  
    }
    int left = Lite_ring_buffer_size_get(uart_config[u_id].uart_lr);

    if (size > left)
    {
        GOVEE_PRINT(LOG_ERROR, "uart data is not enough.\r\n");
        return -1;
    }

    Lite_ring_buffer_read_data(uart_config[u_id].uart_lr, (uint8*)buffer, size);

    return 0;
}

int32 mid_uart_data_send(uint8 u_id,uint8* p_data, uint32 size)
{
    if(u_id>=UART_MAX||uart_config[u_id].uart_lr==NULL){
        GOVEE_PRINT(LOG_ERROR, "Uart %d ring id error.\r\n",u_id);
        return -1;  
    }
    uart_write(uart_config[u_id].uart_port,p_data,size);
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
	uint8 gvalue;
    volatile struct uart_reg_t * const uart_reg_ram = (volatile struct uart_reg_t *)UART0_BASE;
    int_id = uart_reg_ram->u3.iir.int_id;

    if(int_id == 0x04 || int_id == 0x0c )   /* Receiver data available or Character time-out indication */
    {
        if(hci_test())
            return;
        else{
            while(uart_reg_ram->lsr & 0x01)
            {
            	gvalue=uart_reg_ram->u1.data;
                Lite_ring_buffer_write_data(uart_config[UART_ID_0].uart_lr, (uint8*)&(gvalue), 1);				
				char str[8]={0};
				co_sprintf(str,"0x%x ",gvalue);
				for(int i=0;i<strlen(str);i++)
                	uart_putc_noint_no_wait(UART1,str[i]);
            }  
        }

    }
    else if(int_id == 0x06)
    {
        uart_reg_ram->lsr = uart_reg_ram->lsr;
    }
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
__attribute__((section("ram_code"))) void uart1_isr_ram(void)
{
    uint8_t int_id;
    volatile struct uart_reg_t * const uart_reg_ram = (volatile struct uart_reg_t *)UART1_BASE;
    int_id = uart_reg_ram->u3.iir.int_id;

    if(int_id == 0x04 || int_id == 0x0c )   /* Receiver data available or Character time-out indication */
    {
        while(uart_reg_ram->lsr & 0x01)
        {
            Lite_ring_buffer_write_data(uart_config[UART_ID_1].uart_lr, (uint8*)&(uart_reg_ram->u1.data), 1);
            //uart_putc_noint_no_wait(UART1,uart_reg_ram->u1.data);
            
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

int32 mid_uart_init(uint8 u_id,int8 baud_rate)
{
	static uint8 first_init_flag=0;
	if(u_id>=UART_MAX){
        GOVEE_PRINT(LOG_ERROR, "Uart %d ring id error.\r\n",u_id);
        return -1;  
    }
	if(first_init_flag == 0){
		first_init_flag=1;
		
	    uart_config[u_id].uart_lr = Lite_ring_buffer_init(UART_RX_BUFFER_SIZE);
	    if (uart_config[u_id].uart_lr == NULL)
	    {
	        GOVEE_PRINT(LOG_ERROR, "Uart %d ring buffer init failed.\r\n",u_id);
	        return -1;
	    }
	    
	}
    if(u_id==UART_ID_0){
        system_set_port_pull(PIN_PORT_PIN(g_mid_uart0.RX.GPIOx,g_mid_uart0.RX.GPIO_Pin_x),1);
        system_set_port_mux(g_mid_uart0.RX.GPIOx, g_mid_uart0.RX.GPIO_Pin_x, g_mid_uart0.RX.GPIO_Func);            //RX
        system_set_port_mux(g_mid_uart0.TX.GPIOx, g_mid_uart0.TX.GPIO_Pin_x, g_mid_uart0.TX.GPIO_Func);          //TX

        uart_init(uart_config[u_id].uart_port, baud_rate);
    }
   //uart_init_x(gAT_buff_env.uart_param);
   
    NVIC_EnableIRQ(uart_config[u_id].uart_irqn); 
   
   // GOVEE_PRINT(LOG_DEBUG,"mid_uart_init  %d %d\r\n",u_id ,baud_rate);
    uart_write(uart_config[u_id].uart_port,"hello,world",11);

    return 0;
}

void mid_uart_for_hci_test(void)
{
    return;
}
