#include"password.h"
// #include"driver/gpio.h"
#include"esp_log.h"
#include"stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include"event_groups.h"
#include "nvs_flash.h"
#include "nvs.h"
#include"lcd.h"
#include"finger.h"
#include"button.h"
#include"string.h"

void enter_id(uint8_t numb);
void BT_0(void);
void BT_1(void);
void BT_2(void);
void BT_3(void);
void BT_4(void);
void BT_5(void);
void BT_6(void);
void BT_7(void);
void BT_8(void);
void BT_9(void);
void BT_A(void);
void BT_B(void);
void BT_C(void);
void BT_D(void);
void BT_S(void);
void BT_T(void);

ButtonEventCallBack Button_pressEvent_cb[16] = {BT_1/*c0-r1*/, BT_2/*c0-r2*/, BT_3/*c0-r0*/, BT_A/*c0-r3*/,
                                                BT_4/*c1-r1*/, BT_5/*c1-r2*/, BT_6/*c1-r0*/, BT_B/*c1-r3*/, 
                                                BT_7/*c2-r1*/, BT_8/*c2-r2*/, BT_9/*c2-r0*/, BT_C/*c2-r3*/,
                                                BT_S/*c3-r1*/, BT_0/*c3-r2*/, BT_T/*c3-r0*/, BT_D/*c3-r3*/};

void LongBT_0(void);
void LongBT_1(void);
void LongBT_2(void);
void LongBT_3(void);
void LongBT_4(void);
void LongBT_5(void);
void LongBT_6(void);
void LongBT_7(void);
void LongBT_8(void);
void LongBT_9(void);
void LongBT_A(void);
void LongBT_B(void);
void LongBT_C(void);
void LongBT_D(void);
void LongBT_S(void);
void LongBT_T(void);

ButtonEventCallBack Button_LongpressEvent_cb[16] = {LongBT_1/*c0-r1*/, LongBT_2/*c0-r2*/, LongBT_3/*c0-r0*/, LongBT_A/*c0-r3*/,
                                                    LongBT_4/*c1-r1*/, LongBT_5/*c1-r2*/, LongBT_6/*c1-r0*/, LongBT_B/*c1-r3*/, 
                                                    LongBT_7/*c2-r1*/, LongBT_8/*c2-r2*/, LongBT_9/*c2-r0*/, LongBT_C/*c2-r3*/,
                                                    LongBT_S/*c3-r1*/, LongBT_0/*c3-r2*/, LongBT_T/*c3-r0*/, LongBT_D/*c3-r3*/};


Password_t Password[4];

uint8_t *PwData_Subcrib;
uint8_t Password_ST = IDLE_PASS;

uint8_t Session_pass = 0;
uint8_t ENB_regFinger = 0;
uint8_t state = 0;
extern uint8_t ENB_Dislay;

PassWordFunct_t     PasswordMng[3] = {
    enter_password,
    change_password,
    Setup_password
};
extern EventGroupHandle_t Button_EventGroup;
extern EventGroupHandle_t Mqtt_event_group;
extern EventGroupHandle_t LCD_EventGroup;
extern EventGroupHandle_t RegFinger_EventGroup;

nvs_handle_t my_handle;

void Process_store(const char * namespace, char * key, uint8_t *data, uint8_t length)
{

    esp_err_t err = nvs_open(namespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        printf("Done\n");
        nvs_set_blob(my_handle, key, data, length);
        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Close
        nvs_close(my_handle);
    }
}

void PasswordMNG_task(void *pvParameters)
{
    Button_handle_Add_callback(Button_pressEvent_cb);
    Button_longPress_add_callBack(Button_LongpressEvent_cb);
    
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        printf("Done\n");

        // Read
        printf("Reading restart counter from NVS ... ");
        Password[ENTER_CONF_PASS].index = 50;

        err = nvs_get_blob(my_handle, "Password", Password[ENTER_CONF_PASS].data, (size_t *)&Password[ENTER_CONF_PASS].index);
        switch (err) {
            case ESP_OK:
                printf("Done\n");
                printf("Restart counter = %d \n", Password[ENTER_CONF_PASS].index);
                printf("d1 = %d \n", Password[ENTER_CONF_PASS].data[0]);
                printf("d2 = %d \n", Password[ENTER_CONF_PASS].data[1]);
                printf("d3 = %d \n", Password[ENTER_CONF_PASS].data[2]);
                printf("d4 = %d \n", Password[ENTER_CONF_PASS].data[3]);
                printf("d5 = %d \n", Password[ENTER_CONF_PASS].data[4]);
                printf("d6 = %d \n", Password[ENTER_CONF_PASS].data[5]);

                // LCD_Gotoxy(2,2);
                // LCD_STR_Number(Password[ENTER_CONF_PASS].data, Password[ENTER_CONF_PASS].index);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                Password[ENTER_CONF_PASS].index = 6;
    
                Password[ENTER_CONF_PASS].data[0] = 1;
                Password[ENTER_CONF_PASS].data[1] = 2;
                Password[ENTER_CONF_PASS].data[2] = 3;
                Password[ENTER_CONF_PASS].data[3] = 4;
                Password[ENTER_CONF_PASS].data[4] = 5;
                Password[ENTER_CONF_PASS].data[5] = 6;
                nvs_set_blob(my_handle, "Password", Password[ENTER_CONF_PASS].data, Password[ENTER_CONF_PASS].index);
                break;
            default :
                printf("Error (%s) reading!\n", esp_err_to_name(err));
        }
        printf("%d",Password[ENTER_CONF_PASS].index);
        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Close
        nvs_close(my_handle);
    }

    PwData_Subcrib = (uint8_t *)&Password[ENTER_CONF_PASS].data[0];


    EventBits_t waitButton;
    for( ;;)
    {
        waitButton  =  xEventGroupWaitBits ( Button_EventGroup, 
                        ENTERPASSWORD_EVENT | CHANGEPASSWORD_EVENT,
                        pdTRUE, 
                        pdFALSE, 
                        portMAX_DELAY );

        if(Password[ENTER_CONF_PASS].index != 0)
        {
            if((waitButton & CHANGEPASSWORD_EVENT) != 0)
            {
                
                PasswordMng[1]();
                printf("Event change pass \n");
            }
            else if((waitButton & ENTERPASSWORD_EVENT) != 0)
            {
                // Enter_password_screen();
                // LCD_Gotoxy(8 - (Password[ENTER_CONF_PASS].index / 2),2);
                PasswordMng[0]();
                
            }
        }
        else
        {
            SetUp_password_screen();
            LCD_Gotoxy(1,2);
            Password_ST = 3;
            PasswordMng[2]();
        }
    }

}

void Setup_password(void)
{
    printf("Event Set up pass \n");
    for(;;)
    {
        if(Password_ST == FINISH_CONF_PASS)
        {
            Password_ST = IDLE_PASS;
            break;
        }
        vTaskDelay(10);
    }
}

void enter_password(void)
{
    uint16_t count = 0;
    Password_ST = ENTER_CUR_PASS;
    printf("Event enter pass \n");
    Display_Title("Enter Password");
    ENB_Dislay = 0;
    LCD_Gotoxy(8 - (Password[ENTER_CONF_PASS].index / 2),2);
    for( ;; )
    {

        if(Password_ST == FINISH_ENTER_PASS)
        {
            if(CheckPass(Password[ENTER_CONF_PASS], Password[ENTER_CUR_PASS]) == PASS_TRUE)
            {
                printf(" unlock success ! \n");
                Display_Title("unlock success");
                gpio_set_level(GPIO_NUM_23, 0);
                xEventGroupSetBits( Mqtt_event_group, UNLOCK_BYPASS);
                if(ENB_regFinger == 0)
                {
                    xEventGroupSetBits( LCD_EventGroup, END_PROCESS);
                    ENB_Dislay = 1;
                }
                else if(ENB_regFinger == 1)
                {
                    FP_FuncRun = 1;
                    ENB_regFinger = 0;
                }
                if(state == 1)
                {
                    state = 2;
                }
            }
            else
            {
                Display_Title("Pass incorrect");
                if(ENB_regFinger == 0)
                {
                    xEventGroupSetBits( LCD_EventGroup, END_PROCESS);
                    ENB_Dislay = 1;
                }
                else if(ENB_regFinger == 1)
                {
                    ENB_regFinger = 0;
                }
                printf(" Pass incorrect ! \n");
            }
            Password[ENTER_CUR_PASS].index = 0;
            Password_ST = IDLE_PASS;
            break;
        }
        if(++ count == 500)
        {
            Password[ENTER_CUR_PASS].index = 0;
            Password_ST = IDLE_PASS;
            xEventGroupSetBits( LCD_EventGroup, END_PROCESS);
            ENB_Dislay = 1;
            break;
        }
        if(Session_pass != 0)
        {

            break;
        }
        vTaskDelay(10);
    }
}

void change_password(void)
{
    uint16_t count = 0;
    Password_ST = ENTER_CUR_PASS;
    printf("Event change pass \n");
    printf("Event Current pass \n");
    Display_Title("Change Pass");
    vTaskDelay(200);
    Display_Title("Enter CurentPass");
    uint8_t status = 0;
    ENB_Dislay = 0;

    for( ;; )
    {
        if(Password_ST == ENTER_NEW_PASS && status == 0)
        {
            if(CheckPass(Password[ENTER_CONF_PASS], Password[ENTER_CUR_PASS]) == PASS_TRUE)
            {
                count = 0;
                status = 1;
                printf(" pass correct ! \n");
                Display_Title("pass correct");
                vTaskDelay(200);
                printf(" continue enter new pass ! \n");
                Display_Title("Enter NewtPass");

            }
            else
            {
                printf(" pass incorrect ! \n");
                Display_Title("pass incorrect");
                xEventGroupSetBits( LCD_EventGroup, END_PROCESS);
                Password_ST = IDLE_PASS;
                Password[ENTER_CUR_PASS].index = 0;
                break;
            }          

        }
        else if(Password_ST == REWRITE_NEW_PASS && status == 1)
        {
            status = 2;
            count = 0;
            printf(" continue enter new pass again ! \n");
            Display_Title("NewPass again");
        }
        else if(Password_ST == FINISH_CHANGE_PASS && status == 2)
        {
            if(CheckPass(Password[ENTER_NEW_PASS], Password[REWRITE_NEW_PASS]) == PASS_TRUE)
            {
                Password[ENTER_CONF_PASS].index = Password[ENTER_NEW_PASS].index;
                for(uint8_t Index = 0; Index < Password[ENTER_NEW_PASS].index; Index ++)
                {
                    Password[ENTER_CONF_PASS].data[Index] = Password[ENTER_NEW_PASS].data[Index];
                }
                
                printf(" change pass success ! \n");
                Display_Title("Change Success");
                xEventGroupSetBits( Mqtt_event_group, CHANGE_PASS);
                xEventGroupSetBits( LCD_EventGroup, END_PROCESS);
                Process_store("storage", "Password", Password[ENTER_CONF_PASS].data, Password[ENTER_CONF_PASS].index);
            }
            else
            {
                printf(" enter new pass again not match ! \n");
                Display_Title("Not match");
                xEventGroupSetBits( LCD_EventGroup, END_PROCESS);
            }
            ENB_Dislay = 1;
            Password[ENTER_CUR_PASS].index = 0;
            Password[ENTER_NEW_PASS].index = 0;
            Password[REWRITE_NEW_PASS].index = 0;
            Password_ST = IDLE_PASS;
            break;
        }
        if(++ count >= 300)
        {
            Password[ENTER_CUR_PASS].index = 0;
            Password[ENTER_NEW_PASS].index = 0;
            Password[REWRITE_NEW_PASS].index = 0;
            Password_ST = IDLE_PASS;
            xEventGroupSetBits( LCD_EventGroup, END_PROCESS);
            ENB_Dislay = 1;
            break;
        }
        vTaskDelay(10);
    }
    
}

void BT_0(void)
{
    // 0 - 0
    enter_id(0);
    WriteDataPass( 0x00);
    printf("buton 0\n");
}
void BT_1(void){
    WriteDataPass( 0x01);
    printf("buton 1\n");
    enter_id(1);
}
void BT_2(void){
    WriteDataPass( 0x02);
    printf("buton 2\n");
    enter_id(2);
}
void BT_3(void){
    WriteDataPass( 0x03);
    printf("buton 3\n");
    enter_id(3);
}
void BT_4(void){
    WriteDataPass( 0x04); 
    printf("buton 4\n");
    enter_id(4);
}
void BT_5(void)
{
    WriteDataPass( 0x05);
    printf("buton 5\n");
    enter_id(5);
}
void BT_6(void)
{
    WriteDataPass( 0x06);
    printf("buton 6\n");
    enter_id(6);
}
void BT_7(void)
{
    WriteDataPass( 0x07); 
    printf("buton 7\n");
    enter_id(7);
}
void BT_8(void)
{
    WriteDataPass( 0x08);
    printf("buton 8\n");
    enter_id(8);
}
void BT_9(void)
{
    WriteDataPass( 0x09);   
    printf("buton 9\n");
    enter_id(9);
}
void BT_A(void)
{
    WriteDataPass( 0x0A);
    printf("buton A\n");
    
}
void BT_B(void)
{
    WriteDataPass( 0x0B);
    printf("buton B\n");
    
}
void BT_C(void)
{
    WriteDataPass( 0x0C);
    printf("buton C\n");
    
}
void BT_D(void)
{
    WriteDataPass(0x0D);
    printf("buton D\n");
    
}
void BT_S(void)
{
    Session_pass = 0;
    xEventGroupSetBits( Button_EventGroup, ENTERPASSWORD_EVENT);   
        
    printf("buton *\n");
}
void BT_T(void){
    
    
    if(++ Password_ST > 5)
    {
        Password_ST = IDLE_PASS;
    }
    printf("buton #, PassStt: %d \n", Password_ST);
}

void LongBT_0(void){}
void LongBT_1(void){}
void LongBT_2(void){}
void LongBT_3(void){}
void LongBT_4(void){}
void LongBT_5(void){}
void LongBT_6(void){}
void LongBT_7(void){}
void LongBT_8(void){}
void LongBT_9(void){}
void LongBT_A(void){}
void LongBT_B(void)
{
    char str[5];
    sprintf (str, "%d", Finger.NumberID) ;
    char *str1 = " :finger";
    char desStr[15];
    strcpy(desStr, str);
    strcat(desStr, str1);
    
    Display_Title(desStr);
    xEventGroupSetBits( LCD_EventGroup, END_PROCESS);
    ENB_Dislay = 1;
}


uint8_t ID_delete;
void enter_id(uint8_t numb)
{
    if(state == 2)
    {
        ID_delete = ID_delete * 10 + numb;
    }
    
}
void LongBT_C(void)
{
    Display_Title("Delete ones");
    LCD_Gotoxy(2,2);
    I2C_LCD_Puts(" ur Finger");
    if(state == 2)
    {
        finger_DeleteID(ID_delete);
    }
    if(++ state > 1)
    {
        state = 0;
    }
     printf("buton D long %d, %d\n", state, ID_delete);
}
void LongBT_D(void)
{
    Display_Title("Delete all");
    LCD_Gotoxy(2,2);
    I2C_LCD_Puts("Put ur Finger");
    if(state == 2)
    {
        FP_FuncRun = 2;
    }
    if(++ state > 1)
    {
        state = 0;
    }
     printf("buton D long %d\n", state);
}
void LongBT_S(void)
{
    Session_pass = 1;
    xEventGroupSetBits( Button_EventGroup, CHANGEPASSWORD_EVENT);   
        
    printf("buton * long\n");
}
void LongBT_T(void)
{
    Display_Title("Register finger");
    xEventGroupSetBits( Button_EventGroup, ENTERPASSWORD_EVENT);   
    ENB_regFinger = 1;
}


void WriteDataPass(uint8_t Data)
{
    if(Password_ST < IDLE_PASS - 1)
    {
        Password[Password_ST].data[Password[Password_ST].index] = Data;
        LCD_Write_Chr('*');
        if( ++ Password[Password_ST].index >= 50)
        {
            Password[Password_ST].index = 0;
        }
    }
}

uint8_t CheckPass(Password_t confPass, Password_t enterPass)
{
    if(confPass.index != enterPass.index)
    {
        printf("confPass index: %d \n", confPass.index);
        printf("enter Pass index: %d \n", enterPass.index);
        return PASS_FALSE;
    }
    else
    {
        for (uint8_t index = 0; index < confPass.index; index++)
        {
            if(enterPass.data[index] != confPass.data[index])
            {
                printf("confPass data: %d \n", confPass.data[index]);
                printf("enter Pass data: %d \n", enterPass.data[index]);
                return PASS_FALSE;
            }
            
        }
        return PASS_TRUE;
    }
}

