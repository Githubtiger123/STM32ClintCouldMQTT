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
char mqtt_message[300];	//MQTT���ϱ���Ϣ����

//�ı�LED��״̬
void Change_LED_Status(void)
{
	HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin); 
}

/******************************  �������ģʽ����  *****************************/

//�������ģʽ�ȴ��ֶ�����
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
			case 0:user_main_error("ESP8266��ʼ��ʧ�ܣ�\r\n");break;
			case 1:user_main_error("ESP8266�����ȵ�ʧ�ܣ�\r\n");break;
			case 2:user_main_error("ESP8266���Ӱ����Ʒ�����ʧ�ܣ�\r\n");break;
			case 3:user_main_error("ESP8266������MQTT��½ʧ�ܣ�\r\n");break;
			case 4:user_main_error("ESP8266������MQTT��������ʧ�ܣ�\r\n");break;
			default:user_main_info("Nothing\r\n");break;
		}
		user_main_info("������������");
		HAL_GPIO_TogglePin(BEEP_GPIO_Port,BEEP_Pin);
		HAL_Delay(500);
	}
}

/******************************  ������������Դ���  *****************************/

/* ���Դ��ڲ��Ժ��� */
void TEST_usart2(void)
{
	user_main_debug("����USART2���Դ��룡\n");
	HAL_Delay(1000);
}

/* LED���Ժ��� */
void TEST_LED(void)
{
	
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
	HAL_Delay(500);
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
	HAL_Delay(500);
}

/* us������ʱ���Ժ��� */
void TEST_delayus(void)
{
	static uint16_t tim1_test;
	
	//��ʱ1000��1ms
	for(tim1_test = 0;tim1_test<1000;tim1_test++)
	{
		//��ʱ1ms
		TIM1_Delay_us(1000);
	}
	user_main_debug("����us����ʱ�������Դ��룬1s��ӡһ�Σ�\n");
}

/* DHT11��ʪ�ȴ��������Ժ��� */
void TEST_DHT11(void)
{
	uint8_t temperature;
	uint8_t humidity;
	uint8_t get_times;

	// ��ȡ��ʪ����Ϣ���ô��ڴ�ӡ,��ȡʮ�Σ�ֱ���ɹ�����
	for(get_times=0;get_times<10;get_times++)
	{
		if(!dht11Read(&temperature, &humidity))//Read DHT11 Value
		{
				user_main_info("temperature=%d,humidity=%d \n",temperature,humidity);
				break;
		}
	}
}

/* ESP8266&MQTT���Դ��� */
void TEST_ES8266MQTT(void)
{
	uint8_t status=0;

	//��ʼ��
	if(ESP8266_Init())
	{
		user_main_info("ESP8266��ʼ���ɹ���\r\n");
		status++;
	}
	else Enter_ErrorMode(0);

	//�����ȵ�
	if(status==1)
	{
		if(ESP8266_ConnectAP(WIFI_NAME,WIFI_PASSWD))
		{
			user_main_info("ESP8266�����ȵ�ɹ���\r\n");
			status++;
		}
		else Enter_ErrorMode(1);
	}
	
	//���Ӱ�����IOT������
	if(status==2)
	{
		if(ESP8266_ConnectServer("TCP",MQTT_BROKERADDRESS,1883)!=0)
		{
			user_main_info("ESP8266���Ӱ����Ʒ������ɹ���\r\n");
			status++;
		}
		else Enter_ErrorMode(2);
	}
	
	//��½MQTT
	if(status==3)
	{
		if(MQTT_Connect(MQTT_CLIENTID, MQTT_USARNAME, MQTT_PASSWD) != 0)
		{
			user_main_info("ESP8266������MQTT��½�ɹ���\r\n");
			status++;
		}
		else Enter_ErrorMode(3);
	}

	//��������
	if(status==4)
	{
		if(MQTT_SubscribeTopic(MQTT_SUBSCRIBE_TOPIC,0,1) != 0)
		{
			user_main_info("ESP8266������MQTT��������ɹ���\r\n");
		}
		else Enter_ErrorMode(4);
	}
}

/******************************  STM32 MQTTҵ�����  *****************************/

//MQTT��ʼ������
void ES8266_MQTT_Init(void)
{
	uint8_t status=0;

	//��ʼ��
	if(ESP8266_Init())
	{
		user_main_info("ESP8266��ʼ���ɹ���\r\n");
		status++;
	}
	else Enter_ErrorMode(0);

	//�����ȵ�
	if(status==1)
	{
		if(ESP8266_ConnectAP(WIFI_NAME,WIFI_PASSWD))
		{
			OLED_ShowString(100,16*0,"OK",16);OLED_Refresh();
			user_main_info("ESP8266�����ȵ�ɹ���\r\n");
			status++;
		}
		else Enter_ErrorMode(1);
	}
	
	//���Ӱ�����IOT������
	if(status==2)
	{
		if(ESP8266_ConnectServer("TCP",MQTT_BROKERADDRESS,1883)!=0)
		{
			OLED_ShowString(100,16*1,"OK",16);OLED_Refresh();
			user_main_info("ESP8266���Ӱ����Ʒ������ɹ���\r\n");
			status++;
		}
		else Enter_ErrorMode(2);
	}
	
	//��½MQTT
	if(status==3)
	{
		if(MQTT_Connect(MQTT_CLIENTID, MQTT_USARNAME, MQTT_PASSWD) != 0)
		{
			OLED_ShowString(100,16*2,"OK",16);OLED_Refresh();
			user_main_info("ESP8266������MQTT��½�ɹ���\r\n");
			status++;
		}
		else Enter_ErrorMode(3);
	}

	//��������
	if(status==4)
	{
		if(MQTT_SubscribeTopic(MQTT_SUBSCRIBE_TOPIC,0,1) != 0)
		{
			OLED_ShowString(100,16*3,"OK",16);OLED_Refresh();
			user_main_info("ESP8266������MQTT��������ɹ���\r\n");
			HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
			HAL_Delay(200);
			HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
			OLED_Clear();
		}
		else Enter_ErrorMode(4);
	}
}

//��Ƭ��״̬�ϱ�
void STM32_StatusReport(void)
{
	//��ȡ��ʪ����Ϣ
	uint8_t temperature;
	uint8_t humidity;
	uint8_t get_times;
	
	// ��ȡ��ʪ����Ϣ���ô��ڴ�ӡ,��ȡʮ�Σ�ֱ���ɹ�����
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
	
	//�ϱ�һ������ ��ͬƽ̨��ʽ��ͬ 
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

char temp_str[30];    // ��ʱ�Ӵ�
void ReadStrUnit(char * str,char *temp_str,int idx,int len)  // ��ĸ���л�ȡ���Ӵ�������ȵ���ʱ�Ӵ�
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
        //printf("error 1 \n"); // �Ӵ���ĸ����
        return -1;
    }

    while(1)
    {
        ReadStrUnit(str1,temp_str,idx,len2);    // ���ϻ�ȡ�Ĵ� ĸ���� idx λ�ô�������ʱ�Ӵ�
        if(strcmp(str2,temp_str)==0)break;      // ����ʱ�Ӵ����Ӵ�һ�£�����ѭ��
        idx++;                                  // �ı��ĸ����ȡ��ʱ�Ӵ���λ��
        if(idx>=len1)return -1;                 // �� idx �Ѿ�����ĸ�����ȣ�˵��ĸ�����������Ӵ�
    }

    return idx;    // �����Ӵ���һ���ַ���ĸ���е�λ��
}

//Ӧ��MQTT����
void MQTT_response(char * buf,uint16_t len1,uint16_t len2){
										//MQTT��Ӧ���� MQTT��Ӧ���鳤�� ��ƽ̨�������ݳ���
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

//����MQTT�·�����Ϣ
void deal_MQTT_message(uint8_t* buf,uint16_t len)
{
	uint8_t data[512];
	uint16_t data_len = len;
	for(int i=0;i<data_len;i++)
	{
		data[i] = buf[i];
		HAL_UART_Transmit(&huart2,&data[i],1,100);
	}
	memset(usart1_rxbuf,0,sizeof(usart1_rxbuf)); //��ս��ջ���  
	usart1_rxcounter=0;
	user_main_info("MQTT�յ���Ϣ,���ݳ���=%d \n",data_len);
	
	//������Ӧι��
	if(data[data_len-1] == 0&&data[data_len-2] == 0xD0){HAL_IWDG_Refresh(&hiwdg);}
	
	//�����Ƿ񿪹ص�
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

//main ����ģ��


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
//	//����USART1�����ж�
//	HAL_UART_Receive_IT(&huart1,usart1_rxone,1);			//��USART1�жϣ����ն�����Ϣ
//	ES8266_MQTT_Init();																			//��ʼ��MQTT
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
//		//���е���˸
////		if(Counter_RUNLED_Blink++>COUNTER_LEDBLINK)
////		{
////			Counter_RUNLED_Blink = 0;
////			HAL_GPIO_TogglePin(LED_G_GPIO_Port,LED_G_Pin);
////		}
//		
//		//����״̬��ӡ
//		if(Counter_RUNInfo_Send++>COUNTER_RUNINFOSEND)
//		{
//			Counter_RUNInfo_Send = 0;
//			user_main_info("�����������У�\r\n");
//		}
//		
//		//����������
////		if(Counter_MQTT_Heart++>COUNTER_MQTTHEART)
////		{
////			Counter_MQTT_Heart = 0;
////			MQTT_SentHeart();
////		}
//		
//		//����״̬�ϱ�
//		if(Counter_StatusReport++>COUNTER_STATUSREPORT)
//		{
//			Counter_StatusReport = 0;
//			STM32DHT11_StatusReport();
//		}
//		
//		//������ջ���������
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

