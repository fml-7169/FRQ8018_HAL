/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdio.h>
#include <string.h>

#include "gap_api.h"
#include "gatt_api.h"
#include "os_task.h"
#include "os_msg_q.h"

#include "os_timer.h"
#include "os_mem.h"
#include "sys_utils.h"
#include "jump_table.h"
#include "driver_plf.h"
#include "driver_system.h"
#include "driver_i2s.h"
#include "driver_pmu.h"
#include "driver_uart.h"
#include "driver_rtc.h"
#include "driver_flash.h"
#include "driver_efuse.h"
#include "flash_usage_config.h"
#include "middle_uart.h"
#include "hci_test.h"

#define GOVEE_SW_VERSION                    "2.00.05"
#define GOVEE_HW_VERSION                    "1.03.01"

enum EVT_STATUS
{
    EVT_AT,
    EVT_HCI,
};
enum
{
    MAC_ADDR_WRITE,
    MAC_ADDR_READ,
    SKU_WRITE,
    SKU_READ,
    VER_READ,
    EARSE_TESt,
    HCI_TX_SINGLE_POWER,
    FREQ_ADJUST,
};
typedef void (*rwip_eif_callback) (void*, uint8_t);

struct uart_txrxchannel
{
    /// call back function pointer
    rwip_eif_callback callback;
};

struct uart_env_tag
{
    /// rx channel
    struct uart_txrxchannel rx;
    uint32_t rxsize;
    uint8_t *rxbufptr;
    void *dummy;
    /// error detect
    uint8_t errordetect;
    /// external wakeup
    bool ext_wakeup;
};

struct user_uart_recv_t //´®¿Ú½ÓÊÕÊý¾Ý½á¹¹Ìå
{
  uint8_t length;
  uint8_t indx;
  uint8_t recv_data[255];
  uint8_t start_flag;
  uint8_t finish_flag;
};

typedef struct _govee_sku_config_t
{
    uint8_t sku[GOVEE_SKU_MAX_LEN +1];
    uint8_t crc; 
} govee_sku_config_t;

struct user_uart_recv_t uart_recv = {
    .length = 0,
    .indx = 0,
    .recv_data = {0},
    .start_flag = 0,
    .finish_flag = 0,
};

struct dev_msg_init_t
{
    uint8_t dev_sw_version[GOVEE_VERSION_MAX_LEN];
    uint8_t dev_hw_version[GOVEE_VERSION_MAX_LEN];
};

struct lld_test_params
{
    /// Type (0: RX | 1: TX)
    uint8_t type;

    /// RF channel, N = (F - 2402) / 2
    uint8_t channel;

    /// Length of test data
    uint8_t data_len;

    /**
     * Packet payload
     * 0x00 PRBS9 sequence "11111111100000111101" (in transmission order) as described in [Vol 6] Part F, Section 4.1.5
     * 0x01 Repeated "11110000" (in transmission order) sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x02 Repeated "10101010" (in transmission order) sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x03 PRBS15 sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x04 Repeated "11111111" (in transmission order) sequence
     * 0x05 Repeated "00000000" (in transmission order) sequence
     * 0x06 Repeated "00001111" (in transmission order) sequence
     * 0x07 Repeated "01010101" (in transmission order) sequence
     * 0x08-0xFF Reserved for future use
     */
    uint8_t payload;

    /**
     * Tx/Rx PHY
     * For Tx PHY:
     * 0x00 Reserved for future use
     * 0x01 LE 1M PHY
     * 0x02 LE 2M PHY
     * 0x03 LE Coded PHY with S=8 data coding
     * 0x04 LE Coded PHY with S=2 data coding
     * 0x05-0xFF Reserved for future use
     * For Rx PHY:
     * 0x00 Reserved for future use
     * 0x01 LE 1M PHY
     * 0x02 LE 2M PHY
     * 0x03 LE Coded PHY
     * 0x04-0xFF Reserved for future use
     */
    uint8_t phy;
};


enum EVT_STATUS evt_status = EVT_HCI;
//os_timer_t evt_at_timeout;
uint16_t user_at_id;
uint8_t tx_power_mode = 0;
struct dev_msg_init_t dev_msg_init_p;
uint8_t freq_adjust_tab[0x0f] = {10,9,9,8,7,7,6,6,6,6,6,5,5,5,5};
uint8_t cur_freq_adjust_val = 0;

extern uint8_t lld_test_start(struct lld_test_params* params);
extern uint8_t lld_test_stop(void);
extern void flash_OTP_erase(uint32_t offset);
extern void flash_OTP_write(uint32_t offset, uint32_t length, uint8_t *buffer);

uint8_t chargovee_sku_crc_check(uint8_t * sku,uint8_t len)
{
    uint8_t check_sum  = 0x00;
    uint8_t i = 0x00;
    for (i = 0; i < len; i++)
    {
        check_sum ^= sku[i];
    }	
    return 	check_sum ;
}

static int dev_flash_write_data(uint32_t addr, uint8_t* data, uint32_t size)
{
    uint8_t packet[GOVEE_FLASH_DATA_BLOCK_SIZE] = {0};
    uint8_t buff[32] = {0};
    uint8_t i = 0;

    if (size > GOVEE_FLASH_DATA_BLOCK_SIZE - 1)
    {
        co_printf( "Invalid size, must less than %d.\r\n", GOVEE_FLASH_DATA_BLOCK_SIZE - 1);
        return -1;
    }

    for(i = 0; i < GOVEE_FLASH_DATA_BLOCK_COUNT; i++)
    {
        flash_read(addr + i*GOVEE_FLASH_DATA_BLOCK_SIZE, 32, buff);
        if (buff[0] == 0x5a)  // this block has been used
        {
            buff[0] = 0;
            flash_write(addr + i*GOVEE_FLASH_DATA_BLOCK_SIZE, 1, buff); // write 1st byte '0' to override this tag
        }
        else if (buff[0] == 0xff) // this block has been erased and not been used;
        {
            break;
        }
    }

    if (i == GOVEE_FLASH_DATA_BLOCK_COUNT) /* have not searched valid space,then erase this sector and write env data to 1st block; */
    {
        flash_erase(addr, 0x1000);
        packet[0] = 0x5a;
        memcpy(packet + 1, data, size);
        flash_write(addr, size + 1, packet);
    }
    else /* write env data to the i block */
    {
        packet[0] = 0x5a;
        memcpy(packet + 1, data, size);
        flash_write(addr + i*GOVEE_FLASH_DATA_BLOCK_SIZE, size + 1, packet);
    }

    return 0;
}

static int dev_flash_read_data(uint32_t addr, uint8_t* data, uint32_t size)
{
    uint8_t buffer[GOVEE_FLASH_DATA_BLOCK_SIZE] = {0};
    uint8_t i = 0;

    if (NULL == data || size > GOVEE_FLASH_DATA_BLOCK_SIZE - 1)
    {
        co_printf( "Invalid parameter.\r\n");
        return -1;
    }

    for (i = 0; i < GOVEE_FLASH_DATA_BLOCK_COUNT; i++)
    {
        memset(buffer, 0, GOVEE_FLASH_DATA_BLOCK_SIZE);
        flash_read(addr + i*GOVEE_FLASH_DATA_BLOCK_SIZE, size + 1, buffer);
        if (buffer[0] == 0x5a || buffer[0] == 0xff)  // valid env data or have not stored evv_data
        {
            memcpy(data, buffer + 1, size);
            break;
        }
    }
    return 0;
}

static void dev_freq_adjust_store(uint8_t adjust_val)
{
    uint8_t store_data[10] = {'f','r','e','q',0xff,0xff,'c','h','i','p'};
    
    store_data[4] = adjust_val;
    store_data[5] = adjust_val^0xff;
    flash_OTP_erase(0x1000);
    flash_OTP_write(0x1000,sizeof(store_data),store_data);
    flash_OTP_erase(0x2000);
    flash_OTP_write(0x2000,sizeof(store_data),store_data);
}

void dev_freq_adjust_check(void)
{
    uint8_t get_data[10] = {0};
    uint8_t get_data1[10] = {0};
    uint8_t check_temp = 0;
    flash_OTP_read(0x1000,sizeof(get_data),get_data);
    flash_OTP_read(0x2000,sizeof(get_data1),get_data1);

    if(!memcmp(get_data,"freq",4) && !memcmp(&get_data[6],"chip",4))
    {
//        uart_putc_noint_no_wait(UART1,0xf0);
        check_temp = get_data[4]^0xff;
        if(check_temp == get_data[5])
            cur_freq_adjust_val = get_data[4];
    }
    else if(!memcmp(get_data1,"freq",4) && !memcmp(&get_data1[6],"chip",4))
    {
//        uart_putc_noint_no_wait(UART1,0xf1);
        check_temp = get_data1[4]^0xff;
        if(check_temp == get_data1[5])
            cur_freq_adjust_val = get_data1[4];
    }
    
    if(cur_freq_adjust_val && (cur_freq_adjust_val < 0x10))
    {
        //co_printf("=freq aujust=%d\r\n",cur_freq_adjust_val);
        //uart_putc_noint_no_wait(UART1,cur_freq_adjust_val);
        if(cur_freq_adjust_val < 8)
            ool_write(0x10,cur_freq_adjust_val);
        else
        {
            ool_write(0x10,7);
            ool_write(0x10,cur_freq_adjust_val);
        }
    }
}

static int user_at_func(os_event_t *param)
{
    uint8_t rsp_data[32] = {0x01,0xE0,0xFC};
    uint8_t *buff = param->param;
    uint8_t i = 0;
    
    switch(buff[0])
    {
        case MAC_ADDR_WRITE:
//            flash_erase(HCI_TEST_MAC_ADDR,0);
//            dev_flash_write_data(HCI_TEST_MAC_ADDR,buff+1,6);
            rsp_data[3] = 7;
            rsp_data[4] = 0x60;
            memcpy(rsp_data+5,buff+1,6);
            uart_write(UART0,rsp_data,(rsp_data[3]+4));
        break;
        case MAC_ADDR_READ:
        {
//            uint8_t mac_addr[6];
//            flash_read(HCI_TEST_MAC_ADDR,6,mac_addr);
//            dev_flash_read_data(HCI_TEST_MAC_ADDR,mac_addr,6);
            mac_addr_t addr;
            gap_address_get(&addr);
            rsp_data[3] = 7;
            rsp_data[4] = 0x61;
            for(i = 0;i < 6;i++)
                rsp_data[5+i] = addr.addr[5-i];
            //memcpy(rsp_data+5,addr.addr,6);
            uart_write(UART0,rsp_data,(rsp_data[3]+4));
        }
        break;
        case SKU_WRITE:
        {
            uint8_t sku_data[7] = {0};

            memcpy(sku_data,buff+1,GOVEE_SKU_MAX_LEN+1);
            sku_data[6] =  chargovee_sku_crc_check(buff+1,GOVEE_SKU_MAX_LEN);
            //co_printf("=sku val=%x\r\n",sku_data[6]); 
            flash_erase(HCI_TEST_SKU_BASE,0);
            flash_erase(HCI_TEST_SKU_BACK,0);

//            flash_write(HCI_TEST_SKU_BASE,GOVEE_SKU_MAX_LEN+2,sku_data);
//            flash_write(HCI_TEST_SKU_BACK,GOVEE_SKU_MAX_LEN+2,sku_data);
            dev_flash_write_data(HCI_TEST_SKU_BASE,sku_data,GOVEE_SKU_MAX_LEN+2);
            dev_flash_write_data(HCI_TEST_SKU_BACK,sku_data,GOVEE_SKU_MAX_LEN+2);
            rsp_data[3] = 0x01;
            rsp_data[4] = 0x62;
            uart_write(UART0,rsp_data,(rsp_data[3]+4));
        }
        break;
        
        case SKU_READ:
        {
            uint8_t sku_data[7];
            rsp_data[3] = 0x06;
            rsp_data[4] = 0x63;
//            flash_read(HCI_TEST_SKU_BASE,7,sku_data);
            dev_flash_read_data(HCI_TEST_SKU_BASE,sku_data,7);
            memcpy(rsp_data+5,sku_data,GOVEE_SKU_MAX_LEN);
            //show_reg(sku_data,7,1);
            uart_write(UART0,rsp_data,(rsp_data[3]+4));
        }
        break; 
        case VER_READ:
            rsp_data[3] = 1+2*GOVEE_VERSION_MAX_LEN;
            rsp_data[4] = 0x64;
            memcpy(&rsp_data[5],dev_msg_init_p.dev_sw_version,GOVEE_VERSION_MAX_LEN);
            memcpy(&rsp_data[5+GOVEE_VERSION_MAX_LEN],dev_msg_init_p.dev_hw_version,GOVEE_VERSION_MAX_LEN);
            uart_write(UART0,rsp_data,(rsp_data[3]+4));
        break;
        case EARSE_TESt:
        {
            uint8_t buff1[7]={0};
//            flash_read(PCB_TEST,7,buff1);
            //printf("%s\r\n",buff);
           // flash_erase(PCB_TEST,0);
        }
        break;
        
        case HCI_TX_SINGLE_POWER:
            lld_test_stop();
            rsp_data[3] = 0x02;
            rsp_data[4] = 0x66;
            if((buff[1] < RF_TX_POWER_MAX) && (buff[2] < 0x39))
            {
                //¿ÕÔØ²¨·¢ËÍ
                //example,AT#TAaa_bb_cc, aa±íÊ¾channel£¬bb±íÊ¾·¢ËÍpayload type£¬cc±íÊ¾·¢Éä¹¦ÂÊ     
                *(volatile uint32_t *)0x400000d0 = 0x80008000;
                *(volatile uint8_t *)(MODEM_BASE+0x19) = 0x0c;
                system_set_tx_power((enum rf_tx_power_t)buff[1]); // enum rf_tx_power_t tx_power
                struct lld_test_params test_params;
                test_params.type = 1;
                test_params.channel = buff[2];
                test_params.data_len = 0x25;
                test_params.payload = 0;
                test_params.phy = 1; //PHY_1MBPS_BIT;
                lld_test_start(&test_params);
                rsp_data[5] = 0x01;
                tx_power_mode = 1;
            }
            else
            {
                rsp_data[5] = 0x00;
            }
            uart_write(UART0,rsp_data,(rsp_data[3]+4));
            break;
        case FREQ_ADJUST:
            rsp_data[3] = 0x02;
            rsp_data[4] = 0x67;
            // {10,9,9,8,7,7,6,6,6,6,6,5,5,5,5};
            if((buff[1] <= 1) && (buff[2] <= 100))
            {
                uint8_t i = cur_freq_adjust_val,adjust_num = 0;//,j = 0;
                if(buff[1])
                {
                    for(;i < 0x0f;i++)
                    {
                        //for(j = 0;j < (i+1);j++)
                            adjust_num += freq_adjust_tab[i];
                        if(abs(adjust_num-buff[2]) < 10) // 10k
                        {
                            cur_freq_adjust_val = i+1;
                            break;
                        }
//                        else
//                            adjust_num = 0;
                    } 
                }
                else
                {
                    for(;i > 0;i--)
                    {
                        adjust_num += freq_adjust_tab[i-1];
                        if(abs(adjust_num-buff[2]) < 10) // 10k
                        {
                            cur_freq_adjust_val = i-1;
                            break;
                        }
                    }
                }
                if(cur_freq_adjust_val)
                {
                    dev_freq_adjust_store(cur_freq_adjust_val);
                    if(cur_freq_adjust_val < 8)
                        ool_write(0x10,cur_freq_adjust_val);
                    else
                    {
                        ool_write(0x10,7);
                        ool_write(0x10,cur_freq_adjust_val);
                    }
                    rsp_data[5] = 0x01;
                }
                else
                    rsp_data[5] = 0x00;
            }
            else
                rsp_data[5] = 0x00;
            uart_write(UART0,rsp_data,(rsp_data[3]+4));
            //uart_write(UART0,&cur_freq_adjust_val,1);
            break;
        default:
            
        break;
    }
   return EVT_CONSUMED;  
}

//static void evt_at_timeout_handle(void *param)
//{
//    if(uart_recv.start_flag)
//    {
//        uart_recv.indx = 0;
//        evt_status = EVT_HCI;
//        uart_recv.start_flag = 0;
//        uart_recv.finish_flag = 0;
//        os_timer_stop(&evt_at_timeout);
//    }  
//    else
//    {
//        uart_recv.start_flag = 1;
//    }
//}

#if 0
void dev_at_cmd_handle(uint8_t * cmd_data,uint8_t cmd_len)
{
    uint8_t check_flag= 0,i;
    os_event_t evt;
    
    if((cmd_len>3 )&&(cmd_len==cmd_data[2]+3))
    {
        if(cmd_data[1] == 0x0E)
        {
            os_timer_stop(&evt_at_timeout);

            uart_recv.finish_flag = 1;
            uart_recv.indx = 0;
            evt.param = cmd_data+3; //uart_recv.recv_data+3;
            evt.param_len = cmd_data[2];
            os_msg_post(user_at_id,&evt);
            evt_status = EVT_HCI;  
        } 
        else
        {
            os_timer_stop(&evt_at_timeout);
            uart_recv.indx = 0;
            uart_recv.start_flag = 0;
            uart_recv.finish_flag = 0;
            evt_status = EVT_HCI;
        }       
    }
}

void dev_hci_test_mode(uint8_t * cmd_data,uint8_t cmd_len)
{
    rwip_eif_callback callback;
    uint8_t c;
    void *dummy;
    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART0_BASE;
    struct uart_env_tag *uart0_env = (struct uart_env_tag *)0x20000a40;

   //AT TEST
    if((cmd_data[0] == 0x34) && (cmd_data[1] == 0x0E) && (evt_status == EVT_HCI))
    {
       evt_status = EVT_AT; 
    }
    
    if(evt_status == EVT_AT)
    {
        dev_at_cmd_handle(cmd_data,cmd_len);
    }
    //HCI TEST
    else
    {
        if(tx_power_mode)
            lld_test_stop();
        *uart0_env->rxbufptr++ = c;
        uart0_env->rxsize--;
        if((uart0_env->rxsize == 0)
           &&(uart0_env->rx.callback))
        {      
            uart_reg->u3.fcr.data = 0xf1;
            NVIC_DisableIRQ(UART0_IRQn);
            uart_reg->u3.fcr.data = 0x21;
            callback = uart0_env->rx.callback;
            dummy = uart0_env->dummy;
            uart0_env->rx.callback = 0;
            uart0_env->rxbufptr = 0;
            callback(dummy, 0);
        }
    }
}
#endif
#if 1
void user_uart_at(uint8_t c)
{
    os_event_t evt;
    
//    if(uart_recv.indx == 0)
//        os_timer_start(&evt_at_timeout,10,true);
    
    uart_recv.recv_data[uart_recv.indx++] = c;
    
    if((uart_recv.indx>3 )&&(uart_recv.indx>=uart_recv.recv_data[2]+3))
    {
       if(uart_recv.recv_data[1] == 0x0E)
       {
            uart_recv.length = uart_recv.recv_data[2];
            if(uart_recv.indx==uart_recv.length+3 )
            {
//                os_timer_stop(&evt_at_timeout);
                
                uart_recv.finish_flag = 1;
                uart_recv.indx = 0;
                evt.param = uart_recv.recv_data+3;
                evt.param_len = uart_recv.length;
                os_msg_post(user_at_id,&evt);
                evt_status = EVT_HCI;
            }
            else
            {
//                os_timer_stop(&evt_at_timeout);
                uart_recv.indx = 0;
                uart_recv.start_flag = 0;
                uart_recv.finish_flag = 0;
                evt_status = EVT_HCI;
            }   
        } 
        else
        {
//                os_timer_stop(&evt_at_timeout);
                uart_recv.indx = 0;
                uart_recv.start_flag = 0;
                uart_recv.finish_flag = 0;
                evt_status = EVT_HCI;
         }       
    }
}

uint8_t hci_test(void)
{
    rwip_eif_callback callback;
    uint8_t c;
    void *dummy;
    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART0_BASE;
    struct uart_env_tag *uart0_env = (struct uart_env_tag *)0x20000a40;
    
    if(__jump_table.system_option & SYSTEM_OPTION_ENABLE_HCI_MODE)
    {
        while(uart_reg->lsr & 0x01)
        {
            c = uart_reg->u1.data;
            //uart_putc_noint(UART0,c);
            uart_recv.start_flag = 0;
           //AT TEST
            if((c == 0x34) && (evt_status == EVT_HCI)/* && (uart0_env->rxsize == 1)*/)
            {
                evt_status = EVT_AT; 
            }

            if(evt_status == EVT_AT)
            {
                user_uart_at(c);           
            }
            //HCI TEST
            else
            {
                if(tx_power_mode == 0x01)
                {
                    lld_test_stop();
                    tx_power_mode = 0;
                }
                *uart0_env->rxbufptr++ = c;
                uart0_env->rxsize--;
                if((uart0_env->rxsize == 0)
                   &&(uart0_env->rx.callback))
                {      
                    uart_reg->u3.fcr.data = 0xf1;
                    NVIC_DisableIRQ(UART0_IRQn);
                    uart_reg->u3.fcr.data = 0x21;
                    callback = uart0_env->rx.callback;
                    dummy = uart0_env->dummy;
                    uart0_env->rx.callback = 0;
                    uart0_env->rxbufptr = 0;
                    callback(dummy, 0);
                    break;
                }
            }
        }
        return 1;
    }
    else
        return 0;
}
#endif

void dev_msg_init(uint8_t * s_version,uint8_t s_version_len,uint8_t * h_version,uint8_t h_version_len)
{
    if((s_version_len <= GOVEE_VERSION_MAX_LEN) && (h_version_len <= GOVEE_VERSION_MAX_LEN))
    {
        memcpy(dev_msg_init_p.dev_sw_version,s_version,s_version_len);
        memcpy(dev_msg_init_p.dev_hw_version,h_version,h_version_len);
    }
}
#if 0
__attribute__((section("ram_code"))) void uart0_isr_ram(void)
{
    uint8_t int_id;
    uint8_t c;
    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART0_BASE;

    int_id = uart_reg->u3.iir.int_id;

    if(int_id == 0x04 || int_id == 0x0c )   /* Receiver data available or Character time-out indication */
    {
        if(hci_test())
            return;
        else
        {
            c = uart_reg->u1.data;
            uart_putc_noint(UART0,c);
        }
    }
    else if(int_id == 0x06)
    {
        volatile uint32_t line_status = uart_reg->lsr;
    }
}
#endif
static int app_test_mode_serial_gets(uint16_t ms, uint8_t *data_buf, uint32_t buf_size)
{
    int i, n=0;

    for(i=0; i<ms; i++)
    {
        co_delay_100us(10);
        n += uart_get_data_nodelay_noint(UART0, data_buf+n, buf_size-n);
        if(n == buf_size)
        {
            return n;
        }
    }

    return -1;
}

uint8_t dev_check_hci_test_mode(void)
{
    uint8_t test_mode_get[5] = {0}; // RSP:0xBA,0xBA,0xBA,0xBA,0xBA
    uint8_t test_mode_send[5] = {0x34,0x0E,0x01,0x6A,0x01};
    uint8_t test_mode_check[] = {0x34,0x0E,0x02,0x0A,0x01};
    
    system_set_port_pull(GPIO_PA2, true);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_UART0_RXD);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_UART0_TXD);
    uart_init(UART0, BAUD_RATE_115200); 
    
    co_delay_100us(50);
    uart_put_data_noint(UART0,test_mode_send,4);
//    uart_finish_transfers(UART0);
    if(app_test_mode_serial_gets(50,test_mode_get,5) == 5) // wait 50ms
    {
        if(!memcmp(test_mode_get,test_mode_check,sizeof(test_mode_check)))
        {
            test_mode_send[2] = 0x02;
            uart_put_data_noint(UART0,test_mode_send,sizeof(test_mode_send));
            uart_finish_transfers(UART0);
            system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_A2);
            system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_A3);
            return 1;
        }
    }
    
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_A2);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_A3);
    return 0;
}

void govee_hci_test_init(void)
{

    system_set_port_pull(GPIO_PA2, true);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_UART0_RXD);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_UART0_TXD);
    
    system_set_port_pull(GPIO_PB2, true);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_2, PORTB2_FUNC_UART1_RXD);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_3, PORTB3_FUNC_UART1_TXD);
    
    uart_init(UART0, BAUD_RATE_115200);  
    NVIC_EnableIRQ(UART0_IRQn);
//    os_timer_init(&evt_at_timeout,evt_at_timeout_handle,NULL);
    user_at_id = os_task_create(user_at_func);
    mid_uart_for_hci_test();
}



