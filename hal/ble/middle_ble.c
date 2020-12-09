#include <string.h>
#include <stdio.h>
#include "middle_ble.h"
#include "govee_log.h"
#include "Lite-Rbuffer.h"

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

int32 mid_ble_init(ble_config_t* pt_ble)
{

}

