#ifndef __BUTTON_H__
#define __BUTTON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include"driver/gpio.h"


typedef void (* ButtonEventCallBack)(void);

extern ButtonEventCallBack Button_pressEvent_cb[16];


void Button_task(void *pvParameters);
void Button_init_IO(void);

void gpio_outPut_set_level(gpio_num_t gpio_pin, uint32_t level);
int gpio_input_get_level(gpio_num_t gpio_pin);
void Button_handle_Add_callback(ButtonEventCallBack *Callback);
void Button_longPress_add_callBack(ButtonEventCallBack *callback);


 #ifdef __cplusplus
}
#endif
#endif