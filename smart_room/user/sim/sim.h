#ifndef __SIM_H__
#define __SIM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include"stdio.h"


extern uint8_t length;
extern char respond[200];

void sim_call(char *sdt);
void sim_init(void);
uint8_t sim_readrespond(void);
void sim_sendcomamd(char *command);
void sim_sendMessage(char *sdt, char *message);
// void wifi_init_sta(void);
uint8_t sim_waitRespon(char *str);
void check_network_status();
void make_call(const char *phone_number);

 #ifdef __cplusplus
}
#endif
#endif