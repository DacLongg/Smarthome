#include"wifi.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include"driver/gpio.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include"event_groups.h"

#include"finger.h"
#include "password.h"

#define END_PROCESS     0x01

char * topic_suncrib[3] = {"gladys/device/mqtt:livingroom:device:door/feature/mqtt:1/state", "/smarthome/door:changepassword", "/smarthome/door:deleteall"};
extern EventGroupHandle_t Mqtt_event_group;

void Get_sevice_mqtt(uint8_t *MapService, uint8_t length, uint8_t *ID_service, uint8_t service);

// static EventGroupHandle_t s_wifi_event_group;
static const char *TAG_MQTT = "MQTT_EXAMPLE";
uint32_t MQTT_CONNEECTED = 0;
extern uint8_t *FGdata_Subcrib;
extern uint8_t *PwData_Subcrib;
extern EventGroupHandle_t LCD_EventGroup;

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
            if((strncmp(event->data,  "1", event->data_len)) == 0)
            {
                printf("open the door \n");
                gpio_set_level(GPIO_NUM_23, 0);
            }
            else
            {
                printf("close the door \n");
                gpio_set_level(GPIO_NUM_23, 1);
            }
            
            
            esp_mqtt_client_publish(client, "gladys/master/device/mqtt:livingroom:device:door/feature/mqtt:1/text", "OPENED BY APP", 0, 0, 0);
            xEventGroupSetBits( LCD_EventGroup, END_PROCESS);
        }

        else if((strncmp(event->topic,  topic_suncrib[1], event->topic_len)) == 0) //;event->topic == (char *)"/smarthome/door/changepassword")
        {
            xEventGroupSetBits( Mqtt_event_group, MQTT_ACK);
            *PwData_Subcrib = *(uint8_t *)event->data;

            
            Password[ENTER_CONF_PASS].index = event->data_len;
            for(int i = Password[ENTER_CONF_PASS].index - 1; i >= 0; --i)
            {
                Password[ENTER_CONF_PASS].data[i] = (int)(event->data[i] - '0');
            }
            Process_store("storage", "Password", Password[ENTER_CONF_PASS].data, Password[ENTER_CONF_PASS].index);

 
            char  Str_password[20];
            for(uint8_t count = 0; count < Password[ENTER_CONF_PASS].index; count ++)
            {
                sprintf ((char*)&Str_password[count], "%d", Password[ENTER_CONF_PASS].data[count]);
            }
            printf("ret = %s \n", Str_password);

            // esp_mqtt_client_publish(client, "/smarthome/door/passwordchanged", "passwordchanged", 0, 0, 0);
            esp_mqtt_client_publish(client, "gladys/master/device/mqtt:livingroom:device:door/feature/mqtt:40/text", Str_password, 0, 0, 0);

            // printf("password : %d \n", Password[ENTER_CONF_PASS].data[0]);
            // printf("password : %d \n", Password[ENTER_CONF_PASS].data[1]);
            // printf("password : %d \n", Password[ENTER_CONF_PASS].data[2]);
            // printf("password : %d \n", Password[ENTER_CONF_PASS].data[3]);
            // printf("password : %d \n", Password[ENTER_CONF_PASS].data[4]);
            // printf("password : %d \n", Password[ENTER_CONF_PASS].data[5]);
            // printf("password : %d \n", Password[ENTER_CONF_PASS].data[6]);
            // printf("password : %d \n", Password[ENTER_CONF_PASS].data[7]);

        }
        else if((strncmp(event->topic,  topic_suncrib[2], event->topic_len)) == 0)
        {
            xEventGroupSetBits( Mqtt_event_group, MQTT_ACK);

            char *data = "1";
            printf("deleta \n");

            if((strncmp(event->data,  data, event->data_len)) == 0)
            {
                FP_FuncRun = 2;
            }
                            
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
    .broker.address.uri = "mqtt://0.tcp.au.ngrok.io:12126",  //"mqtt://0.tcp.ap.ngrok.io:14923",  //tcp://0.tcp.ap.ngrok.io:10889   //0.tcp.ap.ngrok.io:18919  // ngrok's public URL and port:   mqtt://0.tcp.ap.ngrok.io:19230
    .credentials.username = "smarthome",
    .credentials.authentication.password = "1234567890",
};
    
    client = esp_mqtt_client_init(&mqttConfig);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}



void Publisher_task(void *pvParameters)
{
    EventBits_t waitEvent;
    for( ;; )
    {
        waitEvent  =  xEventGroupWaitBits ( Mqtt_event_group, 
                        UNLOCK_BYPASS | UNLOCK_BYFINGER | CHANGE_PASS | REG_FINGER | MQTT_ACK,
                        pdTRUE, 
                        pdFALSE, 
                        portMAX_DELAY );
        printf("publish \n");
        if(MQTT_CONNEECTED)
        {
            if((waitEvent & UNLOCK_BYPASS) != 0)
            {
                esp_mqtt_client_publish(client, "gladys/master/device/mqtt:livingroom:device:door/feature/mqtt:1/state", "1", 0, 0, 0);
                printf("publish  Open the door by password \n");
            }
            else if((waitEvent & UNLOCK_BYFINGER) != 0)
            {
                esp_mqtt_client_publish(client, "gladys/master/device/mqtt:livingroom:device:door/feature/mqtt:1/state", "1", 0, 0, 0);
                char *Str_FingerID = "Finger_ID:";
                char Finger_ID[6];
                sprintf(Finger_ID, "%d", dataRx.data.comp.PageNumb);
                char Buff_fingerID[20];
                strcpy(Buff_fingerID, Str_FingerID);
                strcat(Buff_fingerID, Finger_ID);
                // esp_mqtt_client_publish(client, "gladys/master/device/mqtt:door:door-lock/feature/mqtt:door:door-lock:password/text", Buff_fingerID, 0, 0, 0);
                printf("publish  Open the door by Finger Print : %s \n", Buff_fingerID);
            }
            else if((waitEvent & CHANGE_PASS) != 0)
            {
                char  Str_password[20];
                printf(" publish change pass \n");
                for(uint8_t count = 0; count < Password[ENTER_CONF_PASS].index; count ++)
                {
                    sprintf ((char*)&Str_password[count], "%d", Password[ENTER_CONF_PASS].data[count]);
                }
                printf("ret = %s \n", Str_password);

                // esp_mqtt_client_publish(client, "/smarthome/door/passwordchanged", "passwordchanged", 0, 0, 0);
                esp_mqtt_client_publish(client, "gladys/master/device/mqtt:livingroom:device:door/feature/mqtt:40/text", Str_password, 0, 0, 0);
                // printf("ret = %d \n", ret);
                // printf("ret = %s \n", Str_password[0]);
            }
            else if((waitEvent & REG_FINGER) != 0)
            {
                esp_mqtt_client_publish(client, "/topic/test3", " Password has been changed, new pass :", 0, 0, 0);
                esp_mqtt_client_publish(client, "/topic/test3", (char *)Password[2].data, 0, 0, 0);
            }
            else if((waitEvent & MQTT_ACK) != 0)
            {
                esp_mqtt_client_publish(client, "gladys/master/device/mqtt:door:door-lock/feature/mqtt:door:door-lock:password/text", "E_OK", 0, 0, 0);
            }
            

        }
    }
}

void Publish_door(char *state)
{
    esp_mqtt_client_publish(client, "gladys/master/device/mqtt:livingroom:device:door/feature/mqtt:1/state", state, 0, 0, 0);
}

