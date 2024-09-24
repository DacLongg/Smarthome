#include <stdio.h>
#include"mainApp.h"
#include "driver/uart.h"
QueueHandle_t uart1_queue;
void app_main(void)
{
            uart_config_t uart1_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    //Install UART driver, and get the queue.
    uart_driver_install(UART_NUM_1, 1024, 1024, 20, &uart1_queue, 0);
    uart_param_config(UART_NUM_1, &uart1_config);

    //Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(UART_NUM_1, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    while(1)
    {
        mainApp();
    }

}
