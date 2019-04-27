#include "onenet.h"
#include "EdpKit.h"
#include "usart.h"
#include "string.h"
#include "stdlib.h"
//·#include "stm32l1xx_it.h"

const char DEID[] = "503223399";
const char INFO[] = "0001";
const char APIKey[] = "2ar=0yznUqoLTIw6t4P2fOIprog=";
extern uint8_t rxbuf2[50];
extern uint8_t rxbuf1[50];
extern uint8_t state2;
extern uint8_t count2;
extern END_Head Head;

/*
 * 函数名： SendValueToOnenet
 * 功能：   上传数据点到onenet，适用于int型数据
 * 参数：   data    数据点名称
 *          value   数据点的值
 * 返回值： 无
 */
void SendValueToOnenet(const char *data, int value)
{
    EdpPacket *send_pkg;
    cJSON *json_data = cJSON_CreateObject();
    cJSON_AddItemToObject(json_data, data, cJSON_CreateNumber(value));
    send_pkg = PacketSavedataJson(NULL, json_data, kTypeSimpleJsonWithoutTime, 0);
    HAL_UART_Transmit(&huart2, (send_pkg->_data), send_pkg->_write_pos, 100);
    DeleteBuffer(&send_pkg);
    cJSON_Delete(json_data);
    delay_ms(200);
}
/*
 * 函数名： SendStringToOnenet
 * 功能：   将终端MAC地址和数据点转化成字符串，上传到Onenet
 * 参数：   END_TypeDef *end          终端设备          
 * 返回值： 无
 */
void SendStringToOnenet(END_TypeDef *end)
{
    EdpPacket *send_pkg;
    char msg_pkg[26];
    msg_pkg[0] = ':';
    msg_pkg[1] = ',';
    ValueToMsg((msg_pkg + 2), end->mac, end->bright, end->proportion, end->on_state);
    send_pkg = PacketSavedataSimpleString(NULL, msg_pkg, 0);
    HAL_UART_Transmit(&huart2, (send_pkg->_data), send_pkg->_write_pos, 100);
    DeleteBuffer(&send_pkg);
    delay_ms(200);
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
        HAL_UART_Transmit(&huart2, (send_pkg->_data), send_pkg->_write_pos, 10);
        delay_ms(200);
    }
    DeleteBuffer(&send_pkg);
    SendValueToOnenet("Online_Device", Head.end_num);
}
/*
 * 函数名： IsOnenetCom
 * 功能：   判断是否Onenet下发命令
 * 参数：   recbuf      返回数据的数组 
 * 返回值： 无
 */
void IsOnenetCmd(const uint8_t *recbuf)
{
    const uint8_t *mtype = recbuf;
    switch (*mtype)
    {
    case CONNRESP:
        Head.online_state = 1;
        break;
    case CMDREQ:
        HandleCmd(rxbuf2);
        break;
    case 0x40:
        Head.online_state = 0;
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
void HandleCmd(const uint8_t *rec_buf)
{
    const uint8_t *prec = rec_buf + 44;
		//uint8_t dev_index;
    uint8_t temp_msg[16];
    temp_msg[0] = 0xFC;
    temp_msg[1] = 0x0E;
    temp_msg[2] = 0x01;
    temp_msg[3] = 0x01;
    StringToHex(prec,12,temp_msg+4);
    // dev_index = FindDevMac(temp_msg+4);
    // Head.save_end[dev_index]->bright = temp_msg[13];
    // Head.save_end[dev_index]->proportion = temp_msg[14];
    // Head.save_end[dev_index]->on_state= temp_msg[15];
    // SendStringToOnenet(Head.save_end[dev_index]);
    HAL_UART_Transmit(&huart1,(uint8_t*)temp_msg,16,10);
}
void SendPing()
{
    EdpPacket *send_pkg;
    send_pkg = PacketPing();
    HAL_UART_Transmit(&huart2, (send_pkg->_data), send_pkg->_write_pos, 10);
    DeleteBuffer(&send_pkg);
}
void StringToHex(const uint8_t *str ,uint32_t num,uint8_t *hex)
{
    for(int i = 0;i< num ; i++)
    {
        if(*(str+i*2)>=65)
        {
            *(hex+i) = (*(str+i*2)-55)*16;
        }else
        {
            *(hex+i) = (*(str+i*2)-48)*16;
        }
        if (*(str+i*2+1)>=65) {
            *(hex+i) |= (*(str+i*2+1)-55);
        }
        else {
            *(hex+i) |= (*(str+i*2+1)-48);
        }
        
    }
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
