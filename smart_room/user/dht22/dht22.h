#ifndef __DHT22_H__
#define __DHT22_H__

#ifdef __cplusplus
extern "C" {
#endif

#include"stdio.h"

#define DHT22_PIN       GPIO_NUM_14

typedef union
{
    struct
    {
        uint16_t Humi;
        uint16_t Temp;
        
        uint8_t  End;
    }param;
    uint8_t Data[5];
}Data_DHT22_t;

extern Data_DHT22_t DHT22_value;


void dht22_init(void);

void gpio_pin_set(uint8_t value);
uint64_t timer_get_cnt(void);
void Timer_init(void);
uint8_t  dht22_readData(uint8_t *value);
uint16_t EndianSwap(uint16_t x);
uint8_t DHT22_Start (void);

void delay_us(uint64_t us);

 #ifdef __cplusplus
}
#endif
#endif