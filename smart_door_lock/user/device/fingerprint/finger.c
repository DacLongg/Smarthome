
#include"finger.h"
#include"esp_log.h"
#include"stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include"event_groups.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "nvs.h"
#include"lcd.h"

#define UART_AS608  1
#define BUFF_OK     1
#define BUFF_NOT_OK 0
#define UNLOCK_BYPASS   0x01
#define UNLOCK_BYFINGER 0x02
#define CHANGE_PASS     0x04
#define REG_FINGER      0x08

extern void uart_transmit(uint8_t uartport, uint8_t *data, uint16_t length);
uint16_t Crc(uint8_t *data, uint16_t length);
uint8_t Check_ACK(void);
void Send_Packet(uint8_t *comd, uint16_t length);
void Send_PacketStore(uint16_t ID);
uint8_t RegModel_FingerPrint(void);
void finger_DeleteALL(void);

uint8_t Header[6]               ={0xEF,0x01,0xFF,0xFF,0xFF,0xFF};
uint8_t FPGetImage[6]           ={0x01,0x00,0x03,0x01,0x00,0x05};
uint8_t FPCreateCharFile1[5]    ={0x01,0x00,0x04,0x02,0x01};
uint8_t FPCreateCharFile2[5]    ={0x01,0x00,0x04,0x02,0x02};
uint8_t FPCreateTemplate[6]     ={0x01,0x00,0x03,0x05,0x00,0x09};
uint8_t FPDeleteAllFinger[6]    ={0x01,0x00,0x03,0x0D,0x00,0x11};
uint8_t FPSearchFinger[11]      ={0x01,0x00,0x08,0x04,0x01,0x00,0x00,0x00,0x40,0x00,0x4E};
uint8_t FPGetNumberOfFinger[6]  ={0x01,0x00,0x03,0x1D,0x00,0x21};

void EndianSwap(uint16_t *x)
{
	*x = (*x >> 8) | (*x << 8);
}

// 4 bytes
void EndianSwap1 (uint32_t *x)
{
	*x = (*x >> 24) |
		((*x << 8) & 0x00FF0000) |
		((*x >> 8) & 0x0000FF00) |
		(*x << 24);
}

Frame_t dataRx;

finger Finger;

typedef void (*FP_function_t)(void);

void Search_database(void);
void Enroll_FingerPrint(void);


FP_function_t FP_funct[3] = {Search_database, Enroll_FingerPrint, finger_DeleteALL};

uint8_t FGdata_Subcrib;
uint8_t FP_FuncRun = 0;
extern EventGroupHandle_t Mqtt_event_group;
extern EventGroupHandle_t LCD_EventGroup;
extern uint8_t ENB_Dislay;

EventGroupHandle_t RegFinger_EventGroup;
nvs_handle_t Flash_handle_Finger;

void Process_store_FingerID(const char * namespace, char * key, uint8_t *data, uint8_t length)
{

    esp_err_t err = nvs_open(namespace, NVS_READWRITE, &Flash_handle_Finger);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        printf("Done\n");
        nvs_set_blob(Flash_handle_Finger, key, data, length);
        err = nvs_commit(Flash_handle_Finger);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Close
        nvs_close(Flash_handle_Finger);
    }
}

void FingerPrint_task(void *pvParameters)
{
    uint8_t MaxLengID = 0;
    
    esp_err_t err = nvs_open("FingerPr_ID", NVS_READWRITE, &Flash_handle_Finger);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        printf("Done\n");
        // Read
        printf("Reading restart counter from NVS ... ");

        err = nvs_get_blob(Flash_handle_Finger, "Finger_ID", Finger.ID, (size_t *)&MaxLengID);
        switch (err) {
            case ESP_OK:
                printf("Done\n");
                printf("Restart counter = %d \n", MaxLengID);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                for(uint8_t count = 0; count < 128; count ++)
                {
                    Finger.ID[count] = 0;
                }
                Finger.NumberID = 0;
                break;
            default :
                printf("Error (%s) reading!\n", esp_err_to_name(err));
        }
       
        err = nvs_commit(Flash_handle_Finger);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Close
        nvs_close(Flash_handle_Finger);
    }

    RegFinger_EventGroup = xEventGroupCreate();
    for( ;; )
    {
        if(FP_FuncRun < 3)
        {
            FP_funct[FP_FuncRun]();
        }
        vTaskDelay(10);
    }
}


uint8_t checkID_empty(uint8_t *index)
{
    for(uint8_t count = 0; count < 128; count ++)
    {
        if(Finger.ID[count] == 0)
        {
            *index = count;
            return SUCCESS;
        }
    }
    return 1;
}


uint8_t Get_FingerPrint(uint8_t BuffID)
{
    Send_Packet(FPGetImage, 4);
    if( Check_ACK() != SUCCESS)
    {
        return dataRx.confirmCode;
    }
    else
    {
        FPCreateCharFile1[4] = BuffID;
        Send_Packet(FPCreateCharFile1, 5); 
        return Check_ACK();   
        // if( Check_ACK() != SUCCESS)
        // {
        //     return dataRx.confirmCode;
        // }
        // return dataRx.confirmCode;
    }
}

void Enroll_FingerPrint(void)
{

    static uint8_t count = 0;
    static uint16_t time = 0;
    LCD_Gotoxy(2,2);
    I2C_LCD_Puts("Put ur Finger");
    ENB_Dislay = 0;
    printf("count : %d\n", count);
    if(Get_FingerPrint(count + 1) == SUCCESS)
    {
        if(++ count == 2)
        {
            count = 0;
            vTaskDelay(50);
            if(RegModel_FingerPrint() == SUCCESS)
            {
                printf("store\n");
                checkID_empty(&Finger.index);
                Send_PacketStore(Finger.index);
                Finger.ID[Finger.index] = 1;
                if( ++ Finger.NumberID == 128)
                {
                    Finger.NumberID = 127;
                }
                FP_FuncRun = 0;
                printf("Success reg ID : %d \n", Finger.index);
                Display_Title("Register Success");
                Process_store_FingerID("FingerPr_ID", "Finger_ID", Finger.ID, 128);
                xEventGroupSetBits( LCD_EventGroup, END_PROCESS);
                ENB_Dislay = 1;
                
            }
            else
            {
                printf("fail \n");
                Display_Title("Register fail");
                xEventGroupSetBits( LCD_EventGroup, END_PROCESS);
                ENB_Dislay = 1;
            }
        }
        vTaskDelay(100);
        
    }
    if(++ time >= 500)
    {
        ENB_Dislay = 1;
        Display_Title("Register fail");
        xEventGroupSetBits( LCD_EventGroup, END_PROCESS);
        FP_FuncRun = 0;
        time = 0;
    }
    
}

extern uint8_t ID_delete;
extern uint8_t state;
void Search_database(void)
{

    if(Get_FingerPrint(1) == SUCCESS)
    {
        Send_Packet(FPSearchFinger, 9);
        
        if( Check_ACK() == SUCCESS)
        {
            EndianSwap(&dataRx.data.comp.PageNumb);
            EndianSwap(&dataRx.data.comp.score);
            printf("unlock: ID_ %d - score = %d \n", dataRx.data.comp.PageNumb, dataRx.data.comp.score);
            Finger.ID[dataRx.data.comp.PageNumb] = 1;
            xEventGroupSetBits( Mqtt_event_group, UNLOCK_BYFINGER);
            Display_Title("unlock success");
            gpio_set_level(GPIO_NUM_23, 0);
            xEventGroupSetBits( LCD_EventGroup, END_PROCESS);
            ENB_Dislay = 1;
            ID_delete = dataRx.data.comp.PageNumb;
            if(state == 1)
            {
                state = 2;
            }
        }
    }


}

uint8_t RegModel_FingerPrint(void)
{
    // Send_Packet(FPCreateTemplate, 4);
    // if( Check_ACK() != SUCCESS)
    // {
    Send_Packet(FPCreateTemplate, 4);
    return Check_ACK();
    // if( Check_ACK() != SUCCESS)
    // {   
    // //FP_FuncRun = 0;
    //     return dataRx.confirmCode;
    // }
    // return SUCCESS;
    // }
}



uint8_t Check_ACK(void)
{
    uint16_t timeout = 0;
    while(dataRx.length == 0)
    {    
        if(++ timeout == 100)
        {
            //  printf(" error \n");
            //FP_FuncRun = 0;
            return ERROR;
        }
        vTaskDelay(2);
    }
    dataRx.length = 0;
    //FP_FuncRun = 0;
    // printf(" confirmCode %d \n", dataRx.confirmCode);
    return dataRx.confirmCode; 
}

void finger_DeleteID(uint16_t FGdata_Subcrib)
{
    uint8_t DeleteID_Data[8] ={0x01,0x00,0x07,0x0C};
    DeleteID_Data[4] = FGdata_Subcrib >> 8;
    DeleteID_Data[5] = FGdata_Subcrib;
    DeleteID_Data[6] = 0;
    DeleteID_Data[7] = 1;
    Send_Packet(DeleteID_Data, 8);
    if(Check_ACK() == SUCCESS)
    {
        Finger.ID[FGdata_Subcrib] = 0;
        Finger.NumberID --;
        Process_store_FingerID("FingerPr_ID", "Finger_ID", Finger.ID, 128);
        printf("delete id %d \n", FGdata_Subcrib);
        Display_Title("Delete ID");
        xEventGroupSetBits( LCD_EventGroup, END_PROCESS);
        ENB_Dislay = 1;
    }
}

void finger_DeleteALL(void)
{
    Send_Packet(FPDeleteAllFinger, 4);
    printf("start delete all \n");
    if(Check_ACK() == SUCCESS)
    {
        for(uint8_t count = 0; count < 128; count ++)
        {
            Finger.ID[count] = 0;
        }
        Finger.NumberID = 0;

        Process_store_FingerID("FingerPr_ID", "Finger_ID", Finger.ID, 128);

        printf("delete all \n");
        Display_Title("Delete all");
        xEventGroupSetBits( LCD_EventGroup, END_PROCESS);
        ENB_Dislay = 1;
    }
    FP_FuncRun = 0;
}

void finger_GetNumbID(uint8_t *numbID)
{
    uint8_t GetnumberID_Data[4] ={0x01,0x00,0x03,0x1D};
    Send_Packet(GetnumberID_Data, 4);
    if(Check_ACK() == SUCCESS)
    {
        EndianSwap(&dataRx.data.comp.PageNumb);
        *numbID = dataRx.data.comp.PageNumb;
    }
}

void Send_Packet(uint8_t *comd, uint16_t length)
{
    uint8_t data[2];
    data[0] = Crc(comd, length) >> 8;
    data[1] = Crc(comd, length);
    uart_transmit(UART_AS608, Header, 6);
    uart_transmit(UART_AS608, comd, length);
    uart_transmit(UART_AS608, data, 2);
}

void Send_PacketStore(uint16_t ID)
{
    uint8_t StoreData[9] ={0x01,0x00,0x06,0x06,0x01};
    StoreData[5] = ID >> 8;
    StoreData[6] = ID;
    StoreData[7] = Crc(StoreData, 7) >> 8;
    StoreData[8] = Crc(StoreData, 7);
    uart_transmit(UART_AS608, Header, 6);
    uart_transmit(UART_AS608, StoreData, 9);
}


uint16_t Crc(uint8_t *data, uint16_t length)
{
    uint16_t CRC = 0;
    for(uint16_t count = 0; count < length; count ++)
    {
        CRC += data[count];
    }
    return CRC;
}



// Buffer* newBuffer(void)
// {
//     Buffer* _newBuff = (Buffer*)malloc(sizeof(Buffer));
//     if (_newBuff != NULL)
//     {
//         _newBuff->data = NULL;
//         _newBuff->length = 0;
//         _newBuff->link = NULL;
//     }
//     return _newBuff;
// }

// uint8_t Buffer_AddData(Buffer* buff, uint8_t* u8DataPtr, uint16_t length)
// {
//     Buffer* _buff = buff;
//     Buffer* head_buff = buff;
    
//     while (_buff != NULL)
//     {
//         if (_buff->length != 0 && _buff->data != NULL)
//         {
//             head_buff = _buff;
//             _buff = _buff->link;
//         }
//         else
//         {
//             _buff->data = (uint8_t*)malloc(length);
//             if (_buff->data == NULL)
//             {
//                 return BUFF_NOT_OK;
//             }

//             for (uint16_t countByte = 0; countByte < length; countByte++)
//             {
//                 _buff->data[countByte] = u8DataPtr[countByte];
//             }
//             _buff->length = length;
//             return BUFF_OK;
//         }
//     }

//     _buff = (Buffer*)malloc(sizeof(Buffer));
//     if (_buff == NULL)
//     {
//         return BUFF_NOT_OK;
//     }

//     _buff->data = (uint8_t*)malloc(length);
//     if (_buff->data == NULL)
//     {
//         free(_buff);
//         return BUFF_NOT_OK;
//     }

//     _buff->length = 0;
//     _buff->link = NULL;
//     if (head_buff != NULL)
//     {
//         head_buff->link = _buff;
//     }

//     for (uint16_t countByte = 0; countByte < length; countByte++)
//     {
//         _buff->data[countByte] = u8DataPtr[countByte];
//     }
//     _buff->length = length;
//     return BUFF_OK;
// }
// uint8_t Buffer_GetData(Buffer* buff, uint16_t position, uint8_t* u8DataPtr, uint16_t* length)
// {
//     Buffer* _buff = buff;
//     uint16_t Count = Buffer_CountData(buff);

//     if (position >= Count)
//     {
//         return BUFF_NOT_OK;
//     }

//     for (uint16_t countData = 0; countData < position; countData++)
//     {
//         _buff = _buff->link;
//     }

//     for (uint16_t countByte = 0; countByte < _buff->length; countByte++)
//     {
//         u8DataPtr[countByte] = _buff->data[countByte];
//     }
//     *length = _buff->length;
//     return BUFF_OK;
// }

// uint8_t Buffer_AppendData(Buffer* buff, uint16_t position, uint8_t* u8DataPtr, uint16_t length)
// {
//     Buffer* _buff = buff;
//     uint16_t Count = Buffer_CountData(buff);

//     if (position >= Count)
//     {
//         return BUFF_NOT_OK;
//     }

//     for (uint16_t countData = 0; countData < position; countData++)
//     {
//         _buff = _buff->link;
//     }

//     uint8_t* newdata = (uint8_t*)realloc(_buff->data, _buff->length + length);
//     if (newdata == NULL)
//     {
//         return BUFF_NOT_OK;
//     }
//     _buff->data = newdata;

//     for (uint16_t countByte = 0; countByte < length; countByte++)
//     {
//         _buff->data[_buff->length + countByte] = u8DataPtr[countByte];
//     }
//     _buff->length = _buff->length + length;

//     return BUFF_OK;
// }

// uint8_t Buffer_SplitData(Buffer* buff, uint16_t posBuff, uint16_t posData)
// {
//     Buffer* _buff = buff;
//     Buffer* next_buff = _buff->link;

//     uint16_t Count = Buffer_CountData(buff);

//     if (posBuff >= Count)
//     {
//         return BUFF_NOT_OK;
//     }

//     for (uint16_t countData = 0; countData < posBuff; countData++)
//     {
//         _buff = _buff->link;
//         next_buff = _buff->link;
//     }

//     if (posData == 0 || posData >= _buff->length)
//     {
//         return BUFF_NOT_OK;
//     }

//     uint8_t* newdatahead = (uint8_t*)malloc(posData - 1);
//     uint8_t* newdatatail = (uint8_t*)malloc(_buff->length - posData + 1);
//     Buffer* newbuff = newBuffer();

//     if (newdatahead == NULL || newdatatail == NULL || newbuff == NULL)
//     {
//         free(newdatahead);
//         free(newdatatail);
//         free(newbuff);
//         return BUFF_NOT_OK;
//     }

//     for (uint16_t countData = 0; countData < _buff->length; countData++)
//     {
//         if (countData < (posData - 1))
//         {
//             newdatahead[countData] = _buff->data[countData];
//         }
//         else
//         {
//             newdatatail[countData - posData + 1] = _buff->data[countData];
//         }
//     }

//     newbuff->data = newdatatail;
//     newbuff->length = _buff->length - posData + 1;
//     newbuff->link = next_buff;

//     free(_buff->data);
//     _buff->data = newdatahead;
//     _buff->length = posData - 1;
//     _buff->link = newbuff;

//     return BUFF_OK;
// }

// uint8_t Buffer_DeleteData(Buffer* buff, uint16_t position)
// {
//     Buffer* lastbuff = buff;
//     Buffer* currbuff = buff;

//     uint16_t Count = Buffer_CountData(buff);

//     if (position >= Count)
//     {
//         return BUFF_NOT_OK;
//     }

//     for (uint16_t countData = 0; countData < (Count - 1); countData++)
//     {
//         if (countData >= position)
//         {
//             if (currbuff->data != NULL && countData == position)
//             {
//                 free(currbuff->data);
//             }
//             currbuff->data = currbuff->link->data;
//             currbuff->length = currbuff->link->length;
//         }
//         lastbuff = currbuff;
//         currbuff = currbuff->link;       
//     }

//     lastbuff->link = NULL;
//     if (Count == 1)
//     {
//         if (currbuff->data != NULL)
//         {
//             free(currbuff->data);
//         }
//         currbuff->data = NULL;
//         currbuff->length = 0;
//         currbuff->link = NULL;
//     }
//     else
//     {
//         free(currbuff);
//     }

//     return BUFF_OK;
// }

// uint16_t Buffer_CountData(Buffer* buff)
// {
//     Buffer* _buff = buff;
//     uint16_t countData = 0;

//     while (_buff != NULL)
//     {
//         countData++;
//         _buff = _buff->link;
//     }
//     return countData;
// }

// void Buffer_Dispose(Buffer* buff)
// {
//     uint16_t nbData = Buffer_CountData(buff);
//     for (uint16_t countData = 0; countData < nbData; countData++)
//     {
//         Buffer_DeleteData(buff, 0);
//     }
//     free(buff);
// }