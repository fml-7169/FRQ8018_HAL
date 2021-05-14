/*
 * Lite-Rbuffer.c
 *
 *  Created on: 2020-9-27
 *      Author: Weili.Hu
 */

#include <string.h>
#include <stdlib.h>
#include "Lite-Rbuffer.h"
#include "govee_log.h"
#include "os_mem.h"


#define LITE_RBUFFER_DEBUG_ENABLE   0

typedef struct _LITE_RBUFFER
{
    uint32 total;
    uint32 size;
    uint32 read;
    uint32 write;
    uint8* addr;
} LITE_RBUFFER_T;


LR_handler Lite_ring_buffer_init(uint32 length)
{
    LITE_RBUFFER_T* lr_buffer = NULL;
    uint8* buffer = NULL;

    lr_buffer = (LITE_RBUFFER_T*)os_malloc(sizeof(LITE_RBUFFER_T));
    if (lr_buffer == NULL)
    {
#if LITE_RBUFFER_DEBUG_ENABLE
        GOVEE_PRINT(LOG_ERROR, "No memory.\r\n");
#endif
        return NULL;
    }

    buffer = (unsigned char*)os_malloc(length);
    if (buffer == NULL)
    {
        os_free(lr_buffer);

#if LITE_RBUFFER_DEBUG_ENABLE
        GOVEE_PRINT(LOG_ERROR, "malloc ring buffer failed.\r\n");
#endif
        return NULL;
    }
    memset(buffer, 0, length);

    lr_buffer->addr = buffer;
    lr_buffer->total = length;
    lr_buffer->size = 0;
    lr_buffer->read = 0;
    lr_buffer->write = 0;

    return lr_buffer;
}

void Lite_ring_buffer_deinit(LR_handler handler)
{
    if (handler == NULL)
    {
#if LITE_RBUFFER_DEBUG_ENABLE
        GOVEE_PRINT(LOG_ERROR, "Invalid param.\r\n");
#endif
        return;
    }

    if (handler->addr)
    {
        os_free(handler->addr);
    }

    os_free(handler);
}

__attribute__((section("ram_code"))) int32 Lite_ring_buffer_write_data(LR_handler handler, uint8* p_data, int32 size)
{
    int left_len = 0;

    if (handler->total - Lite_ring_buffer_size_get(handler) < size)
    {
#if LITE_RBUFFER_DEBUG_ENABLE
        GOVEE_PRINT(LOG_ERROR, "No enough buffer size.\r\n");
#endif
        return -1;
    }

    left_len = handler->total - handler->write;
    if (left_len < size)
    {
        memcpy(handler->addr + handler->write, p_data, left_len);
        memcpy(handler->addr, p_data + left_len, size - left_len);
        handler->write = size - left_len;
    }
    else
    {
        memcpy(handler->addr + handler->write, p_data, size);
        handler->write = (handler->write + size) % handler->total;
    }

    //handler->size += size;

    return 0;
}

__attribute__((section("ram_code"))) int32 Lite_ring_buffer_read_data(LR_handler handler, uint8* p_buffer, int32 size)
{
    int left_len = 0;

    if (Lite_ring_buffer_size_get(handler) < size)
    {
#if LITE_RBUFFER_DEBUG_ENABLE
        GOVEE_PRINT(LOG_ERROR, "No enough data size.\r\n");
#endif
        return -1;
    }

    left_len = handler->total - handler->read;
    if (left_len < size)
    {
        memcpy(p_buffer, handler->addr + handler->read, left_len);
        memcpy(p_buffer + left_len, handler->addr, size - left_len);
        handler->read = size - left_len;
    }
    else
    {
        memcpy(p_buffer, handler->addr + handler->read, size);
        handler->read = (handler->read + size) % handler->total;
    }

    //handler->size -= size;

    return 0;
}

__attribute__((section("ram_code"))) int32 Lite_ring_buffer_left_get(LR_handler handler)
{
    //return (int32)(handler->total - handler->size);
	return (int32)(handler->total - Lite_ring_buffer_size_get(handler)); 
}

__attribute__((section("ram_code"))) int32 Lite_ring_buffer_size_get(LR_handler handler)
{
	if( handler->write >= handler->read )
		handler->size = handler->write - handler->read ;
	else
		handler->size = (handler->write + handler->total - handler->read);
	
    return (int32)handler->size;
}

void Lite_ring_buffer_print(LR_handler handler, int b_newline)
{
    int i = 0;

    for (i = 0; i < handler->total; i++)
    {
        if (b_newline && (i % 16 == 0) && (i != 0))
        {
            co_printf("\r\n");
        }
        co_printf("%02x ", handler->addr[i]);
    }

    co_printf("\r\n");
}
