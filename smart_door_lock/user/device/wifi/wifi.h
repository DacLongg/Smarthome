#ifndef __WIFI_H__
#define __WIFI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include"stdio.h"

#define UNLOCK_BYPASS   0x01
#define UNLOCK_BYFINGER 0x02
#define CHANGE_PASS     0x04
#define REG_FINGER      0x08
#define MQTT_ACK        0x10

void Publisher_task(void *pvParameters);
void mqtt_app_start(void);

void Publish_door(char *state);

 #ifdef __cplusplus
}
#endif
#endif