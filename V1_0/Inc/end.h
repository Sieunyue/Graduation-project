#ifndef __END_H
#define __END_H
#include "stm32f0xx_hal.h"

#define END_DATA                    0x08003c00                          
#define SETBRIGHT                   0x10                                
#define SETPROPORTION               0x11                                
#define SETON                       0x12                                

#define READDATA                    0x01                                
#define JOINNET                     0x02                                


typedef struct 
{
    /* data */  
    uint8_t online;
    uint8_t led_bright;
    uint8_t led_per;
    uint8_t led_state;
    uint8_t *mac_addr;
    uint8_t short_addr[2];
}End_Device;


void Init_Device(void);
void Send_Data(uint8_t data_type);
void Hand_Recbuf(uint8_t *rec);
void Write_Pwm(void);
uint8_t MacCmp(uint8_t *rec);
void Key_Scan(void);
void delay_ms(int time);

#endif
