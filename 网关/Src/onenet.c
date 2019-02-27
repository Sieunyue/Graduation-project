#include "onenet.h"
#include "EdpKit.h"
#include "usart.h"
#include "string.h"
#include "stdlib.h"
#include "end.h"
//·#include "stm32l1xx_it.h"


const char DEID[] = "503223399";
const char INFO[] = "0001";
const char APIKey[] = "2ar=0yznUqoLTIw6t4P2fOIprog=";
extern uint8_t rxbuf2[50];
extern uint8_t rxbuf1[50];
extern uint8_t state2;
extern uint8_t count2;
uint8_t addr[3];

/*
 * 函数名： SenData
 * 功能：   上传数据点到onenet
 * 参数：   data    数据点名称
 *          value   数据点的值
 * 返回值： 无
 */
void SendDataToOnenet(const char *data, int value)
{
    EdpPacket *send_pkg;
    cJSON *json_data = cJSON_CreateObject();
    cJSON_AddItemToObject(json_data, data, cJSON_CreateNumber(value));
    memset(rxbuf2, 0, sizeof(rxbuf2));
    send_pkg = PacketSavedataJson(NULL, json_data, kTypeSimpleJsonWithoutTime, 0);
    HAL_UART_Transmit(&huart2, (send_pkg->_data), send_pkg->_write_pos, 100);
    DeleteBuffer(&send_pkg);
    cJSON_Delete(json_data);
    delay_ms(250);
}
void DevInit()
{
    ConnectDerive();
	state2 = 0x01;
    memset(rxbuf2, 0, sizeof(rxbuf2));
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
	delay_ms(1000);
	delay_ms(1000);
}
/*
 * 函数名： ConnectDerive
 * 功能：   连接到onenet
 * 参数：   无
 * 返回值： 无
 */
void ConnectDerive()
{
    EdpPacket *send_pkg;
    if ((send_pkg = PacketConnect1(DEID, APIKey)) != NULL)
    {
        while (rxbuf2[0] != CONNRESP)
        {
            HAL_UART_Transmit(&huart2, (send_pkg->_data), send_pkg->_write_pos, 10);
            delay_ms(1000);
        }
    }
    DeleteBuffer(&send_pkg);
}
/*
 * 函数名： IsOnenetCom
 * 功能：   判断是否Onenet下发命令
 * 参数：   recbuf      返回数据的数组 
 * 返回值： 无
 */
void IsOnenetCmd(uint8_t *recbuf)
{
    //EdpPacket *rec_pkg = NewBuffer();
    //WriteBytes(rec_pkg, recbuf, len);
    //rec_pkg = GetEdpPacket(rec_pkg);
    //mtype = EdpPacketType(rec_pkg);
    const uint8_t *mtype = recbuf;
    switch (*mtype)
    {
    case CONNRESP:
        break;
    case CMDREQ:
        HandleCmd(rxbuf2);
        break;
    case 0x40:
        ConnectDerive();
        break;
    case PINGRESP:
		break;
    default:
        break;
    }
}
/*
 * 函数名： HandleCmd()
 * 功能：   处理One下发命令，判断命令类型
 * 参数：   rec_buf      返回数据的数组 
 * 返回值： 无
 */
void HandleCmd(uint8_t *rec_buf)
{
    uint8_t cmd_id[2];
    uint8_t *temp = rec_buf;
    uint8_t i = 0;
    uint8_t valu[2]={0x00,0x00};
    while (*temp != '{')
    {
        temp++;
    }
    temp++;
    while (*temp != '}')
    {
        cmd_id[i++] = *temp++;
    }
    temp++;
    i=0;
    while(*temp != '\0')
    {
        valu[i++] = *temp++;
    }
	if(valu[1] == 0x00)
	{
		valu[1]=valu[0];
		valu[0]=0x30;
	}
//    if (cmd_id[0] == 's' && cmd_id[1] == 't')
//    {
//        led1.st = *temp - 48;
//        SendDataToOnenet("statew", led1.st);
//        PushCmdToEnd(&led1,WNF, led1.st, led1.st); 
//    }
//    else if (cmd_id[0] == 'b' && cmd_id[1] == 'r')
//    {
//        led1.br = ((valu[0]-48)*10+(valu[1]-48));
//		led1.bc = led1.br*(led1.per/100.00);
//        led1.bw = led1.br*((100-led1.per)/100.00);
//        led1.bc= led1.bc<=0x01? 0x01:led1.bc;
//        led1.bw= led1.bw<=0x01? 0x01:led1.bw;
//			SendDataToOnenet("bright", led1.br);
//        PushCmdToEnd(&led1,WBR,led1.br,((int)led1.per));
//    }
//    else if (cmd_id[0] == 'p' && cmd_id[1] == 'r')
//    {
//        led1.per =(valu[0]-48)*10+(valu[1]-48);
//        led1.bc = led1.br*(led1.per/100.00);
//        led1.bw = led1.br*((100-led1.per)/100.00);
//        led1.bc= led1.bc<=0x01? 0x01:led1.bc;
//        led1.bw= led1.bw<=0x01? 0x01:led1.bw;
//		SendDataToOnenet("per", (int)led1.per);
//        PushCmdToEnd(&led1,WPR, led1.br,(int) led1.per);
//    }
    
}
void SendPing()
{
    EdpPacket *send_pkg;
    send_pkg = PacketPing();
    HAL_UART_Transmit(&huart2, (send_pkg->_data), send_pkg->_write_pos, 10);
    DeleteBuffer(&send_pkg);
}
void delay_ms(uint32_t time)
{

    int i = 0;
    while (time--)
    {
        i = 12000;
        while (i--)
            ;
    }
}
