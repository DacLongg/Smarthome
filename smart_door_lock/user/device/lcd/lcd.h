#ifndef __LCD_H__
#define __LCD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include"stdio.h"

#define END_PROCESS     0x01

void Enter_password_screen(void);
void SetUp_password_screen(void);
void Change_password_screen(void);
void Reg_fingerPrint_screen(void);
void Unlock_screen(void);
void SetUp_password_screen(void);

void LCD_STR_Number(uint8_t *number, uint8_t length);

void Display_Title(char *sting);

void LCD_task(void *pvParameters);
void LCD_Write_Chr(char chr);
void I2C_LCD_Puts(char *sz);
void I2C_LCD_Clear(void);
void I2C_LCD_NewLine(void);
void LCD_Gotoxy (int col, int row);

 #ifdef __cplusplus
}
#endif
#endif