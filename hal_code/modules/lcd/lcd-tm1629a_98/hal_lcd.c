#include <stdlib.h>
#include <string.h>
#include "hardware.h"
#include "os_timer.h"
#include "co_printf.h"
#include "os_mem.h"
#include "sys_utils.h"

#include "gpio.h"
#include "driver_pmu.h"
#include "driver_system.h"
#include "driver_gpio.h"
#include "hal_lcd.h"



#if 0
#define DEV_ERR(format,...) do { \
co_printf("[LCD] error:"); \
co_printf(format,##__VA_ARGS__); \
} while(0)
#define DEV_DEBUG(format,...) do { \
    co_printf("[LCD] debug:"); \
    co_printf(format,##__VA_ARGS__); \
} while(0)
#else
    #define DEV_ERR(format,...)    
    #define DEV_DEBUG(format,...)
#endif


#define COMMAND_ADDRESS_INCREASE    0x40
#define COMMAND_ADDRESS_KEEPED      0x44
#define COMMAND_DISPLAY_ON          0x88
#define COMMAND_DISPLAY_OFF         0x80
#define COMMAND_SEG_ADDRESS         0xC0 //地址从00H~0FH共16个
#define COMMAND_BRIGHTNESS          0x03



//uint8_t _curpos = 0x00;
uint8_t __lcd_ram[16] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
const uint8_t seg_addr[16]={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x00,0x01,0x02,0x03,0x04,0x05};//no bit of digital segments
//                       SG0  SG1  SG2  SG3  SG4  SG5  SG6  SG7  DVD  VCD  MP3  PLY  PAU  PBC  RET  DTS  DDD  CL1  CL2   //name   -|

#define NEGATIVE 0b01000000 //'-'
const uint8_t char_map[] = {
  0b00111111, // 0 0x3F
  0b00000110, // 1 0x06
  0b01011011, // 2 0x5B
  0b01001111, // 3 0x4F
  0b01100110, // 4 0x66
  0b01101101, // 5 0x4D
  0b01111101, // 6 0x7D
  0b00000111, // 7 0x07
  0b01111111, // 8 0x7F
  0b01101111, // 9 0x6F
  0b01110111, // A 0x77
  0b01111100, // B 0x7C
  0b00111001, // C 0x39
  0b01011110, // D 0x5E
  0b01111001, // E 0x79
  0b01110001, // F 0x71
  0b01000000, // '-'
  0b00000000  // ' '
};

#define NUM_DIGITS_MAX 3
static lcd_TypeDef* __lcd = NULL;


#define STB_OPEN()		gpio_set_pin_value(__lcd->CS->GPIOx, __lcd->CS->GPIO_Pin_x, 1);
#define STB_LOW()	    gpio_set_pin_value(__lcd->CS->GPIOx, __lcd->CS->GPIO_Pin_x, 0);
#define STB_READ		gpio_get_pin_value(__lcd->CS->GPIOx, __lcd->CS->GPIO_Pin_x)

#define CLK_OPEN()		gpio_set_pin_value(__lcd->CL->GPIOx,__lcd->CL->GPIO_Pin_x,1);
#define CLK_LOW()		gpio_set_pin_value(__lcd->CL->GPIOx,__lcd->CL->GPIO_Pin_x,0);
#define CLK_READ		gpio_get_pin_value(__lcd->CL->GPIOx,__lcd->CL->GPIO_Pin_x)

#define DIO_OPEN()		gpio_set_pin_value(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x,1);
#define DIO_LOW()		gpio_set_pin_value(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x,0);
#define DIO_READ		gpio_get_pin_value(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x)

void cmd_init(void);
void sendDataToLcdDriver(void);
void lcd_clear();

#define PORT_FUNC_GPIO              0x00
static void lcd_begin(void) {
    assert_param(__lcd != NULL);
    lcd_TypeDef* lcd=__lcd;
	system_set_port_mux(lcd->CS->GPIOx,lcd->CS->GPIO_Pin_x,PORT_FUNC_GPIO);
	system_set_port_mux(lcd->CL->GPIOx,lcd->CL->GPIO_Pin_x,PORT_FUNC_GPIO);
	system_set_port_mux(lcd->DA->GPIOx,lcd->DA->GPIO_Pin_x,PORT_FUNC_GPIO);	
	
	gpio_set_dir(lcd->CS->GPIOx,lcd->CS->GPIO_Pin_x,GPIO_DIR_OUT);
	gpio_set_dir(lcd->CL->GPIOx,lcd->CL->GPIO_Pin_x,GPIO_DIR_OUT);
	gpio_set_dir(lcd->DA->GPIOx,lcd->DA->GPIO_Pin_x,GPIO_DIR_OUT);
	
	gpio_set_pin_value(lcd->CS->GPIOx,lcd->CS->GPIO_Pin_x,1);	
	gpio_set_pin_value(lcd->CL->GPIOx,lcd->CL->GPIO_Pin_x,1);
	gpio_set_pin_value(lcd->DA->GPIOx,lcd->DA->GPIO_Pin_x,1);   
	cmd_init();
}

lcd_TypeDef* lcd_init(PortPin_Map *CS, PortPin_Map *CL, PortPin_Map *DA)
{
  lcd_TypeDef *ret=NULL;
  
  assert_param(CS != NULL);
  assert_param(CL != NULL);
  assert_param(DA != NULL);

  ret = os_calloc(1, sizeof(lcd_TypeDef));
  if(!ret)
    return NULL;
  ret->CS = CS;
  ret->CL = CL;
  ret->DA = DA;  
  __lcd = ret; // NOTE: This line must exist for all user APIs.
  
  lcd_begin();
  lcd_clear();
  return ret;
}

static void delay(void) 				//几us的延时
{
	unsigned char i = 2;
	while(i--);
}

void cmd_init(void){
	sendCommand(COMMAND_ADDRESS_INCREASE);
	sendCommand(COMMAND_DISPLAY_ON|COMMAND_BRIGHTNESS);
	STB_LOW()
	send(COMMAND_SEG_ADDRESS);
	//lcd_clear();
	STB_OPEN()
		
}

void write(unsigned char dt)
{
    unsigned char i = 8;
    do{
		CLK_LOW()    
		if(dt&0x01){
			DIO_OPEN()
		}else{
			DIO_LOW()
		}
		CLK_OPEN()
		delay();
		delay();
		dt >>= 1;
    }while(--i);
}
#if 0
void TM1629A_Display(unsigned char *ch, unsigned char brightness)
{
   unsigned char tem;
   STB_LOW()
   write(COMMAND_ADDRESS_INCREASE); // 写显存命令，地址自动加1
   STB_OPEN()
   delay();//STB选通脉冲宽度大于1us
    STB_LOW()
    write(COMMAND_SEG_ADDRESS); // 设置显示地址命令
    do{
        tem = *ch;
        if(tem >= '0' && tem <= '9') //为数字
        {
            tem -= '0';
            tem += tem; //15段码一个字符占用两个字节
            write(char_map[tem]); //将数字转化成数组位置
            write(char_map[tem + 1]); //这是第二个字节
            continue;
        }
       
    }while(*(++ch) != 0);
    STB_OPEN()
    delay();
    STB_LOW()
    write(COMMAND_DISPLAY_ON | brightness); 
    STB_OPEN()
}
#endif
/*********** mid level  **********/
void sendData(uint8_t addr, uint8_t data) {
  sendCommand(COMMAND_ADDRESS_INCREASE);
  STB_LOW()
  send(COMMAND_SEG_ADDRESS | addr);
  send(data);
  STB_OPEN()
}

void send(uint8_t data) {

	unsigned char i = 8;
	do{
		CLK_LOW()    
		if(data&0x01){
		  DIO_OPEN()
		}else{
		  DIO_LOW()
		}
		CLK_OPEN()
		delay();
		delay();
		data >>= 1;
	  }while(--i);
	  
}
void sendCommand(uint8_t data) {
  STB_LOW()
  send(data);
  STB_OPEN()
}
void sendDataToLcdDriver(void) {
 	for(int i=0; i<16; i++){
    	sendData(i, __lcd_ram[i]);
  	}
  
  co_printf("\r\n");
}

bool  bitRead(unsigned char n, unsigned char k)
{
    bool bx;
    if(((n >> k) & 1) == 1)
        bx = true;
    else
        bx = false;
    return bx;
//    return (n>>(k-1)) & 1;          // shift n with k - 1 bits then and with 1
}
/*
参数:
x：要写入的数值变量
n：要写入的数值变量的位，右起第一位为0，第二位为1，以此类推。 
b：写入位的数值（0 或 1）
*/
uint16_t bitWrite(uint8_t *x, uint8_t n,bool b)
{
    if(b)
    	*x |= (0x1 << n);            // set k bit of nx = 0;
    else
		*x &=~(0x1 << n);
	//*x|=nx;
	//return n = nx | n;           
}


#define TM_WIFI_LCD      0x01    
#define TM_BLE_LCD       0x03    
#define TM_MUTE_LCD      0x05	
#define TM_ALERT1_LCD    0x07	
#define TM_ALERT2_LCD    0x09	
#define TM_TEMP1_LCD     0x01	
#define TM_TEMP2_LCD     0x03	
#define TM_UNIT_C_LCD    0x05	
#define TM_UNIT_F_LCD    0x07	
#define TM_BAT_LCD       0x09	

#define TM_SEQ1_LCD      0x00	
#define TM_SEQ2_LCD      0x03	
#define TM_SEQ3_LCD      0x06	
#define TM_SEQ4_LCD      0x09	


void lcd_set_seg(uint8_t addr,uint8_t pos,uint8_t enable)
{
//	co_printf("addr %d pos  %d  enable %d\r\n",addr,pos,enable);

	if(enable)
		__lcd_ram[addr] |=(0x01<<seg_addr[pos]);		
	else	
		__lcd_ram[addr] &=~(0x01<<seg_addr[pos]);
}

void lcd_show_num(uint8_t addr, uint8_t num) {
  for(int i=0; i<7; i++){
  	if(addr<8)
      	bitWrite(&__lcd_ram[i*2], seg_addr[addr], bitRead(char_map[num],i));
	else		
		bitWrite(&__lcd_ram[i*2+1], seg_addr[addr], bitRead(char_map[num],i));
    }
}
void lcd_write_ram( unsigned char position, unsigned char digital)
{	
	DEV_DEBUG("pos %d data %d\r\n",position,digital);
	lcd_show_num(position,digital);	
}
#if 0
#ifdef  USE_FULL_ASSERT
  /**
    * @brief  Reports the name of the source file and the source line number
    *         where the assert_param error has occurred.
    * @param  file: pointer to the source file name
    * @param  line: assert_param error line source number
    * @retval None
    */
  void assert_failed(unsigned char *file, uint32_t line)
  {
    char str[32] = "";
    HAL_UART_Transmit(&huart2, (unsigned char*)"File:Line = ", 12, 10);
    HAL_UART_Transmit(&huart2, file, strlen((char*)file), 10);
    sprintf(str, ":%d\n", line);
    HAL_UART_Transmit(&huart2, (unsigned char*)str, strlen(str), 10);
  }
#endif /* USE_FULL_ASSERT */
#endif
void lcd_battery_power(const unsigned char electry)  
{    
    
}

void lcd_tile(int row,int type,int enable){
#if 1
    switch(type)
   { 
         case LCD_TYPE_TEMP:{
		 		lcd_set_seg(TM_ALERT1_LCD,12,0);
				lcd_set_seg(TM_ALERT2_LCD,12,0);
			   	lcd_set_seg(TM_TEMP1_LCD,13,1);
				lcd_set_seg(TM_TEMP2_LCD,13,1);				
         }break;  
        case LCD_TYPE_UNIT:{
            if(enable){	
				lcd_set_seg(TM_UNIT_C_LCD,13,0);
				lcd_set_seg(TM_UNIT_F_LCD,13,1);		
			}else{
				lcd_set_seg(TM_UNIT_F_LCD,13,0);
				lcd_set_seg(TM_UNIT_C_LCD,13,1);
			}
        }break;
		case LCD_TYPE_BAT:{	
			lcd_set_seg(TM_BAT_LCD,13,enable);
        }break;
		case LCD_TYPE_WIFI:{
			lcd_set_seg(TM_WIFI_LCD,12,enable);
        }break;
		case LCD_TYPE_BLE:{      
			lcd_set_seg(TM_BLE_LCD,12,enable);
        }break;
		 case LCD_TYPE_MUTE:{     
				lcd_set_seg(TM_MUTE_LCD,12,enable);
        }break;	
		case LCD_TYPE_ALERT:{		
			lcd_set_seg(TM_TEMP1_LCD,13,enable);
			lcd_set_seg(TM_TEMP2_LCD,13,enable);
			lcd_set_seg(TM_ALERT1_LCD,12,enable);
			lcd_set_seg(TM_ALERT2_LCD,12,enable);			     
        }break;
		case LCD_TYPE_DEFAULT:{		
			lcd_set_seg(0x0E,TM_SEQ1_LCD,enable);	
			lcd_set_seg(0x0E,TM_SEQ2_LCD,enable);
			lcd_set_seg(0x0E,TM_SEQ3_LCD,enable);
			lcd_set_seg(0x0F,TM_SEQ4_LCD,enable);	
			//lcd_set_seg(TM_BAT_LCD,13,enable);
        }break;
		default:{
			DEV_ERR("lcd_tile_tem row is invaild\r\n");
		}break;
    }
#endif
    sendDataToLcdDriver();
}
//electry from 0~100

//clear lcd's context
void lcd_clear(void){
    memset(__lcd_ram,0x00,sizeof(__lcd_ram)); 
    sendDataToLcdDriver();
    return;
}    

//full lcd's context
void lcd_full(void){
  memset(__lcd_ram,0xff,sizeof(__lcd_ram));
  sendDataToLcdDriver();
  return;
}

void lcd_turn_off(void){
  sendCommand(COMMAND_DISPLAY_OFF);
  return;
}

void lcd_turn_on(void){
  sendCommand(COMMAND_DISPLAY_ON);
  return;
}

#if 1
static void lcd_putchar_cached(int index,unsigned char c) {
  assert_param(__lcd != NULL);
   // DEV_DEBUG("index[%d],char is %c[%d]\r\n",index,c,c);

    //clear
    if(c == 0){
        DEV_DEBUG("index[%d],clear %d\r\n",index,c);              
        lcd_write_ram(index,17);
    }
    switch(c)
    { // map the digits to the seg bits
          case '0':
              lcd_write_ram(index,0);
              break;
          case '1':
              lcd_write_ram(index,1);
              break;
          case '2':
              lcd_write_ram(index,2);
              break;
          case '3':
              lcd_write_ram(index,3);
              break;
          case '4':
              lcd_write_ram(index,4);
              break;
          case '5':
              lcd_write_ram(index,5);
              break;
          case '6':
              lcd_write_ram(index,6);
              break;
          case '7':
              lcd_write_ram(index,7);
              break;
          case '8':
              lcd_write_ram(index,8);
              break;
          case '9':
              lcd_write_ram(index,9);
              break;
          case 'A':
              lcd_write_ram(index,10);
              break;
          case 'B':
              lcd_write_ram(index,11);
              break;
          case 'C':
              lcd_write_ram(index,12);
              break;
          case 'D':
              lcd_write_ram(index,13);
              break;
          case 'E':
              lcd_write_ram(index,14);
          case 'F':
              lcd_write_ram(index,15);
              break;       
          case '-':
              lcd_write_ram(index,16);
              break;
          default:{ // do nothing, blank digit!          
			  lcd_write_ram(index,17);
              DEV_ERR("lcd write char[%d] is  invaild\r\n",c);
          }break;
    }
}

//the pos from 0-12
void lcd_put_tem(int pos,unsigned char* str,int str_len,unsigned char unit) {
    int i=0;
	
    assert_param(__lcd != NULL);
    assert_param(str != NULL);  
    assert_param(str_len <= NUM_DIGITS_MAX );
	unsigned char sh_str[NUM_DIGITS_MAX]={0};  //make up string
    memcpy(&sh_str[NUM_DIGITS_MAX-str_len],str,str_len);
   //string must  3 bytes
   for(i=0; i<NUM_DIGITS_MAX; i++) {
	   lcd_putchar_cached(pos*NUM_DIGITS_MAX+i,sh_str[i]);
   }	
   lcd_tile(0,LCD_TYPE_UNIT,unit);
   // lcd_tem_unit(pos,unit);
	sendDataToLcdDriver();
}

#endif

void lcd_release(lcd_device_t *lcd_dev){
    if(lcd_dev){
        if(lcd_dev->type_def){
            os_free(lcd_dev->type_def);
        }
        os_free(lcd_dev);
    }
    return;
}

static int lcd_open(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device)
{
    //malloc dev
	lcd_device_t *dev = os_malloc(sizeof(lcd_device_t));
    memset(dev, 0, sizeof(*dev));
    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
    dev->common.close = (int (*)(struct hw_device_t*))lcd_release;    
    
    dev->lcd_sinit=lcd_init;    
    dev->lcd_sbattery=lcd_battery_power;    
    dev->lcd_stem=lcd_put_tem;
    dev->lcd_stitle=lcd_tile;
    dev->lcd_clear=lcd_clear;
    dev->lcd_full=lcd_full;
	dev->lcd_turn_off=lcd_turn_off;
	dev->lcd_turn_on=lcd_turn_on;
    dev->lcd_begin=lcd_begin;
    dev->type_def=__lcd;
    *device = (struct hw_device_t*)dev;
    return 0;
}
static struct hw_module_methods_t lcd_module_methods = {
    .open =  lcd_open,
};


const struct hw_module_t hal_module_info_lcd = {
    .tag = HARDWARE_MODULE_TAG,       // 规定的tag
    .version_major = 1,
    .version_minor = 0,
    .id = LCD_HARDWARE_MODULE_ID,  // 模块id
    .name = "Govee lcd Module",     // 名称
    .author = "Govee",
    .methods = &lcd_module_methods,// 方法
};

