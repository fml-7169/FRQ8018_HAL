#ifndef MIDDLE_BLE_H_
#define MIDDLE_BLE_H_

#include "hal_types.h"

#include <stdio.h>
#include <string.h>
#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"


/*
 * MACROS (宏定义)
 */

/*
 * CONSTANTS (常量定义)
 */
// Simple Profile attributes index. 
enum
{
    SP_IDX_SERVICE,

    SP_IDX_CHAR1_DECLARATION,
    SP_IDX_CHAR1_VALUE,
    SP_IDX_CHAR1_USER_DESCRIPTION,

    SP_IDX_CHAR2_DECLARATION,
    SP_IDX_CHAR2_VALUE,
    SP_IDX_CHAR2_USER_DESCRIPTION,

    SP_IDX_CHAR3_DECLARATION,
    SP_IDX_CHAR3_VALUE,
    SP_IDX_CHAR3_USER_DESCRIPTION,

    SP_IDX_CHAR4_DECLARATION,
    SP_IDX_CHAR4_VALUE,
    SP_IDX_CHAR4_CFG,
    SP_IDX_CHAR4_USER_DESCRIPTION,
    
    SP_IDX_CHAR5_DECLARATION,
    SP_IDX_CHAR5_VALUE,
    SP_IDX_CHAR5_USER_DESCRIPTION,
    
    SP_IDX_NB,
};

// Simple GATT Profile Service UUID
#define SP_SVC_UUID              0xFFF0

#define SP_CHAR1_UUID            0xFFF1
#define SP_CHAR2_UUID            0xFFF2
#define SP_CHAR3_UUID            0xFFF3
#define SP_CHAR4_UUID            0xFFF4
#define SP_CHAR5_UUID            0xFFF5

/*
 * TYPEDEFS (类型定义)
 */

/*
 * GLOBAL VARIABLES (全局变量)
 */
extern const gatt_attribute_t simple_profile_att_table[];

/*
 * LOCAL VARIABLES (本地变量)
 */


/*
 * PUBLIC FUNCTIONS (全局函数)
 */
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
void sp_gatt_add_service(void);

#define BLE_PKG_DATA_LEN            20
#define BLE_MSG_DATA_LEN            17
#define BLE_MSG_MULT_DATA_LEN       16
#define BLE_MSG_NEW_MULT_LEN        17
#define BLE_UUID_BIT_LEN            128


typedef void (*read_callback)(uint8* p_data, uint32 data_len);
typedef void (*event_callback)(uint8 type, void* args);

typedef enum
{
    BLE_CONNECTED = 1,
    BLE_DISCONNECTED
} ble_event_e;

typedef struct
{
    uint8* p_ble_adv;
    uint8* p_ble_resp;
    int32 ble_adv_len;
    int32 ble_resp_len;
} ble_config_t;

typedef struct _ble_msg_t
{
    uint8 head;
    uint8 type;
    uint8 data[BLE_MSG_DATA_LEN];
    uint8 check_code;
} ble_msg_t;

typedef enum
{
    MSG_PHONE_BLE = 0,
    MSG_WIFI_UART,
    MSG_INFRARED,
    MSG_433,
    MSG_BUTTON
} msg_source_e;

typedef struct _msg_header_t
{
    uint8 source;
    uint8 priority;
    uint16 reserve;
} msg_header_t;

typedef struct _msg_packet_t
{
    msg_header_t t_header;
    ble_msg_t t_message;
} msg_packet_t;


uint8 mid_ble_check_sum(uint8* p_data, uint32 length);
int32 mid_ble_mac_get(uint8* ble_mac);
int32 mid_ble_msg_read(msg_packet_t* pt_packet);
int32 mid_ble_msg_write(uint8 head, uint8 type, uint8* p_data, uint32 data_len);
int32 mid_ble_msg_pack(uint8 head, uint8 type, uint8* p_data, uint32 size, ble_msg_t* output);
int32 mid_ble_msg_save(uint8* p_data, uint32 data_len, uint8 source, uint8 prority);
int32 mid_ble_event_get(void);
int32 mid_ble_init(ble_config_t* pt_ble);

#endif /* MIDDLE_BLE_H_ */
