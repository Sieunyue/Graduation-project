#ifndef __ONENET_H__
#define __ONENET_H__

#include "stm32f1xx_hal.h"
#include "EdpKit.h"



void ConnectDerive(void);
void SendDataToOnenet(const char *data, int value);
void delay_ms(uint32_t nms);

void IsOnenetCmd(uint8_t *recbuf);
void HandleCmd(uint8_t *rec_buf);
void SendPing(void);
void DevInit(void);




#endif

