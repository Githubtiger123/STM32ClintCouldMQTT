#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include "hal_temp_hum.h"
#include "esp8266_at.h"
#include "esp8266_mqtt.h"
#include "Link_xxCould.h"
#include "oled.h"
#include <stdio.h>
#include "iwdg.h"
char mqtt_message[300];	//MQTT的上报消息缓存

//改变LED灯状态
void Change_LED_Status(void)
{
	HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin); 
}

/******************************  进入错误模式代码  *****************************/

//进入错误模式等待手动重启
void Enter_ErrorMode(uint8_t mode)
{
	//HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
	while(1)
	{
		OLED_Clear();
		OLED_ShowString(0,0,"Failed!!",16);
		OLED_ShowString(0,16,"Please restart",16);
		OLED_Refresh();
		switch(mode){
			case 0:user_main_error("ESP8266初始化失败！\r\n");break;
			case 1:user_main_error("ESP8266连接热点失败！\r\n");break;
			case 2:user_main_error("ESP8266连接阿里云服务器失败！\r\n");break;
			case 3:user_main_error("ESP8266阿里云MQTT登陆失败！\r\n");break;
			case 4:user_main_error("ESP8266阿里云MQTT订阅主题失败！\r\n");break;
			default:user_main_info("Nothing\r\n");break;
		}
		user_main_info("请重启开发板");
		HAL_GPIO_TogglePin(BEEP_GPIO_Port,BEEP_Pin);
		HAL_Delay(500);
	}
}

/******************************  开发板外设测试代码  *****************************/

/* 调试串口测试函数 */
void TEST_usart2(void)
{
	user_main_debug("我是USART2测试代码！\n");
	HAL_Delay(1000);
}

/* LED测试函数 */
void TEST_LED(void)
{
	
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
	HAL_Delay(500);
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
	HAL_Delay(500);
}

/* us级别延时测试函数 */
void TEST_delayus(void)
{
	static uint16_t tim1_test;
	
	//延时1000个1ms
	for(tim1_test = 0;tim1_test<1000;tim1_test++)
	{
		//延时1ms
		TIM1_Delay_us(1000);
	}
	user_main_debug("我是us级延时函数测试代码，1s打印一次！\n");
}

/* DHT11温湿度传感器测试函数 */
void TEST_DHT11(void)
{
	uint8_t temperature;
	uint8_t humidity;
	uint8_t get_times;

	// 获取温湿度信息并用串口打印,获取十次，直到成功跳出
	for(get_times=0;get_times<10;get_times++)
	{
		if(!dht11Read(&temperature, &humidity))//Read DHT11 Value
		{
				user_main_info("temperature=%d,humidity=%d \n",temperature,humidity);
				break;
		}
	}
}

/* ESP8266&MQTT测试代码 */
void TEST_ES8266MQTT(void)
{
	uint8_t status=0;

	//初始化
	if(ESP8266_Init())
	{
		user_main_info("ESP8266初始化成功！\r\n");
		status++;
	}
	else Enter_ErrorMode(0);

	//连接热点
	if(status==1)
	{
		if(ESP8266_ConnectAP(WIFI_NAME,WIFI_PASSWD))
		{
			user_main_info("ESP8266连接热点成功！\r\n");
			status++;
		}
		else Enter_ErrorMode(1);
	}
	
	//连接阿里云IOT服务器
	if(status==2)
	{
		if(ESP8266_ConnectServer("TCP",MQTT_BROKERADDRESS,1883)!=0)
		{
			user_main_info("ESP8266连接阿里云服务器成功！\r\n");
			status++;
		}
		else Enter_ErrorMode(2);
	}
	
	//登陆MQTT
	if(status==3)
	{
		if(MQTT_Connect(MQTT_CLIENTID, MQTT_USARNAME, MQTT_PASSWD) != 0)
		{
			user_main_info("ESP8266阿里云MQTT登陆成功！\r\n");
			status++;
		}
		else Enter_ErrorMode(3);
	}

	//订阅主题
	if(status==4)
	{
		if(MQTT_SubscribeTopic(MQTT_SUBSCRIBE_TOPIC,0,1) != 0)
		{
			user_main_info("ESP8266阿里云MQTT订阅主题成功！\r\n");
		}
		else Enter_ErrorMode(4);
	}
}

/******************************  STM32 MQTT业务代码  *****************************/

//MQTT初始化函数
void ES8266_MQTT_Init(void)
{
	uint8_t status=0;

	//初始化
	if(ESP8266_Init())
	{
		user_main_info("ESP8266初始化成功！\r\n");
		status++;
	}
	else Enter_ErrorMode(0);

	//连接热点
	if(status==1)
	{
		if(ESP8266_ConnectAP(WIFI_NAME,WIFI_PASSWD))
		{
			OLED_ShowString(100,16*0,"OK",16);OLED_Refresh();
			user_main_info("ESP8266连接热点成功！\r\n");
			status++;
		}
		else Enter_ErrorMode(1);
	}
	
	//连接阿里云IOT服务器
	if(status==2)
	{
		if(ESP8266_ConnectServer("TCP",MQTT_BROKERADDRESS,1883)!=0)
		{
			OLED_ShowString(100,16*1,"OK",16);OLED_Refresh();
			user_main_info("ESP8266连接阿里云服务器成功！\r\n");
			status++;
		}
		else Enter_ErrorMode(2);
	}
	
	//登陆MQTT
	if(status==3)
	{
		if(MQTT_Connect(MQTT_CLIENTID, MQTT_USARNAME, MQTT_PASSWD) != 0)
		{
			OLED_ShowString(100,16*2,"OK",16);OLED_Refresh();
			user_main_info("ESP8266阿里云MQTT登陆成功！\r\n");
			status++;
		}
		else Enter_ErrorMode(3);
	}

	//订阅主题
	if(status==4)
	{
		if(MQTT_SubscribeTopic(MQTT_SUBSCRIBE_TOPIC,0,1) != 0)
		{
			OLED_ShowString(100,16*3,"OK",16);OLED_Refresh();
			user_main_info("ESP8266阿里云MQTT订阅主题成功！\r\n");
			HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
			HAL_Delay(200);
			HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
			OLED_Clear();
		}
		else Enter_ErrorMode(4);
	}
}

//单片机状态上报
void STM32_StatusReport(void)
{
	//获取温湿度信息
	uint8_t temperature;
	uint8_t humidity;
	uint8_t get_times;
	
	// 获取温湿度信息并用串口打印,获取十次，直到成功跳出
	for(get_times=0;get_times<10;get_times++)
	{
		if(!dht11Read(&temperature, &humidity))//Read DHT11 Value
		{
			char data[10];
			sprintf((char *)data,"%3d",temperature);
			OLED_ShowString(16*3 - 8,16*1,data,16);OLED_Refresh();
			sprintf((char *)data,"%3d",humidity);
			OLED_ShowString(16*3 - 8,16*2,data,16);OLED_Refresh();
			user_main_info("temperature=%d,humidity=%d \n",temperature,humidity);
			break;
		}
	}
	
	//上报一次数据 不同平台格式不同 
//	uint8_t led_r_status = HAL_GPIO_ReadPin(LED_R_GPIO_Port,LED_R_Pin) ? 0:1;
//	uint8_t led_g_status = HAL_GPIO_ReadPin(LED_G_GPIO_Port,LED_G_Pin) ? 0:1;
//	uint8_t led_b_status = HAL_GPIO_ReadPin(LED_B_GPIO_Port,LED_B_Pin) ? 0:1;
//	sprintf(mqtt_message,
//	"{\"method\":\"thing.service.property.set\",\"id\":\"181454577\",\"params\":{\
//		\"DHT11_Temperature\":%.1f,\
//		\"DHT11_Humidity\":%.1f,\
//		\"Switch_LEDR\":%d,\
//		\"Switch_LEDG\":%d,\
//		\"Switch_LEDB\":%d\
//	},\"version\":\"1.0.0\"}",
//	(float)temperature,
//	(float)humidity,
//	led_r_status,
//	led_g_status,
//	led_b_status
//	);
	unsigned char LED_status = HAL_GPIO_ReadPin(LED_GPIO_Port,LED_Pin);
	if(!LED_status){
	
		OLED_ShowChinese(16*4 - 8,16*3,24,16);
	}else{
	
		OLED_ShowChinese(16*4 - 8,16*3,25,16);
	}
	OLED_Refresh();
	sprintf(mqtt_message,
	"{\"id\": 123,\"dp\": {\"temperatrue\": [{\"v\":%2.1f,}], \"humidity\": [{ \"v\":%2.1f,}], \"Lamp\": [{ \"v\":%d,}]}}",
	(float)temperature,
	(float)humidity,
	!LED_status);
	
	

	MQTT_PublishData(MQTT_PUBLISH_TOPIC,mqtt_message,0);
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

//应答MQTT请求
void MQTT_response(char * buf,uint16_t len1,uint16_t len2){
										//MQTT响应数组 MQTT响应数组长度 云平台请求内容长度
	char response_data[37] = "";
	int i;
	for(i=0;i<36;i++){
	
		response_data[i] = buf[len1 - len2 + i - 36 + 1];
	}
	char str[300];
	strcpy (str,MQTT_SUBSCRIBE_RESPONSE_TOPIC);
	strcat (str,response_data);
	MQTT_PublishData(str,"OK",0);
	
}

//处理MQTT下发的消息
void deal_MQTT_message(uint8_t* buf,uint16_t len)
{
	uint8_t data[512];
	uint16_t data_len = len;
	for(int i=0;i<data_len;i++)
	{
		data[i] = buf[i];
		HAL_UART_Transmit(&huart2,&data[i],1,100);
	}
	memset(usart1_rxbuf,0,sizeof(usart1_rxbuf)); //清空接收缓冲  
	usart1_rxcounter=0;
	user_main_info("MQTT收到消息,数据长度=%d \n",data_len);
	
	//心跳回应喂狗
	if(data[data_len-1] == 0&&data[data_len-2] == 0xD0){HAL_IWDG_Refresh(&hiwdg);}
	
	//查找是否开关灯
	int i = GetSubStrPos((char*)data,"LAMPOPEN",data_len,sizeof("LAMPOPEN"));
	if( i>0 )
	{
		HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
		MQTT_response((char*)data,data_len,sizeof("LAMPOPEN"));
		i=0;
	}
	i = GetSubStrPos((char*)data,"LAMPCLOSE",data_len,sizeof("LAMPCLOSE"));
	if( i>0 )
	{
		HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
		MQTT_response((char*)data,data_len,sizeof("LAMPCLOSE"));
		i=0;
	}
}

//main 函数模板


//int main(void)
//{
//  /* USER CODE BEGIN 1 */
//
//  /* USER CODE END 1 */
//
//  /* MCU Configuration--------------------------------------------------------*/
//
//  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
//  HAL_Init();
//
//  /* USER CODE BEGIN Init */
//
//  /* USER CODE END Init */
//
//  /* Configure the system clock */
//  SystemClock_Config();
//
//  /* USER CODE BEGIN SysInit */
//
//  /* USER CODE END SysInit */
//
//  /* Initialize all configured peripherals */
//  MX_GPIO_Init();
//  MX_USART1_UART_Init();
//  MX_USART2_UART_Init();
//  MX_TIM1_Init();
//  /* USER CODE BEGIN 2 */
//	//开启USART1接收中断
//	HAL_UART_Receive_IT(&huart1,usart1_rxone,1);			//打开USART1中断，接收订阅消息
//	ES8266_MQTT_Init();																			//初始化MQTT
//  /* USER CODE END 2 */
//
//  /* Infinite loop */
//  /* USER CODE BEGIN WHILE */
//	uint16_t Counter_RUNLED_Blink = 0;
//	uint16_t Counter_RUNInfo_Send = 0;
//	uint16_t Counter_MQTT_Heart = 0;
//	uint16_t Counter_StatusReport = 0;
//  while (1)
//  {
//    /* USER CODE END WHILE */
//
//    /* USER CODE BEGIN 3 */
//		//运行灯闪烁
////		if(Counter_RUNLED_Blink++>COUNTER_LEDBLINK)
////		{
////			Counter_RUNLED_Blink = 0;
////			HAL_GPIO_TogglePin(LED_G_GPIO_Port,LED_G_Pin);
////		}
//		
//		//运行状态打印
//		if(Counter_RUNInfo_Send++>COUNTER_RUNINFOSEND)
//		{
//			Counter_RUNInfo_Send = 0;
//			user_main_info("程序正在运行！\r\n");
//		}
//		
//		//心跳包发送
////		if(Counter_MQTT_Heart++>COUNTER_MQTTHEART)
////		{
////			Counter_MQTT_Heart = 0;
////			MQTT_SentHeart();
////		}
//		
//		//本机状态上报
//		if(Counter_StatusReport++>COUNTER_STATUSREPORT)
//		{
//			Counter_StatusReport = 0;
//			STM32DHT11_StatusReport();
//		}
//		
//		//如果接收缓存有数据
//		if(usart1_rxcounter)
//		{
//			deal_MQTT_message(usart1_rxbuf,usart1_rxcounter);
//		}
//
//		HAL_Delay(LOOPTIME);
//		
//  }
//  /* USER CODE END 3 */
//}

