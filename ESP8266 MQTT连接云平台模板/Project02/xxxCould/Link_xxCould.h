#include <stdint.h>

#ifndef __LINKXXCOULD_H
#define __LINKXXCOULD_H


#define USER_MAIN_DEBUG

#ifdef USER_MAIN_DEBUG
#define user_main_printf(format, ...) printf( format "\r\n",##__VA_ARGS__)
#define user_main_info(format, ...) printf("【main】info:" format "\r\n",##__VA_ARGS__)
#define user_main_debug(format, ...) printf("【main】debug:" format "\r\n",##__VA_ARGS__)
#define user_main_error(format, ...) printf("【main】error:" format "\r\n",##__VA_ARGS__)
#else
#define user_main_printf(format, ...)
#define user_main_info(format, ...)
#define user_main_debug(format, ...)
#define user_main_error(format, ...)
#endif

//此处根据自己的wifi作调整
#define WIFI_NAME "HappyOneDay"
#define WIFI_PASSWD "1234567890"

//此处是阿里云服务器的登陆配置
#define MQTT_BROKERADDRESS "a1lAoazdH1w.iot-as-mqtt.cn-shanghai.aliyuncs.com"
#define MQTT_CLIENTID "00001|securemode=3,signmethod=hmacsha1|"
#define MQTT_USARNAME "BZL01&a1lAoazdH1w"
#define MQTT_PASSWD "51A5BB10306E976D6C980F73037F2D9496D2813A"
#define	MQTT_PUBLISH_TOPIC "/sys/a1lAoazdH1w/BZL01/thing/event/property/post"
#define MQTT_SUBSCRIBE_TOPIC "/sys/a1lAoazdH1w/BZL01/thing/service/property/set"
#define MQTT_SUBSCRIBE_RESPONSE_TOPIC "$sys/554632/esp01s/cmd/response/"

//此处是主循环运行延时宏定义
#define LOOPTIME 30 	//程序周期循环延时时间：30ms
#define COUNTER_LEDBLINK			(300/LOOPTIME)		//LED运行闪烁时间：300ms
#define COUNTER_RUNINFOSEND		(5000/LOOPTIME)		//运行串口提示：5s
#define COUNTER_MQTTHEART     (5000/LOOPTIME)		//MQTT发送心跳包：5s
#define COUNTER_STATUSREPORT	(3000/LOOPTIME)		//状态上传：3s

void Enter_ErrorMode(uint8_t mode);
void ES8266_MQTT_Init(void);
void Change_LED_Status(void);
void STM32_StatusReport(void);
void deal_MQTT_message(uint8_t* buf,uint16_t len);
void MQTT_response(char * buf,uint16_t len1,uint16_t len2);
void TEST_LED(void);

#endif
