#include "esp8266_at.h"
#include "esp8266_mqtt.h"

//usart1发送和接收数组
uint8_t usart1_txbuf[256];
uint8_t usart1_rxbuf[512];
uint8_t usart1_rxone[1];
uint8_t usart1_rxcounter;


//串口1发送一个字节
static void USART1_SendOneByte(uint8_t val)
{
	((UART_HandleTypeDef *)&huart1)->Instance->DR = ((uint16_t)val & (uint16_t)0x01FF);
	while((((UART_HandleTypeDef *)&huart1)->Instance->SR&0X40)==0);//等待发送完成
}


//向ESP8266发送定长数据
void ESP8266_ATSendBuf(uint8_t* buf,uint16_t len)
{
	memset(usart1_rxbuf,0, 256);
	
	//每次发送前将接收串口接收总数置0,为了接收
	usart1_rxcounter = 0;	
	
	//定长发送
	HAL_UART_Transmit(&huart1,(uint8_t *)buf,len,0xFFFF);
}

//向ESP8266发送字符串
void ESP8266_ATSendString(char* str)
{
  memset(usart1_rxbuf,0, 256);
	
	//每次发送前将接收串口接收总数置0,为了接收
	usart1_rxcounter = 0;	
	
	//发送方法1
	while(*str)		USART1_SendOneByte(*str++);
	
	//发送法法2
//	HAL_UART_Transmit(&huart1,(uint8_t *)str,strlen(str),0xFFFF);
}

//退出透传
void ESP8266_ExitUnvarnishedTrans(void)
{
	ESP8266_ATSendString("+++");HAL_Delay(50);
	ESP8266_ATSendString("+++");HAL_Delay(50);	
}

//查找字符串中是否包含另一个字符串
uint8_t FindStr(char* dest,char* src,uint16_t retry_nms)
{
	retry_nms/=10;                   //超时时间

	while(strstr(dest,src)==0 && retry_nms--)//等待串口接收完毕或超时退出
	{		
		HAL_Delay(10);
	}

	if(retry_nms) return 1;                       

	return 0; 
}

/**
 * 功能：检查ESP8266是否正常
 * 参数：None
 * 返回值：ESP8266返回状态
 *        非0 ESP8266正常
 *        0 ESP8266有问题  
 */
uint8_t ESP8266_Check(void)
{
	uint8_t check_cnt=5;
	while(check_cnt--)
	{
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf)); 	 //清空接收缓冲
		ESP8266_ATSendString("AT\r\n");     		 			//发送AT握手指令	
		if(FindStr((char*)usart1_rxbuf,"OK",200) != 0)
		{
			return 1;
		}
	}
	return 0;
}

/**
 * 功能：初始化ESP8266
 * 参数：None
 * 返回值：初始化结果，非0为初始化成功,0为失败
 */
uint8_t ESP8266_Init(void)
{
	//清空发送和接收数组
	memset(usart1_txbuf,0,sizeof(usart1_txbuf));
	memset(usart1_rxbuf,0,sizeof(usart1_rxbuf));
	
	ESP8266_ExitUnvarnishedTrans();		//退出透传
	HAL_Delay(500);
	ESP8266_ATSendString("AT+RST\r\n");
	HAL_Delay(800);
	if(ESP8266_Check()==0)              //使用AT指令检查ESP8266是否存在
	{
		return 0;
	}
	
	memset(usart1_rxbuf,0,sizeof(usart1_rxbuf));    //清空接收缓冲
	ESP8266_ATSendString("ATE0\r\n");     	//关闭回显 
	if(FindStr((char*)usart1_rxbuf,"OK",500)==0)  //设置不成功
	{
			return 0;      
	}
	return 1;                         //设置成功
}

/**
 * 功能：恢复出厂设置
 * 参数：None
 * 返回值：None
 * 说明:此时ESP8266中的用户设置将全部丢失回复成出厂状态
 */
void ESP8266_Restore(void)
{
	ESP8266_ExitUnvarnishedTrans();          	//退出透传
  HAL_Delay(500);
	ESP8266_ATSendString("AT+RESTORE\r\n");		//恢复出厂 	
}

/**
 * 功能：连接热点
 * 参数：
 *         ssid:热点名
 *         pwd:热点密码
 * 返回值：
 *         连接结果,非0连接成功,0连接失败
 * 说明： 
 *         失败的原因有以下几种(UART通信和ESP8266正常情况下)
 *         1. WIFI名和密码不正确
 *         2. 路由器连接设备太多,未能给ESP8266分配IP
 */
uint8_t ESP8266_ConnectAP(char* ssid,char* pswd)
{
	uint8_t cnt=5;
	while(cnt--)
	{
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf));     
		ESP8266_ATSendString("AT+CWMODE_CUR=1\r\n");              //设置为STATION模式	
		if(FindStr((char*)usart1_rxbuf,"OK",200) != 0)
		{
			break;
		}             		
	}
	if(cnt == 0)
		return 0;

	cnt=2;
	while(cnt--)
	{                    
		memset(usart1_txbuf,0,sizeof(usart1_txbuf));//清空发送缓冲
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf));//清空接收缓冲
		sprintf((char*)usart1_txbuf,"AT+CWJAP_CUR=\"%s\",\"%s\"\r\n",ssid,pswd);//连接目标AP
		ESP8266_ATSendString((char*)usart1_txbuf);	
		if(FindStr((char*)usart1_rxbuf,"OK",8000)!=0)                      //连接成功且分配到IP
		{
			return 1;
		}
	}
	return 0;
}

//开启透传模式
static uint8_t ESP8266_OpenTransmission(void)
{
	//设置透传模式
	uint8_t cnt=2;
	while(cnt--)
	{
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf));    
		ESP8266_ATSendString("AT+CIPMODE=1\r\n");  
		if(FindStr((char*)usart1_rxbuf,"OK",200)!=0)
		{	
			return 1;
		}
	}
	return 0;
}

/**
 * 功能：使用指定协议(TCP/UDP)连接到服务器
 * 参数：
 *         mode:协议类型 "TCP","UDP"
 *         ip:目标服务器IP
 *         port:目标是服务器端口号
 * 返回值：
 *         连接结果,非0连接成功,0连接失败
 * 说明： 
 *         失败的原因有以下几种(UART通信和ESP8266正常情况下)
 *         1. 远程服务器IP和端口号有误
 *         2. 未连接AP
 *         3. 服务器端禁止添加(一般不会发生)
 */
uint8_t ESP8266_ConnectServer(char* mode,char* ip,uint16_t port)
{
	uint8_t cnt;
   
	ESP8266_ExitUnvarnishedTrans();                   //多次连接需退出透传
	HAL_Delay(500);

	//连接服务器
	cnt=2;
	while(cnt--)
	{
		memset(usart1_txbuf,0,sizeof(usart1_txbuf));//清空发送缓冲
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf));//清空接收缓冲   
		sprintf((char*)usart1_txbuf,"AT+CIPSTART=\"%s\",\"%s\",%d\r\n",mode,ip,port);
		ESP8266_ATSendString((char*)usart1_txbuf);
		if(FindStr((char*)usart1_rxbuf,"CONNECT",8000) !=0 )
		{
			break;
		}
	}
	if(cnt == 0) 
		return 0;
	
	//设置透传模式
	if(ESP8266_OpenTransmission()==0) return 0;
	
	//开启发送状态
	cnt=2;
	while(cnt--)
	{
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf)); //清空接收缓冲   
		ESP8266_ATSendString("AT+CIPSEND\r\n");//开始处于透传发送状态
		if(FindStr((char*)usart1_rxbuf,">",200)!=0)
		{
			return 1;
		}
	}
	return 0;
}

/**
 * 功能：主动和服务器断开连接
 * 参数：None
 * 返回值：
 *         连接结果,非0断开成功,0断开失败
 */
uint8_t DisconnectServer(void)
{
	uint8_t cnt;
	
	ESP8266_ExitUnvarnishedTrans();	//退出透传
	HAL_Delay(500);
	
	while(cnt--)
	{
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf)); //清空接收缓冲   
		ESP8266_ATSendString("AT+CIPCLOSE\r\n");//关闭链接

		if(FindStr((char*)usart1_rxbuf,"CLOSED",200)!=0)//操作成功,和服务器成功断开
		{
			break;
		}
	}
	if(cnt) return 1;
	return 0;
}

/******************************  进入错误模式代码  *****************************/

//进入错误模式等待手动重启
void Enter_ErrorMode(uint8_t mode)
{
	while(1)
	{
		switch(mode){
			case 0:
				HAL_UART_Transmit(&huart3,(uint8_t *)"ESP8266初始化失败！\r\n",sizeof("ESP8266初始化失败！\r\n"),500);
				break;
			case 1:
				HAL_UART_Transmit(&huart3,(uint8_t *)"ESP8266连接热点失败！\r\n",sizeof("ESP8266连接热点失败！\r\n"),500);
			break;
			case 2:
				HAL_UART_Transmit(&huart3,(uint8_t *)"ESP8266连接阿里云服务器失败！\r\n",sizeof("ESP8266连接阿里云服务器失败！\r\n"),500);
			break;
			case 3:
				HAL_UART_Transmit(&huart3,(uint8_t *)"ESP8266阿里云MQTT登陆失败！\r\n",sizeof("ESP8266阿里云MQTT登陆失败！\r\n"),500);
			break;
			case 4:
				HAL_UART_Transmit(&huart3,(uint8_t *)"ESP8266阿里云MQTT订阅主题失败！\r\n",sizeof("ESP8266阿里云MQTT订阅主题失败！\r\n"),500);
			break;
			default:
				HAL_UART_Transmit(&huart3,(uint8_t *)"Nothing\r\n",sizeof("Nothing\r\n"),500);
			break;
		}
		HAL_UART_Transmit(&huart3,(uint8_t *)"请重启开发板",sizeof("请重启开发板"),500);
		
		HAL_GPIO_TogglePin(BEEP_GPIO_Port,BEEP_Pin);
		HAL_Delay(200);
	}
}

//MQTT初始化函数
void ES8266_MQTT_Init(void)
{
	uint8_t status=0;

	//初始化
	if(ESP8266_Init())
	{
		HAL_UART_Transmit(&huart3,(unsigned char *)"ESP8266初始化成功！\r\n",sizeof("ESP8266初始化成功！\r\n"),500);
		status++;
	}
	else Enter_ErrorMode(0);

	//连接热点
	if(status==1)
	{
		if(ESP8266_ConnectAP(WIFI_NAME,WIFI_PASSWD))
		{
			HAL_UART_Transmit(&huart3,(unsigned char *)"ESP8266连接热点成功！\r\n",sizeof("ESP8266连接热点成功！\r\n"),500);
			status++;
		}
		else Enter_ErrorMode(1);
	}
	
	//连接阿里云IOT服务器
	if(status==2)
	{
		if(ESP8266_ConnectServer("TCP",MQTT_BROKERADDRESS,1883)!=0)
		{
			HAL_UART_Transmit(&huart3,(unsigned char *)"ESP8266连接阿里云服务器成功！\r\n",sizeof("ESP8266连接阿里云服务器成功！\r\n"),500);
			status++;
		}
		else Enter_ErrorMode(2);
	}
	
	//登陆MQTT
	if(status==3)
	{
		if(MQTT_Connect(MQTT_CLIENTID, MQTT_USARNAME, MQTT_PASSWD) != 0)
		{
			HAL_UART_Transmit(&huart3,(unsigned char *)"ESP8266阿里云MQTT登陆成功！\r\n",sizeof("ESP8266阿里云MQTT登陆成功！\r\n"),500);
			status++;
		}
		else Enter_ErrorMode(3);
	}

	//订阅主题
	if(status==4)
	{
		if(MQTT_SubscribeTopic(MQTT_SUBSCRIBE_TOPIC,0,1) != 0)
		{
			HAL_UART_Transmit(&huart3,(unsigned char *)"ESP8266阿里云MQTT订阅主题成功！\r\n",sizeof("ESP8266阿里云MQTT订阅主题成功！\r\n"),500);
		}
		else Enter_ErrorMode(4);
	}
}
