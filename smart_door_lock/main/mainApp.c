/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "nvs_flash.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"



#include "lwip/err.h"
#include "lwip/sys.h"

#include"button.h"
#include"event_groups.h"
#include"lcd.h"
#include"finger.h"
#include"wifi.h"
#include"password.h"

#define EXAMPLE_ESP_WIFI_SSID      "VietD"    //"be Moon"
#define EXAMPLE_ESP_WIFI_PASS      "vietzzzz2001"
#define EXAMPLE_ESP_MAXIMUM_RETRY  10


#define EX_UART_NUM UART_NUM_0
#define PATTERN_CHR_NUM    (3)
#define ESP_INTR_FLAG_LEVEL2        (1<<2)

// #define ENTERPASSWORD_EVENT         (1<<0)
// #define CHANGEPASSWORD_EVENT        (1<<1)
#define WIFI_CONNECTED_BIT (1<<0)
#define WIFI_FAIL_BIT      (1<<1)




// static QueueHandle_t uart0_queue;
QueueHandle_t uart1_queue;



void uart_task1(void *pvParameters);


// void uart_task(void *pvParameters);




EventGroupHandle_t Button_EventGroup;
EventGroupHandle_t s_wifi_event_group;
EventGroupHandle_t Mqtt_event_group;
extern EventGroupHandle_t RegFinger_EventGroup;


static const char *TAG = "wifi station";

static int s_retry_num = 0;


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    printf("handle");
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        printf("wifi connect");
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) 
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } 
        else 
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        mqtt_app_start();
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg.capable = true,
            .pmf_cfg.required = false
        },
    };
    
    esp_wifi_set_mode(WIFI_MODE_STA);
    
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    
    esp_wifi_start();

    

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) 
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
        // printf(" connected");
    } 
    else if (bits & WIFI_FAIL_BIT) 
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
        // printf(" fail ");
    } 
    else 
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}


void app_main(void)
{

    /* Configure the peripheral according to the LED type */
    printf("Start\n");
        //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_sta();
    printf("Start\n");

    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL2);
    Button_init_IO();
 

    gpio_config_t output5 = 
    {
        .pin_bit_mask =  (uint64_t)(1 << 2),
        .mode           = GPIO_MODE_OUTPUT,
        .pull_down_en   = GPIO_PULLDOWN_ENABLE,
        .pull_up_en     = GPIO_PULLUP_DISABLE,
        .intr_type      = GPIO_INTR_DISABLE
    };
    gpio_config(&output5);

        gpio_config_t output6 = 
    {
        .pin_bit_mask =  (uint64_t)(1 << 23),
        .mode           = GPIO_MODE_OUTPUT,
        .pull_down_en   = GPIO_PULLDOWN_DISABLE,
        .pull_up_en     = GPIO_PULLUP_ENABLE,
        .intr_type      = GPIO_INTR_DISABLE
    };
    gpio_config(&output6);
    gpio_set_level(GPIO_NUM_23, 1);

    // gpio_config_t output6 = 
    // {
    //     .pin_bit_mask =  (uint64_t)(1 << 21),
    //     .mode           = GPIO_MODE_OUTPUT,
    //     .pull_down_en   = GPIO_PULLDOWN_ENABLE,
    //     .pull_up_en     = GPIO_PULLUP_DISABLE,
    //     .intr_type      = GPIO_INTR_DISABLE
    // };
    // gpio_config(&output6);

   
    // Button_handle_Add_callback(Button_pressEvent_cb);
    // Button_longPress_add_callBack(Button_LongpressEvent_cb);


    uart_config_t uart1_config = {
        .baud_rate = 57600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_2,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    //Install UART driver, and get the queue.
    uart_driver_install(UART_NUM_1, 1024, 1024, 20, &uart1_queue, 0);
    uart_param_config(UART_NUM_1, &uart1_config);

    //Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(UART_NUM_0, 1, 3, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_set_pin(UART_NUM_1, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    Button_EventGroup = xEventGroupCreate();
    Mqtt_event_group  = xEventGroupCreate();

//    xTaskCreate(uart_task, "uart_task", 1024, NULL, 10, NULL);
    xTaskCreate(uart_task1, "uart_task1", 2048, NULL, 10, NULL);
    xTaskCreate(Button_task, "Button_task", 2048, NULL, 6, NULL);
    xTaskCreate(LCD_task, "LCD_task", 2048, NULL, 8, NULL);
    xTaskCreate(PasswordMNG_task, "PasswordMNG_task", 2048, NULL, 8, NULL);
    xTaskCreate(FingerPrint_task, "FingerPrint_task", 2048, NULL, 7, NULL);
    xTaskCreate(Publisher_task, "Publisher_Task", 1024 * 3, NULL, 5, NULL);

    // char *str = "123456";
   	// int l2 = strlen(str);
    // printf("len : %d \n", l2);
	// int num2 = 0;
    // uint8_t buff[10];
	// for(int i = l2 - 1; i >= 0; --i){
	// 	buff[i] = (int)(str[i] - '0');
	// }
    //               printf("d1 = %d \n", buff[0]);
    //             printf("d2 = %d \n", buff[1]);
    //             printf("d3 = %d \n", buff[2]);
    //             printf("d4 = %d \n", buff[3]);
    //             printf("d5 = %d \n", buff[4]);
    //             printf("d6 = %d \n", buff[5]);

    while (1)
    {
                
        
       vTaskDelay(500);
    }
}




// void uart_task(void *pvParameters)
// {
//    uart_event_t event;
//     size_t buffered_size;
//     uint8_t* dtmp = (uint8_t*) malloc(1024);
//     for(;;) {
//         //Waiting for UART event.
//         if(xQueueReceive(uart0_queue, (void * )&event, (TickType_t)portMAX_DELAY)) {
//             bzero(dtmp, 1024);
//             switch(event.type) {
//                 //Event of UART receving data
//                 /*We'd better handler data event fast, there would be much more data events than
//                 other types of events. If we take too much time on data event, the queue might
//                 be full.*/
//                 case UART_DATA:
                    
//                     uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
                    
//                     uart_write_bytes(EX_UART_NUM, (const char*) dtmp, event.size);
//                     break;
//                 //Event of HW FIFO overflow detected
//                 case UART_FIFO_OVF:
//                     // If fifo overflow happened, you should consider adding flow control for your application.
//                     // The ISR has already reset the rx FIFO,
//                     // As an example, we directly flush the rx buffer here in order to read more data.
//                     uart_flush_input(EX_UART_NUM);
//                     xQueueReset(uart0_queue);
//                     break;
//                 //Event of UART ring buffer full
//                 case UART_BUFFER_FULL:                    
//                     // If buffer full happened, you should consider increasing your buffer size
//                     // As an example, we directly flush the rx buffer here in order to read more data.
//                     uart_flush_input(EX_UART_NUM);
//                     xQueueReset(uart0_queue);
//                     break;
//                 //Event of UART RX break detected
//                 case UART_BREAK:
//                     break;
//                 //Event of UART parity check error
//                 case UART_PARITY_ERR:
//                     break;
//                 //Event of UART frame error
//                 case UART_FRAME_ERR:
//                     break;
//                 //UART_PATTERN_DET
//                 case UART_PATTERN_DET:
//                     uart_get_buffered_data_len(EX_UART_NUM, &buffered_size);
//                     int pos = uart_pattern_pop_pos(EX_UART_NUM);
//                     if (pos == -1) {
//                         // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
//                         // record the position. We should set a larger queue size.
//                         // As an example, we directly flush the rx buffer here.
//                         uart_flush_input(EX_UART_NUM);
//                     } else {
//                         uart_read_bytes(EX_UART_NUM, dtmp, pos, 100 / portTICK_PERIOD_MS);
//                         uint8_t pat[PATTERN_CHR_NUM + 1] = {0};                       
//                         uart_read_bytes(EX_UART_NUM, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                                              
//                     }
//                     break;
//                 //Others
//                 default:
                    
//                     break;
//             }
//         }
//     }
//     free(dtmp);
//     dtmp = NULL;
//     vTaskDelete(NULL);
// }

void uart_task1(void *pvParameters)
{
       uart_event_t event;
    size_t buffered_size;
    uint8_t* dtmp = (uint8_t*) malloc(1024);
    for(;;) {
        //Waiting for UART event.
        if(xQueueReceive(uart1_queue, (void * )&event, (TickType_t)portMAX_DELAY)) {
            bzero(dtmp, 1024);
            switch(event.type) {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_DATA:
                    
                    uart_read_bytes(UART_NUM_1, dtmp, event.size, portMAX_DELAY);
                    dataRx.length = (uint16_t)(dtmp[7] << 8) | dtmp[8];
                    dataRx.confirmCode = dtmp[9];
                    // printf("uartrx \n");

                    if(dataRx.length > 3)
                    {
                        for(uint8_t count = 0; count < dataRx.length - 3; count ++)
                        {
                            dataRx.data.rawdata[count] = dtmp[10 + count];                                                
                        }
                    }

                    
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(UART_NUM_1);
                    xQueueReset(uart1_queue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:                    
                    // If buffer full happened, you should consider increasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(UART_NUM_1);
                    xQueueReset(uart1_queue);
                    break;
                //Event of UART RX break detected
                case UART_BREAK:
                    break;
                //Event of UART parity check error
                case UART_PARITY_ERR:
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                    break;
                //UART_PATTERN_DET
                case UART_PATTERN_DET:
                    uart_get_buffered_data_len(UART_NUM_1, &buffered_size);
                    int pos = uart_pattern_pop_pos(UART_NUM_1);
                    if (pos == -1) {
                        // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                        // record the position. We should set a larger queue size.
                        // As an example, we directly flush the rx buffer here.
                        uart_flush_input(UART_NUM_1);
                    } else {
                        uart_read_bytes(UART_NUM_1, dtmp, pos, 100 / portTICK_PERIOD_MS);
                        uint8_t pat[PATTERN_CHR_NUM + 1] = {0};                       
                        uart_read_bytes(UART_NUM_1, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                                              
                    }
                    break;
                //Others
                default:
                    
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

void uart_transmit(uint8_t uartport, uint8_t *data, uint16_t length)
{
    uart_write_bytes(uartport, (uint8_t*) data, length);
}