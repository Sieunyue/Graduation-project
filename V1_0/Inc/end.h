#ifndef __END_H
#define __END_H
#include "stm32f0xx_hal.h"

typedef enum
{
    false = 0,
		true = 1
}bool;

typedef enum 
{
    Dev_Init,
    Dev_JoinNet,
    Dev_Run
}DevState_e;
typedef struct
{
    uint8_t EndMac[8];
    uint32_t FlashAddr;
    uint8_t LedBright;
    bool LedOn;
    uint8_t LedPer;
} EndParameter_t;

typedef struct
{
    bool IsJoinNet;
    bool KeyFlag;
    EndParameter_t EndParameter;
    // void (*Join_Net)(void);
    void (*Start_Led)(void);
    // void (*Read_Flash)(void);
    // void (*Write_Flash)(void);
    void (*Send)(uint8_t data_type);
    void (*Process)(void);

} EndDevice_t;
extern EndDevice_t EndDev;
void Init_Device(void);
void End_Init(void);
void Send_Data(uint8_t data_type);
void Hand_Recbuf(uint8_t *rec);
void Write_Pwm(void);
uint8_t MacCmp(uint8_t *rec);
void Key_Scan(void);
void delay_ms(int time);
void HexToInt(void);
#endif
