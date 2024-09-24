#include"sim.h"
#include"string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include"esp_log.h"
// #include"freertos/ringbuf.h"
// #include "driver/uart.h"

#define UART_NUM UART_NUM_1
#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)
#define BUF_SIZE (1024)

static const char *TAG = "SIM_CALL";

char respond[200];
uint8_t length = 0;
void sim_sendcomamd(char *command);
extern void uart_transmit( uint8_t *data, uint16_t length);
extern int uart_read(uint8_t *data, uint16_t length);

void sim_init(void)
{
    sim_sendcomamd("AT\r\n");
    sim_waitRespon("\r\nOK");

    sim_sendcomamd("AT+CPIN?");
    sim_waitRespon("\r\nOK");

    sim_sendcomamd("AT+CFUN=1");
    sim_waitRespon("\r\nOK");
    
}

void refeshBuff(void)
{
    for(uint8_t count = 0; count < 100; count ++)
    {
        respond[count] = 0;
    }
}

void sim_call(char *sdt)
{
    static uint8_t status = 0;
    uint8_t count = 0;
    uint8_t timeout = 0;
    char str[50];
    char *head = "ATD";
    char *tail = ";\r\n";
    strcpy(str, head);
    strcat(str, sdt);
    strcat(str, tail);
    if(status == 0)
    {
        status = 1;
        printf("cmd : %s \n", str);
        sim_sendcomamd(str);
        while(1)
        {
            if(sim_waitRespon("\r\nOK") == 1 ||  sim_waitRespon("+CIEV: \"CALL\",1") == 1)
            {
                printf("calling \n");
                vTaskDelay(500);
                printf("end call \n");
                break;
            }
            if(++ count == 10)
            {
                count = 0;
                printf("cmd : %s \n", str);
                sim_sendcomamd(str);
                if(++ timeout == 5)
                {
                    break;
                }
            }
            vTaskDelay(2);
        }
        
        status = 0;
    }
}

void sim_sendMessage(char *sdt, char *message)
{
    char command[50];
    // Đặt chế độ SMS văn bản
    sim_sendcomamd("AT+CMGF=1\r\n");
    // HAL_Delay(1000);
    sim_waitRespon("\r\nOK");

    // sim_sendcomamd("AT+CSCS=\"GSM\"");
    // sim_waitRespon("\r\nOK");
    // Thiết lập số điện thoại nhận tin nhắn
    snprintf(command, sizeof(command), "AT+CMGS=\"%s\"\r\n", sdt);
    sim_sendcomamd(command);
    // HAL_Delay(1000);
    sim_waitRespon("\r\n<");

    // Gửi nội dung tin nhắn
    sim_sendcomamd(message);
    // HAL_Delay(1000);

    // Gửi tổ hợp phím Ctrl+Z để kết thúc tin nhắn
    char ctrlz = 26;
    uart_transmit((uint8_t *)&ctrlz, 1);
    sim_waitRespon("OK");
}

void sim_sendcomamd(char *command)
{
    uart_transmit((uint8_t*) command, strlen(command));
}

uint8_t sim_readrespond(void)
{
    // printf("%s \n", respond);
    // while(length == 0)
    // {
    //     vTaskDelay(1);
    // }
    if(strncmp(respond, "OK", strlen("OK")) == 0)
    {
        // printf("ok \n");
        length = 0;
        return 1;
    }
    else
    {
        length = 0;
        return 0;
    }
}

uint8_t sim_waitRespon(char *str)
{
    uint8_t count = 0;
    uint8_t len = 0;
    while(1)
    {
        len = strlen(respond);

        if(strncmp(respond, str, strlen(str)) == 0)
        {
            printf("return ok \n");
            refeshBuff();
            return 1;
        }
        else if(strncmp(respond, "ERROR", strlen("ERROR")) == 0)
        {
            refeshBuff();
            return 0;
        }
        else if( ++ count == 50)
        {
            printf("time out \n");
            refeshBuff();
            return 0;
        }
        if(len > 0)
        {
            printf("respond = %s, %d \n", respond, len);
            // printf("str = %s, %d \n", str, strlen(str));
            // int cpm = strncmp(respond, str, strlen(str));
            // printf(" compare %d \n", cpm);
            refeshBuff();
        }
        vTaskDelay(1);
    }
}


void send_command(const char *cmd) {
    uart_transmit( cmd, strlen(cmd));
    uart_transmit( (uint8_t *)"\r\n", 2);  // Thêm ký tự CRLF để kết thúc lệnh
}

void read_response(char *response, int timeout) {
    bzero(response, BUF_SIZE);
    while(length == 0){vTaskDelay(1);}
    if (length > 0) {
        response[length] = '\0';
        ESP_LOGI(TAG, "Response: %s", response);
    }
}

void check_network_status() {
    char response[BUF_SIZE];

    // Check SIM status
    send_command("AT+CPIN?");
    read_response(response, 1000);

    // Set phone functionality to full
    send_command("AT+CFUN=1");
    read_response(response, 1000);

    // Set to automatic network selection
    send_command("AT+COPS=0");
    read_response(response, 1000);

    // Check network registration status
    send_command("AT+CREG?");
    read_response(response, 1000);

    // Check signal quality
    send_command("AT+CSQ");
    read_response(response, 1000);
}

void make_call(const char *phone_number) {
    char cmd[64];
    char response[BUF_SIZE];

    // Check if the module is ready
    send_command("AT");
    read_response(response, 1000);

    // Set phone functionality to full
    send_command("AT+CFUN=1");
    read_response(response, 1000);

    // Check network registration status
    send_command("AT+CREG?");
    read_response(response, 1000);

    // Check signal quality
    send_command("AT+CSQ");
    read_response(response, 1000);

    // Set text mode
    send_command("AT+CMGF=1");
    read_response(response, 1000);

    // Dial the number
    snprintf(cmd, sizeof(cmd), "ATD%s;", phone_number);
    send_command(cmd);
    read_response(response, 1000);

    if (strstr(response, "ERROR") != NULL) {
        ESP_LOGE(TAG, "Failed to make call, received ERROR response.");
    } else {
        ESP_LOGI(TAG, "Call initiated successfully.");
    }
}
