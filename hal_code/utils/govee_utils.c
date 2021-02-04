/*
 * govee_utils.c
 *
 *  Created on: 2020-9-22
 *      Author: Weili.Hu
 */


#include "app_bridge.h"

void govee_utils_data_print(uint8* p_data, uint32 length, int b_newline)
{
    int i = 0;

    for (i = 0; i < length; i++)
    {
        if (b_newline && (i % 16 == 0) && (i != 0))
        {
            co_printf("\r\n");
        }
        co_printf("%02x ", p_data[i]);
    }

    co_printf("\r\n");
}
