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
#include "iwdg.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "esp8266_at.h"
#include "esp8266_mqtt.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */


//此处是主循环运行延时宏定义
#define LOOPTIME 30 	//程序周期循环延时时间：30ms
#define COUNTER_LEDBLINK			(300/LOOPTIME)		//LED运行闪烁时间：300ms
#define COUNTER_RUNINFOSEND		(5000/LOOPTIME)		//运行串口提示：5s
#define COUNTER_MQTTHEART     (5000/LOOPTIME)		//MQTT发送心跳包：5s
#define COUNTER_STATUSREPORT	(5000/LOOPTIME)		//状态上传：5s

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
unsigned char ADAM4017RequastData[] = {0x02,0x03,0x00,0x00,0x00,0x08,0x44,0x3F};
unsigned char ADAM4150DIRequastData[] = {0x01,0x01,0x00,0x00,0x00,0x07,0x7D,0xC8};
unsigned char ADAM4150DORequastData[] = {0x01,0x01,0x00,0x00,0x00,0x18,0x3C,0x00};
unsigned char Rx2Buff[30];
unsigned char Rx2Count = 0;
unsigned char aRxBuffer = 0;
float ADAM4017SensorData[7];
unsigned char ADAM4150DOData[8];
unsigned char ADAM4150DIData[7];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void ES8266_MQTT_Init(void);
void STM32_StatusReport(void);
void deal_MQTT_message(uint8_t* buf,uint16_t len);
void getADAM4017Data(void);
void getADAM4150Data(void);
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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_USART3_UART_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
	
	//开启USART1接收中断
	HAL_UART_Receive_IT(&huart1,usart1_rxone,1);			//打开USART1中断，接收订阅消息
	HAL_UART_Receive_IT(&huart2,&aRxBuffer,1);			//打开USART2中断，接收订阅消息
	
	ES8266_MQTT_Init();
	MQTT_SentHeart();		//初始化后发心跳包以防时间太长复位
	
	HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
	HAL_Delay(300);
	HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
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
		
		//运行灯闪烁
//		if(Counter_RUNLED_Blink++>COUNTER_LEDBLINK)
//		{
//			Counter_RUNLED_Blink = 0;
//			HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);
//		}
		
//		//运行状态打印
		if(Counter_RUNInfo_Send++>COUNTER_RUNINFOSEND)
		{
			HAL_UART_Transmit(&huart3,(unsigned char *)"程序正在运行！\r\n",sizeof("程序正在运行！\r\n"),500);
			Counter_RUNInfo_Send = 0;
		}
		
		//心跳包发送
		if(Counter_MQTT_Heart++>COUNTER_MQTTHEART)
		{
			Counter_MQTT_Heart = 0;
		  MQTT_SentHeart();
		}
		
		//本机状态上报
		if(Counter_StatusReport++>COUNTER_STATUSREPORT)
		{
			Counter_StatusReport = 0;
			getADAM4017Data();//请求获取ADAM4017数据
			HAL_Delay(50);
			getADAM4150Data();
			STM32_StatusReport();
		}

		//如果接收缓存有数据
		if(usart1_rxcounter)
		{
			HAL_Delay(50);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
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



/******************************  USART1接收中断代码  *****************************/

// ES8266驱动串口接收中断处理函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)	// 判断是由哪个串口触发的中断
	{
		//将接收到的数据放入接收usart1接收数组
		usart1_rxbuf[usart1_rxcounter] = usart1_rxone[0];
		usart1_rxcounter++;	//接收数量＋1
		
		//重新使能串口1接收中断
		HAL_UART_Receive_IT(&huart1,usart1_rxone,1);		
	}else if(huart->Instance == USART2){
	
		Rx2Buff[Rx2Count] = aRxBuffer;
		Rx2Count++;
		HAL_UART_Receive_IT(&huart2,&aRxBuffer,1);
	}
}

void getADAM4017Data(){

	Rx2Count = 0;
	memset(Rx2Buff,0,sizeof(Rx2Buff));	//注意发送之前先清除,以防和DO口回传数据冲突
	//发送请求指令
	HAL_UART_Transmit(&huart2,ADAM4017RequastData,sizeof(ADAM4017RequastData),500);
	HAL_Delay(50);
	if(Rx2Count!=0){
	
		HAL_Delay(50);
		for(int i=0;i<7;i++){
		
			int a = Rx2Buff[i*2+3]<<8|Rx2Buff[i*2+4];
			int min,max;
			switch(i){
			
				case 0:min = -10;max = 60;
					break;
				case 1:min = 0;max = 100;
					break;
				case 2:min = 0;max = 5000;
					break;
				case 3:min = 30;max = 120;
					break;
				case 4:min = 0;max = 20000;
					break;
				case 5:min = 0;max = 30;
					break;
				case 6:min = 0;max = 30;
					break;
			}
			ADAM4017SensorData[i] = (max - min) * 1.0/ 65535.0 * a + min;
		}
		Rx2Count = 0;
		memset(Rx2Buff,0,sizeof(Rx2Buff));
	}
}

void getADAM4150Data(){

	//获取DI口
	Rx2Count = 0;
	memset(Rx2Buff,0,sizeof(Rx2Buff));		//注意发送之前先清除,以防和DO口回传数据冲突
	HAL_UART_Transmit(&huart2,ADAM4150DIRequastData,sizeof(ADAM4150DIRequastData),500);
	HAL_Delay(50);
	if(Rx2Count!=0){
	
		HAL_Delay(50);
		for(int i=0;i<7;i++){
		
			ADAM4150DIData[i] = (Rx2Buff[3])&(0x01<<i);
			if(ADAM4150DIData[i]>0)ADAM4150DIData[i] = 1;
		}
		Rx2Count = 0;
		memset(Rx2Buff,0,sizeof(Rx2Buff));
	}
	HAL_Delay(50);
	//获取DO口
	Rx2Count = 0;
	memset(Rx2Buff,0,sizeof(Rx2Buff));		//注意发送之前先清除,以防和DO口回传数据冲突
	HAL_UART_Transmit(&huart2,ADAM4150DORequastData,sizeof(ADAM4150DORequastData),500);
	HAL_Delay(50);
	if(Rx2Count!=0){
	
		HAL_Delay(50);
		for(int i=0;i<8;i++){
		
			ADAM4150DOData[i] = (Rx2Buff[5])&(0x01<<i);
			if(ADAM4150DOData[i]>0)ADAM4150DOData[i] = 1;
		}
		if(ADAM4150DOData[4]>0&&ADAM4150DOData[5]==0)ADAM4150DOData[4] = 0;
		else if(ADAM4150DOData[4]==0&&ADAM4150DOData[5]>0)ADAM4150DOData[4] = 2;
		else if(ADAM4150DOData[4]==0&&ADAM4150DOData[5]==0)ADAM4150DOData[4] = 1;
		Rx2Count = 0;
		memset(Rx2Buff,0,sizeof(Rx2Buff));
	}
}

//单片机状态上报
void STM32_StatusReport(void)
{

	char mqtt_message[255];//注意报文长度有限制255,太长发送不出去
	sprintf(mqtt_message,
	"{\"id\":\"123\",\"version\":\"1.0\",\
	\"params\":{\"m_temp\":%2.2f,\"m_hum\":%2.2f,\
	\"m_co2\":%2.2f,\"m_noise\":%2.2f,\"m_light\":%3.2f,\"m_wind\":%2.2f\
	},\"method\":\"thing.event.property.post\"}",
	ADAM4017SensorData[0],ADAM4017SensorData[1],ADAM4017SensorData[2],ADAM4017SensorData[3],
	ADAM4017SensorData[4],ADAM4017SensorData[5]);
	MQTT_PublishData(MQTT_PUBLISH_TOPIC,mqtt_message,0);
	//HAL_Delay(30);
	sprintf(mqtt_message,
	"{\"id\":\"123\",\"version\":\"1.0\",\
	\"params\":{\"m_weight\":%2.2f,\"m_switch2\":%d,\"m_infrared\":%d\
	},\"method\":\"thing.event.property.post\"}",
	ADAM4017SensorData[6],ADAM4150DIData[5],ADAM4150DIData[6]);
	MQTT_PublishData(MQTT_PUBLISH_TOPIC,mqtt_message,0);
	//HAL_Delay(30);
	sprintf(mqtt_message,
	"{\"id\":\"123\",\"version\":\"1.0\",\
	\"params\":{\"m_body\":%d,\"m_smoke\":%d,\"m_microswitch\":%d,\"m_travel\":%d,\"m_switch1\":%d\
	},\"method\":\"thing.event.property.post\"}",
	ADAM4150DIData[0],ADAM4150DIData[1],ADAM4150DIData[2],ADAM4150DIData[3],ADAM4150DIData[4]);
	MQTT_PublishData(MQTT_PUBLISH_TOPIC,mqtt_message,0);
	//HAL_Delay(30);
	sprintf(mqtt_message,
	"{\"id\":\"123\",\"version\":\"1.0\",\
	\"params\":{\"m_red\":%d,\"m_green\":%d,\"m_yellow\":%d,\"m_alarm\":%d,\"m_pushrod\":%d\
	},\"method\":\"thing.event.property.post\"}",
	ADAM4150DOData[0],ADAM4150DOData[1],ADAM4150DOData[2],ADAM4150DOData[3],ADAM4150DOData[4]);
	MQTT_PublishData(MQTT_PUBLISH_TOPIC,mqtt_message,0);
	//HAL_Delay(30);
	sprintf(mqtt_message,
	"{\"id\":\"123\",\"version\":\"1.0\",\
	\"params\":{\"m_lamp\":%d,\"m_fan\":%d\
	},\"method\":\"thing.event.property.post\"}",
	ADAM4150DOData[7],ADAM4150DOData[6]);
	MQTT_PublishData(MQTT_PUBLISH_TOPIC,mqtt_message,0);
}


/******************************  按键中断代码  *****************************/

//KEY1按下动作执行函数
void KEY1_Pressed(void)
{
	HAL_UART_Transmit(&huart3,(unsigned char *)"我按下了KEY_1\r\n",sizeof("我按下了KEY_1\r\n"),500);
}

//KEY2按下动作执行函数
void KEY2_Pressed(void)
{
	HAL_UART_Transmit(&huart3,(unsigned char *)"我按下了KEY_2\r\n",sizeof("我按下了KEY_2\r\n"),500);
}

//按键中断处理函数
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch(GPIO_Pin)  
    {  
        case SW1_Pin:KEY1_Pressed();break;  
        case SW2_Pin:KEY2_Pressed();break;   
        default:break;  
    }  
}


char temp_str[30];    // 临时子串
void ReadStrUnit(char * str,char *temp_str,int idx,int len)  // 从母串中获取与子串长度相等的临时子串
{
    int index;
    for(index = 0; index < len; index++)
    {
        temp_str[index] = str[idx+index];
    }
    temp_str[index] = '\0';
}
int GetSubStrPos(char *str1,char *str2,uint16_t len1,uint16_t len2)
{
    int idx = 0;

    if( len1 < len2)
    {
        //printf("error 1 \n"); // 子串比母串长
        return -1;
    }

    while(1)
    {
        ReadStrUnit(str1,temp_str,idx,len2);    // 不断获取的从 母串的 idx 位置处更新临时子串
        if(strcmp(str2,temp_str)==0)break;      // 若临时子串和子串一致，结束循环
        idx++;                                  // 改变从母串中取临时子串的位置
        if(idx>=len1)return -1;                 // 若 idx 已经超出母串长度，说明母串不包含该子串
    }

    return idx;    // 返回子串第一个字符在母串中的位置
}

////应答MQTT请求
//void MQTT_response(char * buf,uint16_t len1,uint16_t len2){
//																				//MQTT响应数组 MQTT响应数组长度 云平台请求内容长度
//	char response_data[37] = "";
//	int i;
//	for(i=0;i<36;i++){
//	
//		response_data[i] = buf[len1 - len2 + i - 36 + 1];
//	}
//	char str[300];
//	strcpy (str,MQTT_SUBSCRIBE_RESPONSE_TOPIC);
//	strcat (str,response_data);
//	MQTT_PublishData(str,"OK",0);
//	
//}

//处理MQTT下发的消息
void deal_MQTT_message(uint8_t* buf,uint16_t len)
{
	uint8_t data[512];
	uint16_t data_len = len;
	for(int i=0;i<data_len;i++)
	{
		data[i] = buf[i];
	}
	uint8_t UART_data[20];
	sprintf((char *)UART_data,"MQTT收到消息,数据长度=%d \n",data_len);
	HAL_UART_Transmit(&huart3,(unsigned char *)UART_data,sizeof(UART_data),500);
	HAL_UART_Transmit(&huart3,(unsigned char *)data,data_len,500);
	
	//心跳回应
	unsigned char d[] = {0xD0,0x00};
	if(GetSubStrPos((char *)data,(char *)d,data_len,2)!=-1||(data[0]==0xC8&&data[1]==0x3D)){	
	
		HAL_IWDG_Refresh(&hiwdg);	//喂狗
	}
	
	//红灯 DO0
	int i = GetSubStrPos((char *)data,"m_red",data_len,sizeof("m_red")-1);	//注意""这种sizeof("")算\n所以要减一
	if(i!=-1){
	
		if(data[i+sizeof("m_red")-1+2] - '0'){
		
			unsigned char ADAM4150DOSetData[] = {0x01,0x05,0x00,0x10,0xFF,0x00,0x8D,0xFF};
			HAL_UART_Transmit(&huart2,ADAM4150DOSetData,sizeof(ADAM4150DOSetData),500);
		}else{
		
			unsigned char ADAM4150DOSetData[] = {0x01,0x05,0x00,0x10,0x00,0x00,0xCC,0x0F};
			HAL_UART_Transmit(&huart2,ADAM4150DOSetData,sizeof(ADAM4150DOSetData),500);
		}
	}
	
	//绿灯 DO1
	i = GetSubStrPos((char *)data,"m_green",data_len,sizeof("m_green")-1);
	if(i!=-1){
	
		if(data[i+sizeof("m_green")-1+2] - '0'){
		
			unsigned char ADAM4150DOSetData[] = {0x01,0x05,0x00,0x11,0xFF,0x00,0xDC,0x3F};
			HAL_UART_Transmit(&huart2,ADAM4150DOSetData,sizeof(ADAM4150DOSetData),500);
		}else{
		
			unsigned char ADAM4150DOSetData[] = {0x01,0x05,0x00,0x11,0x00,0x00,0x9D,0xCF};
			HAL_UART_Transmit(&huart2,ADAM4150DOSetData,sizeof(ADAM4150DOSetData),500);
		}
	}
	
		
	//黄灯 DO2
	i = GetSubStrPos((char *)data,"m_yellow",data_len,sizeof("m_yellow")-1);
	if(i!=-1){
	
		if(data[i+sizeof("m_yellow")-1+2] - '0'){
		
			unsigned char ADAM4150DOSetData[] = {0x01,0x05,0x00,0x12,0xFF,0x00,0x2C,0x3F};
			HAL_UART_Transmit(&huart2,ADAM4150DOSetData,sizeof(ADAM4150DOSetData),500);
		}else{
		
			unsigned char ADAM4150DOSetData[] = {0x01,0x05,0x00,0x12,0x00,0x00,0x6D,0xCF};
			HAL_UART_Transmit(&huart2,ADAM4150DOSetData,sizeof(ADAM4150DOSetData),500);
		}
	}
	
	//报警灯 DO3
	i = GetSubStrPos((char *)data,"m_alarm",data_len,sizeof("m_alarm")-1);
	if(i!=-1){
	
		if(data[i+sizeof("m_alarm")-1+2] - '0'){
		
			unsigned char ADAM4150DOSetData[] = {0x01,0x05,0x00,0x13,0xFF,0x00,0x7D,0xFF};
			HAL_UART_Transmit(&huart2,ADAM4150DOSetData,sizeof(ADAM4150DOSetData),500);
		}else{
		
			unsigned char ADAM4150DOSetData[] = {0x01,0x05,0x00,0x13,0x00,0x00,0x3C,0x0F};
			HAL_UART_Transmit(&huart2,ADAM4150DOSetData,sizeof(ADAM4150DOSetData),500);
		}
	}
	
	//电动推杆 DO4,DO5
	i = GetSubStrPos((char *)data,"m_pushrod",data_len,sizeof("m_pushrod")-1);
	if(i!=-1){
	
		if((data[i+sizeof("m_pushrod")-1+2] - '0')==0){
		
			unsigned char ADAM4150DOSetData[] = {0x01,0x05,0x00,0x14,0xFF,0x00,0xCC,0x3E};//4 OPEN
			HAL_UART_Transmit(&huart2,ADAM4150DOSetData,sizeof(ADAM4150DOSetData),500);
			unsigned char ADAM4150DOSetData1[] = {0x01,0x05,0x00,0x15,0x00,0x00,0xDC,0x0E};//5 CLOSE
			HAL_UART_Transmit(&huart2,ADAM4150DOSetData1,sizeof(ADAM4150DOSetData1),500);
		}else if((data[i+sizeof("m_pushrod")-1+2] - '0')==1){
		
			unsigned char ADAM4150DOSetData[] = {0x01,0x05,0x00,0x14,0x00,0x00,0x8D,0xCE};//4 CLOSE
			HAL_UART_Transmit(&huart2,ADAM4150DOSetData,sizeof(ADAM4150DOSetData),500);
			unsigned char ADAM4150DOSetData1[] = {0x01,0x05,0x00,0x15,0x00,0x00,0xDC,0x0E};//5 CLOSE
			HAL_UART_Transmit(&huart2,ADAM4150DOSetData1,sizeof(ADAM4150DOSetData1),500);
		}else if((data[i+sizeof("m_pushrod")-1+2] - '0')==2){
		
			unsigned char ADAM4150DOSetData1[] = {0x01,0x05,0x00,0x15,0xFF,0x00,0x9D,0xFE};//5 OPEN
			HAL_UART_Transmit(&huart2,ADAM4150DOSetData1,sizeof(ADAM4150DOSetData1),500);
			unsigned char ADAM4150DOSetData[] = {0x01,0x05,0x00,0x14,0x00,0x00,0x8D,0xCE};//4 CLOSE
			HAL_UART_Transmit(&huart2,ADAM4150DOSetData,sizeof(ADAM4150DOSetData),500);
		}
	}

	//照明灯 DO7
	i = GetSubStrPos((char *)data,"m_lamp",data_len,sizeof("m_lamp")-1);
	if(i!=-1){
	
		if(data[i+sizeof("m_lamp")-1+2] - '0'){
		
			unsigned char ADAM4150DOSetData[] = {0x01,0x05,0x00,0x17,0xFF,0x00,0x3C,0x3E};
			HAL_UART_Transmit(&huart2,ADAM4150DOSetData,sizeof(ADAM4150DOSetData),500);
		}else{
		
			unsigned char ADAM4150DOSetData[] = {0x01,0x05,0x00,0x17,0x00,0x00,0x7D,0xCE};
			HAL_UART_Transmit(&huart2,ADAM4150DOSetData,sizeof(ADAM4150DOSetData),500);
		}
	}
	
		//风扇 DO6
	i = GetSubStrPos((char *)data,"m_fan",data_len,sizeof("m_fan")-1);
	if(i!=-1){
	
		if(data[i+sizeof("m_fan")-1+2] - '0'){
		
			unsigned char ADAM4150DOSetData[] = {0x01,0x05,0x00,0x16,0xFF,0x00,0x6D,0xFE};
			HAL_UART_Transmit(&huart2,ADAM4150DOSetData,sizeof(ADAM4150DOSetData),500);
		}else{
		
			unsigned char ADAM4150DOSetData[] = {0x01,0x05,0x00,0x16,0x00,0x00,0x2C,0x0E};
			HAL_UART_Transmit(&huart2,ADAM4150DOSetData,sizeof(ADAM4150DOSetData),500);
		}
	}
	memset(usart1_rxbuf,0,sizeof(usart1_rxbuf)); //清空接收缓冲  
	usart1_rxcounter=0;
	
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
