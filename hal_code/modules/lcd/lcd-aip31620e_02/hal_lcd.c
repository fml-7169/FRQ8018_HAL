#include <stdlib.h>
#include <string.h>
#include "hardware.h"
#include "os_timer.h"
#include "co_printf.h"
#include "os_mem.h"

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
/* A local copy of a pointer to an lcd_TypeDef instance passed from user app.
 * This copy is used by internal calls to low-level functions. The advantage
 * is that we dont have to pass around the pointer to the lcd_TypeDef within
 * the library call space. In other words, functions that are only accessible
 * and meaningful to the library itself(not the library user) uses this variable.
 */
enum 
{
	HT_S3_LCD=0,
	HT_4E_LCD,
	HT_4G_LCD,
	HT_4F_LCD,	// 1

	HT_4D_LCD,
	HT_4C_LCD,
	HT_4B_LCD,
	HT_4A_LCD,	//6
	
	HT_S4_LCD,
	HT_5E_LCD,
	HT_5G_LCD,
	HT_5F_LCD,	// 1

	HT_5D_LCD,
	HT_5C_LCD,
	HT_5B_LCD,
	HT_5A_LCD,
	
	HT_S5_LCD,
	HT_Z_LCD,
	HT_Z1_LCD,	// 4
	HT_NO1_LCD,
	
	HT_Z4_LCD,
	HT_Z2_LCD,
	HT_Z3_LCD,
	HT_NO2_LCD, // 5
	
	HT_3D_LCD,	
	HT_3C_LCD,
	HT_3B_LCD,
	HT_3A_LCD,
	
	HT_P5_LCD,
	HT_3E_LCD,
	HT_3G_LCD,
	HT_3F_LCD,	// 5
		
	HT_NO3_LCD,
	HT_P4_LCD,
	HT_P3_LCD,
	HT_P2_LCD,

	HT_2D_LCD,	
	HT_2C_LCD,
	HT_2B_LCD,
	HT_2A_LCD,
	
	HT_NO4_LCD,
	HT_2E_LCD,
	HT_2G_LCD,
	HT_2F_LCD,	// 5
	
	HT_1D_LCD,	
	HT_1C_LCD,
	HT_1B_LCD,
	HT_1A_LCD,
	
	HT_S2_LCD,
	HT_1E_LCD,
	HT_1G_LCD,
	HT_1F_LCD,		//10
	
	HT_NO5_LCD,
	HT_P1_LCD,
	HT_P_LCD,
	HT_S1_LCD,
	
}lcd_com;

#if 0
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
#endif
#define NUM_DIGITS_MAX 3
static lcd_TypeDef* __lcd = NULL;
/*
*       a
*       -
*    f | | b
*       -  <-g
*    e | | c
*       -  h->for icon
*       d
*       
*   h g f e d c b a
*   1 1 1 1 1 1 1 1 (all on)
      1 1 1 0 1 1 1
*   
*
*/
#define NEGATIVE 0b01000000 //'-'
 const unsigned char char_map[] = {
   0b00111111, //0
   0b00000110, //1
   0b01011011, //2
   0b01001111, //3
   0b01100110, //4
   0b01101101, //5
   0b01111101, //6
   0b00000111, //7
   0b01111111, //8
   0b01101111, //9
   0b01110111, //A  ,10
   0b01111100, //B  ,11
   0b00111001, //C   ,12
   0b01011110, //D ,13
   0b01111001, //E ,14
   0b01110001, //F  ,15
   0b01111101, //G   ,16
   0b01110110, //H   ,17
   0b00111000, //L  ,18
 };
 #define LCD_ADDR_LEN   14
 static unsigned char __lcd_ram[LCD_ADDR_LEN] = {0};

 #define CHAR_POS_LEN   5
 #define CHAR_SEG_LEN   7
 static const unsigned char lcd_num_arr[CHAR_POS_LEN][CHAR_SEG_LEN] = {
     {HT_1A_LCD,HT_1B_LCD,HT_1C_LCD,HT_1D_LCD,HT_1E_LCD,HT_1F_LCD,HT_1G_LCD},
     {HT_2A_LCD,HT_2B_LCD,HT_2C_LCD,HT_2D_LCD,HT_2E_LCD,HT_2F_LCD,HT_2G_LCD},
     {HT_3A_LCD,HT_3B_LCD,HT_3C_LCD,HT_3D_LCD,HT_3E_LCD,HT_3F_LCD,HT_3G_LCD},
     {HT_4A_LCD,HT_4B_LCD,HT_4C_LCD,HT_4D_LCD,HT_4E_LCD,HT_4F_LCD,HT_4G_LCD},
     {HT_5A_LCD,HT_5B_LCD,HT_5C_LCD,HT_5D_LCD,HT_5E_LCD,HT_5F_LCD,HT_5G_LCD},
 };

 //const unsigned char digital_table_negative[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};


 //#define lcd_WR_H()		 gpio_set_pin_value(__lcd->WR->GPIOx,__lcd->WR->GPIO_Pin_x,1);
 //#define lcd_WR_L()		 gpio_set_pin_value(__lcd->WR->GPIOx,__lcd->WR->GPIO_Pin_x,0);


#define HT_SCL_OPEN()		gpio_set_pin_value(__lcd->CS->GPIOx, __lcd->CS->GPIO_Pin_x, 1);
#define HT_SCL_LOW()		gpio_set_pin_value(__lcd->CS->GPIOx, __lcd->CS->GPIO_Pin_x, 0);
#define HT_SCL_READ		gpio_get_pin_value(__lcd->CS->GPIOx, __lcd->CS->GPIO_Pin_x)

#define HT_SDA_OPEN()		gpio_set_pin_value(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x,1);
#define HT_SDA_LOW()		gpio_set_pin_value(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x,0);
#define HT_SDA_READ		gpio_get_pin_value(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x)

#if 0
void HT_I2cPin_Init(void)                       /* -- adapt the init for your uC -- */
{
	system_set_port_mux(__lcd->CS->GPIOx,__lcd->CS->GPIO_Pin_x,PORTA6_FUNC_A6);
	system_set_port_mux(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x,PORTA7_FUNC_A7);
	gpio_set_dir(__lcd->CS->GPIOx,__lcd->CS->GPIO_Pin_x,GPIO_DIR_OUT);
	gpio_set_dir(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x,GPIO_DIR_OUT);
	
	gpio_set_pin_value(__lcd->CS->GPIOx, __lcd->CS->GPIO_Pin_x, 1);
	gpio_set_pin_value(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x,1);

	HT_SDA_OPEN();                  // I2C-bus idle mode SDA released
	HT_SCL_OPEN();                  // I2C-bus idle mode SCL released
}
#endif


static void delay(void)					//几us的延时
{
	unsigned char i = 2;
	while(i--);
}

void i2c_start(void)			//开始信号
{													//SCL为高时，SDA下降沿
	HT_SDA_OPEN();
	HT_SCL_OPEN();
	delay();
	HT_SDA_LOW();
	delay();	
}
void i2c_stop(void)				//停止信号
{													//SCL为高时，SDA上升沿
	HT_SDA_LOW();
	HT_SCL_OPEN();
	delay();
	HT_SDA_OPEN();
	delay();
}
void ack(void)						//ACK应答
{													//第九个时钟，应答检测
	HT_SCL_LOW();
	delay();
	HT_SDA_OPEN();
	HT_SCL_OPEN();
	if(HT_SDA_READ);
	delay();
	HT_SCL_LOW();
	delay();
}
void i2c_write(unsigned char dat)	//写8bit数据
{
	unsigned char i;
	for(i = 0; i < 8; i++)
	{
		HT_SCL_LOW();
		if((dat & 0x80) == 0) {
			HT_SDA_LOW();	// write 0 to SDA-Line
		} else {
			HT_SDA_OPEN(); // write 1 to SDA-Line
		}
		delay();
		HT_SCL_OPEN();
		dat <<= 1;
		delay();	 
	} 
	ack();
}
void Ht_I2CWriteSingle(unsigned char dat)	//写8bit数据
{
	unsigned char i;
	for(i = 0; i < 8; i++)
	{
		HT_SCL_LOW();
		if((dat & 0x80) == 0) {
			HT_SDA_LOW();	// write 0 to SDA-Line
		} else {
			HT_SDA_OPEN(); // write 1 to SDA-Line
		}
		delay();
		HT_SCL_OPEN();
		dat <<= 1;
		delay();
	}
	ack();
}
void Ht_I2CWriteSeries(unsigned char addr,unsigned char * data,unsigned char len)
{
	unsigned char i;
	unsigned char high_len = 0;
	i2c_start();
	i2c_write(0x7c);				//从机地址
	i2c_write(0xc8);				//开显示、1/3偏置
	i2c_write(0xe8);                //设置地址位AD5为0，不执行software，设置为内部振荡
	i2c_write(addr&0x0f);				//设置地址位AD4~AD0为0x00
	if(len<=16){
		for(i=0;i<len;i++)
		i2c_write(data[i]);				//写0x00~0x1F地址内的显示数据
		i2c_stop();
	}
	else{
		high_len = len - 16;
		if(high_len>2)high_len = 2;
		for(i=0;i<16;i++)
			i2c_write(data[i]); 			//写0x00~0x1F地址内的显示数据
		i2c_stop();
		i2c_start();
		i2c_write(0x7c);				//从机地址
		i2c_write(0xeC);				//设置地址位AD5为1，不执行software，设置为内部振荡
		i2c_write(0x00);				//设置地址位AD4~AD0为0x00
		for(i=0;i<high_len;i++)
			i2c_write(data[i+16]);				//写0x20~0x23地址内的显示数据
		i2c_stop();
	}
}

void Ht_16c22Initial(void)				//初始化
{
	unsigned char i;
	co_delay_100us(2);
	i2c_stop();							//停止信号
	i2c_start();						//开始信号
	i2c_write(0x7c);				//从机地址
	i2c_write(0xea);				//ICSET执行software，内部震荡，AD5=0
	
	i2c_write(0xf0);				//BLKCTL关闪烁
	i2c_write(0xc8);				//MODESET关显示，1/3bias
	//i2c_write(0xbf);
	//i2c_write(0x00);
	#if 1	
	//i2c_write(0xbc);				//DISCTL，FRAME翻转，mode3、mode1
	i2c_write(0xae);
	
	i2c_write(0x00);				//ADSET设置地址
	/*清零*/
	for(i=0;i<16;i++)
		i2c_write(0xFF);				//写0x00~0x1F地址内的显示数据
	i2c_stop();
	i2c_start();
	i2c_write(0x7c);				//从机地址
	i2c_write(0xee);				//设置地址位AD5为1
	i2c_write(0x00);				//设置地址位AD4~AD0为0x00
	for(i=0;i<2;i++)
		i2c_write(0xFF);				//写0x20~0x23地址内的显示数据
	#endif
	i2c_stop();	

}

void sendDataToLcdDriver()
{
	Ht_I2CWriteSeries(0,__lcd_ram,sizeof(__lcd_ram));
}

////////////////////////////////ht1621/////////////////////////////////

static void lcd_set_seg(unsigned char seg_num);
static void lcd_begin(void);
void lcd_show_num( unsigned char position, unsigned char digital);

#define PORT_FUNC_GPIO              0x00
static void lcd_begin(void) {
    assert_param(__lcd != NULL);
    lcd_TypeDef* lcd=__lcd;
	system_set_port_mux(lcd->CS->GPIOx,lcd->CS->GPIO_Pin_x,PORT_FUNC_GPIO);
	system_set_port_mux(lcd->DA->GPIOx,lcd->DA->GPIO_Pin_x,PORT_FUNC_GPIO);
	
	gpio_set_dir(lcd->CS->GPIOx,lcd->CS->GPIO_Pin_x,GPIO_DIR_OUT);
	gpio_set_dir(lcd->DA->GPIOx,lcd->DA->GPIO_Pin_x,GPIO_DIR_OUT);
	
	gpio_set_pin_value(lcd->CS->GPIOx,lcd->CS->GPIO_Pin_x,1);	
	gpio_set_pin_value(lcd->DA->GPIOx,lcd->DA->GPIO_Pin_x,1);    
}


lcd_TypeDef* lcd_init(PortPin_Map *CS, PortPin_Map *WR, PortPin_Map *DA)
{
  lcd_TypeDef *ret=NULL;
  
  assert_param(CS != NULL);
  //assert_param(WR != NULL);
  assert_param(DA != NULL);

  ret = os_calloc(1, sizeof(lcd_TypeDef));
  if(!ret)
    return NULL;
  ret->CS = CS;
 // ret->WR = WR;
  ret->DA = DA;  
  __lcd = ret; // NOTE: This line must exist for all user APIs.
  lcd_begin();
  Ht_16c22Initial();
  sendDataToLcdDriver();
  return ret;
}
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


void lcd_release(lcd_device_t *lcd_dev){
    if(lcd_dev){
        if(lcd_dev->type_def){
            os_free(lcd_dev->type_def);
        }
        os_free(lcd_dev);
    }
    return;
}
#if 0
void lcd_show_num( unsigned char position, unsigned char digital)
{
	unsigned char i = 0;
	unsigned char addr = 0;
	unsigned char value = 0;
	for(i = 0;i<7;i++)
	{
		if(__lcd_ram[digital]&(0x01<<i)){
			lcd_set_seg(lcd_num_arr[position][i]);
		}
		else{
			lcd_clear_seg(lcd_num_arr[position][i]);
		}
	}
}
#endif
void lcd_set_seg(unsigned char seg_num)
{
	unsigned char addr;
	addr = 13- seg_num/8;
	__lcd_ram[addr] |=(0x01<<((11-(seg_num%8))%8));
}
void lcd_clear_seg(unsigned char seg_num)
{
	unsigned char addr;
	addr = 13- seg_num/8;
	__lcd_ram[addr] &=~(0x01<<((11-(seg_num%8))%8));
}
#if 0
static void lcd_write_ram(const unsigned char position,const unsigned char digital)
 {
     unsigned char i = 0;
     unsigned char addr = 0;
     unsigned char value = 0;
     for(i = 0;i<CHAR_SEG_LEN;i++)
     {
         if(digital&(0x01<<i)){
             addr = lcd_num_arr[position][i]/4;
             value = __lcd_ram[addr]|(0x01<<(lcd_num_arr[position][i]%4));      
             __lcd_ram[addr] = value;
         }
         else{
             addr =lcd_num_arr[position][i]/4;
             value = __lcd_ram[addr]&(~(0x01<<(lcd_num_arr[position][i]%4)));
             __lcd_ram[addr] = value;
         }
     }
 }
#else
static void lcd_write_ram(const unsigned char position,const unsigned char digital)
 {
     unsigned char i = 0;
     unsigned char addr = 0;
     unsigned char value = 0;
     for(i = 0;i<CHAR_SEG_LEN;i++)
     {
     	//DEV_DEBUG("digital is 0x%x,position[%d],seg:%d %s,value is %d\r\n",digital,position,i,(digital&(0x01<<i))?"set":"clear",lcd_num_arr[position][i]);
         if(digital&(0x01<<i)){ //0101 1011
			 lcd_set_seg(lcd_num_arr[position][i]);
         }
         else{
			 lcd_clear_seg(lcd_num_arr[position][i]);
         }
     }
 }

#endif

void lcd_show_num( unsigned char position, unsigned char digital)
{
	unsigned char i = 0;
	unsigned char addr = 0;
	unsigned char value = 0;
	for(i = 0;i<7;i++)
	{
		if(char_map[digital]&(0x01<<i)){
			lcd_set_seg(lcd_num_arr[position][i]);
		}
		else{
			lcd_clear_seg(lcd_num_arr[position][i]);
		}
	}
}

//electry from 0~100
#define BATT_LEVEL 5
void lcd_battery_power(const unsigned char electry)  
{    
    assert_param(__lcd != NULL);
	unsigned char electry_level=0;
	unsigned char i = 0;
	unsigned char electry_icon[BATT_LEVEL]={HT_Z_LCD,HT_Z4_LCD,HT_Z3_LCD,HT_Z2_LCD,HT_Z1_LCD};    
	if(electry<=10){
		electry_level = 0;
	}
	else if(electry<=25){
		electry_level = 1;
	}
	else if(electry<=50){
		electry_level = 2;
	}
	else if(electry<=75){
		electry_level = 3;
	}
	else {
		electry_level = 4;
	}    
    DEV_DEBUG("electry_level is %d\r\n",electry_level);
	for(i=0;i<sizeof(electry_icon);i++)//clear electry icon
	{
		lcd_clear_seg(electry_icon[i]);
	}
	for(i=0;i<electry_level+1;i++)//set electry icon
	{
		lcd_set_seg(electry_icon[i]);
	}    
	sendDataToLcdDriver();
}

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

//show the default context into lcd
static void lcd_putchar_cached(int index,unsigned char c) {
  assert_param(__lcd != NULL);
    DEV_DEBUG("index[%d],char is %c[%d]\r\n",index,c,c);

    //clear
    if(c == 0){
        DEV_DEBUG("index[%d],clear %d\r\n",index,c);              
        lcd_write_ram(index,0);
    }
    
    switch(c)
    { // map the digits to the seg bits
          case '0':
              lcd_write_ram(index,char_map[0]);
              break;
          case '1':
              lcd_write_ram(index,char_map[1]);
              break;
          case '2':
              lcd_write_ram(index,char_map[2]);
              break;
          case '3':
              lcd_write_ram(index,char_map[3]);
              break;
          case '4':
              lcd_write_ram(index,char_map[4]);
              break;
          case '5':
              lcd_write_ram(index,char_map[5]);
              break;
          case '6':
              lcd_write_ram(index,char_map[6]);
              break;
          case '7':
              lcd_write_ram(index,char_map[7]);
              break;
          case '8':
              lcd_write_ram(index,char_map[8]);
              break;
          case '9':
              lcd_write_ram(index,char_map[9]);
              break;
          case 'A':
              lcd_write_ram(index,char_map[10]);
              break;
          case 'B':
              lcd_write_ram(index,char_map[11]);
              break;
          case 'C':
              lcd_write_ram(index,char_map[12]);
              break;
          case 'D':
              lcd_write_ram(index,char_map[13]);
              break;
          case 'E':
              lcd_write_ram(index,char_map[14]);
          case 'F':
              lcd_write_ram(index,char_map[15]);
              break;
          case 'G':
              lcd_write_ram(index,char_map[16]);              
          case 'H':
              lcd_write_ram(index,char_map[17]);
              break;
          case 'L':
              lcd_write_ram(index,char_map[18]);
              break;
          case '-':
              lcd_write_ram(index,NEGATIVE);
              break;
          default:{ // do nothing, blank digit!          
              lcd_write_ram(index,0);
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

//    unsigned char sh_str[NUM_DIGITS_MAX]={0};  //make up string
    //memcpy(&sh_str[NUM_DIGITS_MAX-str_len],str,str_len);
    //string must  3 bytes
    for(i=0; i<str_len; i++) { //11.6 ,80
        lcd_putchar_cached(pos+i,str[i]);
    }
   // lcd_tem_unit(pos,unit);
	sendDataToLcdDriver();
}

//the pos map with meat temp and set temp that from 0~3
//type is num index or meat temp ,set temp,ble logo,mute logo

void lcd_tile(int row,int type,int enable){
   
    if(type == LCD_TYPE_BLE){
        if(enable)
            lcd_set_seg(HT_S1_LCD);    
        else
            lcd_clear_seg(HT_S1_LCD);
    }
    else if(type == LCD_TYPE_SMILEY){
        if(enable){
			if(row==0){
				lcd_clear_seg(HT_S4_LCD);
	            lcd_set_seg(HT_S3_LCD); 
			}
			else{
				lcd_clear_seg(HT_S3_LCD);
	            lcd_set_seg(HT_S4_LCD); 
			}
        }
        else{
            lcd_clear_seg(HT_S3_LCD);
			lcd_clear_seg(HT_S4_LCD);
        }
    }   
	else if(type == LCD_TYPE_NEGATIVE){
		 if(enable)
			 lcd_set_seg(HT_P_LCD);	
		 else
			 lcd_clear_seg(HT_P_LCD);
	 }
	else if(type == LCD_TYPE_HUNDRED){
	 if(enable)
		 lcd_set_seg(HT_P1_LCD);	
	 else
		 lcd_clear_seg(HT_P1_LCD);
	 }
	else if(type == LCD_TYPE_UNIT){
	 if(enable){
	 	 lcd_clear_seg(HT_P4_LCD);
		 lcd_set_seg(HT_P3_LCD);
	 }
	 else{
	 	 lcd_clear_seg(HT_P3_LCD);
		 lcd_set_seg(HT_P4_LCD);
		 
	 }
	 }
	else if(type == LCD_TYPE_DEFAULT){
	 if(enable){
		 lcd_set_seg(HT_P5_LCD);
		 lcd_set_seg(HT_P2_LCD);	
	 	 lcd_set_seg(HT_S2_LCD);
	 	 lcd_set_seg(HT_S5_LCD);	
	 	}
	 else{

		 lcd_clear_seg(HT_P5_LCD); 
		 lcd_clear_seg(HT_P2_LCD);		
	 	 lcd_clear_seg(HT_S2_LCD);
	 	 lcd_clear_seg(HT_S5_LCD);	
	 }
	 }
	sendDataToLcdDriver();
    
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

