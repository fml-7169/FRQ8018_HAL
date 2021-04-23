/**
 * Copyright (c) 2021, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */
#ifndef HCI_TEST_H
#define HCI_TEST_H
 
 /*
 * INCLUDES (包含头文件)
 */
#include "stdint.h"
/*
 * MACROS (宏定义)
 */
#define GOVEE_TEST_MODE

#define GOVEE_SKU_MAX_LEN 5
#define GOVEE_VERSION_MAX_LEN   7
#define GOVEE_FLASH_DATA_BLOCK_SIZE         512
#define GOVEE_FLASH_DATA_BLOCK_COUNT        (0x1000/512)
#define HCI_TEST_MAC_ADDR 0x77000
#define HCI_TEST_SKU_BASE 0x79000
#define HCI_TEST_SKU_BACK 0x78000
#define PCB_TEST          0x76000
/*
 * CONSTANTS (常量定义)
 */


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

/*
 * EXTERN FUNCTIONS (外部函数)
 */

/*
 * PUBLIC FUNCTIONS (全局函数)
 */

void dev_freq_adjust_check(void);

//void dev_hci_test_mode(uint8_t * cmd_data,uint8_t cmd_len);

void dev_msg_init(uint8_t * s_version,uint8_t s_version_len,uint8_t * h_version,uint8_t h_version_len);

uint8_t hci_test(void);

uint8_t dev_check_hci_test_mode(void);

void govee_hci_test_init(void);

#endif
