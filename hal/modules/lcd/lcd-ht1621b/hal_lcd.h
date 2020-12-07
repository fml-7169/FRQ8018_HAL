#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#define LCD_HARDWARE_MODULE_ID "lcd"

enum 
{
	HT_S3_LCD=0,
	HT_1F_LCD,
	HT_1G_LCD,
	HT_1E_LCD,	// 1
	
	HT_1A_LCD,
	HT_1B_LCD,
	HT_1C_LCD,
	HT_1D_LCD,		// 2
	
	HT_S4_LCD,
	HT_2F_LCD,
	HT_2G_LCD,
	HT_2E_LCD, 	// 3
	
	HT_2A_LCD,
	HT_2B_LCD,
	HT_2C_LCD,	// 4
	HT_2D_LCD,
	
	HT_S5_LCD,
	HT_3F_LCD,
	HT_3G_LCD,
	HT_3E_LCD,	// 5
	
	HT_3A_LCD,
	HT_3B_LCD,
	HT_3C_LCD,
	HT_3D_LCD,	//6
	
	HT_S2_LCD,
	HT_S6_LCD,
	HT_S7_LCD,
	HT_S8_LCD,		//7
		
	HT_S0_LCD,
	HT_4F_LCD,
	HT_4G_LCD,
	HT_4E_LCD,		//8
	
	HT_4A_LCD,
	HT_4B_LCD,
	HT_4C_LCD,
	HT_4D_LCD, 	//9
	
	HT_S9_LCD,
	HT_5F_LCD,
	HT_5G_LCD,
	HT_5E_LCD,		//10
	
	HT_5A_LCD,
	HT_5B_LCD,
	HT_5C_LCD,
	HT_5D_LCD,	// 11 
	
	HT_S10_LCD,
	HT_6F_LCD,
	HT_6G_LCD,
	HT_6E_LCD,		// 12
	
	HT_6A_LCD,
	HT_6B_LCD,
	HT_6C_LCD,
	HT_6D_LCD,		// 17
	
	HT_S1_LCD,
	HT_S11_LCD,
	HT_S12_LCD,
	HT_S13_LCD,

	HT_7E_LCD,
	HT_7G_LCD,
	HT_7F_LCD,
	HT_S15_LCD,

	HT_7D_LCD,
	HT_7C_LCD,
	HT_7B_LCD,
	HT_7A_LCD,

	HT_8E_LCD,
	HT_8G_LCD,
	HT_8F_LCD,
	HT_S16_LCD,

	HT_8D_LCD,
	HT_8C_LCD,
	HT_8B_LCD,
	HT_8A_LCD,
	
	HT_9E_LCD,
	HT_9G_LCD,
	HT_9F_LCD,
	HT_S17_LCD,

	HT_9D_LCD,
	HT_9C_LCD,
	HT_9B_LCD,
	HT_9A_LCD,

	HT_S20_LCD,
	HT_S19_LCD,
	HT_S18_LCD,
	HT_S14_LCD,

	HT_10E_LCD,
	HT_10G_LCD,
	HT_10F_LCD,
	HT_T3_LCD,
	
	HT_10D_LCD,
	HT_10C_LCD,
	HT_10B_LCD,
	HT_10A_LCD,


	HT_11E_LCD,
	HT_11G_LCD,
	HT_11F_LCD,
	HT_S21_LCD,
	
	HT_11D_LCD,
	HT_11C_LCD,
	HT_11B_LCD,
	HT_11A_LCD,

	HT_12E_LCD,
	HT_12G_LCD,
	HT_12F_LCD,
	HT_S22_LCD,
	
	HT_12D_LCD,
	HT_12C_LCD,
	HT_12B_LCD,
	HT_12A_LCD,

	HT_S25_LCD,
	HT_S24_LCD,
	HT_S23_LCD,
	HT_T4_LCD,
	
	HT_NO_LCD,//empty
	HT_T5_LCD,
	HT_T1_LCD,
	HT_T2_LCD,
}lcd_com;

enum {
    LCD_TYPE_NUM,
    LCD_TYPE_MEAT,
    LCD_TYPE_SET,
    LCD_TYPE_BLE,
    LCD_TYPE_MUTE
}lcd_type;



#define BIAS 		0x52
#define SYSDIS	    0X00
#define SYSEN 		0X02
#define LCDOFF 		0X04
#define LCDON 		0X06
#define XTAL 		0x28
#define RC256 		0X30
#define TONEON 		0X12
#define TONEOFF 	0X10
#define WDTDIS 		0X0A
#define XTAL_32K	0x28
#define TONE_OFF	0x10
#define RC_32K 		0X30

#define NUM_DIGITS_MAX 3
/* Just keeping a combo of the port and pin together in one piece. */
typedef struct _PortPin_Map {
  uint32_t GPIOx;
  uint32_t GPIO_Pin_x;
} PortPin_Map;

/* Structure for a standard (character)LCD display.   */
typedef struct _lcd_TypeDef {
  
  PortPin_Map *CS; // Selects command(0) or data(1) register.
  PortPin_Map *WR; // Selects read(1) or write(0) operation.
  PortPin_Map *DA;  //  Data pins.
} lcd_TypeDef;
/* Exported macro ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT
/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function
  *         which reports the name of the source file and the source
  *         line number of the call that failed. 
  *         If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((unsigned char *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(unsigned char* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */  


typedef struct lcd_device_t {    
    struct hw_device_t common;    
    lcd_TypeDef *type_def;

    /*init lcd's pin*/
    lcd_TypeDef * (*lcd_sinit)(PortPin_Map *CS, PortPin_Map *WR, PortPin_Map *DA);
    
    /*Show tile by type*/
    void (*lcd_stitle)(int row,int type,int enable);    
    
    /*Show the contents of the temperature that is xxx*/
    void (*lcd_stem)(int pos,unsigned char* str,int str_len,unsigned char unit);    
    
    /*  Show the contents of the battery that from 0~100. */
    void (*lcd_sbattery)(const unsigned char electry);
}lcd_device_t;


/*  Put a string of chars starting from the current cursor position.  */
/* Sends a command byte to the LCD  */
/* Sends a data byte to the LCD  */

#endif
