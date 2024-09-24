#ifndef __USER_MQTT_H__
#define __USER_MQTT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include"stdio.h"

#define PUBLISH_TEMP_HUMI   0x01
#define UNLOCK_BYFINGER 0x02
#define CHANGE_PASS     0x04
#define REG_FINGER      0x08

extern uint32_t MQTT_CONNEECTED;

void mqtt_app_start(void);


void Publisher_task(void *pvParameters);

 #ifdef __cplusplus
}
#endif
#endif