#include"dht22.h"
#include"driver/gpio.h"
#include "esp_log.h"
#include"lcd.h"
#include "driver/timer.h"
#include"string.h"



void DHT22_Gpio_set_Input(void);
void DHT22_Gpio_set_output(void);
void Timer_init(void);
void reset_counter(void);

uint16_t EndianSwap(uint16_t x)
{
	return (x >> 8) | (x << 8);
}

// 4 bytes
void EndianSwap1 (uint32_t *x)
{
	*x = (*x >> 24) |
		((*x << 8) & 0x00FF0000) |
		((*x >> 8) & 0x0000FF00) |
		(*x << 24);
}


volatile const uint32_t * Timer0_G0_Low;
volatile const uint32_t *  Timer0_G0_High;
volatile uint32_t * const Timer0_Update = (uint32_t *)0x3FF5F00C;
volatile uint32_t * const Timer0_Reload_lo = (uint32_t *)0x3FF5F018;
volatile uint32_t * const Timer0_Reload_hi = (uint32_t *)0x3FF5F01C;



Data_DHT22_t DHT22_value;


void dht22_init(void)
{
    Timer0_G0_Low = (uint32_t *)0x3FF5F004;
    Timer0_G0_High = (uint32_t *)0x3FF5F008;
    Timer_init();

    // DHT22_Gpio_set_output();
}

uint8_t DHT22_Start (void)
{
    uint8_t Response = 0;
    uint32_t timeout = 0;

	DHT22_Gpio_set_output(); // set the pin as output
	gpio_pin_set(0);   // pull the pin low
	delay_us(1200);   // wait for > 1ms

	gpio_pin_set(1);   // pull the pin high
	delay_us (30);   // wait for 30us
    // gpio_set_level(GPIO_NUM_14, 0);
	DHT22_Gpio_set_Input();   // set as input
    // delay_us (40);  // wait for 40us
    // gpio_set_level(GPIO_NUM_14, 1);

    if(gpio_get_level(DHT22_PIN) == 0) // if the pin is low
	{
		delay_us (70);   // wait for 80us
        // printf("Check rp \n");
    // gpio_set_level(GPIO_NUM_14, 0);
		if(gpio_get_level(DHT22_PIN) == 1) 
        {
            Response = 1;  // if the pin is high, response is ok
        }
		else 
        {
            printf("respond error \n");
            Response = 0;
        }
	}

	while (gpio_get_level(DHT22_PIN))
    {
        if(++ timeout == 100000)
        {
            printf("respond timeout \n");
            return 0;
        }
        delay_us(1);
    }  // wait for the pin to go low
	return Response;
}

void reset_counter(void)
{
    *Timer0_Reload_lo = 0;
    *Timer0_Reload_hi = 0;
}

uint8_t  dht22_readData(uint8_t *value)
{

	uint8_t i = 0;
    uint8_t j = 0;
    uint32_t timeout = 0;
    for(uint8_t count = 0; count < 5; count ++)
    {
        for (j=0;j<8;j++)
        {
            while (!gpio_get_level(DHT22_PIN))   // wait for the pin to go high
            {
                if(++ timeout == 100000)
                {
                    printf("error1 timeout \n");
                    return 0;
                }
                // delay_us(1);
            }
            delay_us (40);   // wait for 40 us


            if (!(gpio_get_level(DHT22_PIN)))   // if the pin is low
            {
                value[count]&= ~(1<<(7-j));   // write 0
            }
            else 
            {
                value[count]|= (1<<(7-j));
            }  // if the pin is high, write 1
            while ((gpio_get_level(DHT22_PIN)))
            {
                if(++ timeout == 100000)
                {

                    printf("error2 time out  i = %d , j = %d\n",i,j);
                    return 0;
                }
                // delay_us(1);
            }  // wait for the pin to go low
        }
    }

	return 1;
    
}




uint64_t timer_get_cnt(void)
{
    // timer_get_counter_value(0,0, &timeStart);
    *Timer0_Update = 0x01;
    uint64_t cntvalue = (uint64_t)(*Timer0_G0_High) << 32 | (*Timer0_G0_Low);
    return cntvalue;

}



void Timer_init(void)
{
    timer_config_t config;
    config.divider = 80;
    config.intr_type = TIMER_INTR_NONE;
    config.counter_en = TIMER_COUNT_UP;
    config.auto_reload = TIMER_AUTORELOAD_EN;
    config.clk_src = TIMER_SRC_CLK_APB;

    timer_init(0, 0,  &config);
    // printf("Timer0_G0_Low  %ld \n", (uint32_t)*Timer0_G0_Low);

    timer_start(0,0);
    // sprintf("Timer0_G0_Low  %ld \n", (uint32_t)*Timer0_G0_Low);
}

void gpio_pin_set(uint8_t value)
{
    gpio_set_level(DHT22_PIN, value);
}


void DHT22_Gpio_set_Input(void)
{
    gpio_set_direction(DHT22_PIN, GPIO_MODE_INPUT);

}

void DHT22_Gpio_set_output(void)
{
    gpio_set_direction(DHT22_PIN, GPIO_MODE_OUTPUT);
}

void delay_us(uint64_t us)
{
    reset_counter();
    uint64_t start = timer_get_cnt();
    uint64_t run   = start;
    // printf("start = %lld \n", start);

    while((start - run) < us)
    {
        run =  timer_get_cnt();
    }


    // while(--us)
    // {
    //     for(uint8_t count = 0; count < 2; count ++)
    //     {
    //         asm("NOP");
    //     }
    // }
}


