#include "end.h"
#include "tim.h"
#include "string.h"
#include "usart.h"
#include "config.h"
EndDevice_t EndDev;
uint8_t ENDMAC1[8] = {0XAD, 0X39, 0XA5, 0X1A, 0X00, 0X4B, 0X12, 0X00};
static void Flash_Write(uint32_t flash_addr)
{
    if (flash_addr != 0)
    {
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
static void Flash_Read(uint32_t flash_addr, uint32_t *recbuff)
{
    if (flash_addr != 0 && recbuff != NULL)
    {
        *recbuff = 0x00000000;
        *recbuff |= *(__IO uint32_t *)flash_addr;
    }
}
static void Led_Start()
{
    // __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, Endcool_pwm);
    // __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, end1.warm_pwm);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}
static void Led_Stop()
{
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
}
static void Led_Set()
{
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 1);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 1);
}
static void Send_Msg(uint8_t data_type)
{
    uint8_t temp[12];
    const uint8_t *p_temp = EndDev.EndParameter.EndMac;
    temp[0] = data_type;
    for (int i = 1; i < 13; i++)
    {
        temp[i] = *p_temp++;
    }
    temp[9] = EndDev.EndParameter.LedBright;
    temp[10] = EndDev.EndParameter.LedPer;
    temp[11] = (uint8_t)EndDev.EndParameter.LedOn;
    Usart1.Send(temp, 12);
}

static bool Check_Mac(uint8_t *mac_buff)
{
    const uint8_t *p_buff = mac_buff ;
	for (uint8_t i = 0; i < 8; i++)
    {
        if (*(p_buff + i) != EndDev.EndParameter.EndMac[i])
        {
            return false;
        }
    }
    return true;
}


static void Process()
{
    if (Usart1.UsartState == UsartRxDone)
    {
        if (Check_Mac(Usart1.UsartBuff+1))
        {
            switch (Usart1.UsartBuff[0])
            {
            case REJOIN:
                EndDev.IsJoinNet = true;
                break;
            case SetBright:
                Led_Set();
                Flash_Write(EndDev.EndParameter.FlashAddr);
                break;
            case SETPROPORTION:
                Led_Set();
                Flash_Write(EndDev.EndParameter.FlashAddr);
                break;
            case SETON:
                if (EndDev.EndParameter.LedOn == true)
                {
                    Led_Stop();
                    EndDev.EndParameter.LedOn = false;
                }
                else
                {
                    Led_Start();
                    EndDev.EndParameter.LedOn = true;
                }
                Flash_Write(EndDev.EndParameter.FlashAddr);
                break;
            }
        }
        Usart1.Rx();
    }
}
void End_Init()
{
    uint32_t led_parameter;
    EndDev.EndParameter.LedOn = false;
    EndDev.IsJoinNet = false;
    EndDev.EndParameter.FlashAddr = FLASH_DATAADDR;
    Flash_Read(EndDev.EndParameter.FlashAddr, &led_parameter);
    EndDev.EndParameter.LedBright = led_parameter & 0xFF;
    EndDev.EndParameter.LedPer = (led_parameter >> 8) & 0xFF;
    strncpy((char*)EndDev.EndParameter.EndMac, (char*)ENDMAC1, 8);
    EndDev.Process = Process;
    EndDev.Send = Send_Msg;
}
// void Key_Scan(void)
// {
//     key_flag = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10);
//     if (key_flag == 0)
//     {
//         do
//         {
//             key_flag = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10);
//         } while (key_flag == 0);

//         if (end1.led_state == 0)
//         {
//             __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, end1.cool_pwm);
//             __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, end1.warm_pwm);
//             end1.led_state = 1;
//         }
//         else if (end1.led_state == 1)
//         {

//             __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
//             __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
//             end1.led_state = 0;
//         }
//         Send_Data(SETON);
//     }
// }
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
