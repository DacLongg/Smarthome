#include"user_mqtt.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include"event_groups.h"

extern void TurnOn(uint8_t deviec);
extern void TurnOff(uint8_t deviec);
extern void buzzer(uint8_t state);
extern uint8_t enable_peopleSS;
char * topic_suncrib[3] = {"gladys/device/mqtt:kitchen:device:lamp/feature/mqtt:16/state", "gladys/device/mqtt:kitchen:device:airconditionor/feature/mqtt:18/state", "gladys/device/mqtt:kitchen:device:siren/feature/mqtt:15/state"};


void Get_sevice_mqtt(uint8_t *MapService, uint8_t length, uint8_t *ID_service, uint8_t service);

// static EventGroupHandle_t s_wifi_event_group;
static const char *TAG_MQTT = "MQTT_EXAMPLE";
uint32_t MQTT_CONNEECTED = 0;


static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    //ESP_LOGD(TAG_MQTT, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_CONNECTED");
        MQTT_CONNEECTED=1;
        
        msg_id = esp_mqtt_client_subscribe(client, topic_suncrib[0], 0);
        ESP_LOGI(TAG_MQTT, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, topic_suncrib[1], 1);
        ESP_LOGI(TAG_MQTT, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, topic_suncrib[2], 2);
        ESP_LOGI(TAG_MQTT, "sent subscribe successful, msg_id=%d", msg_id);

        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DISCONNECTED");
        MQTT_CONNEECTED=0;
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        if((strncmp(event->topic,  topic_suncrib[0], event->topic_len)) == 0)
        {
            uint8_t staus = atoi(event->data);
            if(staus == 1)
            {
                enable_peopleSS = 0;
                TurnOn(1);                
            }
            else if(staus == 0)
            {
                enable_peopleSS = 1;
                TurnOff(1);               
            }
            printf("turn light , %d \n", staus);
        }

        else if((strncmp(event->topic,  topic_suncrib[1], event->topic_len)) == 0) //;event->topic == (char *)"/smarthome/door/changepassword")
        {
            
            uint8_t staus = atoi(event->data);
            if(staus == 1)
            {
                
                TurnOn(0);
            }
            else if(staus == 0)
            {
                
                TurnOff(0);
            }
            printf("turn fan %d \n", staus);
        }
        else if((strncmp(event->topic,  topic_suncrib[2], event->topic_len)) == 0)
        {
            uint8_t status = atoi(event->data);
            if(status != 0)
            {
                buzzer(1);
            }
            else if(status == 0)
            {
                
                buzzer(0);
            }
            printf("turn fan %d \n", status);
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGI(TAG_MQTT, "Other event id:%d", event->event_id);
        break;
    }
}


esp_mqtt_client_handle_t client = NULL;
void mqtt_app_start(void)
{
    ESP_LOGI(TAG_MQTT, "STARTING MQTT");
    // esp_mqtt_client_config_t mqttConfig = {
    //     .broker.address.uri = "mqtt://113.190.53.253:1883",
    //     .broker.address.transport = MQTT_TRANSPORT_OVER_SSL,
    //     // .broker.address.port = 1883,
    //     .credentials.username = "smarthome",
    //     .credentials.authentication.password = "66zLSUOIJTr2AAVINhWx"};

esp_mqtt_client_config_t mqttConfig = {
    .broker.address.uri = "mqtt://0.tcp.au.ngrok.io:12126",  // ngrok's public URL and port:   mqtt://0.tcp.ap.ngrok.io:19230
    .credentials.username = "smarthome",
    .credentials.authentication.password = "1234567890",
};
    
    client = esp_mqtt_client_init(&mqttConfig);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

EventGroupHandle_t Mqtt_event_group;







