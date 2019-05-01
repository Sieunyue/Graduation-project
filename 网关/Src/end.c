#include "end.h"
#include "onenet.h"
#include "stdlib.h"
#include "usart.h"

#define MsgBuffSize 16
END_TypeDef *led[10];

extern uint8_t count1;
extern uint8_t state1;
extern uint8_t rxbuf1[];

END_Head Head;
/*
 * 函数名： EndMsg
 * 功能：   处理终端传来的消息
 * 参数：   recbuf       串口缓冲区
 * 返回值： 无
 */
void EndMsg() {
  if (Usart_Z.State == RxDone) {
      const uint8_t *p_msg = Usart_Z.buff;
    if (*(p_msg) == JOINNET) {
      if (FindDevMac(p_msg) == 11) {
        CreatNewDevice(p_msg);
      }
      uint8_t temp_msg[MsgBuffSize];
      temp_msg[0] = 0xFC;
      temp_msg[1] = 0x0E;
      temp_msg[2] = 0x01;
      temp_msg[3] = 0x01;
      temp_msg[4] = REJOIN;
      for (uint8_t i = 5, j = 0; i < 13; i++, j++) {
        temp_msg[i] = Head.save_end[Head.end_num - 1]->mac[j];
      }
      temp_msg[13] = 0xFF;
      temp_msg[14] = 0xFF;
      temp_msg[15] = 0xFF;
      HAL_UART_Transmit(&huart1, temp_msg, MsgBuffSize, 10);
    } else if (*(p_msg) == SETON) {
      uint32_t num;
      num = FindDevMac(p_msg);
      if (num != 11) {
        Head.save_end[num]->on_state = *(p_msg + 11);
        ETHDev.SendString(Head.save_end[num]);
      }
    } else if (*(p_msg) == READDATA) {
      uint32_t num;
      num = FindDevMac(p_msg);
      if (num != 11) {
        Head.save_end[num]->on_state = *(p_msg + 11);
        Head.save_end[num]->bright = *(p_msg + 9);
        Head.save_end[num]->proportion = *(p_msg + 10);
        ETHDev.SendString(Head.save_end[num]);
      }
    }
    Usart_Z.Rx();
  }
}
/*
 * 函数名： EndMsg
 * 功能：   新建一个终端设备
 * 参数：   recbuf       串口缓冲区
 * 返回值： 无
 */
void CreatNewDevice(const uint8_t *recbuf) {
  const uint8_t *p_mac = recbuf + 1;
  END_TypeDef *temp = (END_TypeDef *)malloc(sizeof(END_TypeDef));
  Head.save_end[Head.end_num] = temp;
  for (int i = 0; i < 8; i++) {
    temp->mac[i] = *p_mac++;
  }
  temp->bright = *p_mac++;
  temp->proportion = *p_mac++;
  temp->on_state = *p_mac;
  Head.end_num++;
  ETHDev.SendString(temp);
}

/*
 * 函数名： HexsToChar
 * 功能：   将16进制数组转换成字符串
 * 参数：   *hex            16进制数组
 *          num             数组长度
 *          *save_char      存储字符串
 * 返回值： 无
 */
void HexsToChar(uint8_t *hex, uint32_t num, char *save_char) {
  for (int i = 0; i < num; i++) {
    save_char[(i * 2)] = ((*(hex + i)) / 16);
    save_char[(i * 2) + 1] = (*(hex + i)) & 0x0F;
    if (save_char[(i * 2)] <= 0x09) {
      save_char[(i * 2)] += 48;
    } else {
      save_char[(i * 2)] += 55;
    }
    if (save_char[(i * 2) + 1] <= 0x09) {
      save_char[(i * 2) + 1] += 48;
    } else {
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
void HexToChar(uint8_t hex, char *save_char) {
  save_char[0] = hex / 16;
  save_char[1] = hex & 0x0F;
  if (save_char[0] <= 0x09) {
    save_char[0] += 48;
  } else {
    save_char[0] += 55;
  }
  if (save_char[1] <= 0x09) {
    save_char[1] += 48;
  } else {
    save_char[1] += 55;
  }
}
void ValueToMsg(char *msg_pkg, uint8_t *msg_id, uint8_t msg_bright,
                uint8_t msg_porportion, uint8_t msg_on) {
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
uint32_t FindDevMac(const uint8_t *msg) {
  uint32_t k, i;
  const uint8_t *des_mac = msg + 1;
  if (Head.end_num == 0) {
    return 11;
  }
  for (i = 0; i < Head.end_num; i++) {
    for (k = 0; k < 8; k++) {
      if (*(des_mac++) != Head.save_end[i]->mac[k]) {
        break;
      }
    }
    des_mac = msg + 1;
    if (k == 8) {
      return i;
    }
  }
  return 11;
}
