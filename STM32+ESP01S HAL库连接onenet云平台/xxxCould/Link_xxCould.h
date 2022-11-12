#include <stdint.h>

#ifndef __LINKXXCOULD_H
#define __LINKXXCOULD_H


#define USER_MAIN_DEBUG

#ifdef USER_MAIN_DEBUG
#define user_main_printf(format, ...) printf( format "\r\n",##__VA_ARGS__)
#define user_main_info(format, ...) printf("��main��info:" format "\r\n",##__VA_ARGS__)
#define user_main_debug(format, ...) printf("��main��debug:" format "\r\n",##__VA_ARGS__)
#define user_main_error(format, ...) printf("��main��error:" format "\r\n",##__VA_ARGS__)
#else
#define user_main_printf(format, ...)
#define user_main_info(format, ...)
#define user_main_debug(format, ...)
#define user_main_error(format, ...)
#endif

//�˴������Լ���wifi������
#define WIFI_NAME "Internet Of Things Skills"
#define WIFI_PASSWD "ThingInternets@fSkillsB44752"

//�˴��ǰ����Ʒ������ĵ�½����
#define MQTT_BROKERADDRESS "mqtts.heclouds.com"
#define MQTT_CLIENTID "device01"
#define MQTT_USARNAME "555339"
#define MQTT_PASSWD "version=2018-10-31&res=products%2F555339%2Fdevices%2Fdevice01&et=2037255523&method=md5&sign=YuH5MY%2Br6zplI2QBgBjlNQ%3D%3D"
#define	MQTT_PUBLISH_TOPIC "$sys/555339/device01/dp/post/json"
#define MQTT_SUBSCRIBE_TOPIC "$sys/555339/device01/cmd/request/+"
#define MQTT_SUBSCRIBE_RESPONSE_TOPIC "$sys/555339/device01/cmd/response/"

//�˴�����ѭ��������ʱ�궨��
#define LOOPTIME 30 	//��������ѭ����ʱʱ�䣺30ms
#define COUNTER_LEDBLINK			(300/LOOPTIME)		//LED������˸ʱ�䣺300ms
#define COUNTER_RUNINFOSEND		(5000/LOOPTIME)		//���д�����ʾ��5s
#define COUNTER_MQTTHEART     (5000/LOOPTIME)		//MQTT������������5s
#define COUNTER_STATUSREPORT	(3000/LOOPTIME)		//״̬�ϴ���3s

void Enter_ErrorMode(uint8_t mode);
void ES8266_MQTT_Init(void);
void Change_LED_Status(void);
void STM32_StatusReport(void);
void deal_MQTT_message(uint8_t* buf,uint16_t len);
void MQTT_response(char * buf,uint16_t len1,uint16_t len2);

#endif
