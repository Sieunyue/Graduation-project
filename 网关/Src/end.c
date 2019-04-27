#include "end.h"
#include "usart.h"
#include "stdlib.h"
#include "onenet.h"
END_TypeDef *led[10];

extern uint8_t count1;
extern uint8_t state1;
extern uint8_t rxbuf1[];

END_Head Head;
//void PushCmdToEnd(uint8_t num, uint8_t cmd_type, int val1 ,int val2)
//{
////    uint8_t send_dat[12];
////    uint8_t len = 0x00;
////	uint8_t *paddr = mend->short_addr;
////	send_dat[len++] = 0xFC;
////	len++;
////	send_dat[len++] = 0x03;
////    send_dat[len++] = 0x01; //透传方式
////    paddr++;
////    send_dat[len++] = *paddr++;
////    send_dat[len++] = *paddr; //点播地址
////    send_dat[len++] = cmd_type;
////    if (cmd_type == WBR || cmd_type == WPR || cmd_type == WNF)
////    {
////        send_dat[len++] = val1;
////        send_dat[len++] = val2;
////    }
////    else if (cmd_type == RBR)
////    {
////        send_dat[len++] = 0xFF;
////        send_dat[len++] = 0xFF;
////    }
////    send_dat[1] = len - 2;
////    HAL_UART_Transmit(&huart1, send_dat, len, 10);

//}
/*
 * 函数名： EndMsg
 * 功能：   处理终端传来的消息
 * 参数：   recbuf       串口缓冲区
 * 返回值： 无
 */
void EndMsg(const uint8_t *recbuf)
{
    if (*(recbuf + 8) == JOINNET)
    {
        if (FindDevMac(recbuf) == 11)
        {
            CreatNewDevice(recbuf);
            uint8_t temp_msg[7];
            temp_msg[0] = 0xFC;
            temp_msg[1] = 0x05;
            temp_msg[2] = 0x01;
            temp_msg[3] = 0x01;
            temp_msg[4] = 0xFF;
            temp_msg[5] = 0xAA;
            temp_msg[6] = 0xFF;
            HAL_UART_Transmit(&huart1,temp_msg,7,10);
        }
    }
    else if (*(recbuf + 8) == SETON)
    {
        uint32_t num;
        num = FindDevMac(recbuf);
        if (num != 11)
        {
            Head.save_end[num]->on_state = *(recbuf + 11);
            SendStringToOnenet(Head.save_end[num]);
        }
    }
    else if (*(recbuf + 8) == READDATA)
    {
        uint32_t num;
        num = FindDevMac(recbuf);
        if (num != 11)
        {
            Head.save_end[num]->on_state = *(recbuf + 11);
            Head.save_end[num]->bright = *(recbuf + 9);
            Head.save_end[num]->proportion = *(recbuf + 10);
            SendStringToOnenet(Head.save_end[num]);
        }
    }
}
/*
 * 函数名： EndMsg
 * 功能：   新建一个终端设备
 * 参数：   recbuf       串口缓冲区
 * 返回值： 无
 */
void CreatNewDevice(const uint8_t *recbuf)
{
    END_TypeDef *temp = (END_TypeDef *)malloc(sizeof(END_TypeDef));
    Head.save_end[Head.end_num] = temp;
    for (int i = 0; i < 8; i++)
    {
        temp->mac[i] = *recbuf++;
    }
    recbuf++;
    temp->bright = *recbuf++;
    temp->proportion = *recbuf++;
    temp->on_state = *recbuf;
    Head.end_num++;
    SendStringToOnenet(temp);
}

/*
 * 函数名： HexsToChar
 * 功能：   将16进制数组转换成字符串
 * 参数：   *hex            16进制数组
 *          num             数组长度
 *          *save_char      存储字符串
 * 返回值： 无
 */
void HexsToChar(uint8_t *hex, uint32_t num, char *save_char)
{
    for (int i = 0; i < num; i++)
    {
        save_char[(i * 2)] = ((*(hex + i)) / 16);
        save_char[(i * 2) + 1] = (*(hex + i)) & 0x0F;
        if (save_char[(i * 2)] <= 0x09)
        {
            save_char[(i * 2)] += 48;
        }
        else
        {
            save_char[(i * 2)] += 55;
        }
        if (save_char[(i * 2) + 1] <= 0x09)
        {
            save_char[(i * 2) + 1] += 48;
        }
        else
        {
            save_char[(i * 2) + 1] += 55;
        }
    }
}
/*
 * 函数名： HexsToChar
 * 功能：   将16进制转换成字符串
 * 参数：   hex            16进制
 *         *save_char      存储字符串
 * 返回值： 无
 */
void HexToChar(uint8_t hex, char *save_char)
{
    save_char[0] = hex / 16;
    save_char[1] = hex & 0x0F;
    if (save_char[0] <= 0x09)
    {
        save_char[0] += 48;
    }
    else
    {
        save_char[0] += 55;
    }
    if (save_char[1] <= 0x09)
    {
        save_char[1] += 48;
    }
    else
    {
        save_char[1] += 55;
    }
}
void ValueToMsg(char *msg_pkg, uint8_t *msg_id, uint8_t msg_bright, uint8_t msg_porportion, uint8_t msg_on)
{

    char *temp = msg_pkg + 16;
    HexsToChar(msg_id, 8, msg_pkg);
    *temp = ':';
    temp++;
    HexToChar(msg_bright, temp);
    temp += 2;
    HexToChar(msg_porportion, temp);
    temp += 2;
    HexToChar(msg_on, temp);
    temp += 2;
    *temp = '\0';
}
uint32_t FindDevMac(const uint8_t *mac)
{
    uint32_t k, i;
    const uint8_t *des_mac;
    for (i = 0; i < Head.end_num; i++)
    {

        for (k = 0; k < 8; k++)
        {
            des_mac = Head.save_end[i]->mac + k;
            if (*(des_mac) != *(mac + k))
            {
                break;
            }
        }
        if (k == 8)
        {
            return i;
        }
    }
    return 11;
}
