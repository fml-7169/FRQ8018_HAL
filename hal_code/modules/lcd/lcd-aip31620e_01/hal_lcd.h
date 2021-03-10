#ifndef LCD_H_4
#define LCD_H_4

#include <stdint.h>
#define LCD_HARDWARE_MODULE_ID "lcd"
#if 0
enum {
    LCD_TYPE_NUM,
    LCD_TYPE_MEAT,
    LCD_TYPE_SET,
    LCD_TYPE_BLE,
    LCD_TYPE_MUTE,    
    LCD_TYPE_ALERT,  //only for hal_lcd_4
}lcd_type;
#endif
enum {
    LCD_TYPE_BLE,
	LCD_TYPE_SMILEY,
 	LCD_TYPE_NEGATIVE,
 	LCD_TYPE_HUNDRED,
 	LCD_TYPE_DEFAULT,
 	LCD_TYPE_UNIT,
}lcd_type;



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

    
    //init gpio 
    void (*lcd_begin)(void);

    //default context
    void (*lcd_default_context)(void);    
    
    //clear lcd's context
    void (*lcd_clear)(void);    
    
    //full lcd's context
    void (*lcd_full)(void);    

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
