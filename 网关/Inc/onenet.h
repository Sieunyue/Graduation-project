#ifndef __ONENET_H__
#define __ONENET_H__

#include "stm32f1xx_hal.h"
#include "EdpKit.h "
#include "end.h"

typedef struct 
{
    void (*Connect)(void);
    void (*SendValue)(const char *data,int value);
    void (*SendString)(END_TypeDef *end);
    void (*Process)(void);

}ETH_t;

void ETH_Init(void);
void delay_ms(uint32_t nms);
void StringToHex(const uint8_t *str ,uint32_t num,uint8_t *hex);
extern ETH_t ETHDev;


#endif

