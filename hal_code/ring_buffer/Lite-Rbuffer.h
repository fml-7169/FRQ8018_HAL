/*
 * Lite-Rbuffer.h
 *
 *  Created on: 2020-9-27
 *      Author: Weili.Hu
 */

#ifndef LITE_RBUFFER_H_
#define LITE_RBUFFER_H_

#include "hal_types.h"

typedef struct _LITE_RBUFFER* LR_handler;

LR_handler Lite_ring_buffer_init(uint32 length);
void Lite_ring_buffer_deinit(LR_handler handler);
int32 Lite_ring_buffer_write_data(LR_handler handler, uint8* p_data, int32 size);
int32 Lite_ring_buffer_read_data(LR_handler handler, uint8* p_buffer, int32 size);
int32 Lite_ring_buffer_left_get(LR_handler handler);
int32 Lite_ring_buffer_size_get(LR_handler handler);
void Lite_ring_buffer_print(LR_handler handler, int b_newline);

#endif /* LITE_RBUFFER_H_ */
