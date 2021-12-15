#ifndef _MIDDLE_BLE_H_
#define _MIDDLE_BLE_H_
#include "hal_types.h"

#define BLE_PKG_DATA_LEN            20
#define BLE_MSG_DATA_LEN            17
#define BLE_MSG_MULT_DATA_LEN       16
#define BLE_MSG_NEW_MULT_LEN        17
#define BLE_UUID_BIT_LEN            128

#define MULTI_PACKAGE               0x01
#define MULTI_PACKAGE_START         0x02
#define MULTI_PACKAGE_END           0x04

typedef void (*read_callback)(uint8* p_data, uint32 data_len);
typedef void (*event_callback)(uint8 type, void* args);

typedef void (* connect_callback)(uint8 type);          // 0 disconncet 1 disconnect

typedef enum
{
    BLE_CONNECTED = 1,
    BLE_DISCONNECTED
} ble_event_e;

typedef struct
{
    uint8* p_ble_adv;
    uint8* p_ble_resp;
    uint8* local_name;
    int32 ble_adv_len;
    int32 ble_resp_len;
    int32 ble_name_len;
    int32 ble_adv_intv;
    connect_callback ble_connect;
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

typedef enum 
{
    MSG_BLE_EVT_NULL = 0,
    MSG_BLE_EVT_CONNECT ,
    MSG_BLE_EVT_DISCONNECT
}msg_ble_evt_e;

typedef struct _msg_header_t
{
    uint8 source;
    uint8 flag;
    uint8 priority;
    uint8 length;
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
void mid_ble_gap_update(ble_config_t* pt_ble);
void mid_ble_ota_init(void);
void mid_ble_disconnect_gatt(void);
int8 mid_ble_advertising_switch(int8 on);
int8 mid_ble_update_time(uint32 ble_adv_intv);
int32 mid_ble_read_bytes(uint8* buffer, uint32 length);
int32 mid_ble_write_bytes(uint8* buffer, uint32 length);
uint8 govee_conn_flag_get(void);
int32_t mid_ble_ring_buf_init(void);

#endif /* MIDDLE_BLE_H_ */
