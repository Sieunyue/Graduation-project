#include "end.h"
#include "config.h"
#include "string.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
EndDevice_t EndDev;
extern Usart_t Usart1;
uint8_t ENDMAC1[8] = {0XC5, 0X15, 0XA6, 0X1A, 0X00, 0X4B, 0X12, 0X00};
uint8_t int_bright, int_per;
uint32_t pwm1, pwm2;

/**
 * Function: Flash_Write
 * Description: Write data to Flash address
 * parameter:  4byte flash address
 */
static void Flash_Write(uint32_t flash_addr) {
  if (flash_addr != 0) {
    uint32_t temp;
    uint32_t pagerror;
    FLASH_EraseInitTypeDef save_dat;
    temp = EndDev.EndParameter.LedBright | EndDev.EndParameter.LedPer * 256;
    save_dat.TypeErase = FLASH_TYPEERASE_PAGES;
    save_dat.PageAddress = flash_addr;
    save_dat.NbPages = 1;
    HAL_FLASH_Unlock();
    HAL_FLASHEx_Erase(&save_dat, &pagerror);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, save_dat.PageAddress, temp);
    HAL_FLASH_Lock();
  }
}
/**
 * Function: Flash_Read
 * Description: Read the data for the specified flash address
 * parameter:  flash        4byte flash address
 *             recbuff      array that stores data
 */
static void Flash_Read(uint32_t flash_addr, uint32_t *recbuff) {
  if (flash_addr != 0 && recbuff != NULL) {
    *recbuff = 0x00000000;
    *recbuff |= *(__IO uint32_t *)flash_addr;
  }
}
/**
 * Function: Led_Start
 * Description: Open the Led
 * Parameter: None
 * Return: None
 */
static void Led_Start() {
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}
/**
 * Function: Led_Stop
 * Description: Close the Led
 * parameter: None
 */
static void Led_Stop() {
  HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
}
/**
 * Function: Led_Set
 * Description: Set the led bright
 * parameter: None
 */
static void Led_Set() {
  uint8_t valueH, valueL;
  valueL = EndDev.EndParameter.LedBright & 0x0F;
  valueH = ((EndDev.EndParameter.LedBright>> 4) & 0x0F);
  int_bright = valueL + valueH * 10;
valueL = EndDev.EndParameter.LedPer & 0x0F;
  valueH = ((EndDev.EndParameter.LedPer>> 4) & 0x0F);
  int_per = valueL + valueH * 10;
  pwm1 = (int_bright) * (int_per / 100.00);
  pwm2 = (int_bright) * ((100 - int_per) / 100.00);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pwm1);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pwm2);
}
/**
 * Function: Send_Msg
 * Description: Send message to gateway
 * parameter: data_type          type of data
 */
static void Send_Msg(uint8_t data_type) {
  uint8_t temp[18];
  const uint8_t *p_temp = EndDev.EndParameter.EndMac;
  temp[0] = 0xFC;
  temp[1] = 0x10;
  temp[2] = 0x03;
  temp[3] = 0x01;
  temp[4] = 0x00;
  temp[5] = 0x00;
  temp[6] = data_type;
  for (int i = 7; i < 15; i++) {
    temp[i] = *p_temp++;
  }
  temp[15] = EndDev.EndParameter.LedBright;
  temp[16] = EndDev.EndParameter.LedPer;
  temp[17] = (uint8_t)EndDev.EndParameter.LedOn;
  Usart1.Send(temp, 18);
}
/**
 * Function: Get_Mac
 * Description: Get Zigbee module mac address
 * Parameter: None
 * Return: None
 */
static void Get_Mac() {
  for (uint8_t i = 0; i < 8; i++) {
    EndDev.EndParameter.EndMac[i] = ENDMAC1[i];
  }
}
/**
 * Function: Check_Mac
 * Description: Check mac address in message
 * Parameter: mac_buff          array of msg
 * Return:  true                the message is own
 *          false               the message isn't own or mac is empty
 */
static bool Check_Mac(uint8_t *mac_buff) {
  const uint8_t *p_buff = mac_buff;
  if (EndDev.EndParameter.EndMac[0] == 0x00) {
    return false;
  }
  for (uint8_t i = 0; i < 8; i++) {
    if (*(p_buff + i) != EndDev.EndParameter.EndMac[i]) {
      return false;
    }
  }
  return true;
}
/**
 * Function: Process
 * Description: Processing the message
 * Parameter: None
 * Return: None
 */
static void Process() {
  if (Usart1.UsartState == RxDone) {
    if (Check_Mac(Usart1.UsartBuff + 1)) {
      switch (Usart1.UsartBuff[0]) {
        case REJOIN:
          EndDev.IsJoinNet = true;

            HAL_GPIO_WritePin(GPIOA, RUN_Pin,0);

          break;
        case SetBright:
          EndDev.EndParameter.LedBright = Usart1.UsartBuff[9];
          Led_Set();
          Flash_Write(EndDev.EndParameter.FlashAddr);
          HAL_GPIO_TogglePin(GPIOA, RUN_Pin);
          break;
        case SETPROPORTION:
          EndDev.EndParameter.LedPer = Usart1.UsartBuff[10];
          Led_Set();
          Flash_Write(EndDev.EndParameter.FlashAddr);
          HAL_GPIO_TogglePin(GPIOA, RUN_Pin);
          break;
        case SETON:
          if (Usart1.UsartBuff[11] == 0) {
            Led_Stop();
            EndDev.EndParameter.LedOn = false;
          } else {
            Led_Start();
            EndDev.EndParameter.LedOn = true;
          }
          HAL_GPIO_TogglePin(GPIOA, RUN_Pin);
          Flash_Write(EndDev.EndParameter.FlashAddr);
          break;
      }
      Send_Msg(READDATA);
    }
    Usart1.Rx();
  }
}
/**
 * Function: End_Init
 * Description: Initialization Zigbee
 * Parameter: None
 * Return: None
 */
void End_Init() {
  uint32_t led_parameter;
  EndDev.EndParameter.LedOn = false;
  EndDev.IsJoinNet = false;
  EndDev.EndParameter.FlashAddr = FLASH_DATAADDR;
  Flash_Read(EndDev.EndParameter.FlashAddr, &led_parameter);
  EndDev.EndParameter.LedBright = led_parameter & 0xFF;
  EndDev.EndParameter.LedPer = (led_parameter >> 8) & 0xFF;
	  Led_Set();
  EndDev.Process = Process;
  EndDev.Send = Send_Msg;
  EndDev.Get_Mac = Get_Mac;
}
void KeyProcess()
{
    uint8_t flag;
    flag = HAL_GPIO_ReadPin(GPIOA, KEY_Pin);
    if (flag == 0)
    {
        do
        {
            flag = HAL_GPIO_ReadPin(GPIOA, KEY_Pin);
        } while (flag == 0);

        if (EndDev.EndParameter.LedOn == false)
        {
            Led_Start();
            EndDev.EndParameter.LedOn = true;
        }
        else if (EndDev.EndParameter.LedOn == true)
        {

            Led_Stop();
            EndDev.EndParameter.LedOn = false;
        }
        Send_Msg(SETON);
    }
}
// void Init_Device()
// {
//     // uint32_t end_data = END_DATA;
//     // end1.online = 0;
//     // end1.led_bright = *(__IO uint8_t *)end_data;
//     // end1.led_per = *(__IO uint8_t *)(end_data + 1);
//     // end1.mac_addr = end_mac;
//     // __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
//     // __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
//     // HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
//     // HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
//     // HexToInt();
// }

// void Send_Data()
// {

// }

// void Hand_Recbuf(uint8_t *rec)
// {
//     if (*rec == 0xAD)
//     {
//         if (MacCmp(rec))
//         {

//             if (*(rec + 8) == SETBRIGHT) //设置亮度
//             {
//                 end1.led_bright = *(rec + 9);
//                 Write_Pwm();
//             }
//             else if (*(rec + 8) == SETPROPORTION) //设置比例
//             {
//                 end1.led_per = *(rec + 10);
//                 Write_Pwm();
//             }
//             else if (*(rec + 8) == SETON) //设置开关
//             {
//                 end1.led_state = *(rec + 11);
//                 if (end1.led_state == 0)
//                 {
//                 }
//                 else if (end1.led_state == 1)
//                 {
//                     __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
//                     __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
//                 }
//             }
//             Send_Data(READDATA);
//         }
//     }
//     else if ((*rec == 0xFF) && *(rec + 1) == 0xAA && *(rec + 2) == 0xFF)
//     {
//         end1.online = 1;
//     }
//     memset(rec, 0, 20);
// }

// // void HexToInt()
// // {
// //     float intbright, intper;
// //     intbright = (end1.led_bright / 16 * 10) + (end1.led_bright & 0x0F);
// //     intper = (end1.led_per / 16 * 10) + (end1.led_per & 0x0F);
// //     end1.cool_pwm = intbright * (intper / 100.00);
// //     end1.warm_pwm = intbright * ((100.00 - intper) / 100.00);
// // }

// uint8_t MacCmp(uint8_t *rec)
// {

//     for (int i = 0; i < 8; i++)
//     {
//         if (*(rec + i) != *(end1.mac_addr + i))
//         {
//             return 0;
//         }
//     }
//     return 1;
// }

// void delay_ms(int time)
// {
//     int i = 0;
//     while (time--)
//     {
//         i = 12000;
//         while (i--)
//             ;
//     }
// }
