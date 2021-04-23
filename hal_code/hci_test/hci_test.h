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
 * INCLUDES (����ͷ�ļ�)
 */
#include "stdint.h"
/*
 * MACROS (�궨��)
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
 * CONSTANTS (��������)
 */


/*
 * TYPEDEFS (���Ͷ���)
 */



/*
 * GLOBAL VARIABLES (ȫ�ֱ���)
 */



/*
 * LOCAL VARIABLES (���ر���)
 */
 
/*
 * LOCAL FUNCTIONS (���غ���)
 */

/*
 * EXTERN FUNCTIONS (�ⲿ����)
 */

/*
 * PUBLIC FUNCTIONS (ȫ�ֺ���)
 */

void dev_freq_adjust_check(void);

//void dev_hci_test_mode(uint8_t * cmd_data,uint8_t cmd_len);

void dev_msg_init(uint8_t * s_version,uint8_t s_version_len,uint8_t * h_version,uint8_t h_version_len);

uint8_t hci_test(void);

uint8_t dev_check_hci_test_mode(void);

void govee_hci_test_init(void);

#endif
