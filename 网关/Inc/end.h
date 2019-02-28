#ifndef __END_H__
#define __END_H__

#include "stm32f1xx_hal.h"
#define SETBRIGHT 0x10     //设置亮度
#define SETPROPORTION 0x11 //设置冷光比例
#define SETON 0x12         //开/关

#define READDATA 0x01 //读取数据
#define JOINNET 0x02  //加入网络
typedef struct
{
    uint8_t bright;
    uint8_t proportion;
    uint8_t mac[8];
    uint8_t on_state;
} END_TypeDef;
typedef struct
{
    uint32_t end_num;
    END_TypeDef *save_end[10];
    uint8_t online_state;
} END_Head;

void PushCmdToEnd(uint8_t cmd_type, int val1, int val2);
void EndMsg(const uint8_t *recbuf);
void CreatNewDevice(const uint8_t *recbuf);
void HexsToChar(uint8_t *hex, uint32_t num, char *save_char);
void HexToChar(uint8_t hex, char *save_char);
void ValueToMsg(char *msg_pkg, uint8_t *msg_id, uint8_t msg_bright, uint8_t msg_porportion, uint8_t msg_on);
uint32_t FindDevMac(const uint8_t *mac);
#endif
