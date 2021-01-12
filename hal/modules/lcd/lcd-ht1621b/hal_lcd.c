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
 #define LCD_ADDR_LEN   32
 static unsigned char __lcd_ram[LCD_ADDR_LEN] = {0};

 #define CHAR_POS_LEN   12
 #define CHAR_SEG_LEN   7
 static const unsigned char lcd_num_arr[CHAR_POS_LEN][CHAR_SEG_LEN] = {
     {HT_1A_LCD,HT_1B_LCD,HT_1C_LCD,HT_1D_LCD,HT_1E_LCD,HT_1F_LCD,HT_1G_LCD},
     {HT_2A_LCD,HT_2B_LCD,HT_2C_LCD,HT_2D_LCD,HT_2E_LCD,HT_2F_LCD,HT_2G_LCD},
     {HT_3A_LCD,HT_3B_LCD,HT_3C_LCD,HT_3D_LCD,HT_3E_LCD,HT_3F_LCD,HT_3G_LCD},
     {HT_4A_LCD,HT_4B_LCD,HT_4C_LCD,HT_4D_LCD,HT_4E_LCD,HT_4F_LCD,HT_4G_LCD},
     {HT_5A_LCD,HT_5B_LCD,HT_5C_LCD,HT_5D_LCD,HT_5E_LCD,HT_5F_LCD,HT_5G_LCD},
     {HT_6A_LCD,HT_6B_LCD,HT_6C_LCD,HT_6D_LCD,HT_6E_LCD,HT_6F_LCD,HT_6G_LCD},
     {HT_7A_LCD,HT_7B_LCD,HT_7C_LCD,HT_7D_LCD,HT_7E_LCD,HT_7F_LCD,HT_7G_LCD},
     {HT_8A_LCD,HT_8B_LCD,HT_8C_LCD,HT_8D_LCD,HT_8E_LCD,HT_8F_LCD,HT_8G_LCD},
     {HT_9A_LCD,HT_9B_LCD,HT_9C_LCD,HT_9D_LCD,HT_9E_LCD,HT_9F_LCD,HT_9G_LCD},
     {HT_10A_LCD,HT_10B_LCD,HT_10C_LCD,HT_10D_LCD,HT_10E_LCD,HT_10F_LCD,HT_10G_LCD},
     {HT_11A_LCD,HT_11B_LCD,HT_11C_LCD,HT_11D_LCD,HT_11E_LCD,HT_11F_LCD,HT_11G_LCD}, 
     {HT_12A_LCD,HT_12B_LCD,HT_12C_LCD,HT_12D_LCD,HT_12E_LCD,HT_12F_LCD,HT_12G_LCD},
 };

 //const unsigned char digital_table_negative[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};

#define lcd_CS_H()		gpio_set_pin_value(__lcd->CS->GPIOx, __lcd->CS->GPIO_Pin_x, 1);
#define lcd_CS_L()		gpio_set_pin_value(__lcd->CS->GPIOx, __lcd->CS->GPIO_Pin_x, 0);
#define lcd_WR_H()		gpio_set_pin_value(__lcd->WR->GPIOx,__lcd->WR->GPIO_Pin_x,1);
#define lcd_WR_L()		gpio_set_pin_value(__lcd->WR->GPIOx,__lcd->WR->GPIO_Pin_x,0);
#define lcd_DA_H()		gpio_set_pin_value(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x,1);
#define lcd_DA_L()		gpio_set_pin_value(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x,0);


void ht1621_sendbit_high(unsigned char data , unsigned char cnt)
{
	unsigned char i ;
	for(i = 0; i< cnt ; i++)
	{
		lcd_WR_L();
		if((data&0x80)==0){
			lcd_DA_L();
		}
		else{
			lcd_DA_H();
		}
		lcd_WR_H();
		data<<=1;
	}
}

void ht1621_sendbit_low(unsigned char data , unsigned char cnt)
{
	unsigned char i ;
	for(i = 0; i< cnt ; i++)
	{
		lcd_WR_L();
		//co_delay_10us(1);
		if((data&0x01)==0){
			lcd_DA_L();
		}
		else{
			lcd_DA_H();
		}
		lcd_WR_H();
		data>>=1;
	}
}


void ht1621_SendCmd(unsigned char command)
{
	lcd_CS_L();
	//co_delay_10us(3);
	ht1621_sendbit_high(0x080,4);		//  100  write
	ht1621_sendbit_high(command , 8) ;  // 
	lcd_CS_H();
}
void ht1621b_disable(void)
{

	lcd_CS_H();
	//lcd_RD_H();
	lcd_WR_H();
	lcd_DA_H();
}

void ht1621_WriteByte(unsigned char addr , unsigned char data)
{
	lcd_CS_L();
	//co_delay_10us(3);
	ht1621_sendbit_high(0xa0,3);
	ht1621_sendbit_high(addr<<2 , 6);
	ht1621_sendbit_low(data,4);
	lcd_CS_H(); 
}

void ht1621_WriteData(unsigned char addr ,unsigned char *pdata  , unsigned char cnt)
{
	unsigned char i = 0;
	lcd_CS_L();
	//co_delay_10us(3);
	ht1621_sendbit_high(0xa0,3);
	ht1621_sendbit_high(addr<<2 , 6);
	for(i = 0;i<cnt;i++,pdata++){
		ht1621_sendbit_low(*pdata,8);
	}
	lcd_CS_H(); 	
}	

void ht1621b_init(void)
{	
	
	ht1621_SendCmd(BIAS);
	ht1621_SendCmd(RC_32K);
	//ht1621_SendCmd(SYSDIS);
	ht1621_SendCmd(WDTDIS);
	ht1621_SendCmd(TONE_OFF);
	ht1621_SendCmd(SYSEN);
}

void ht1621_Update(void)
{
	unsigned char i = 0; 
	unsigned char addr = 0;
	for(i = 0; i< sizeof(__lcd_ram) ;i++)
	{
		ht1621_WriteByte(i,__lcd_ram[i]);
	}	
}
////////////////////////////////ht1621/////////////////////////////////

static void lcd_set_seg(unsigned char seg_num);
static void lcd_default_context(void);
static void lcd_begin(void);

#define PORT_FUNC_GPIO              0x00
static void lcd_begin(void) {
    assert_param(__lcd != NULL);
    lcd_TypeDef* lcd=__lcd;
	system_set_port_mux(lcd->CS->GPIOx,lcd->CS->GPIO_Pin_x,PORT_FUNC_GPIO);
	system_set_port_mux(lcd->WR->GPIOx,lcd->WR->GPIO_Pin_x,PORT_FUNC_GPIO);	
	system_set_port_mux(lcd->DA->GPIOx,lcd->DA->GPIO_Pin_x,PORT_FUNC_GPIO);
	
	gpio_set_dir(lcd->CS->GPIOx,lcd->CS->GPIO_Pin_x,GPIO_DIR_OUT);
	gpio_set_dir(lcd->WR->GPIOx,lcd->WR->GPIO_Pin_x,GPIO_DIR_OUT);
	gpio_set_dir(lcd->DA->GPIOx,lcd->DA->GPIO_Pin_x,GPIO_DIR_OUT);
	
	gpio_set_pin_value(lcd->CS->GPIOx,lcd->CS->GPIO_Pin_x,1);	
	gpio_set_pin_value(lcd->WR->GPIOx,lcd->WR->GPIO_Pin_x,1);
	gpio_set_pin_value(lcd->DA->GPIOx,lcd->DA->GPIO_Pin_x,1);

    
    ht1621b_init();    
	ht1621_SendCmd(LCDON);
    ht1621b_disable();    
}
void lcd_default_context(void){
    assert_param(__lcd != NULL);

    //show NUM 1
    lcd_set_seg(HT_S3_LCD);    
    
     //NUM 1 MEAT TEMP
    lcd_set_seg(HT_S4_LCD);    
	lcd_set_seg(HT_S5_LCD);    

    //NUM 1 SET TEMP    
    lcd_set_seg(HT_S9_LCD);    
	lcd_set_seg(HT_S10_LCD);    

    //Dividing line
    lcd_set_seg(HT_S14_LCD);    


    //show NUM 2
    lcd_set_seg(HT_S15_LCD);   

    //NUM 2 MEAT TEMP
    lcd_set_seg(HT_S16_LCD);    
	lcd_set_seg(HT_S17_LCD);    

    //NUM 2 SET TEMP    
    lcd_set_seg(HT_S21_LCD);    
	lcd_set_seg(HT_S22_LCD); 

    ht1621_Update();
    return;
}

lcd_TypeDef* lcd_init(PortPin_Map *CS, PortPin_Map *WR, PortPin_Map *DA)
{
  lcd_TypeDef *ret=NULL;
  
  assert_param(CS != NULL);
  assert_param(WR != NULL);
  assert_param(DA != NULL);

  
  ret = os_calloc(1, sizeof(lcd_TypeDef));
  if(!ret)
    return NULL;
  ret->CS = CS;
  ret->WR = WR;
  ret->DA = DA;  
  __lcd = ret; // NOTE: This line must exist for all user APIs.
  lcd_begin();
  lcd_default_context();
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

void lcd_set_seg(unsigned char seg_num)
{
	unsigned char addr;
	addr = seg_num/4;
	__lcd_ram[addr] |=(0x01<<(seg_num%4));
}
void lcd_clear_seg(unsigned char seg_num)
{
	unsigned char addr;
	addr = seg_num/4;
	__lcd_ram[addr] &=~(0x01<<(seg_num%4));
}


//electry from 0~100
#define BATT_LEVEL 5
void lcd_battery_power(const unsigned char electry)  
{    
    assert_param(__lcd != NULL);
	unsigned char electry_level=0;
	unsigned char i = 0;
	unsigned char electry_icon[BATT_LEVEL]={HT_T5_LCD,HT_T4_LCD,HT_T3_LCD,HT_T2_LCD,HT_T1_LCD};    
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
    ht1621_Update();
}

//clear lcd's context
void lcd_clear(void){
    memset(__lcd_ram,0x00,sizeof(__lcd_ram));
    ht1621_Update();
    return;
}    

//full lcd's context
void lcd_full(void){
    memset(__lcd_ram,0xff,sizeof(__lcd_ram));
    ht1621_Update();
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
static void lcd_tem_unit(int pos,int unit){
    assert_param(__lcd != NULL);
    switch(pos){
        case 0:{
            lcd_set_seg(HT_S6_LCD);
            if(unit ==true){ //temerature unit c            
                lcd_clear_seg(HT_S8_LCD);
                lcd_set_seg(HT_S7_LCD);
            }else{    ////temerature unit F
                lcd_clear_seg(HT_S7_LCD);
                lcd_set_seg(HT_S8_LCD);
            }
        }break;
        case 1:{
            lcd_set_seg(HT_S11_LCD);
            if(unit ==true){ //temerature unit c            
                lcd_clear_seg(HT_S13_LCD);
                lcd_set_seg(HT_S12_LCD);
            }else{    ////temerature unit F
                lcd_set_seg(HT_S13_LCD);                
                lcd_clear_seg(HT_S12_LCD);
            }
        }break;
        case 2:{
            lcd_set_seg(HT_S18_LCD);
            if(unit ==true){ //temerature unit c            
                lcd_clear_seg(HT_S20_LCD);
                lcd_set_seg(HT_S19_LCD);
            }else{    ////temerature unit F             
                lcd_clear_seg(HT_S19_LCD);
                lcd_set_seg(HT_S20_LCD);
            }
        }break;
        case 3:{
            lcd_set_seg(HT_S23_LCD);
            if(unit ==true){ //temerature unit c            
                lcd_clear_seg(HT_S25_LCD);
                lcd_set_seg(HT_S24_LCD);
            }else{    ////temerature unit F
                lcd_set_seg(HT_S25_LCD);                
                lcd_clear_seg(HT_S24_LCD);
            }
        }break;
        default:{}break;
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
    lcd_tem_unit(pos,unit);
    ht1621_Update();
}

//the pos map with meat temp and set temp that from 0~3
//type is num index or meat temp ,set temp,ble logo,mute logo

void lcd_tile(int row,int type,int enable){
    switch(row)
   { // map the digits to the seg bits
         case 0:{
            if(type == LCD_TYPE_NUM){
                if(enable)
                    lcd_set_seg(HT_S3_LCD);    
                else
                    lcd_clear_seg(HT_S3_LCD);
            }
            if(type == LCD_TYPE_MEAT){
                if(enable){
                    lcd_set_seg(HT_S4_LCD);    
                    lcd_set_seg(HT_S5_LCD);
                }else{
                    lcd_clear_seg(HT_S4_LCD);
                    lcd_clear_seg(HT_S5_LCD);
                }
            }
            if(type == LCD_TYPE_SET){
                if(enable){
                    lcd_set_seg(HT_S9_LCD);    
                    lcd_set_seg(HT_S10_LCD);
                }else{
                    lcd_clear_seg(HT_S9_LCD);    
                    lcd_clear_seg(HT_S10_LCD);
                }
            }
         }break;
        case 1:{
            if(type == LCD_TYPE_NUM){
                if(enable)
                    lcd_set_seg(HT_S15_LCD);    
                else
                    lcd_clear_seg(HT_S15_LCD);
            }
            if(type == LCD_TYPE_MEAT){
                if(enable){
                    lcd_set_seg(HT_S16_LCD);    
                    lcd_set_seg(HT_S17_LCD);
                }else{
                    lcd_clear_seg(HT_S16_LCD);    
                    lcd_clear_seg(HT_S17_LCD);
                }
            }
            if(type == LCD_TYPE_SET){
                if(enable){
                    lcd_set_seg(HT_S21_LCD);    
                    lcd_set_seg(HT_S22_LCD);
                }else{
                    lcd_clear_seg(HT_S21_LCD);    
                    lcd_clear_seg(HT_S22_LCD);
                }
            }
         }break;
         default:{
            DEV_ERR("lcd_tile_tem row is invaild\r\n");
            }break;
    }

    
    if(type == LCD_TYPE_BLE){  //num 0  ble logo
        if(enable){
            lcd_set_seg(HT_S1_LCD);
        }else{
            lcd_clear_seg(HT_S1_LCD);    
        }
    }
    if(type == LCD_TYPE_MUTE){  // num 0 mute logo            
        lcd_set_seg(HT_S2_LCD);    
        if(enable){ 
            lcd_set_seg(HT_S0_LCD);            
            lcd_set_seg(HT_S2_LCD);    
        }else{            
            lcd_clear_seg(HT_S2_LCD);
            lcd_clear_seg(HT_S0_LCD);
        }
    }
    ht1621_Update();
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
    dev->lcd_default_context=lcd_default_context;
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

