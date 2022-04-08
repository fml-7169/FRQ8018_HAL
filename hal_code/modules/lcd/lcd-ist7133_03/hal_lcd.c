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

enum
{
	OED_SIMILE_LCD=0,
	OED_SAD_LCD,
	OED_BLE_LCD,
	OED_WIFI_LCD,
	OED_R1A_LCD,
	OED_R1B_LCD,
	OED_R1C_LCD,
	OED_NO0_LCD,

	OED_P1A_LCD,
	OED_P1B_LCD,
	OED_P1C_LCD,
	OED_P1D_LCD,
	OED_P1E_LCD,
	OED_P1F_LCD,
	OED_P1G_LCD,
	OED_NO1_LCD,

	OED_P2A_LCD,
	OED_P2B_LCD,
	OED_P2C_LCD,
	OED_P2D_LCD,
	OED_P2E_LCD,
	OED_P2F_LCD,
	OED_P2G_LCD,
	OED_PION_LCD,


	OED_P3A_LCD,
	OED_P3B_LCD,
	OED_P3C_LCD,
	OED_P3D_LCD,
	OED_P3E_LCD,
	OED_P3F_LCD,
	OED_P3G_LCD,
	OED_NO2_LCD,

	OED_P4A_LCD,
	OED_P4B_LCD,
	OED_P4C_LCD,
	OED_P4D_LCD,
	OED_P4E_LCD,
	OED_P4F_LCD,
	OED_P4G_LCD,
	OED_NO3_LCD,


	OED_P5A_LCD,
	OED_P5B_LCD,
	OED_P5C_LCD,
	OED_P5D_LCD,
	OED_P5E_LCD,
	OED_P5F_LCD,
	OED_P5G_LCD,
	OED_NO4_LCD,

	OED_Q1A_LCD,
	OED_Q1B_LCD,
	OED_Q1C_LCD,
	OED_Q1D_LCD,
	OED_Q1E_LCD,
	OED_NO5_LCD,
	OED_NO6_LCD,
	OED_NO7_LCD,

	OED_PERCENT_LCD,
	OED_NO8_LCD,
	OED_UNITC_LCD,
	OED_UNITF_LCD,
	OED_UNITA_LCD

}lcd_com;

#define NUM_DIGITS_MAX 5
static lcd_TypeDef* __lcd = NULL;
#define PORT_FUNC_GPIO              0x00

#define CS_HIGH		pmu_set_gpio_value(__lcd->CS->GPIOx,(1<<__lcd->CS->GPIO_Pin_x), 1);
#define CS_LOW	    pmu_set_gpio_value(__lcd->CS->GPIOx, (1<<__lcd->CS->GPIO_Pin_x), 0);
#define CS_READ		pmu_set_gpio_value(__lcd->CS->GPIOx,__lcd->CS->GPIO_Pin_x)

#define CLK_HIGH	gpio_set_pin_value(__lcd->CL->GPIOx,__lcd->CL->GPIO_Pin_x,1);
#define CLK_LOW		gpio_set_pin_value(__lcd->CL->GPIOx,__lcd->CL->GPIO_Pin_x,0);
#define CLK_READ	gpio_get_pin_value(__lcd->CL->GPIOx,__lcd->CL->GPIO_Pin_x)

#define SDA_HIGH	gpio_set_pin_value(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x,1);
#define SDA_LOW		gpio_set_pin_value(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x,0);
#define SDA_READ	gpio_get_pin_value(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x)
#define SDA_DIR_OUT gpio_set_dir(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x,GPIO_DIR_OUT);
#define SDA_DIR_IN	gpio_set_dir(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x,GPIO_DIR_IN);

#define RST_HIGH	gpio_set_pin_value(__lcd->RST->GPIOx,__lcd->RST->GPIO_Pin_x,1);
#define RST_LOW		gpio_set_pin_value(__lcd->RST->GPIOx,__lcd->RST->GPIO_Pin_x,0);
#define RST_READ	gpio_get_pin_value(__lcd->RST->GPIOx,__lcd->RST->GPIO_Pin_x)

#define IO_IDLE 		pmu_get_gpio_value(__lcd->BUSY->GPIOx,__lcd->BUSY->GPIO_Pin_x)

#if 0
#define CS_HIGH		gpio_set_pin_value(__lcd->CS->GPIOx, __lcd->CS->GPIO_Pin_x, 1);
#define CS_LOW	    gpio_set_pin_value(__lcd->CS->GPIOx, __lcd->CS->GPIO_Pin_x, 0);
#define CS_READ		    gpio_get_pin_value(__lcd->CS->GPIOx, __lcd->CS->GPIO_Pin_x)

#define CLK_HIGH		gpio_set_pin_value(__lcd->CL->GPIOx,__lcd->CL->GPIO_Pin_x,1);
#define CLK_LOW		gpio_set_pin_value(__lcd->CL->GPIOx,__lcd->CL->GPIO_Pin_x,0);
#define CLK_READ		gpio_get_pin_value(__lcd->CL->GPIOx,__lcd->CL->GPIO_Pin_x)

#define SDA_HIGH		gpio_set_pin_value(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x,1);
#define SDA_LOW		gpio_set_pin_value(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x,0);
#define SDA_READ		gpio_get_pin_value(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x)
#define SDA_DIR_OUT 	gpio_set_dir(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x,GPIO_DIR_OUT);
#define SDA_DIR_IN	    gpio_set_dir(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x,GPIO_DIR_IN);

#define RST_HIGH		gpio_set_pin_value(__lcd->RST->GPIOx,__lcd->RST->GPIO_Pin_x,1);
#define RST_LOW		gpio_set_pin_value(__lcd->RST->GPIOx,__lcd->RST->GPIO_Pin_x,0);
#define RST_READ		gpio_get_pin_value(__lcd->RST->GPIOx,__lcd->RST->GPIO_Pin_x)


#define IO_IDLE 			gpio_get_pin_value(__lcd->BUSY->GPIOx,__lcd->BUSY->GPIO_Pin_x)
#endif
#define DISPLAY_ON     		   0xAF
#define DISPLAY_OFF    		   0xAE
#define SLEEP_MODE_EN     	   0xAD
#define SLEEP_MODE_DIS    	   0xAC
#define FIRST_SRAM_DATA_LATCH  0xA9
#define SECOND_SRAM_DATA_LATCH 0xAB
#define RAM_DATA               0xAB
#define RAM_ADDRESS            0xAB
#define POWER_CTRL			   0x29
#define CLOCK_FREQUENCY        0xA0 //1/8  0xA1 1/4 0xA2 1/2 0xA3 1/1
enum {
    LCD_TYPE_BLE,
	LCD_TYPE_WIFI,
	LCD_TYPE_SMILEY,
 	LCD_TYPE_NEGATIVE,
 	LCD_TYPE_HUNDRED,
 	LCD_TYPE_DEFAULT,
 	LCD_TYPE_UNIT,
}lcd_type;


#define LCD_ADDR_LEN   8
static unsigned char __lcd_ram[LCD_ADDR_LEN] = {0};

#define uchar unsigned char
#define uint  unsigned int

#define Data_Bytes               8  // IST7133 64segments=8BYTES

#define NEGATIVE 0b01000000 //'-'
const uint8_t char_map[] = {
  0b01110111, // 0 0x77
  0b01100000, // 1 0x60
  0b00111110, // 2 0x3e
  0b01111100, // 3 0x7c
  0b01101001, // 4 0x69
  0b01011101, // 5 0x5D
  0b01011111, // 6 0x5f
  0b01100100, // 7 0x64
  0b01111111, // 8 0x7f
  0b01111101, // 9 0x7d
  0b00010000, // '-'
  0b00000000  // ' '
};

#define CHAR_POS_LEN   6
#define CHAR_SEG_LEN   7
static const unsigned char lcd_num_arr[CHAR_POS_LEN][CHAR_SEG_LEN] = {
  {OED_P1A_LCD,OED_P1B_LCD,OED_P1C_LCD,OED_P1D_LCD,OED_P1E_LCD,OED_P1F_LCD,OED_P1G_LCD},  //8-14
  {OED_P2A_LCD,OED_P2B_LCD,OED_P2C_LCD,OED_P2D_LCD,OED_P2E_LCD,OED_P2F_LCD,OED_P2G_LCD},  //16-22
  {OED_P5A_LCD,OED_P5B_LCD,OED_P5C_LCD,OED_P5D_LCD,OED_P5E_LCD,OED_P5F_LCD,OED_P5G_LCD},  //40-46
  {OED_P3A_LCD,OED_P3B_LCD,OED_P3C_LCD,OED_P3D_LCD,OED_P3E_LCD,OED_P3F_LCD,OED_P3G_LCD},  //24-30
  {OED_P4A_LCD,OED_P4B_LCD,OED_P4C_LCD,OED_P4D_LCD,OED_P4E_LCD,OED_P4F_LCD,OED_P4G_LCD},  //32-38
};

void lcd_tile(int row,int type,int enable);

void cmd_init(void);

void lcd_set_seg(unsigned char seg_num)
{
	unsigned char addr;
	addr = seg_num/8;
	__lcd_ram[addr] |=(0x01<<(seg_num%8));
}
void lcd_clear_seg(unsigned char seg_num)
{
	unsigned char addr;
	addr = seg_num/8;
	__lcd_ram[addr] &=~(0x01<<(seg_num%8));
}

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



void wait_1ms()
{
  unsigned char i;
  for(i=1; i<199; ++i); // delay 1ms for MSP430 DCO frequency=1MHZ/
}
void delayms(uint Delay)
{
   uint j;
   for(j=0; j<Delay; ++j)
   wait_1ms();
 }

void ResetIC(void)
{
  RST_HIGH;
  CS_HIGH;
  CLK_HIGH;
  SDA_LOW;
  delayms(10);
  RST_LOW;
  delayms(10);
  RST_HIGH;
  delayms(10);
}

void READBUSY(void)      // 检查硬件BUSY线是否忙？注意FPC接口要连接良好，否则会一直在这里死循环。L: interface is BUSY and not ready for write command and data.
{
  unsigned char busy;
  while(1)
  {
    busy =IO_IDLE;            // 读取I/O口对应硬件BUSY线的输入值
    if (busy!=0) break;       // 硬件BUSY线高电平=不忙时跳出循环。注意高电平时不一定是1，可能是其它值，看具体I/O口的位。
  }
}

void SendData(unsigned char Command)
{
  unsigned char j;
  CS_LOW;
  SDA_HIGH;   // A0=1: Data
  CLK_LOW;
  CLK_HIGH;
  for(j=0;j<8;j++)
  {
    if(Command&0x80){
      SDA_HIGH;
   }else{
      SDA_LOW;
   	}
    CLK_LOW;
    CLK_HIGH;
    Command<<=0x01;
  }
  CS_HIGH;
}

void SendCmd(unsigned char Command)
{
  unsigned char j;
  CS_LOW;
  SDA_LOW;   // A0=0: Command
  CLK_LOW;
  CLK_HIGH;
  for(j=0;j<8;j++)
  {
    if(Command&0x80){
      SDA_HIGH;
    }else{
      SDA_LOW;
    	}
    CLK_LOW;
    CLK_HIGH;
    Command<<=0x01;
  }
  CS_HIGH;
}

void SendCmd1(unsigned char Command)  // 结束没有CS_HIGH
{
  unsigned char j;
  CS_LOW;
  SDA_LOW;   // A0=0: Command
  CLK_LOW;
  CLK_HIGH;
  for(j=0;j<8;j++)
  {
    if(Command&0x80){
      SDA_HIGH;
    }else{
      SDA_LOW;
    	}
    CLK_LOW;
    CLK_HIGH;
    Command<<=0x01;
  }
}

void ReadData(signed char *INIT_DATA)
{
  unsigned char tmp,scnt;
  *INIT_DATA=0;
  CS_LOW;
  SDA_LOW;   // A0=0: Command
  CLK_LOW;
  CLK_HIGH;
  SDA_DIR_IN;                   // 方向为输入
  for(scnt=0;scnt<8;scnt++)
  {
    CLK_LOW;
    *INIT_DATA=*INIT_DATA<<1;
    tmp=SDA_READ;  // TODO SDA_IN
    if (tmp!=0)
    *INIT_DATA=*INIT_DATA+1;
    CLK_HIGH;                    // 在时钟下降沿读取输入数据才会改变。
  }
  SDA_DIR_OUT;                  // 方向为输出
  CS_HIGH;
}

void WriteScreen(unsigned char  *DisplayData)  // BG0=BG1=0=WHITE 背景白
{
	unsigned char  j;
	SendCmd(0xac);
	SendCmd(0x2b);
	SendCmd(0x40);
	SendCmd(0xA9);
	SendCmd(0xA8);
	for(j=0;j<(Data_Bytes);j++)
	{
		SendData(*DisplayData++);
	}
	SendData(0x00); // BG0=BG1=0=WHITE
	SendCmd(0xAB);
	SendCmd(0xAA);
	SendCmd(0xAF);
//	wait_start_timer();
	/*
	delayms(10);
	READBUSY();
	SendCmd(0xAe);
	SendCmd(0x28);
	SendCmd(0xad);  */
}
void Writeram(unsigned char  *DisplayData)  // BG0=BG1=0=WHITE 背景白
{
	unsigned char  j;
	SendCmd(0xac);
	SendCmd(0x2b);
	SendCmd(0x40);
	SendCmd(0xA9);
	SendCmd(0xA8);
	for(j=0;j<(Data_Bytes);j++)
	{
		SendData(*DisplayData++);
	}
	SendData(0x00); // BG0=BG1=0=WHITE
	SendCmd(0xAB);
	SendCmd(0xAA);
	SendCmd(0xAF);
	delayms(10);
	READBUSY();

	SendCmd(0xAe);
	SendCmd(0x28);
	SendCmd(0xad);
}

void lut_GC()  // GC 3段波形 黑剩余+白+黑置位
{
  	SendCmd(0x82); //set wave form
    SendCmd(0x20);
    SendCmd(0x00);
    SendCmd(0xA0);
    SendCmd(0x80);
    SendCmd(0x40);
  	SendCmd(0x63);   // 设置3段波形长度
}
void lut_DU_WB()  // DU波形 白消图+黑出图 局刷波形
{
  	SendCmd(0x82);
    SendCmd(0x80);
    SendCmd(0x00);
    SendCmd(0xC0);
    SendCmd(0x80);
    SendCmd(0x80);
  	SendCmd(0x62);   // 设置2段波形长度
}
 static int  VAR_Temperature=25;  // 温度值

void Temperature(void)  // 建议定时测量温度，修改驱动参数
{
  // 客户程序把外部温度传感器测量的实时温度值赋值给全局变量VAR_Temperature
  // VAR_Temperature=外部温度传感器测量实时温度值，单位：摄氏度
    if (VAR_Temperature<10)
    {
      SendCmd(0x7E);
      SendCmd(0x81);
        SendCmd(0xE0); // HV=18V
    }
    else
    {
      SendCmd(0x7b);
      SendCmd(0x81);
        SendCmd(0xE4); // HV=15V
    }
    SendCmd(0xe7);     // Set default frame time
    if (VAR_Temperature<5)
      SendCmd(0x31); //0x22  (49+1)*20ms=1000ms
    else if (VAR_Temperature<10)
      SendCmd(0x22); //0x22  (34+1)*20ms=700ms
    else if (VAR_Temperature<15)
      SendCmd(0x18); //0x18  (24+1)*20ms=500ms
    else if (VAR_Temperature<20)
      SendCmd(0x13); //0x13  (19+1)*20ms=400ms
    else
      SendCmd(0x0e); //0x0e  (14+1)*20ms=300ms
                   //0x09  ( 9+1)*20ms=200ms
}

//const unsigned char Const_Count=61;  // 多少次DU局刷更新1次GC全刷更新，建议值：10次
//unsigned int           VAR_Count=0;
#define REFRESH_TIMER    30*60*1000
void LUT_nDU_1GC(void)  // 每局刷更新Const_Count次，GC全刷更新1次，以清除残影
{
	static int pass_tick=0;
	int  cur_tick=0;
	cur_tick=system_get_curr_time();
	if (cur_tick-pass_tick>=REFRESH_TIMER){
		cmd_init();
		lut_GC();
		pass_tick=cur_tick;
	}else
		lut_DU_WB();
	if(cur_tick-pass_tick<=0){
		pass_tick=cur_tick;
		co_printf("cur_tick-pass_tick %d\r\n",cur_tick-pass_tick);
	}
	Temperature();        // 测量温度，修改驱动参数
}
enum{LCD_IDLE,LCD_DOING};

static int lcd_state=LCD_IDLE;
#define RESET_TIMER  5*60*1000
static void lcd_reset(void)
{
	static int pass_tick=0;
	int cur_tick=system_get_curr_time();

	if (cur_tick-pass_tick>=RESET_TIMER){
		system_set_port_mux(__lcd->BUSY->GPIOx,__lcd->BUSY->GPIO_Pin_x,PORT_FUNC_GPIO);
		gpio_set_dir(__lcd->RST->GPIOx,__lcd->RST->GPIO_Pin_x,GPIO_DIR_OUT);
		co_printf(" RESET timer----------------------- %d\r\n",cur_tick-pass_tick);
		cmd_init();
		pass_tick=cur_tick;
	}
	if(cur_tick-pass_tick<0){
		pass_tick=cur_tick;
		co_printf(" RESET time cur_tick-pass_tick %d\r\n",cur_tick-pass_tick);
	}

}
void lcd_update(void)
{
/*
	unsigned char busy;
	busy =IO_IDLE;
	if(busy){
		LUT_nDU_1GC();
		WriteScreen(__lcd_ram);
	}*/

	system_set_port_mux(__lcd->CL->GPIOx,__lcd->CL->GPIO_Pin_x,PORT_FUNC_GPIO);
	system_set_port_mux(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x,PORT_FUNC_GPIO);
	gpio_set_dir(__lcd->CL->GPIOx,__lcd->CL->GPIO_Pin_x,GPIO_DIR_OUT);
	gpio_set_dir(__lcd->DA->GPIOx,__lcd->DA->GPIO_Pin_x,GPIO_DIR_OUT);
//	co_printf("lcd_update %d lcd_state %d \r\n",IO_IDLE,lcd_state);
//	ResetIC();
	lcd_reset(); //5 minute reset once
	if(!IO_IDLE){
		return;
	}
	if(lcd_state==LCD_IDLE){
		LUT_nDU_1GC();
		WriteScreen(__lcd_ram);
		lcd_state=LCD_DOING;
		//co_printf("doing..................... \r\n");
		return;
	}else if(lcd_state==LCD_DOING){
		SendCmd(0xAe);
		SendCmd(0x28);
		SendCmd(0xad);
		lcd_state=LCD_IDLE;
		//co_printf("idle...................... \r\n");
		return;
	}
	co_printf("lcd_state-----------------------%d \r\n",lcd_state);
}
bool lcd_is_block(){
	return  (lcd_state==LCD_DOING?true:false);
}

void WriteScreen1(unsigned char  *DisplayData) // BG0=BG1=1=BLACK 背景黑
{
  unsigned char  j;
  SendCmd(0xac);
  SendCmd(0x2b);
  SendCmd(0x40);
  SendCmd(0xA9);
  SendCmd(0xA8);
	for(j=0;j<(Data_Bytes);j++)
	{
    SendData(*DisplayData++);
  }
  SendData(0x03);  // BG0=BG1=1=BLACK
  SendCmd(0xAB);
  SendCmd(0xAA);
  SendCmd(0xAF);
  delayms(10);
  READBUSY();
  SendCmd(0xAe);
  SendCmd(0x28);
  SendCmd(0xad);
}

void lcd_clear(void){
	memset(__lcd_ram,0,sizeof(__lcd_ram));
	//co_printf("---------------------------------------lcd_clear \r\n");
	lut_GC();
	Writeram(__lcd_ram);
    return;
}
void lcd_envconfig(int temp){
    VAR_Temperature=temp/100;
    return ;
}
//full lcd's context
void lcd_full(void){
  memset(__lcd_ram,0xff,sizeof(__lcd_ram));
  LUT_nDU_1GC();
  Writeram(__lcd_ram);
  return;
}

void lcd_turn_off(void){
	//lut_GC();
	return;
}

void lcd_turn_on(void){
  return;
}
void initLCDM(void)
{
    SendCmd(0xa7);
    SendCmd(0xe0);
	Temperature();
}

void cmd_init(void){
	ResetIC();
	initLCDM();
}
static void lcd_begin(void) {
    assert_param(__lcd != NULL);
    lcd_TypeDef* lcd=__lcd;

	system_set_port_mux(lcd->CS->GPIOx,(1<<lcd->CS->GPIO_Pin_x),PORT_FUNC_GPIO);
	system_set_port_mux(lcd->BUSY->GPIOx,lcd->BUSY->GPIO_Pin_x,PORT_FUNC_GPIO);
    pmu_set_pin_to_PMU(lcd->CS->GPIOx, (1<<lcd->CS->GPIO_Pin_x));
    pmu_set_pin_to_PMU(lcd->BUSY->GPIOx, (1<<lcd->BUSY->GPIO_Pin_x));
    pmu_set_pin_dir(lcd->CS->GPIOx, (1<<lcd->CS->GPIO_Pin_x), GPIO_DIR_OUT);
    pmu_set_pin_dir(lcd->BUSY->GPIOx, (1<<lcd->BUSY->GPIO_Pin_x), GPIO_DIR_IN);
	pmu_set_gpio_value(lcd->CS->GPIOx,(1<<lcd->CS->GPIO_Pin_x),1);

	system_set_port_mux(lcd->CL->GPIOx,lcd->CL->GPIO_Pin_x,PORT_FUNC_GPIO);
	system_set_port_mux(lcd->DA->GPIOx,lcd->DA->GPIO_Pin_x,PORT_FUNC_GPIO);
	system_set_port_mux(lcd->RST->GPIOx,lcd->RST->GPIO_Pin_x,PORT_FUNC_GPIO);
	gpio_set_dir(lcd->CL->GPIOx,lcd->CL->GPIO_Pin_x,GPIO_DIR_OUT);
	gpio_set_dir(lcd->DA->GPIOx,lcd->DA->GPIO_Pin_x,GPIO_DIR_OUT);
	gpio_set_dir(lcd->RST->GPIOx,lcd->RST->GPIO_Pin_x,GPIO_DIR_OUT);
	gpio_set_pin_value(lcd->CL->GPIOx,lcd->CL->GPIO_Pin_x,1);
	gpio_set_pin_value(lcd->DA->GPIOx,lcd->DA->GPIO_Pin_x,1);
	gpio_set_pin_value(lcd->RST->GPIOx,lcd->RST->GPIO_Pin_x,1);
 }

lcd_TypeDef* lcd_init(PortPin_Map *CS, PortPin_Map *CL, PortPin_Map *DA,PortPin_Map *RST,PortPin_Map *BUSY)
{
  lcd_TypeDef *ret=NULL;

  assert_param(CS != NULL);
  assert_param(CL != NULL);
  assert_param(DA != NULL);
  assert_param(RST != NULL);
  assert_param(BUSY!= NULL);

  ret = os_calloc(1, sizeof(lcd_TypeDef));
  if(!ret)
    return NULL;
  ret->CS = CS;
  ret->CL = CL;
  ret->DA = DA;
  ret->RST = RST;
  ret->BUSY = BUSY;
  __lcd = ret; // NOTE: This line must exist for all user APIs.

  lcd_begin();
  cmd_init();
  lcd_clear();
  return ret;
}
void lcd_test(){
	static int i=0;
	lcd_show_num(0,i);
	lcd_show_num(1,i);
	lcd_show_num(2,i);
	lcd_show_num(3,i);
	lcd_show_num(4,i);
	i++;
	if(i==9)
		i=0;
	co_printf("i=%d\r\n",i);
}
void lcd_write_ram( unsigned char position, unsigned char digital)
{
	DEV_DEBUG("pos %d data %d\r\n",position,digital);
	//lcd_test();
	lcd_show_num(position,digital);
}

void lcd_battery_power(const unsigned char electry)
{

	 assert_param(__lcd != NULL);
	unsigned char electry_level=0;
	unsigned char i = 0;
	unsigned char electry_icon[5]={OED_Q1A_LCD,OED_Q1E_LCD,OED_Q1D_LCD,OED_Q1C_LCD,OED_Q1B_LCD};
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
	//co_printf("electr  y_level is %d\r\n",electry_level);
	for(i=0;i<sizeof(electry_icon);i++)//clear electry icon
	{
		lcd_clear_seg(electry_icon[i]);
	}
	for(i=0;i<electry_level+1;i++)//set electry icon
	{
		lcd_set_seg(electry_icon[i]);
	}
	//lcd_update();
}
void lcd_tile(int row,int type,int enable){
	//co_printf("row %d type %d enable %d\r\n",row,type,enable);
    if(type == LCD_TYPE_BLE){
        if(enable)
            lcd_set_seg(OED_BLE_LCD);
        else
            lcd_clear_seg(OED_BLE_LCD);
    }else if(type == LCD_TYPE_WIFI){
        if(enable)
            lcd_set_seg(OED_WIFI_LCD);
        else
            lcd_clear_seg(OED_WIFI_LCD);
    }else if(type == LCD_TYPE_SMILEY){
        if(enable){
			if(row==0){
				lcd_clear_seg(OED_SAD_LCD);
	            lcd_set_seg(OED_SIMILE_LCD);
			}else{
				lcd_clear_seg(OED_SIMILE_LCD);
	            lcd_set_seg(OED_SAD_LCD);
			}
        }else{
            lcd_clear_seg(OED_SIMILE_LCD);
			lcd_clear_seg(OED_SAD_LCD);
        }
    }else if(type == LCD_TYPE_NEGATIVE){
		 if(enable)
			 lcd_set_seg(OED_R1A_LCD);
		 else
			 lcd_clear_seg(OED_R1A_LCD);
	 }else if(type == LCD_TYPE_HUNDRED){
	  if(enable){
		  	 lcd_set_seg(OED_R1B_LCD);
	  		 lcd_set_seg(OED_R1C_LCD);
		 }else{
		 	 lcd_clear_seg(OED_R1B_LCD);
		 	 lcd_clear_seg(OED_R1C_LCD);
		 }
	 }else if(type == LCD_TYPE_UNIT){
		 if(enable){
		 	 lcd_clear_seg(OED_UNITC_LCD);
		  	 lcd_set_seg(OED_UNITF_LCD);
		 }else{
		 	 lcd_clear_seg(OED_UNITF_LCD);
		  	 lcd_set_seg(OED_UNITC_LCD);
		 }
	 }
	lcd_set_seg(OED_PION_LCD);
	lcd_set_seg(OED_PERCENT_LCD);
	lcd_set_seg(OED_UNITA_LCD);
	//lcd_update();
}


#if 1
static void lcd_putchar_cached(int index,unsigned char c) {
  assert_param(__lcd != NULL);
   //co_printf("index[%d],char is %c[%d]\r\n",index,c,c);

    //clear
    if(c == 0){
        DEV_DEBUG("index[%d],clear %d\r\n",index,c);
       // lcd_write_ram(index,17);
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
		  #if 0
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
          case 'G':
              lcd_write_ram(index,16);
              break;
		   case 'H':
              lcd_write_ram(index,17);
              break;
          case 'L':
              lcd_write_ram(index,18);
              break;
          case '-':
              lcd_write_ram(index,19);
              break;
		  #endif
          default:{ // do nothing, blank digit!
			//  lcd_write_ram(index,20);
              DEV_ERR("lcd write char[%d] is  invaild\r\n",c);
          }break;
    }
}

void IsNegnative(unsigned char* sh_str,int str_len){

	if(str_len==4){
		if(sh_str[0]=='1'){
		  	lcd_tile(0,LCD_TYPE_HUNDRED,true);
		}else{
		  	lcd_tile(0,LCD_TYPE_HUNDRED,false);
		}
		for(int i=str_len-1; i>=1; i--) {
			  lcd_putchar_cached(i-1,sh_str[i]);
		}

		lcd_tile(0,LCD_TYPE_NEGATIVE,false);
		return;
	}


	if(str_len>=5){
		if(sh_str[1]=='1'){
			lcd_tile(0,LCD_TYPE_HUNDRED,true);
		}else{
			lcd_tile(0,LCD_TYPE_HUNDRED,false);
		}
		if(sh_str[0]=='-'){
			lcd_tile(0,LCD_TYPE_NEGATIVE,true);
		}else{
			lcd_tile(0,LCD_TYPE_NEGATIVE,false);
		}
		for(int i=str_len-1; i>=2; i--) {
			lcd_putchar_cached(i-2,sh_str[i]);
		}
		return;
	}
	return;
}

//the pos from 0-12
static bool isValueForTemperature(int len)
{
	if(len>=4)
		return true;
	return false;
}
void lcd_put_tem(int pos,unsigned char* str,int str_len,unsigned char unit) {
	//co_printf("str_len %d\r\n",str_len);
    int i=0;
    assert_param(__lcd != NULL);
 	assert_param(str != NULL);
    assert_param(str_len <= NUM_DIGITS_MAX );
	unsigned char sh_str[NUM_DIGITS_MAX+1]={0};  //make up string
    memcpy(sh_str,str,str_len);
   //string must  3 bytes
 //  co_printf("str_len %d unit %d str %s\r\n",str_len,unit,sh_str);
//   lcd_test();
#if 1
   if(isValueForTemperature(str_len)){ //temp
	   IsNegnative(sh_str,str_len);
   }else{ //humi
	   for(i=0; i<str_len; i++) {
		   lcd_putchar_cached(pos+i,sh_str[i]);
	   }
   	}
   #endif
   lcd_tile(0,LCD_TYPE_UNIT,unit);

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
	dev->lcd_update=lcd_update;
	dev->lcd_envconfig=lcd_envconfig;
	dev->lcd_is_block=lcd_is_block;
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

