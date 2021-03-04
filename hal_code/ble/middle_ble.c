#include <stdbool.h>
#include "gap_api.h"
#include "gatt_api.h"
#include "driver_gpio.h"
#include <string.h>
#include <stdio.h>
#include "middle_ble.h"
#include "govee_log.h"
#include "Lite-Rbuffer.h"
#include "gatt_sig_uuid.h"
#include "sys_utils.h"
#include "flash_usage_config.h"
#include "middle_ble.h"
#include "ota_service.h"
#include "govee_utils.h"

#define GATT_CHAR1_VALUE_LEN  20
#define GATT_CHAR2_VALUE_LEN  20

static uint8_t gatt_char1_value[GATT_CHAR2_VALUE_LEN] = {0};
static uint8_t gatt_char2_value[GATT_CHAR2_VALUE_LEN] = {0};

// Characteristic 1 User Description
#define GATT_CHAR1_DESC_LEN   20
#define GATT_CHAR2_DESC_LEN   20

static const uint8_t gatt_char1_desc[GATT_CHAR1_DESC_LEN] = "for gatt Read";
static const uint8_t gatt_char2_desc[GATT_CHAR2_DESC_LEN] = "for gatt Write";

static LR_handler gt_ble_lr = NULL;
static int32  g_ble_event = MSG_BLE_EVT_NULL;
static uint8_t g_ble_conidx = 0;
static connect_callback s_connect_ble_event = NULL;
static uint8_t g_ble_update_para = 0;
static uint32_t g_ble_adv_intv = 300;

/*
 * TYPEDEFS (类型定义)
 */
/*
 * GLOBAL VARIABLES (全局变量)
 */
uint8_t govee_sp_svc_id = 0;

/*
 * LOCAL VARIABLES (本地变量)
 */
static gatt_service_t govee_profile_svc;

enum
{
    GOVEE_GATT_IDX_SERVICE,
    GOVEE_GATT_IDX_CHAR1_DECLARATION,
    GOVEE_GATT_IDX_CHAR1_VALUE,
    GOVEE_GATT_IDX_CHAR1_CFG,
    GOVEE_GATT_IDX_CHAR1_USER_DESCRIPTION,
    GOVEE_GATT_IDX_CHAR2_DECLARATION,
    GOVEE_GATT_IDX_CHAR2_VALUE,
    GOVEE_GATT_IDX_CHAR2_CFG,
    GOVEE_GATT_IDX_CHAR2_USER_DESCRIPTION,
    GOVEE_GATT_IDX_NB,
};

#if 0
#define GOVEE_GATT_SPP_UUID_SERVICE        "\x57\x48\x5f\x53\x4b\x43\x4f\x52\x5f\x49\x4c\x4c\x45\x54\x4e\x49"
#define GOVEE_GATT_SVC1_PROTOCOL_UUID_128  "\x11\x20\x5f\x53\x4b\x43\x4f\x52\x5f\x49\x4c\x4c\x45\x54\x4e\x49"
#define GOVEE_GATT_SVC1_CMD_UUID_128       "\x12\x20\x5f\x53\x4b\x43\x4f\x52\x5f\x49\x4c\x4c\x45\x54\x4e\x49"
#define GOVEE_GATT_SVC1_DATA_UUID_128      "\x13\x20\x5f\x53\x4b\x43\x4f\x52\x5f\x49\x4c\x4c\x45\x54\x4e\x49"
#endif

static uint8 g_server_uuid[16] = {0x10, 0x19, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00};
#define GOVEE_GATT_SVC1_TX_UUID_128     "\x10\x2B\x0D\x0C\x0B\x0A\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00"
#define GOVEE_GATT_SVC1_RX_UUID_128     "\x11\x2B\x0D\x0C\x0B\x0A\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00"

#define BLE_GATT_MSG_BUFFER_SIZE    (sizeof(msg_packet_t)*20)
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

const gatt_attribute_t govee_gatt_profile_att_table[GOVEE_GATT_IDX_NB] =
{
    // Simple gatt Service Declaration
        [GOVEE_GATT_IDX_SERVICE]                    =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_PRIMARY_SERVICE_UUID) },     /* UUID */
                                                    GATT_PROP_WRITE_REQ|GATT_PROP_READ|GATT_PROP_NOTI,                                             /* Permissions */
                                                    UUID_SIZE_16,                                                /* Max size of the value */     /* Service UUID size in service declaration */
                                                    (uint8_t*)g_server_uuid,                                      /* Value of the attribute */    /* Service UUID value in service declaration */
                                                },

        //Write
        // Characteristic 1 Declaration
        [GOVEE_GATT_IDX_CHAR1_DECLARATION]          =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    0,                                                          /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */
                                                },
        // Characteristic 1 Value
        [GOVEE_GATT_IDX_CHAR1_VALUE]                =   {
                                                    { UUID_SIZE_16, GOVEE_GATT_SVC1_TX_UUID_128},                 /* UUID */
                                                    GATT_PROP_READ|GATT_PROP_NOTI,                           /* Permissions */
                                                    GATT_CHAR1_VALUE_LEN,                                         /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                },

        // Characteristic 1 client characteristic configuration
        [GOVEE_GATT_IDX_CHAR1_CFG]                  =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID) },     /* UUID */
                                                    GATT_PROP_READ | GATT_PROP_WRITE,                           /* Permissions */
                                                    2,                                           /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                },
        // Characteristic 1 User Description
        [GOVEE_GATT_IDX_CHAR1_USER_DESCRIPTION]     =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },      /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    GATT_CHAR1_DESC_LEN,                                          /* Max size of the value */
                                                    (uint8_t *)gatt_char1_desc,                                   /* Value of the attribute */
                                                },

        //Read
        // Characteristic 2 Declaration
        [GOVEE_GATT_IDX_CHAR2_DECLARATION]          =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    0,                                                          /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */
                                                },
        // Characteristic 2 Value
        [GOVEE_GATT_IDX_CHAR2_VALUE]                =   {
                                                    { UUID_SIZE_16, GOVEE_GATT_SVC1_RX_UUID_128},                 /* UUID */
                                                    GATT_PROP_WRITE|GATT_PROP_READ|GATT_PROP_NOTI,                           /* Permissions */
                                                    GATT_CHAR1_VALUE_LEN,                                         /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                },
        // Characteristic 2 client characteristic configuration
        [GOVEE_GATT_IDX_CHAR2_CFG]                  =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID) },     /* UUID */
                                                    GATT_PROP_READ | GATT_PROP_WRITE,                           /* Permissions */
                                                    2,                                           /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                },
        // Characteristic 2 User Description
        [GOVEE_GATT_IDX_CHAR2_USER_DESCRIPTION]     =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },      /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    GATT_CHAR1_DESC_LEN,                                          /* Max size of the value */
                                                    (uint8_t *)gatt_char1_desc,                                   /* Value of the attribute */
                                                },
};


// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
// GAP-广播包的内容,最长31个字节.短一点的内容可以节省广播时的系统功耗.
static uint8_t adv_data[31] = {0x00};
static uint16_t adv_data_len = 0;

// GAP - Scan response data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
// GAP-Scan response内容,最长31个字节.短一点的内容可以节省广播时的系统功耗.
static uint8_t scan_rsp_data[31] = {0x00};
static uint16_t scan_rsp_data_len = 0;
static uint8_t g_att_idx = 0;

 /*
 * LOCAL FUNCTIONS (本地函数)
 */
static void sp_start_adv(void);


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

uint8 mid_ble_check_sum(uint8* p_data, uint32 length)
{
    return ble_check_sum(p_data, length);
}

int32 mid_ble_msg_pack(uint8 head, uint8 type, uint8* p_data, uint32 size, ble_msg_t* output)
{
    uint8 check_sum = 0;

    memset(output, 0, sizeof(ble_msg_t));
    output->head = head;
    output->type = type;

    if (p_data && size > 0)
    {
        memcpy(output->data, p_data, size);
    }

    check_sum = ble_check_sum((uint8*)output, BLE_PKG_DATA_LEN - 1);
    output->check_code = check_sum;

    return 0;
}

int32 mid_ble_msg_save(uint8* p_data, uint32 data_len, uint8 source, uint8 prority)
{
    msg_packet_t ble_msg;
    int i = 0;

    if (NULL == p_data)
    {
        GOVEE_PRINT(LOG_ERROR, "BLE read data invalid.\r\n");
        return 0;
    }
    for(i = 0; i < data_len / BLE_PKG_DATA_LEN; i++)
    {
        memset(&ble_msg, 0, sizeof(msg_packet_t));
        ble_msg.t_header.source = source;
        memcpy(&ble_msg.t_message, p_data + i * BLE_PKG_DATA_LEN, BLE_PKG_DATA_LEN);

        Lite_ring_buffer_write_data(gt_ble_lr, (uint8*)&ble_msg, sizeof(msg_packet_t));
    }

    return 0;
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


int32 mid_ble_read_bytes(uint8* buffer, uint32 length)
{
    uint32 left = Lite_ring_buffer_size_get(gt_ble_lr);
    if (NULL == buffer || length > left)
    {
        return -1;
    }
    memset(buffer, 0, length);

    return Lite_ring_buffer_read_data(gt_ble_lr, buffer, length);
}

int32 mid_ble_msg_read(msg_packet_t* pt_packet)
{
    int32 data_len = 0;

    data_len = Lite_ring_buffer_size_get(gt_ble_lr);
    if (data_len < sizeof(msg_packet_t))
    {
        //GOVEE_PRINT(LOG_DEBUG, "ble data is not enough.\r\n");
        return -1;
    }
    memset(pt_packet, 0, sizeof(msg_packet_t));
    Lite_ring_buffer_read_data(gt_ble_lr, (uint8*)pt_packet, sizeof(msg_packet_t));
    return 0;
}

uint8_t protocolNotify2App(uint8_t *send_data,uint16_t data_len)
{
    #if 0
    for(uint8_t i=0;i<data_len;i++)
        co_printf("send_data[%d]=%x\r\n",i,send_data[i]);
    #endif
    gatt_ntf_t ntf_att;
    ble_msg_t* pt_msg = (ble_msg_t*)send_data;
    uint8 check_sum = 0;

    check_sum = ble_check_sum((uint8*)pt_msg, BLE_PKG_DATA_LEN - 1);
    pt_msg->check_code = check_sum;

    ntf_att.att_idx = GOVEE_GATT_IDX_CHAR1_VALUE;
    //co_printf("ntf_att.att_idx=%x\r\n",ntf_att.att_idx);
    ntf_att.conidx = 0;
    ntf_att.svc_id = govee_sp_svc_id;
    ntf_att.data_len = data_len;
    ntf_att.p_data = send_data;
    gatt_notification(ntf_att);

    //co_printf("ble send : ");
    //govee_utils_data_print(send_data, data_len, 0);
}



int32 mid_ble_write_bytes(uint8* buffer, uint32 length)
{
    if (!buffer || length == 0)
    {
        return -1;
    }
    gatt_ntf_t ntf_att;
    ntf_att.att_idx = GOVEE_GATT_IDX_CHAR1_VALUE;
    ntf_att.conidx = 0;
    ntf_att.svc_id = govee_sp_svc_id;
    ntf_att.data_len = length;
    ntf_att.p_data = buffer;
    gatt_notification(ntf_att);
    return 0;
}

int32 mid_ble_msg_write(uint8 head, uint8 type, uint8* p_data, uint32 data_len)
{
    ble_msg_t t_message;

    if (data_len > BLE_MSG_DATA_LEN)
    {
        GOVEE_PRINT(LOG_ERROR, "BLE write data too long.\r\n");
        return -1;
    }

    memset(&t_message, 0, sizeof(ble_msg_t));
    t_message.head = head;
    t_message.type = type;

    if (p_data && data_len > 0)
    {
        memcpy(t_message.data, p_data, data_len);
    }
    protocolNotify2App((uint8_t *)&t_message,sizeof(ble_msg_t));
    return 1;
}

static void sp_gatt_read_cb(uint8_t *p_read, uint16_t *len, uint16_t att_idx)
{
    //co_printf("%d Read request: len: %d  value: 0x%x 0x%x \r\n", att_idx ,*len, (p_read)[0], (p_read)[*len-1]);
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
static void sp_gatt_write_cb(uint8_t *write_buf, uint16_t len, uint16_t att_idx)        //
{
    msg_header_t t_header;

    if (NULL == write_buf)
    {
        GOVEE_PRINT(LOG_ERROR, "BLE read data invalid.\r\n");
        return;
    }

    if (Lite_ring_buffer_left_get(gt_ble_lr) < sizeof(msg_header_t) + len)
    {
        GOVEE_PRINT(LOG_ERROR, "Ble ring buffer full.\r\n");
        return;
    }

    memset(&t_header, 0, sizeof(msg_header_t));
    t_header.source = MSG_PHONE_BLE;
    t_header.length = len;
    Lite_ring_buffer_write_data(gt_ble_lr, (uint8*)&t_header, sizeof(msg_header_t));
    Lite_ring_buffer_write_data(gt_ble_lr, (uint8*)write_buf, len);
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
static uint16_t govee_sp_gatt_msg_handler(gatt_msg_t *p_msg)
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
 * @fn      govee_gatt_add_service
 *
 * @brief   Simple Profile add GATT service function.
 *          添加GATT service到ATT的数据库里面。
 *
 * @param   None.
 *
 *
 * @return  None.
 */

void govee_gatt_add_service(void)
{
    govee_profile_svc.p_att_tb = govee_gatt_profile_att_table;
    govee_profile_svc.att_nb = GOVEE_GATT_IDX_NB;
    govee_profile_svc.gatt_msg_handler = govee_sp_gatt_msg_handler;

    govee_sp_svc_id = gatt_add_service(&govee_profile_svc);
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
    adv_param.disc_mode = GAP_ADV_DISC_MODE_GEN_DISC;
    adv_param.adv_addr_type = GAP_ADDR_TYPE_PUBLIC;
    adv_param.adv_chnl_map = GAP_ADV_CHAN_ALL;
    adv_param.adv_filt_policy = GAP_ADV_ALLOW_SCAN_ANY_CON_ANY;
    if(g_ble_adv_intv<0x20)g_ble_adv_intv=0x20;     // must >=0x20
    adv_param.adv_intv_min = g_ble_adv_intv;    
    adv_param.adv_intv_max = g_ble_adv_intv;

    gap_set_advertising_param(&adv_param);

    // Set advertising data & scan response data
    gap_set_advertising_data(adv_data, adv_data_len);
    gap_set_advertising_rsp_data(scan_rsp_data, scan_rsp_data_len);
    // Start advertising
    co_printf("Start advertising...\r\n");
    gap_start_advertising(0);
}

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
static void govee_gap_evt_cb(gap_event_t *p_event)
{
    switch(p_event->type)
    {
        case GAP_EVT_ADV_END:
        {
            co_printf("adv_end,status:0x%02x\r\n",p_event->param.adv_end.status);
            if(g_ble_update_para==1){
                g_ble_update_para = 0;
                sp_start_adv();
            }
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
            g_ble_event = MSG_BLE_EVT_CONNECT;
            g_ble_conidx = p_event->param.slave_connect.conidx;
            gap_conn_param_update(p_event->param.link_update.conidx,20,50,0,300);
            if(s_connect_ble_event!=NULL)s_connect_ble_event(1);
            co_printf("slave[%d],connect. link_num:%d\r\n",p_event->param.slave_connect.conidx,gap_get_connect_num());
        }
        break;

        case GAP_EVT_DISCONNECT:
        {
            g_ble_event = MSG_BLE_EVT_DISCONNECT;
            g_ble_conidx = 0;
            if(s_connect_ble_event!=NULL)s_connect_ble_event(0);
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

void mid_ble_gap_update(ble_config_t* pt_ble)
{
    memcpy(adv_data,pt_ble->p_ble_adv,pt_ble->ble_adv_len);
    adv_data_len = pt_ble->ble_adv_len;

    memcpy(scan_rsp_data,pt_ble->p_ble_resp,pt_ble->ble_resp_len);
    scan_rsp_data_len = pt_ble->ble_resp_len;
    
    gap_set_advertising_data(adv_data, adv_data_len);
    gap_set_advertising_rsp_data(scan_rsp_data, scan_rsp_data_len);
}

int32 mid_ble_init(ble_config_t* pt_ble)
{

    mac_addr_t addr;

    if(pt_ble==NULL){
        return -1;
    }
    // set local device name
    gap_set_dev_name(pt_ble->local_name, pt_ble->ble_name_len);
    GOVEE_PRINT(LOG_DEBUG,"%s\r\n",pt_ble->local_name);
    memcpy(adv_data,pt_ble->p_ble_adv,pt_ble->ble_adv_len);
    adv_data_len = pt_ble->ble_adv_len;

    memcpy(scan_rsp_data,pt_ble->p_ble_resp,pt_ble->ble_resp_len);
    scan_rsp_data_len = pt_ble->ble_resp_len;

    if(pt_ble->ble_adv_intv>0x20)g_ble_adv_intv = pt_ble->ble_adv_intv;
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

    s_connect_ble_event = pt_ble->ble_connect;
    gap_security_param_init(&param);

    gap_set_cb_func(govee_gap_evt_cb);

    gap_bond_manager_init(BLE_BONDING_INFO_SAVE_ADDR, BLE_REMOTE_SERVICE_SAVE_ADDR, 8, true);
    gap_bond_manager_delete_all();

    gap_address_get(&addr);
    co_printf("Local BDADDR: 0x%2X%2X%2X%2X%2X%2X\r\n", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3], addr.addr[4], addr.addr[5]);

    // Adding services to database
    govee_gatt_add_service();

    gt_ble_lr = Lite_ring_buffer_init(BLE_GATT_MSG_BUFFER_SIZE);
    if (gt_ble_lr == NULL)
    {
        GOVEE_PRINT(LOG_ERROR, "BLE ring buffer init failed.\r\n");
        return -1;
    }

    return 0;
}

void mid_ble_disconnect_gatt(void)
{
    gap_disconnect_req(g_ble_conidx);
}

int32 mid_ble_mac_get(uint8* ble_mac)
{
    if(ble_mac==NULL){
        return -1;
    }
    mac_addr_t addr;
    gap_address_get(&addr);
    memcpy(ble_mac,&addr.addr[0],6);
    return 0;
}

int32 mid_ble_event_get(void)
{
    int32 ble_event = g_ble_event;
    g_ble_event = MSG_BLE_EVT_NULL;
    return ble_event;
}

void mid_ble_ota_init(void)
{
    ota_gatt_add_service();
}

int8 mid_ble_advertising_switch(int8 on)       //   1 on  0 off
{
    if(0!=on){
        gap_start_advertising(0);
    }
    else{
        gap_stop_advertising();
    }
    return 0;
}

int8 mid_ble_update_time(uint32 ble_adv_intv)
{
    g_ble_update_para = 1;
    g_ble_adv_intv = ble_adv_intv;
    gap_stop_advertising();
    return 0;
}



