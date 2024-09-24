

#include"mainApp.h"

#define EXAMPLE_ESP_WIFI_SSID      "VietD" //"be Moon" // // 
#define EXAMPLE_ESP_WIFI_PASS      "vietzzzz2001" // "yeubemoon" //
#define EXAMPLE_ESP_MAXIMUM_RETRY  10
#define WIFI_CONNECTED_BIT (1<<0)
#define WIFI_FAIL_BIT      (1<<1)

#define WARNNING_CO     0.005
#define WARNNING_LPG    0.05
#define WARNNING_SMOKE  1


#define DENGER_CO       0.01
#define DENGER_LPG      0.1
#define DENGER_SMOKE    3

extern uint64_t denta_t;

uint64_t value;

extern volatile uint32_t * Timer0_G0_Low;
extern volatile uint32_t * Timer0_G0_High;

extern EventGroupHandle_t Mqtt_event_group;

static const char *TAG = "wifi station";
EventGroupHandle_t s_wifi_event_group;

static int s_retry_num = 0;
extern QueueHandle_t uart1_queue;
uint8_t enable_peopleSS = 0;


void event_handler(void* arg, esp_event_base_t event_base,
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

void Gpio_init(void)
{
        gpio_config_t IO_dht22 = {
        .pin_bit_mask = 1 << DHT22_PIN,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&IO_dht22);

           gpio_config_t IO_buzzer = {
        .pin_bit_mask = 1 << BUZZER_PIN,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&IO_buzzer);

    gpio_config_t Relay_pin1 = {
    .pin_bit_mask = 1 << RELAY1_PIN,
    .intr_type = GPIO_INTR_DISABLE,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&Relay_pin1);


    gpio_config_t Relay_pin2 = 
    {
        .pin_bit_mask = 1 << RELAY2_PIN,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&Relay_pin2);


    gpio_config_t Pwk = 
    {
        .pin_bit_mask = 1 << PWK_PIN,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&Pwk);

    gpio_config_t button2 = 
    {
        .pin_bit_mask = 1 << GPIO_NUM_18,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .mode = GPIO_MODE_INPUT,
    };
    gpio_config(&button2);

    gpio_config_t button3 = 
    {
        .pin_bit_mask = 1 << GPIO_NUM_15,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .mode = GPIO_MODE_INPUT,
    };
    gpio_config(&button3);

    esp_rom_gpio_pad_select_gpio(GPIO_NUM_33);
    gpio_set_direction(GPIO_NUM_33, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_33, GPIO_PULLUP_ONLY);
}

Mq2_t mq2_value;
float temperature;

void mainApp(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    wifi_init_sta();
    // mqtt_app_start();
    Gpio_init();
    I2C_LCD_Init();
    Adc_Init();
    dht22_init();





    xTaskCreate(uart_task1, "uart_task1", 2048, NULL, 10, NULL);
    xTaskCreate(Publisher_task, "Publisher_Task", 1024 * 3, NULL, 5, NULL);
    xTaskCreate(Button_task, "button_Task", 2048, NULL, 5, NULL);
    xTaskCreate(tash_handle, "handle_Task", 2048, NULL, 5, NULL);



    sim_init();
    vTaskDelay(100);
    sim_readrespond();

    // printf("init sim \n");
    // sim_sendcomamd("AT+CREG=1\r\n");
    // vTaskDelay(200);
    // sim_readrespond();

    // sim_sendcomamd("AT+CSQ\r\n");
    // vTaskDelay(200);
    // sim_readrespond();

    // sim_sendcomamd("AT+CREG?\r\n");
    // vTaskDelay(200);
    // sim_readrespond();

    // sim_sendcomamd("AT+COPS=?\r\n");
    // vTaskDelay(200);
    // sim_readrespond();

    // sim_sendcomamd("AT+CLCC");
    // vTaskDelay(200);
    // sim_readrespond();
    // sim_sendcomamd("AT+CMGF=1");
    // vTaskDelay(200);
    // sim_readrespond();


    // check_network_status();
    // make_call("0962758485");
    MQ_begin();
    

    while(1)
    {
    
        Dht22_getParan();
        MQ_read(0);
        printf("co = %.3f\n",mq2_value.param.CO);
        printf("gas = %.3f\n",mq2_value.param.LPG);
        printf("smoke = %.3f\n",mq2_value.param.Smoke);
        
        vTaskDelay(100);
        // sim_sendMessage("0962758485", "PHAT HIEN CHAY, GOI CUU HOA KHAN CAP");
  
    }
    
}

void tash_handle(void *pvParameters)
{
    for( ;; )
    {
        detect_pe0ple();
        vTaskDelay(20);
    }
}

void Publisher_task(void *pvParameters)
{
    char Co[8];
    char gas[8];
    char smoke[8];
    float Co_ppm;
    float Gas_ppm;
    // float Smoke_ppm;

    for( ;; )
    {
        fire_alarm();
        if(MQTT_CONNEECTED)
        {
            Co_ppm = mq2_value.param.CO * 10000.0;
            Gas_ppm =  mq2_value.param.LPG * 10000.0;
            // Smoke_ppm = mq2_value.param.Smoke * 10000.0;

            // uint32_t Co_ppm_i = (uint32_t)Co_ppm;
            // uint32_t Gas_ppm_i = (uint32_t)Gas_ppm;
            // uint32_t Smoke_ppm_i = (uint32_t)Smoke_ppm;
    
    
            sprintf(Co, "%ld", (uint32_t)Co_ppm);
            sprintf(gas, "%ld", (uint32_t)Gas_ppm);
            sprintf(smoke, "%.3f", mq2_value.param.Smoke);

            printf(" %s, %s, %s \n", Co, gas, smoke);

            esp_mqtt_client_publish(client, "gladys/master/device/mqtt:kitchen:device:gassensor/feature/mqtt:28/state", Co, 0, 0, 0);

            esp_mqtt_client_publish(client, "gladys/master/device/mqtt:kitchen:device:gassensor/feature/mqtt:26/state", smoke, 0, 0, 0);
            esp_mqtt_client_publish(client, "gladys/master/device/mqtt:kitchen:device:gassensor/feature/mqtt:25/state", gas, 0, 0, 0);
            
            
        }
        vTaskDelay(10);
    }
}

typedef void (* ButtonEventCallBack)(void);
const gpio_num_t BUTTON[3] = {GPIO_NUM_33, GPIO_NUM_18};
const gpio_num_t RELAY[2]  = {RELAY1_PIN, RELAY2_PIN };
bool     state[2] = {0};
char    *strState[2] = {"1", "0"};
char    *strDevice[2] = {"Fan: ", "light: "};

void TurnOn(uint8_t deviec)
{
    state[deviec] = false;
    gpio_set_level(RELAY[deviec], state[deviec]);
    if(MQTT_CONNEECTED)
    {
        printf("turn on %d", deviec);
        vTaskDelay(30);
        if(deviec == 1)
        {
            esp_mqtt_client_publish(client, "gladys/master/device/mqtt:livingroom:device:lamp/feature/mqtt:16/state", "1", 0, 0, 0);
        }
        else
        {
            esp_mqtt_client_publish(client, "gladys/master/device/mqtt:livingroom:device:airconditionor/feature/mqtt:18/state", "1", 0, 0, 0);
        }
        
    } 
}

void detect_pe0ple(void)
{
    if(MQTT_CONNEECTED)
    {
        if(gpio_get_level(GPIO_NUM_15) == 1)
        {
            esp_mqtt_client_publish(client, "gladys/master/device/mqtt:livingroom:device:motion/feature/mqtt:21/state", "1", 0, 0,0);
        }
        else
        {
            esp_mqtt_client_publish(client, "gladys/master/device/mqtt:livingroom:device:motion/feature/mqtt:21/state", "0", 0, 0,0);
        }
    }
    if(enable_peopleSS == 1)
    {
        printf(" enanble people ss \n");
        if(gpio_get_level(GPIO_NUM_15) == 1)
        {
                    // printf(" detect people \n");
            gpio_set_level(RELAY[1], 0);
            if(MQTT_CONNEECTED)
            {
                printf("turn on %d", 1);       
                esp_mqtt_client_publish(client, "gladys/master/device/mqtt:livingroom:device:lamp/feature/mqtt:16/state", "1", 0, 0, 0);          
            } 
            // TurnOn(1);
        }
        else
        {
            // TurnOff(1);
            printf(" no detect people \n");
            gpio_set_level(RELAY[1], 1);
            if(MQTT_CONNEECTED)
            {
                printf("turn on %d", 1);       
                esp_mqtt_client_publish(client, "gladys/master/device/mqtt:livingroom:device:lamp/feature/mqtt:16/state", "0", 0, 0, 0);          
            } 
        }
    }
}

void TurnOff(uint8_t deviec)
{
    state[deviec] = true;
    gpio_set_level(RELAY[deviec], state[deviec]);
    if(MQTT_CONNEECTED)
    {
        // printf("turn on %d", deviec);
        if(deviec == 1)
        {
            esp_mqtt_client_publish(client, "gladys/master/device/mqtt:livingroom:device:lamp/feature/mqtt:16/state", "0", 0, 0, 0);
        }
        else
        {
            esp_mqtt_client_publish(client, "gladys/master/device/mqtt:livingroom:device:airconditionor/feature/mqtt:18/state", "0", 0, 0, 0);
        }
        
    } 
}

void Button_task(void *pvParameters)
{
    uint8_t     u8Index = 0;
    uint16_t   Keyboard_arrSingleCnt[2] = {0};
    for( ;; )
    {
        for (u8Index = 0; u8Index < 2; u8Index++)
        {
            if(gpio_get_level(BUTTON[u8Index]) == 0)
            {
                if(++ Keyboard_arrSingleCnt[u8Index] == 3)
                {
                    state[u8Index] = !state[u8Index];
                    gpio_set_level(RELAY[u8Index], state[u8Index]);
                        char str_buff[25];
                        strcpy(str_buff, strState[state[u8Index]]);
                        // strcat(str_buff, strState[state[u8Index]]);
                        printf("%s \n", str_buff);
                    
                    if(state[1] == 1)
                    {
                        printf(" bat sensor ss \n");
                        vTaskDelay(30);
                        enable_peopleSS = 1;
                    }
                    else
                    {
                        enable_peopleSS = 0;
                    }

                    if(MQTT_CONNEECTED)
                    {
                        if(u8Index == 1)
                        {
                            esp_mqtt_client_publish(client, "gladys/master/device/mqtt:livingroom:device:lamp/feature/mqtt:16/state", strState[state[u8Index]], 0, 0, 0);
                        }
                        else
                        {
                            esp_mqtt_client_publish(client, "gladys/master/device/mqtt:livingroom:device:airconditionor/feature/mqtt:18/state", strState[state[u8Index]], 0, 0, 0);
                        }
                        
                    }                 
                }
            }
            else
            {
                Keyboard_arrSingleCnt[u8Index] = 0;

            }
        }
        vTaskDelay(5);
    }
}

void fire_alarm(void)
{
    int status = 0;
    int data_flame = 10;
    static uint8_t count_time = 0;
    read_flame(&data_flame);


    if(data_flame < 1000)
    {
        if(MQTT_CONNEECTED)
        {
            esp_mqtt_client_publish(client, "gladys/master/device/mqtt:kitchen:device:firesensor/feature/mqtt:27/state", "1", 0, 0, 0);
        }
        gpio_set_level(BUZZER_PIN, 1);
        TurnOff(0);
        TurnOff(1);

        printf("call \n");
        sim_call("0962758485");
        vTaskDelay(500);
        sim_sendMessage("0962758485", "PHONG BEP: PHAT HIEN CHAY, GOI CUU HOA KHAN CAP");
        printf("phat hien lua \n");

        vTaskDelay(50);
    }
    else
    {
        if(MQTT_CONNEECTED)
        {
            esp_mqtt_client_publish(client, "gladys/master/device/mqtt:kitchen:device:firesensor/feature/mqtt:27/state", "0", 0, 0, 0);
        }
        gpio_set_level(BUZZER_PIN, 0);
        if(mq2_value.param.Smoke > WARNNING_SMOKE && mq2_value.param.Smoke < DENGER_SMOKE)
        {
            printf("WARNING SMOKE \n");
            sim_sendMessage("0962758485", "PHONG BEP: PHAT HIEN KHOI, VUI LONG KIEM TRA LAI HE THONG NHA");
            vTaskDelay(50);
        }
        else if(mq2_value.param.Smoke > DENGER_SMOKE)
        {
            printf("DENGEROUS SMOKE \n");
            status = 1;
        }
        if(mq2_value.param.LPG > WARNNING_LPG && mq2_value.param.LPG < DENGER_LPG)
        {
            printf("WARNING GASS \n");
            sim_sendMessage("0962758485", "PHONG BEP: PHAT HIEN KHI GAS, VUI LONG KIEM TRA LAI HE THONG NHA");
            vTaskDelay(50);
        }
        else if(mq2_value.param.LPG > DENGER_LPG)
        {
            printf("DENGEROUS GASS \n");
            status = 2;
        }
        if(temperature > 50.0)
        {
            sim_sendMessage("0962758485", "PHONG BEP: NHIET DO CAO BAT THUONG, VUI LONG KIEM TRA LAI HE THONG NHA");
            vTaskDelay(50);
        }
        if(mq2_value.param.CO > WARNNING_CO && mq2_value.param.CO < DENGER_CO)
        {
            sim_sendMessage("0962758485", "PHONG BEP: PHAT HIEN KHI CO, VUI LONG KIEM TRA LAI HE THONG NHA");
        }
        else if(mq2_value.param.CO > DENGER_CO)
        {
            printf("denger CO \n");
            status = 3;
        }
        
        if(status != 0)
        {
            gpio_set_level(BUZZER_PIN, 1);
            TurnOff(0);
            TurnOff(1);
            printf("call \n");
            sim_call("0962758485");
            vTaskDelay(500);
            if(status == 1)
            {
                sim_sendMessage("0962758485", "PHONG BEP: PHAT HIEN KHOI O MUC NGUY HIEM, VUI LONG KIEM TRA LAI HE THONG");
                vTaskDelay(50);
            }
            if(status == 2)
            {
                sim_sendMessage("0962758485", "PHONG BEP: PHAT HIEN KHI GAS O MUC NGUY HIEM, VUI LONG KIEM TRA LAI HE THONG");
                vTaskDelay(50);
            }
            if(status == 3)
            {
                sim_sendMessage("0962758485", "PHONG BEP: PHAT HIEN KHI CO O MUC NGUY HIEM, VUI LONG KIEM TRA LAI HE THONG");
                vTaskDelay(50);
            }
            
        }
        else
        {
            gpio_set_level(BUZZER_PIN, 0);
        }
    }

    // }
    // else
    // {
    //     if(data_flame < 1000)
    //     {
           
    //         gpio_set_level(BUZZER_PIN, 1);
    //         TurnOff(0);
    //         TurnOff(1);
    //         printf("call \n");
    //         sim_call("0962758485");
    //         vTaskDelay(500);
    //         sim_sendMessage("0962758485", "PHAT HIEN CHAY, GOI CUU HOA KHAN CAP");
    //         printf("phat hien lua \n");
    //         vTaskDelay(50);
    //     }
    //     else
    //     {
    //         gpio_set_level(BUZZER_PIN, 0);
    //         if(mq2_value.param.Smoke > WARNNING_SMOKE)
    //         {
    //             sim_sendMessage("0962758485", "PHAT HIEN KHOI, KIEM TRA LAI HE THONG NHA");
    //             vTaskDelay(50);
    //         }
    //         else if(mq2_value.param.Smoke > DENGER_SMOKE)
    //         {
    //             status = 1;
    //         }
    //         if(mq2_value.param.LPG > WARNNING_LPG)
    //         {
    //             sim_sendMessage("0962758485", "PHAT HIEN KHI GAS, KIEM TRA LAI HE THONG NHA");
    //             vTaskDelay(50);
    //         }
    //         else if(mq2_value.param.LPG > DENGER_LPG)
    //         {
    //             status = 2;
    //         }
    //         if(temperature > 50.0)
    //         {
    //             sim_sendMessage("0962758485", "NHIET DO CAO BAT THUONG, KIEM TRA LAI HE THONG NHA");
    //             vTaskDelay(50);
    //         }
            
    //         if(status != 0)
    //         {
    //             gpio_set_level(BUZZER_PIN, 1);
    //             TurnOff(0);
    //             TurnOff(1);
    //             printf("call \n");
    //             sim_call("0962758485");
    //             vTaskDelay(500);
    //             if(status == 1)
    //             {
    //                 sim_sendMessage("0962758485", "PHAT HIEN KHOI O MUC NGUY HIEM, KIEM TRA LAI HE THONG");
    //                 vTaskDelay(50);
    //             }
    //             if(status == 2)
    //             {
    //                 sim_sendMessage("0962758485", "PHAT HIEN KHI GAS O MUC NGUY HIEM, KIEM TRA LAI HE THONG");
    //                 vTaskDelay(50);
    //             }
                
    //         }
    //         else
    //         {
    //             gpio_set_level(BUZZER_PIN, 0);
    //         }
    //     }
    // }
    vTaskDelay(100);

}

void Dht22_getParan(void)
{
    DHT22_Start();

    if(1 == dht22_readData(DHT22_value.Data))
    {
        //    DHT22_value.Data[1] = dht22_readData();
        //    DHT22_value.Data[0] = dht22_readData();
        //    DHT22_value.Data[3] = dht22_readData();
        //    DHT22_value.Data[2] = dht22_readData();
        //    DHT22_value.Data[4] = dht22_readData();
        DHT22_value.param.Temp = EndianSwap(DHT22_value.param.Temp);
        DHT22_value.param.Humi = EndianSwap(DHT22_value.param.Humi);

        float temp = DHT22_value.param.Temp / 10.0;
        float humi = DHT22_value.param.Humi / 10.0;

        temperature = temp;
        // printf("DHT22_value temp = %d \n", DHT22_value.param.Temp);
        // printf("DHT22_value humi = %d \n", DHT22_value.param.Humi);

        char *str  = "Temp :";
        char Temp[8];

        char str_buff[25];
        sprintf(Temp, "%.1f°C", temp);
        strcpy(str_buff, str);
        strcat(str_buff, Temp);
        printf("%s \n", str_buff);
        I2C_LCD_BackLight(1);
        // I2C_LCD_Clear();
        LCD_Gotoxy(1, 1);
        I2C_LCD_Puts(str_buff);
        
        char *str1 = "Humi :";
        char Humi[6];
        char str_buff1[40];
        sprintf(Humi, "%.1f%%", humi);
        strcpy(str_buff1, str1);
        strcat(str_buff1, Humi);
        printf("%s \n", str_buff1);
        LCD_Gotoxy(1, 2);
        I2C_LCD_Puts(str_buff1);

        // strcat(str_buff1, str_buff);

        uint8_t Ret;

        if(MQTT_CONNEECTED)
        {
            Ret = esp_mqtt_client_publish(client, "gladys/master/device/mqtt:kitchen:device:temperature/feature/mqtt:23/state", Temp, 0, 0, 0);
            // printf("ret pub temp: %d \n", Ret);
            Ret = esp_mqtt_client_publish(client, "gladys/master/device/mqtt:kitchen:device:humidity/feature/mqtt:24/state", Humi, 0, 0, 0);
            // printf("ret pub humi: %d \n", Ret);
        }
    }
    
}


void uart_task1(void *pvParameters)
{
    printf("uart task \n");
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
                    
                    // memcpy(respond, 0, 100);
                    uart_read_bytes(UART_NUM_1, dtmp, event.size, portMAX_DELAY);
                    memcpy(respond, dtmp, event.size);
                    length = event.size;

                    printf("%s\n", respond);
                    
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

void uart_transmit( uint8_t *data, uint16_t length)
{
    uart_write_bytes(UART_NUM_1, (uint8_t*) data, length);
}

int uart_read(uint8_t *data, uint16_t length)
{
    // bzero(data, 1024);
    return uart_read_bytes(UART_NUM_1, data, length, portMAX_DELAY);
}

void buzzer(uint8_t state)
{
    gpio_set_level(BUZZER_PIN, state);
}