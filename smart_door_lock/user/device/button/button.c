#include"button.h"
// #include"driver/gpio.h"
#include"esp_log.h"
#include"stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


const gpio_num_t GPIO_Pin_row[4] = {GPIO_NUM_13, GPIO_NUM_4, GPIO_NUM_14, GPIO_NUM_27};
const gpio_num_t GPIO_pin_col[4] = {GPIO_NUM_26, GPIO_NUM_25, GPIO_NUM_18, GPIO_NUM_19}; 
ButtonEventCallBack Button_pressEvent[4][4] = {NULL};
ButtonEventCallBack Button_longPressEvent[4][4] = {NULL};


void Button_init_input(void);
void Button_init_ouput(void);

void Button_init_IO(void)
{
    Button_init_input();
    Button_init_ouput();
}

void Button_init_input(void)
{
    for(uint8_t count_IO =0; count_IO < 4; count_IO ++)
    {
        gpio_config_t Input1 = 
        {
            .pin_bit_mask = (uint64_t)(1 << GPIO_Pin_row[count_IO]),
            .mode           = GPIO_MODE_INPUT,
            .pull_down_en   = GPIO_PULLDOWN_DISABLE,
            .pull_up_en     = GPIO_PULLUP_ENABLE,
            .intr_type      = GPIO_INTR_DISABLE
        };
    gpio_config(&Input1);
    }
}

void Button_init_ouput(void)
{
    for (uint8_t count_io = 0; count_io < 4; count_io++)
    {
        /* code */
        gpio_config_t output1 = 
        {
            .pin_bit_mask = (uint64_t)(1 << GPIO_pin_col[count_io]),
            .mode           = GPIO_MODE_OUTPUT,
            .pull_down_en   = GPIO_PULLDOWN_ENABLE,
            .pull_up_en     = GPIO_PULLUP_DISABLE,
            .intr_type      = GPIO_INTR_DISABLE
        };
        gpio_config(&output1);
    }
    
}


void Button_task(void *pvParameters)
{
    uint8_t u8Index = 0;
    uint16_t   Keyboard_arrSingleCnt[4] = {0};
    for( ;; )
    {
        for (u8Index = 0; u8Index < 4; u8Index++)
        {
            if(gpio_input_get_level(GPIO_Pin_row[u8Index]) == 0)
            {
                if(++ Keyboard_arrSingleCnt[u8Index] == 3)
                {
                    // printf("gpio: %d row\n", u8Index);
                    for(uint8_t countPin = 0; countPin < 4; countPin ++)
                    {
                        gpio_outPut_set_level(GPIO_pin_col[countPin], 1);
                        if(gpio_input_get_level(GPIO_Pin_row[u8Index]) == 1)
                        {
                            // printf("gpio: %d col\n", countPin);
                            Button_pressEvent[u8Index][countPin]();
                        }
                        gpio_outPut_set_level(GPIO_pin_col[countPin], 0);
                    }
                }
                else if( Keyboard_arrSingleCnt[u8Index] == 50)
                {
                    for(uint8_t countPin = 0; countPin < 4; countPin ++)
                    {
                        gpio_outPut_set_level(GPIO_pin_col[countPin], 1);
                        if(gpio_input_get_level(GPIO_Pin_row[u8Index]) == 1)
                        {
                            // printf("gpio: %d col\n", countPin);
                            Button_longPressEvent[u8Index][countPin]();
                        }
                        gpio_outPut_set_level(GPIO_pin_col[countPin], 0);
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



int gpio_input_get_level(gpio_num_t gpio_pin)
{
    return gpio_get_level(gpio_pin);
}

void gpio_outPut_set_level(gpio_num_t gpio_pin, uint32_t level)
{
    gpio_set_level(gpio_pin, level);
}

void Button_longPress_add_callBack(ButtonEventCallBack *callback)
{
    uint8_t countfunc = 0;
    for(uint8_t countcol = 0; countcol < 4; countcol ++)
    {
        for(uint8_t countrow = 0; countrow < 4; countrow ++)
        {
            Button_longPressEvent[countcol][countrow] = callback[countfunc];
            countfunc ++;
        }
    }
}

void Button_handle_Add_callback(ButtonEventCallBack *Callback)
{
    uint8_t countfunc = 0;
    for(uint8_t countcol = 0; countcol < 4; countcol ++)
    {
        for(uint8_t countrow = 0; countrow < 4; countrow ++)
        {
            Button_pressEvent[countcol][countrow] = Callback[countfunc];
            countfunc ++;
        }
    }
}

void Button_Reset_callBack(void)
{
    for(uint8_t countcol = 0; countcol < 4; countcol ++)
    {
        for(uint8_t countrow = 0; countrow < 4; countrow ++)
        {
            Button_pressEvent[countcol][countrow] = NULL;
        
        }
    }
     printf("reset cb\n");
}

