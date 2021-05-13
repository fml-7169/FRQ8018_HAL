#ifndef HAL_TYPES_H_
#define HAL_TYPES_H_

typedef unsigned  int   uint32;
typedef int             int32;
typedef unsigned short  uint16;
typedef short           int16;
typedef char            int8;
typedef unsigned char   uint8;

typedef unsigned long long  uint64;

typedef struct _Pin_Map {
  uint32 GPIOx;
  uint32 GPIO_Pin_x;
  uint32 GPIO_Func;
} Pin_Map;

#define PIN_PORT_PIN(GPIO,PIN)  ((uint32)1<<((GPIO*8)+PIN))

#define PLATFORM_HAL_VERSION    "1.00.10"

#endif