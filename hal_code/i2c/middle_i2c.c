/*
 * middle_uart.c
 *
 *  Created on: 2020-9-17
 *      Author: Weili.Hu
 */


#include <string.h>
#include <stdlib.h>
#include "sys_utils.h"
#include "Lite-Rbuffer.h"
#include "middle_uart.h"
#include "govee_log.h"
#include "app_bridge.h"
#include "govee_utils.h"
#include "driver_system.h"
#include "os_mem.h"
#include "driver_iic.h"
#include "middle_i2c.h"
#include "driver_gpio.h"

#if _I2C_USE_HARD_
/*
PD6     SCL
PD7     SDA
*/

int32 mid_i2c_init(uint16 rate)
{
    system_set_port_pull(GPIO_PD6, 1);
    system_set_port_pull(GPIO_PD7, 1);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_6, PORTD6_FUNC_I2C1_CLK);            //SCL
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_7, PORTD7_FUNC_I2C1_DAT);            //SDA

    iic_init(IIC_CHANNEL_1,rate,0x70);
    return 0;
}

void mid_i2c_write(uint8 addr, uint8 reg, uint8* buffer, uint32 len)
{
    iic_write_bytes(IIC_CHANNEL_1,addr,reg,buffer,(uint16_t)len&0xffff);
}


#else

#define GPIO_I2C_CLK_PORT       GPIO_PORT_A
#define GPIO_I2C_CLK_PIN        GPIO_BIT_6

#define GPIO_I2C_DAT_PORT       GPIO_PORT_A   
#define GPIO_I2C_DAT_PIN        GPIO_BIT_7

#define SCL_SET_HIGH()      gpio_set_pin_value(GPIO_I2C_CLK_PORT,GPIO_I2C_CLK_PIN,1)
#define SCL_SET_LOW()       gpio_set_pin_value(GPIO_I2C_CLK_PORT,GPIO_I2C_CLK_PIN,0)
#define SCL_GET_VAL()       gpio_get_pin_value(GPIO_I2C_CLK_PORT,GPIO_I2C_CLK_PIN)
#define SCL_SET_OUTPUT()    gpio_set_dir(GPIO_I2C_CLK_PORT,GPIO_I2C_CLK_PIN,GPIO_DIR_OUT)
#define SCL_SET_INPUT()     gpio_set_dir(GPIO_I2C_CLK_PORT,GPIO_I2C_CLK_PIN,GPIO_DIR_IN)

#define SDA_SET_HIGH()      gpio_set_pin_value(GPIO_I2C_DAT_PORT,GPIO_I2C_DAT_PIN,1)
#define SDA_SET_LOW()       gpio_set_pin_value(GPIO_I2C_DAT_PORT,GPIO_I2C_DAT_PIN,0)
#define SDA_GET_VAL()       gpio_get_pin_value(GPIO_I2C_DAT_PORT,GPIO_I2C_DAT_PIN)
#define SDA_SET_OUTPUT()    gpio_set_dir(GPIO_I2C_DAT_PORT,GPIO_I2C_DAT_PIN,GPIO_DIR_OUT)
#define SDA_SET_INPUT()     gpio_set_dir(GPIO_I2C_DAT_PORT,GPIO_I2C_DAT_PIN,GPIO_DIR_IN)

#define I2C_DELAY()         {__NOP(); __NOP();}//{__NOP();__NOP();__NOP();__NOP();}

#define I2C_ACK_OP()        SCL_SET_HIGH();I2C_DELAY();I2C_DELAY();I2C_DELAY();I2C_DELAY();I2C_DELAY();\
                            I2C_DELAY();I2C_DELAY();I2C_DELAY();I2C_DELAY();I2C_DELAY();SCL_SET_LOW()//;I2C_DELAY()
#define SDA_SET_HIGH_EXT()  SDA_SET_HIGH();I2C_ACK_OP()
#define SDA_SET_LOW_EXT()   SDA_SET_LOW();I2C_ACK_OP()

static void i2c_start(void)
{
    // Make sure both SDA and SCL high.
    SDA_SET_HIGH();
    I2C_DELAY();
    SCL_SET_HIGH();
    I2C_DELAY();
    
    // SDA: \__
    SDA_SET_LOW();
    I2C_DELAY();

    // SCL: \__
    SCL_SET_LOW();//delay_us(2);
    I2C_DELAY();
}

static void i2c_stop(void)
{
    // Make sure SDA low.
    SDA_SET_LOW();
    I2C_DELAY();

    // SCL: __/
    SCL_SET_HIGH();
    I2C_DELAY();
    // SDA: __/
    SDA_SET_HIGH();
    I2C_DELAY();
}

static void i2c_read_byte(uint8 *data, uint8 ack)
{
    //uint8 ret = 1;
    uint8 i, byte_read = 0;

    // Before call the function, SCL should be low.

    // Make sure SDA is an input
    SDA_SET_INPUT();

    // MSB first
    for (i = 0x80; i != 0; i >>= 1) 
    {
        SCL_SET_HIGH();
        I2C_DELAY();

        if (1==(uint8)(SDA_GET_VAL())) 
        {
            byte_read |= i;
        }

        SCL_SET_LOW();
        I2C_DELAY();
    }

    // Make sure SDA is an output before we exit the function
    SDA_SET_OUTPUT();

    *data = byte_read;

    // Send ACK bit
    // SDA high == NACK, SDA low == ACK
    if (ack) 
    {
        SDA_SET_LOW();
    }
    else 
    {
        SDA_SET_HIGH();
    }

    // Let SDA line settle for a moment
    I2C_DELAY();

    // Drive SCL high to start ACK/NACK bit transfer
    SCL_SET_HIGH();
    I2C_DELAY();

    // Finish ACK/NACK bit clock cycle and give slave a moment to react
    SCL_SET_LOW();
    I2C_DELAY();
}


static uint8 i2c_write_byte(uint8 data)
{
    uint8 ret = 1;
    uint8 i;

    for (i = 0x80; i != 0; i >>= 1)
    {
        if (data & i) 
        {
            SDA_SET_HIGH();
        }
        else 
        {
            SDA_SET_LOW();
        }
        I2C_DELAY();

        SCL_SET_HIGH();
        I2C_DELAY();
        I2C_DELAY();
        I2C_DELAY();
        I2C_DELAY();
        SCL_SET_LOW();
        I2C_DELAY();
    }

    // Configure SDA pin as input for receiving the ACK bit
    //SDA_SET_HIGH();
    SDA_SET_INPUT();
    I2C_DELAY();
    SCL_SET_HIGH();

    //raed ack
    if (1==(uint8)(SDA_GET_VAL()))
    {
        ret = 0;
    }

    // Finish ACK/NACK bit clock cycle and give slave a moment to release control
    // of the SDA line
    SCL_SET_LOW();//delay_us(2);
    I2C_DELAY();

    // Configure SDA pin as output as other module functions expect that
    SDA_SET_HIGH();
    SDA_SET_OUTPUT();
    
    return ret;
}

static uint8 i2c_write_byte_ext(uint8 data)
{
    uint8 ret = 1;

    // Before call the function, SCL should be low.
    if (  data & 0x80 )
    {
        SDA_SET_HIGH_EXT();
    }
    else
    {
        SDA_SET_LOW_EXT();
    }
    if (  data & 0x40 )
    {
        SDA_SET_HIGH_EXT();
    }
    else
    {
        SDA_SET_LOW_EXT();
    }
    if (  data & 0x20 )
    {
        SDA_SET_HIGH_EXT();
    }
    else
    {
        SDA_SET_LOW_EXT();
    }
        if (  data & 0x10 )
    {
        SDA_SET_HIGH_EXT();
    }
    else
    {
        SDA_SET_LOW_EXT();
    }
    if (  data & 0x08 )
    {
        SDA_SET_HIGH_EXT();
    }
    else
    {
        SDA_SET_LOW_EXT();
    }
    if (  data & 0x04 )
    {
        SDA_SET_HIGH_EXT();
    }
    else
    {
        SDA_SET_LOW_EXT();
    }
    if (  data & 0x02 )
    {
        SDA_SET_HIGH_EXT();
    }
    else
    {
        SDA_SET_LOW_EXT();
    }
    if (  data & 0x01 )
    {
        SDA_SET_HIGH_EXT();
    }
    else
    {
        SDA_SET_LOW_EXT();
    }
    
    // Configure SDA pin as input for receiving the ACK bit
    SDA_SET_HIGH();
    SDA_SET_INPUT();
    //I2C_DELAY();
    SCL_SET_HIGH();

    //raed ack
    if (1==(uint8)(SDA_GET_VAL()))
    {
        ret = 0;
    }

    // Finish ACK/NACK bit clock cycle and give slave a moment to release control
    // of the SDA line
    SCL_SET_LOW();//delay_us(2);
    I2C_DELAY();

    // Configure SDA pin as output as other module functions expect that
    SDA_SET_HIGH();
    SDA_SET_OUTPUT();
    
    return ret;
}

void drv_i2c_write_gpio(uint8 i2cAddr, uint8 reg, uint8 *pBuf, uint16_t len)
{
    i2c_start();
    i2c_write_byte_ext(i2cAddr);
    i2c_write_byte_ext(0);
    i2c_write_byte_ext(reg);

    while (len --)
    {
        //i2c_write_byte(*pBuf ++);
        i2c_write_byte_ext(*pBuf ++);
    }

    i2c_stop();
}


void drv_i2c_read_gpio(uint8 i2cAddr, uint8 reg, uint8 *pBuf, uint8 len)
{
    i2c_start();
    i2c_write_byte(i2cAddr);
    i2c_write_byte(reg);

    i2c_start();
    i2c_write_byte(i2cAddr + 1);

    while (1)
    {
        if (len <= 1) 
        {
            i2c_read_byte(pBuf, 0);
            break;
        }
        else 
        {
            i2c_read_byte(pBuf, 1);
            len --;
        }
        
        pBuf ++;
        
        co_delay_10us(1);
    }
    
  i2c_stop();
}

#define PORT_FUNC_GPIO 0x00
int32 mid_i2c_init(uint16 rate)
{
    system_set_port_mux(GPIO_I2C_CLK_PORT,GPIO_I2C_CLK_PIN,PORT_FUNC_GPIO);
    gpio_set_dir(GPIO_I2C_CLK_PORT,GPIO_I2C_CLK_PIN,GPIO_DIR_OUT);
    gpio_set_pin_value(GPIO_I2C_CLK_PORT,GPIO_I2C_CLK_PIN,1);   

    system_set_port_mux(GPIO_I2C_DAT_PORT,GPIO_I2C_DAT_PIN,PORT_FUNC_GPIO);
    gpio_set_dir(GPIO_I2C_DAT_PORT,GPIO_I2C_DAT_PIN,GPIO_DIR_OUT);
    gpio_set_pin_value(GPIO_I2C_DAT_PORT,GPIO_I2C_DAT_PIN,1);   
    return 0;
}

void mid_i2c_write(uint8 addr, uint8 reg, uint8* buffer, uint32 len)
{
    drv_i2c_write_gpio(addr,reg,buffer,(uint16_t)len&0xffff);
}
#endif

