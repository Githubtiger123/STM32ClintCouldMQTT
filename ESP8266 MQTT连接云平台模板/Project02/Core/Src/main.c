/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hal_temp_hum.h"
#include "esp8266_at.h"
#include "esp8266_mqtt.h"
#include "Link_xxCould.h"
#include "oled.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
	HAL_UART_Receive_IT(&huart1,usart1_rxone,1);			//��USART1�жϣ����ն�����Ϣ
	ES8266_MQTT_Init();			
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	
	uint16_t Counter_RUNLED_Blink = 0;
	uint16_t Counter_RUNInfo_Send = 0;
	uint16_t Counter_MQTT_Heart = 0;
	uint16_t Counter_StatusReport = 0;
  while (1)
  {
    /* USER CODE END WHILE */

		
    /* USER CODE BEGIN 3 */
  //		//���е���˸
//		if(Counter_RUNLED_Blink++>COUNTER_LEDBLINK)
//		{
//			Counter_RUNLED_Blink = 0;
//			HAL_GPIO_TogglePin(LED_G_GPIO_Port,LED_G_Pin);
//		}
		
		//����״̬��ӡ
		if(Counter_RUNInfo_Send++>COUNTER_RUNINFOSEND)
		{
			Counter_RUNInfo_Send = 0;
			user_main_info("�����������У�\r\n");
		}
		
		//����������
//		if(Counter_MQTT_Heart++>COUNTER_MQTTHEART)
//		{
//			Counter_MQTT_Heart = 0;
//			MQTT_SentHeart();
//		}
		
		//����״̬�ϱ�
		if(Counter_StatusReport++>COUNTER_STATUSREPORT)
		{
			Counter_StatusReport = 0;
			STM32_StatusReport();
		}
		
		//������ջ���������
		if(usart1_rxcounter)
		{
			deal_MQTT_message(usart1_rxbuf,usart1_rxcounter);
		}

		HAL_Delay(LOOPTIME);
		
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/******************************  USART1�����жϴ���  *****************************/

// ES8266�������ڽ����жϴ�����
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)	// �ж������ĸ����ڴ������ж�
	{
		//�����յ������ݷ������usart1��������
		usart1_rxbuf[usart1_rxcounter] = usart1_rxone[0];
		usart1_rxcounter++;	//����������1
		
		//����ʹ�ܴ���1�����ж�
		HAL_UART_Receive_IT(&huart1,usart1_rxone,1);		
	}
}

/******************************  �����жϴ���  *****************************/

//KEY1���¶���ִ�к���
void KEY1_Pressed(void)
{
	user_main_debug("�Ұ�����KEY_1\r\n");
	Change_LED_Status();
}

//KEY2���¶���ִ�к���
void KEY2_Pressed(void)
{
	user_main_debug("�Ұ�����KEY_2\r\n");
}

//�����жϴ�����
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch(GPIO_Pin)  
    {  
        case SW1_Pin:KEY1_Pressed();break;  
        case SW2_Pin:KEY2_Pressed();break;   
        default:break;  
    }  
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
