#ifndef TM1628_h
#define TM1628_h
#include <stdint.h>
#include <stdbool.h>
#define LCD_HARDWARE_MODULE_ID "lcd"

typedef struct _PortPin_Map {
  uint32_t GPIOx;
  uint32_t GPIO_Pin_x;
} PortPin_Map;

typedef struct _lcd_TypeDef {
  
  PortPin_Map *CS; // Selects command(0) or data(1) register.
  PortPin_Map *CL; // Selects read(1) or write(0) operation.
  PortPin_Map *DA;  //  Data pins.
  PortPin_Map *RST;
  PortPin_Map *BUSY;
} lcd_TypeDef;

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
	
    void (*lcd_update)(void);
	
    //turn off lcd's context
	void (*lcd_turn_off)(void);
	
	void (*lcd_turn_on)(void);
	
    void (*lcd_luminance)(unsigned char grade);

    /*Show tile by type*/
    void (*lcd_stitle)(int row,int type,int enable);    
    
    /*Show the contents of the temperature that is xxx*/
    void (*lcd_stem)(int type,int pos,unsigned char* str,int str_len,unsigned char unit);    
    
    /*  Show the contents of the battery that from 0~100. */
    void (*lcd_sbattery)(const unsigned char electry);
}lcd_device_t;



//TM1628(uint8_t _dio_pin, uint8_t _clk_pin, uint8_t _stb_pin);
void setSeg(uint8_t addr, uint8_t num);
void sendData(uint8_t addr, uint8_t data);
void sendCommand(uint8_t data);
void send(uint8_t data);


#endif
