#ifndef __PASSWORD_H__
#define __PASSWORD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include"stdio.h"

#define ENTERPASSWORD_EVENT         (1<<0)
#define CHANGEPASSWORD_EVENT        (1<<1)
#define ENTER_CUR_PASS          0x00
#define ENTER_NEW_PASS          0x01
#define REWRITE_NEW_PASS        0x02
#define FINISH_CHANGE_PASS      0x03
#define IDLE_PASS               0x05
#define FINISH_ENTER_PASS       0x01

#define FINISH_CONF_PASS        0x04
#define ENTER_CONF_PASS         0x03

#define PASS_TRUE       0x01
#define PASS_FALSE      0x00

#define UNLOCK_BYPASS   0x01
#define UNLOCK_BYFINGER 0x02
#define CHANGE_PASS     0x04
#define REG_FINGER      0x08

typedef struct
{
    /* data */
    uint8_t index;
    uint8_t data[30];
    
}Password_t;

extern Password_t Password[4];
extern uint8_t Password_ST;

extern uint8_t Session_pass;

void PasswordMNG_task(void *pvParameters);

typedef void (*PassWordFunct_t)(void);

void Setup_password(void);
void enter_password(void);
void change_password(void);
void WriteDataPass(uint8_t data);
uint8_t CheckPass(Password_t confPass, Password_t enterPass);
void Process_store(const char * namespace, char * key, uint8_t *data, uint8_t length);

 #ifdef __cplusplus
}
#endif
#endif