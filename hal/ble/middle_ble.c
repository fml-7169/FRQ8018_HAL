#include <stdbool.h>
#include "gap_api.h"
#include "gatt_api.h"
#include "driver_gpio.h"
#include "simple_gatt_service.h"
#include "ble_simple_peripheral.h"

#include <string.h>
#include <stdio.h>
#include "middle_ble.h"
#include "govee_log.h"
#include "Lite-Rbuffer.h"

/*
 * INCLUDES (包含头文件)
 */
#include <stdio.h>
#include <string.h>
#include "co_printf.h"
#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"

#include "simple_gatt_service.h"


/*
 * MACROS (宏定义)
 */

/*
 * CONSTANTS (常量定义)
 */


// Simple GATT Profile Service UUID: 0xFFF0
const uint8_t sp_svc_uuid[] = UUID16_ARR(SP_SVC_UUID);

/******************************* Characteristic 1 defination *******************************/
// Characteristic 1 UUID: 0xFFF1
// Characteristic 1 data 
#define SP_CHAR1_VALUE_LEN  10
uint8_t sp_char1_value[SP_CHAR1_VALUE_LEN] = {0};
// Characteristic 1 User Description
#define SP_CHAR1_DESC_LEN   17
const uint8_t sp_char1_desc[SP_CHAR1_DESC_LEN] = "Characteristic 1";

/******************************* Characteristic 2 defination *******************************/
// Characteristic 2 UUID: 0xFFF2
// Characteristic 2 data 
#define SP_CHAR2_VALUE_LEN  20
uint8_t sp_char2_value[SP_CHAR2_VALUE_LEN] = {0};
// Characteristic 2 User Description
#define SP_CHAR2_DESC_LEN   17
const uint8_t sp_char2_desc[SP_CHAR2_DESC_LEN] = "Characteristic 2";

/******************************* Characteristic 3 defination *******************************/
// Characteristic 3 UUID: 0xFFF3
// Characteristic 3 data 
#define SP_CHAR3_VALUE_LEN  30
uint8_t sp_char3_value[SP_CHAR3_VALUE_LEN] = {0};
// Characteristic 3 User Description
#define SP_CHAR3_DESC_LEN   17
const uint8_t sp_char3_desc[SP_CHAR3_DESC_LEN] = "Characteristic 3";

/******************************* Characteristic 4 defination *******************************/
// Characteristic 4 UUID: 0xFFF4
// Characteristic 4 data 
#define SP_CHAR4_VALUE_LEN  40
uint8_t sp_char4_value[SP_CHAR4_VALUE_LEN] = {0};
// Characteristic 4 client characteristic configuration
#define SP_CHAR4_CCC_LEN   2
uint8_t sp_char4_ccc[SP_CHAR4_CCC_LEN] = {0};
// Characteristic 4 User Description
#define SP_CHAR4_DESC_LEN   17
const uint8_t sp_char4_desc[SP_CHAR4_DESC_LEN] = "Characteristic 4";

/******************************* Characteristic 5 defination *******************************/
// Characteristic 5 UUID: 0xFFF5
uint8_t sp_char5_uuid[UUID_SIZE_2] =
{ 
  LO_UINT16(SP_CHAR5_UUID), HI_UINT16(SP_CHAR5_UUID)
};
// Characteristic 5 data 
#define SP_CHAR5_VALUE_LEN  50
uint8_t sp_char5_value[SP_CHAR5_VALUE_LEN] = {0};
// Characteristic 5 User Description
#define SP_CHAR5_DESC_LEN   17
const uint8_t sp_char5_desc[SP_CHAR5_DESC_LEN] = "Characteristic 5";

/*
 * TYPEDEFS (类型定义)
 */

/*
 * GLOBAL VARIABLES (全局变量)
 */
uint8_t sp_svc_id = 0;


/*
 * LOCAL VARIABLES (本地变量)
 */
static gatt_service_t simple_profile_svc;


#include "sys_utils.h"
#include "flash_usage_config.h"


#define BLE_GATT_MSG_BUFFER_SIZE    (sizeof(msg_packet_t)*10)


static uint8 g_adv_data[] = {0x11, 0x09, 'G','o','v','e','e','_','H','9','9','9','9','_','0','0','0','0',
                             0x07, 0xff, 0x01, 0x88, 0xec,0x00, 0x01, 0x01,
                             0x02, 0x01, 0x05};
static uint8 g_adv_resp[] = {0x11, 0x09, 'G','o','v','e','e','_','H','9','9','9','9','_','0','0','0','0'};
static uint8 g_server_uuid[16] = {0x10, 0x19, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00};
static uint8 g_tx_uuid[16] = {0x10, 0x2B, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00};
static uint8 g_rx_uuid[16] = {0x11, 0x2B, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00};


static LR_handler gt_ble_lr = NULL;
static uint8 g_ble_event = 0;


/*********************************************************************
 * Profile Attributes - Table
 * 每一项都是一个attribute的定义。
 * 第一个attribute为Service 的的定义。
 * 每一个特征值(characteristic)的定义，都至少包含三个attribute的定义；
 * 1. 特征值声明(Characteristic Declaration)
 * 2. 特征值的值(Characteristic value)
 * 3. 特征值描述符(Characteristic description)
 * 如果有notification 或者indication 的功能，则会包含四个attribute的定义，除了前面定义的三个，还会有一个特征值客户端配置(client characteristic configuration)。
 *
 */

const gatt_attribute_t simple_profile_att_table[SP_IDX_NB] =
{
    // Simple gatt Service Declaration
    [SP_IDX_SERVICE]                        =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_PRIMARY_SERVICE_UUID) },     /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    UUID_SIZE_2,                                                /* Max size of the value */     /* Service UUID size in service declaration */
                                                    (uint8_t*)sp_svc_uuid,                                      /* Value of the attribute */    /* Service UUID value in service declaration */
                                                },

        // Characteristic 1 Declaration           
        [SP_IDX_CHAR1_DECLARATION]          =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    0,                                                          /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */
                                                },
        // Characteristic 1 Value                  
        [SP_IDX_CHAR1_VALUE]                =   {
                                                    { UUID_SIZE_2, UUID16_ARR(SP_CHAR1_UUID) },                 /* UUID */
                                                    GATT_PROP_READ | GATT_PROP_WRITE,                           /* Permissions */
                                                    SP_CHAR1_VALUE_LEN,                                         /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                },             
        // Characteristic 1 User Description
        [SP_IDX_CHAR1_USER_DESCRIPTION]     =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },      /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    SP_CHAR1_DESC_LEN,                                          /* Max size of the value */
                                                    (uint8_t *)sp_char1_desc,                                   /* Value of the attribute */
                                                },


        // Characteristic 2 Declaration
        [SP_IDX_CHAR2_DECLARATION]          =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    0,                                                          /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */
                                                },
        // Characteristic 2 Value   
        [SP_IDX_CHAR2_VALUE]                =   {
                                                    { UUID_SIZE_2, UUID16_ARR(SP_CHAR2_UUID) },                 /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    SP_CHAR2_VALUE_LEN,                                         /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                },   
        // Characteristic 2 User Description
        [SP_IDX_CHAR2_USER_DESCRIPTION]     =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },       /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    SP_CHAR2_DESC_LEN,                                          /* Max size of the value */
                                                    (uint8_t *)sp_char2_desc,                                   /* Value of the attribute */
                                                },


        // Characteristic 3 Declaration
        [SP_IDX_CHAR3_DECLARATION]          =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    0,                                                          /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */
                                                },
        // Characteristic 3 Value
        [SP_IDX_CHAR3_VALUE]                =   {
                                                    { UUID_SIZE_2, UUID16_ARR(SP_CHAR3_UUID) },                 /* UUID */
                                                    GATT_PROP_WRITE,                                            /* Permissions */
                                                    SP_CHAR3_VALUE_LEN,                                         /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                },
        // Characteristic 3 User Description
        [SP_IDX_CHAR3_USER_DESCRIPTION]     =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },      /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    SP_CHAR3_DESC_LEN,                                          /* Max size of the value */
                                                    (uint8_t *)sp_char3_desc,                                   /* Value of the attribute */
                                                },


        // Characteristic 4 Declaration
        [SP_IDX_CHAR4_DECLARATION]          =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    0,                                                          /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */
                                                },
        // Characteristic 4 Value
        [SP_IDX_CHAR4_VALUE]                =   {
                                                    { UUID_SIZE_2, UUID16_ARR(SP_CHAR4_UUID) },                 /* UUID */
                                                    GATT_PROP_WRITE | GATT_PROP_NOTI,                           /* Permissions */
                                                    SP_CHAR4_VALUE_LEN,                                         /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                },  
        // Characteristic 4 client characteristic configuration
        [SP_IDX_CHAR4_CFG]                  =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID) },     /* UUID */
                                                    GATT_PROP_READ | GATT_PROP_WRITE,                           /* Permissions */
                                                    SP_CHAR4_CCC_LEN,                                           /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                }, 
        // Characteristic 4 User Description
        [SP_IDX_CHAR4_USER_DESCRIPTION]     =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },      /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    SP_CHAR4_DESC_LEN,                                          /* Max size of the value */
                                                    (uint8_t *)sp_char4_desc,                                   /* Value of the attribute */
                                                },


        // Characteristic 5 Declaration 
        [SP_IDX_CHAR5_DECLARATION]          =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    0,                                                          /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */
                                                },
        // Characteristic 5 Value
        [SP_IDX_CHAR5_VALUE]                =   {
                                                    { UUID_SIZE_2, UUID16_ARR(SP_CHAR5_UUID) },                 /* UUID */
                                                    GATT_PROP_AUTHEN_WRITE | GATT_PROP_AUTHEN_READ,             /* Permissions */
                                                    SP_CHAR5_VALUE_LEN,                                         /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                },
        // Characteristic 5 User Description
        [SP_IDX_CHAR5_USER_DESCRIPTION]     =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },      /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    SP_CHAR5_DESC_LEN,                                          /* Max size of the value */
                                                    (uint8_t *)sp_char5_desc,                                   /* Value of the attribute */
                                                },
};



static uint8 ble_check_sum(uint8* p_data, uint32 length)
{
    int i = 0;
    uint8 check_sum = 0;

    if (p_data != NULL)
    {
        for(i = 0; i < length; i++)
        {
            check_sum ^= p_data[i];
        }
    }

    return check_sum;
}

static int32 ble_msg_write(ble_msg_t* pt_message)
{

}

static void ble_msg_callback(uint8* p_data, uint32 data_len)
{

}

static void ble_event_callback(uint8 type, void* args)
{

}

uint8 mid_ble_check_sum(uint8* p_data, uint32 length)
{

}

int32 mid_ble_msg_read(msg_packet_t* pt_packet)
{

}

int32 mid_ble_msg_write(uint8 head, uint8 type, uint8* p_data, uint32 data_len)
{

}

int32 mid_ble_msg_pack(uint8 head, uint8 type, uint8* p_data, uint32 size, ble_msg_t* output)
{

}

int32 mid_ble_msg_save(uint8* p_data, uint32 data_len, uint8 source, uint8 prority)
{

}

int32 mid_ble_config_update(uint8* ble_mac)
{

}

int32 mid_ble_event_get(void)
{

}



/**
 * Copyright (c) 2019, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */
 
/*********************************************************************
 * @fn      sp_gatt_read_cb
 *
 * @brief   Simple Profile user application handles read request in this callback.
 *          应用层在这个回调函数里面处理读的请求。
 *
 * @param   p_read  - the pointer to read buffer. NOTE: It's just a pointer from lower layer, please create the buffer in application layer.
 *                    指向读缓冲区的指针。 请注意这只是一个指针，请在应用程序中分配缓冲区. 为输出函数, 因此为指针的指针.
 *          len     - the pointer to the length of read buffer. Application to assign it.
 *                    读缓冲区的长度，用户应用程序去给它赋值.
 *          att_idx - index of the attribute value in it's attribute table.
 *                    Attribute的偏移量.
 *
 * @return  读请求的长度.
 */
static void sp_gatt_read_cb(uint8_t *p_read, uint16_t *len, uint16_t att_idx)
{
    switch (att_idx)
    {
        case SP_IDX_CHAR1_VALUE:
            for (int i = 0; i < SP_CHAR1_VALUE_LEN; i++)
                sp_char1_value[i] = sp_char1_value[0] + i + 1;
            memcpy(p_read, sp_char1_value, SP_CHAR1_VALUE_LEN);
            *len = SP_CHAR1_VALUE_LEN;
        break;

        case SP_IDX_CHAR2_VALUE:
            for (int i = 0; i < SP_CHAR2_VALUE_LEN; i++)
                sp_char2_value[i] = sp_char2_value[0] + i + 1;
            memcpy(p_read, sp_char2_value, SP_CHAR2_VALUE_LEN);
            *len = SP_CHAR2_VALUE_LEN;
       break;
        
        case SP_IDX_CHAR4_CFG:
            *len = 2;
            memcpy(p_read, sp_char4_ccc, 2);
        break;
        
        case SP_IDX_CHAR5_VALUE:
            for (int i = 0; i < SP_CHAR5_VALUE_LEN; i++)
                sp_char5_value[i] = sp_char3_value[0] + i + 1;
            memcpy(p_read, sp_char5_value, SP_CHAR5_VALUE_LEN);
           *len = SP_CHAR5_VALUE_LEN;
        break;
        
        default:
        break;
    }
    
    co_printf("Read request: len: %d  value: 0x%x 0x%x \r\n", *len, (p_read)[0], (p_read)[*len-1]);
    
}

/*********************************************************************
 * @fn      sp_gatt_write_cb
 *
 * @brief   Simple Profile user application handles write request in this callback.
 *          应用层在这个回调函数里面处理写的请求。
 *
 * @param   write_buf   - the buffer for write
 *                        写操作的数据.
 *                    
 *          len         - the length of write buffer.
 *                        写缓冲区的长度.
 *          att_idx     - index of the attribute value in it's attribute table.
 *                        Attribute的偏移量.
 *
 * @return  写请求的长度.
 */
static void sp_gatt_write_cb(uint8_t *write_buf, uint16_t len, uint16_t att_idx)
{
    for (int i = 0; i < len; i++)
    {
        co_printf("Write request: len: %d, 0x%x \r\n", len, write_buf[i]);
        if (att_idx == SP_IDX_CHAR1_VALUE)
            memcpy(sp_char1_value, write_buf, len);
        
        if (att_idx == SP_IDX_CHAR3_VALUE)
            memcpy(sp_char3_value, write_buf, len);
        
        if (att_idx == SP_IDX_CHAR5_VALUE)
            memcpy(sp_char5_value, write_buf, len);
    }
    
    uint16_t uuid = BUILD_UINT16( simple_profile_att_table[att_idx].uuid.p_uuid[0], simple_profile_att_table[att_idx].uuid.p_uuid[1] );
    if (uuid == GATT_CLIENT_CHAR_CFG_UUID)
    {
        co_printf("Notification status changed\r\n");
        if (att_idx == SP_IDX_CHAR4_CFG)
        {
            sp_char4_ccc[0] = write_buf[0];
            sp_char4_ccc[1] = write_buf[1];
            co_printf("Char4 ccc: 0x%x 0x%x \r\n", sp_char4_ccc[0], sp_char4_ccc[1]);
        }
    }

}

/*********************************************************************
 * @fn      sp_gatt_msg_handler
 *
 * @brief   Simple Profile callback funtion for GATT messages. GATT read/write
 *          operations are handeled here.
 *
 * @param   p_msg       - GATT messages from GATT layer.
 *
 * @return  uint16_t    - Length of handled message.
 */
static uint16_t sp_gatt_msg_handler(gatt_msg_t *p_msg)
{
    switch(p_msg->msg_evt)
    {
        case GATTC_MSG_READ_REQ:
            sp_gatt_read_cb((uint8_t *)(p_msg->param.msg.p_msg_data), &(p_msg->param.msg.msg_len), p_msg->att_idx);
            break;
        
        case GATTC_MSG_WRITE_REQ:
            sp_gatt_write_cb((uint8_t*)(p_msg->param.msg.p_msg_data), (p_msg->param.msg.msg_len), p_msg->att_idx);
            break;
            
        default:
            break;
    }
    return p_msg->param.msg.msg_len;
}

/*********************************************************************
 * @fn      sp_gatt_add_service
 *
 * @brief   Simple Profile add GATT service function.
 *          添加GATT service到ATT的数据库里面。
 *
 * @param   None. 
 *        
 *
 * @return  None.
 */
void sp_gatt_add_service(void)
{
    simple_profile_svc.p_att_tb = simple_profile_att_table;
    simple_profile_svc.att_nb = SP_IDX_NB;
    simple_profile_svc.gatt_msg_handler = sp_gatt_msg_handler;
    
    sp_svc_id = gatt_add_service(&simple_profile_svc);
}







/*********************************************************************
 * @fn      sp_start_adv
 *
 * @brief   Set advertising data & scan response & advertising parameters and start advertising
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
static void sp_start_adv(void)
{
    // Set advertising parameters
    gap_adv_param_t adv_param;
    adv_param.adv_mode = GAP_ADV_MODE_UNDIRECT;
    adv_param.adv_addr_type = GAP_ADDR_TYPE_PUBLIC;
    adv_param.adv_chnl_map = GAP_ADV_CHAN_ALL;
    adv_param.adv_filt_policy = GAP_ADV_ALLOW_SCAN_ANY_CON_ANY;
    adv_param.adv_intv_min = 300;
    adv_param.adv_intv_max = 300;
        
    gap_set_advertising_param(&adv_param);
    
    // Set advertising data & scan response data
    gap_set_advertising_data(adv_data, sizeof(adv_data));
    gap_set_advertising_rsp_data(scan_rsp_data, sizeof(scan_rsp_data));
    // Start advertising
    co_printf("Start advertising...\r\n");
    gap_start_advertising(0);
}


/**
 * Copyright (c) 2019, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */
 
 /*
 * INCLUDES (包含头文件)
 */
/*
 * MACROS (宏定义)
 */

/*
 * CONSTANTS (常量定义)
 */

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
// GAP-广播包的内容,最长31个字节.短一点的内容可以节省广播时的系统功耗.
static uint8_t adv_data[] =
{
  // service UUID, to notify central devices what services are included
  // in this peripheral. 告诉central本机有什么服务, 但这里先只放一个主要的.
  0x03,   // length of this data
  GAP_ADVTYPE_16BIT_MORE,      // some of the UUID's, but not all
  0xFF,
  0xFE,
};

// GAP - Scan response data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
// GAP-Scan response内容,最长31个字节.短一点的内容可以节省广播时的系统功耗.
static uint8_t scan_rsp_data[] =
{
  // complete name 设备名字
  0x12,   // length of this data
  GAP_ADVTYPE_LOCAL_NAME_COMPLETE,
  'S','i','m','p','l','e',' ','P','e','r','i','p','h','e','r','a','l',

  // Tx power level 发射功率
  0x02,   // length of this data
  GAP_ADVTYPE_POWER_LEVEL,
  0,       // 0dBm
};

/*
 * TYPEDEFS (类型定义)
 */

/*
 * GLOBAL VARIABLES (全局变量)
 */

/*
 * LOCAL VARIABLES (本地变量)
 */


 
/*
 * LOCAL FUNCTIONS (本地函数)
 */
static void sp_start_adv(void);
/*
 * EXTERN FUNCTIONS (外部函数)
 */

/*
 * PUBLIC FUNCTIONS (全局函数)
 */

/** @function group ble peripheral device APIs (ble外设相关的API)
 * @{
 */

/*********************************************************************
 * @fn      app_gap_evt_cb
 *
 * @brief   Application layer GAP event callback function. Handles GAP evnets.
 *
 * @param   p_event - GAP events from BLE stack.
 *       
 *
 * @return  None.
 */
void app_gap_evt_cb(gap_event_t *p_event)
{
    switch(p_event->type)
    {
        case GAP_EVT_ADV_END:
        {
            co_printf("adv_end,status:0x%02x\r\n",p_event->param.adv_end.status);
            //gap_start_advertising(0);
        }
        break;
        
        case GAP_EVT_ALL_SVC_ADDED:
        {
            co_printf("All service added\r\n");
            sp_start_adv();
#ifdef USER_MEM_API_ENABLE
            //show_mem_list();
            //show_msg_list();
            //show_ke_malloc();
#endif
        }
        break;

        case GAP_EVT_SLAVE_CONNECT:
        {
            co_printf("slave[%d],connect. link_num:%d\r\n",p_event->param.slave_connect.conidx,gap_get_connect_num());
        }
        break;

        case GAP_EVT_DISCONNECT:
        {
            co_printf("Link[%d] disconnect,reason:0x%02X\r\n",p_event->param.disconnect.conidx
                      ,p_event->param.disconnect.reason);
            sp_start_adv();
#ifdef USER_MEM_API_ENABLE
            show_mem_list();
            //show_msg_list();
            show_ke_malloc();
#endif
        }
        break;

        case GAP_EVT_LINK_PARAM_REJECT:
            co_printf("Link[%d]param reject,status:0x%02x\r\n"
                      ,p_event->param.link_reject.conidx,p_event->param.link_reject.status);
            break;

        case GAP_EVT_LINK_PARAM_UPDATE:
            co_printf("Link[%d]param update,interval:%d,latency:%d,timeout:%d\r\n",p_event->param.link_update.conidx
                      ,p_event->param.link_update.con_interval,p_event->param.link_update.con_latency,p_event->param.link_update.sup_to);
            break;

        case GAP_EVT_PEER_FEATURE:
            co_printf("peer[%d] feats ind\r\n",p_event->param.peer_feature.conidx);
            show_reg((uint8_t *)&(p_event->param.peer_feature.features),8,1);
            break;

        case GAP_EVT_MTU:
            co_printf("mtu update,conidx=%d,mtu=%d\r\n"
                      ,p_event->param.mtu.conidx,p_event->param.mtu.value);
            break;
        
        case GAP_EVT_LINK_RSSI:
            co_printf("link rssi %d\r\n",p_event->param.link_rssi);
            break;
                
        case GAP_SEC_EVT_SLAVE_ENCRYPT:
            co_printf("slave[%d]_encrypted\r\n",p_event->param.slave_encrypt_conidx);
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      sp_start_adv
 *
 * @brief   Set advertising data & scan response & advertising parameters and start advertising
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
static void sp_start_adv(void)
{
    // Set advertising parameters
    gap_adv_param_t adv_param;
    adv_param.adv_mode = GAP_ADV_MODE_UNDIRECT;
    adv_param.adv_addr_type = GAP_ADDR_TYPE_PUBLIC;
    adv_param.adv_chnl_map = GAP_ADV_CHAN_ALL;
    adv_param.adv_filt_policy = GAP_ADV_ALLOW_SCAN_ANY_CON_ANY;
    adv_param.adv_intv_min = 300;
    adv_param.adv_intv_max = 300;
        
    gap_set_advertising_param(&adv_param);
    
    // Set advertising data & scan response data
    gap_set_advertising_data(adv_data, sizeof(adv_data));
    gap_set_advertising_rsp_data(scan_rsp_data, sizeof(scan_rsp_data));
    // Start advertising
    co_printf("Start advertising...\r\n");
    gap_start_advertising(0);
}


int32 mid_ble_init(ble_config_t* pt_ble)
{
    // set local device name
    uint8_t local_name[] = "Simple Peripheral";
    gap_set_dev_name(local_name, sizeof(local_name));
    
    // Initialize security related settings.
    gap_security_param_t param =
    {
        .mitm = false,
        .ble_secure_conn = false,
        .io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT,
        .pair_init_mode = GAP_PAIRING_MODE_WAIT_FOR_REQ,
        .bond_auth = true,
        .password = 0,
    };
    
    gap_security_param_init(&param);
    
    gap_set_cb_func(app_gap_evt_cb);

    gap_bond_manager_init(BLE_BONDING_INFO_SAVE_ADDR, BLE_REMOTE_SERVICE_SAVE_ADDR, 8, true);
    gap_bond_manager_delete_all();
    
    mac_addr_t addr;
    gap_address_get(&addr);
    co_printf("Local BDADDR: 0x%2X%2X%2X%2X%2X%2X\r\n", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3], addr.addr[4], addr.addr[5]);
    
    // Adding services to database
    sp_gatt_add_service();  
}


