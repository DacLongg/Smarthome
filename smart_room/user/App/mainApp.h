#ifndef __MAINAPP_H__
#define __MAINAPP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include"stdio.h"
#include"esp_log.h"
#include"stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include"string.h"
#include "driver/i2c.h"
#include "nvs_flash.h"
#include "esp_wifi.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "mqtt_client.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include"event_groups.h"
#include "driver/uart.h"
// #include"freertos/ringbuf.h"
// #include "uart_types.h"

#include"lcd.h"
#include"mq2.h"
#include"dht22.h"
#include"user_mqtt.h"
#include "stdbool.h"
#include"sim.h"

#define PATTERN_CHR_NUM    (3)

#define BUZZER_PIN      GPIO_NUM_13
#define RELAY1_PIN      GPIO_NUM_4
#define RELAY2_PIN      GPIO_NUM_5
#define PWK_PIN         GPIO_NUM_12

extern EventGroupHandle_t Mqtt_event_group;

void mainApp(void);
void uart_task1(void *pvParameters);
void Button_task(void *pvParameters);
void tash_handle(void *pvParameters);
extern esp_mqtt_client_handle_t client;

void TurnOn(uint8_t deviec);
void TurnOff(uint8_t deviec);

void Dht22_getParan(void);
void Session_communi(void);
void fire_alarm(void);

void detect_pe0ple(void);
void buzzer(uint8_t state);

 #ifdef __cplusplus
}
#endif
#endif