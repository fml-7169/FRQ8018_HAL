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
#include "driver_system.h"
#include "os_mem.h"
#include "driver_iic.h"
#include "middle_i2c.h"

/*
PD6     SCL
PD7     SDA
*/

int32 mid_i2c_init(uint16 rate)
{
    system_set_port_pull(GPIO_PD6, true);
    system_set_port_pull(GPIO_PD7, true);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_6, PORTD6_FUNC_I2C1_CLK);            //SCL
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_7, PORTD7_FUNC_I2C1_DAT);            //SDA

    iic_init(IIC_CHANNEL_1,rate,0x70);
    return 0;
}

void mid_i2c_write(uint8 addr, uint8 reg, uint8* buffer, uint32 len)
{
    iic_write_bytes(IIC_CHANNEL_1,addr,reg,buffer,(uint16_t)len&0xffff);
}

