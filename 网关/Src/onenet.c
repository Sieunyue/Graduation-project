#include "onenet.h"
#include "EdpKit.h"
#include "stdlib.h"
#include "string.h"
#include "usart.h"
#include "main.h"
const char DEID[] = "503223399";
const char INFO[] = "0001";
const char APIKey[] = "2ar=0yznUqoLTIw6t4P2fOIprog=";
extern END_Head Head;
extern DevState_e DevState;
ETH_t ETHDev;
    /**
     * Function: SendValueToOnenet
     * Description: Send Value to Onenet
     * Parameter: data           array of datapoints name
     *            value          Number was sent
     * Return: None
     */
static void SendValueToOnenet(const char *data, int value) {
  EdpPacket *send_pkg;
  cJSON *json_data = cJSON_CreateObject();
  cJSON_AddItemToObject(json_data, data, cJSON_CreateNumber(value));
  send_pkg = PacketSavedataJson(NULL, json_data, kTypeSimpleJsonWithoutTime, 0);
  HAL_UART_Transmit(&huart2, (send_pkg->_data), send_pkg->_write_pos, 100);
  DeleteBuffer(&send_pkg);
  cJSON_Delete(json_data);
}
/**
 * Function:    SendStringToOnenet
 * Description:    Send a string to Onenet
 * Parameter:    Zigbee device struct
 * Return:    None
 */
void SendStringToOnenet(END_TypeDef *end) {
  EdpPacket *send_pkg;
  char msg_pkg[26];
  msg_pkg[0] = ':';
  msg_pkg[1] = ',';
  ValueToMsg((msg_pkg + 2), end->mac, end->bright, end->proportion,
             end->on_state);
  send_pkg = PacketSavedataSimpleString(NULL, msg_pkg, 0);
  HAL_UART_Transmit(&huart2, (send_pkg->_data), send_pkg->_write_pos, 100);
  DeleteBuffer(&send_pkg);
}
/**
 * Function: ConnectOnenet
 * Description: Connect to Onenet
 * Parameter:   None
 * Return: None
 */
static void ConnectOnenet() {
  EdpPacket *send_pkg;
  if ((send_pkg = PacketConnect1(DEID, APIKey)) != NULL) {
    Usart_E.Send(send_pkg->_data, send_pkg->_write_pos);
  }
  DeleteBuffer(&send_pkg);
}
/**
* Function:    StringToHex
* Description:    Converting strings to hex
* Parameter:    str        will be converting str
*               num        length of str
*               hex        recive hex array
* Return:    None
*/
static void StringToHex(const uint8_t *str, uint32_t num, uint8_t *hex) {
  for (int i = 0; i < num; i++) {
    if (*(str + i * 2) >= 65) {
      *(hex + i) = (*(str + i * 2) - 55) * 16;
    } else {
      *(hex + i) = (*(str + i * 2) - 48) * 16;
    }
    if (*(str + i * 2 + 1) >= 65) {
      *(hex + i) |= (*(str + i * 2 + 1) - 55);
    } else {
      *(hex + i) |= (*(str + i * 2 + 1) - 48);
    }
  }
}
/**
* Function:    SendToNode
* Description:    Repeat onenet message to node
* Parameter:    rec_buf        array of onenet message
* Return:    None
*/
static void SendToNode(const uint8_t *rec_buf) {
  const uint8_t *prec = rec_buf + 44;
  uint8_t temp_msg[16];
  temp_msg[0] = 0xFC;
  temp_msg[1] = 0x0E;
  temp_msg[2] = 0x01;
  temp_msg[3] = 0x01;
  StringToHex(prec, 12, temp_msg + 4);
  Usart_Z.Send((uint8_t *)temp_msg, 16);
}

/**
 * Function:    Process
 * Description:    Process onenect cmd
 * Parameter:    recbuf    array of onenet message
 * Return:    None
 */
void Process() {
  if (Usart_E.State == RxDone) {
    const uint8_t *p_msg = Usart_E.buff;
    switch (*p_msg) {
      case CONNRESP:
        Head.IsNet = true;
				ETHDev.SendValue("Online_Device", Head.end_num);
        break;
      case CMDREQ:
        SendToNode(Usart_E.buff);
        break;
      case 0x40:
        Head.IsNet = true;
        DevState = DevJoin;
        ConnectOnenet();
        break;
      case PINGRESP:
        break;
      default:
        break;
    }
    Usart_E.Rx();
  }
}

/**
 * Function: ETH_Init
 * Description: Initialization of Ethernet module
 * Parameter: None
 * Return: None
 */
void ETH_Init(void) {
  ETHDev.Connect = ConnectOnenet;
  ETHDev.SendValue = SendValueToOnenet;
  ETHDev.SendString = SendStringToOnenet;
  ETHDev.Process = Process;
}
