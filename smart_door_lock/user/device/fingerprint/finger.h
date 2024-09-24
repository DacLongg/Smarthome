#ifndef __FINGER_H__
#define __FINGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include"stdio.h"

#define SUCCESS         0x00
#define ERROR           0x01


typedef union
{
    /* data */
    struct
    {
        /* data */
        uint16_t PageNumb;
        uint16_t score;    
    }comp;
    uint8_t rawdata[4];
}Data_t;

typedef struct 
{
    uint16_t length;
    uint8_t confirmCode;
    Data_t  data;
    uint16_t crc;
}Frame_t;


// typedef union
// {
//     /* data */
//     struct
//     {
//         /* data */
//         uint8_t status : 1;
//         uint8_t ID     : 7;
//     }bit;
//     uint8_t byte;
// }ManagerID_t;


typedef struct 
{
    uint8_t ID[128];
    uint8_t index;
    uint8_t NumberID;
}finger;

typedef struct buff
{
    /* data */
    uint8_t         *data;
    uint16_t        length;
    struct buff     *link;
}Buffer;

extern finger Finger;
extern Frame_t dataRx;
extern uint8_t FP_FuncRun;


uint8_t finger_Check(void);
void finger_DeleteALL(void);
void finger_DeleteID(uint16_t FGdata_Subcrib);
void FingerPrint_task(void *pvParameters);

 #ifdef __cplusplus
}
#endif
#endif