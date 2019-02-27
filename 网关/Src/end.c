#include "end.h"
#include "usart.h"
#include "stdlib.h"
END_TypeDef *led[10];

extern uint8_t count1;
extern uint8_t state1;
#define SETBRIGHT                   0x10 //设置亮度
#define SETPROPORTION               0x11 //设置冷光比例
#define SETON                       0x14 //开/关

#define READDATA                    0x01 //读取数据
#define JOINNET                     0x02 //加入网络
END_Head Head;
void PushCmdToEnd(uint num, uint8_t cmd_type, int val1 ,int val2)
{
    uint8_t send_dat[12];
    uint8_t len = 0x00;
	uint8_t *paddr = mend->short_addr;
	send_dat[len++] = 0xFC;
	len++;
	send_dat[len++] = 0x03;
    send_dat[len++] = 0x01; //透传方式
    paddr++;
    send_dat[len++] = *paddr++;
    send_dat[len++] = *paddr; //点播地址
    send_dat[len++] = cmd_type;
    if (cmd_type == WBR || cmd_type == WPR || cmd_type == WNF)S
    {
        send_dat[len++] = val1;
        send_dat[len++] = val2;
    }
    else if (cmd_type == RBR)
    {
        send_dat[len++] = 0xFF;
        send_dat[len++] = 0xFF;
    }
    send_dat[1] = len - 2;    
    HAL_UART_Transmit(&huart1, send_dat, len, 10);

}

void EndMsg(uint8_t *recbuf)
{
	uint8_t *prec_buf = recbuf;
    if(*(prec_buf+8) = JOINNET)
    {
        CreatNewDevice();
    }
    
}

void CreatNewDevice(uint8_t *recbuf)
{
    uint8_t *prec_buf = recbuf;
    Head.save_end[end_num] = (END_TypeDef *)malloc(sizeof(END_TypeDef));
    end_num++;
    for(int i=0;i<8;i++)
    {
        Head.save_end[end_num]->mac[i] = *prec_buf++;
    }
    Head.save_end[end_num]->bright=*(prec_buf++)++;
    Head.save_end[end_num]->proportion=*prec_buf++;
    Head.save_end[end_num]->on_state = *prec_buf;
}
