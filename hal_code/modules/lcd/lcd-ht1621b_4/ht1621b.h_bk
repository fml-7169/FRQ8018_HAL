#ifndef _HW_HT1621B_H
#define _HW_HT1621B_H
#include "stdlib.h"	  
#include <stdint.h>
#include <stdio.h>



//#define uint8_t u8
//#define uint16_t u16


void lcd_delay_us(uint16_t time);

void ht1621b_pin_init(void);
void ht1621_SendCmd(uint8_t command);
void ht1621_WriteByte(uint8_t addr , uint8_t data);
void ht1621_WriteData(uint8_t addr ,uint8_t *pdata  , uint8_t cnt);
void ht1621b_pin_release(void);

void CS_pin_test(void);
void LCD_CS_H();

#endif

