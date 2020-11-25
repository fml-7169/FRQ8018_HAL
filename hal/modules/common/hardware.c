/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hardware.h"

#include <string.h>
#include <errno.h>
#include <limits.h>
#include "co_printf.h"
#define LOG_TAG "HAL"
/**
 * Load the file defined by the variant and if successful
 * return the dlopen handle and the hmi.
 * @return 0 = success, !0 = failure.
 */
static int load(const char *id,
        const char *path,
        const struct hw_module_t **pHmi)
{
    int status=-1;    
    #ifdef HAL_KEYS_CONFIG_
    if (strcmp(id, hal_module_info_light.id) == 0) {
        co_printf("load: id=%s == hmi->id=%s\r\n", id, hal_module_info_light.id);
        *pHmi = &hal_module_info_light;
        status=0;
    }
    #endif
    
    #ifdef HAL_KEYS_CONFIG_
    if (strcmp(id, hal_module_info_key.id) == 0) {
        co_printf("load: id=%s == hmi->id=%s\r\n", id, hal_module_info_key.id);
        *pHmi = &hal_module_info_key;
        status=0;
    }
    #endif
    if(status !=0){
        co_printf("load: id=%s fail\r\n", id);
    }
    return status;
}



static int hw_get_module_by_class(const char *class_id, const char *inst,
                           const struct hw_module_t **module)
{
    return load(class_id, NULL, module);
}

int hw_get_module(const char *id, const struct hw_module_t **module)
{
    return hw_get_module_by_class(id, NULL, module);
}
