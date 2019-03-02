#include "end.h"
#include "tim.h"
#include "string.h"
#include "usart.h"
End_Device end1;
uint8_t end_mac[8] = {0XAD, 0X39, 0XA5, 0X1A, 0X00, 0X4B, 0X12, 0X00};
uint8_t key_flag;
void Init_Device()
{
  uint32_t end_data = END_DATA;
  end1.online = 0;
  end1.led_bright = *(__IO uint8_t *)end_data;
  end1.led_per = *(__IO uint8_t *)(end_data + 1);
  end1.mac_addr = end_mac;
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HexToInt();
}

void Send_Data(uint8_t data_type)
{
  uint8_t temp[12];
  uint8_t *p_temp = end_mac;
  for (int i = 0; i < 12; i++)
  {
    temp[i] = *p_temp++;
  }
  temp[8] = data_type;
  temp[9] = end1.led_bright;
  temp[10] = end1.led_per;
  temp[11] = end1.led_state;
  HAL_UART_Transmit(&huart1, temp, 12, 200);
}
void Hand_Recbuf(uint8_t *rec)
{
  if (*rec == 0xAD)
  {
    if (MacCmp(rec))
    {

      if (*(rec + 8) == SETBRIGHT) //设置亮度
      {
        end1.led_bright = *(rec + 9);
        Write_Pwm();
      }
      else if (*(rec + 8) == SETPROPORTION) //设置比例
      {
        end1.led_per = *(rec + 10);
        Write_Pwm();
      }
      else if (*(rec + 8) == SETON) //设置开关
      {
        end1.led_state = *(rec + 11);
        if (end1.led_state == 0)
        {
          __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, end1.cool_pwm);
          __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, end1.warm_pwm);
        }
        else if (end1.led_state == 1)
        {
          __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
          __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
        }
      }
        Send_Data(READDATA);

    }
  }
  else if ((*rec == 0xFF) && *(rec + 1) == 0xAA)
  {
    Send_Data(JOINNET);
    delay_ms(1000);
    Send_Data(JOINNET);
    end1.online = 1;
  }
  memset(rec, 0, 20);
}

void HexToInt()
{
    float intbright,intper;
    intbright = (end1.led_bright/16*10)+(end1.led_bright&0x0F);
    intper = (end1.led_per/16*10)+(end1.led_per&0x0F);
    end1.cool_pwm = intbright * (intper/100.00);
    end1.warm_pwm = intbright * ((100.00 - intper)/100.00);
}

uint8_t MacCmp(uint8_t *rec)
{

  for (int i = 0; i < 8; i++)
  {
    if (*(rec + i) != *(end1.mac_addr + i))
    {
      return 0;
    }
  }
  return 1;
}
void Write_Pwm()
{
  uint32_t temp;
  uint32_t pagerror;
  FLASH_EraseInitTypeDef save_dat;
    HexToInt();
    if (end1.led_state == 1) {
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, end1.cool_pwm);
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, end1.warm_pwm);
    } 
  temp = end1.led_bright + end1.led_per * 256 + end1.led_state * 256 * 256;
  save_dat.TypeErase = FLASH_TYPEERASE_PAGES;
  save_dat.PageAddress = END_DATA;
  save_dat.NbPages = 1;
  HAL_FLASH_Unlock();
  HAL_FLASHEx_Erase(&save_dat, &pagerror);
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, save_dat.PageAddress, temp);
  HAL_FLASH_Lock();
}
void delay_ms(int time)
{
  int i = 0;
  while (time--)
  {
    i = 12000;
    while (i--);
    
  }
}
void Key_Scan(void)
{
  key_flag = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10);
  if (key_flag == 0)
  {
    do
    {
      key_flag = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10);
    } while (key_flag == 0);

    if (end1.led_state == 0)
    {
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, end1.cool_pwm);
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, end1.warm_pwm); 
      end1.led_state = 1;
    }
    else if (end1.led_state == 1)
    {

      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
      end1.led_state = 0;
    }
    Send_Data(SETON);
  }
}
