#ifndef __MQ2_H__
#define __MQ2_H__

#ifdef __cplusplus
extern "C" {
#endif

#include"stdio.h"
#include"stdbool.h"
#include <math.h> 

# define RL_VALUE 3.3
// given constant
# define RO_CLEAN_AIR_FACTOR 9.83

// reads 10 times the sensor every 50ms and takes the average
// NOTE: it is encouraged to take more samples during the calibration
# define CALIBARAION_SAMPLE_TIMES 10
# define CALIBRATION_SAMPLE_INTERVAL 50

// reads 5 times the sensor every 50ms and takes the average
# define READ_SAMPLE_TIMES 5
# define READ_SAMPLE_INTERVAL 50

// 10s, time elapsed before new data can be read.
# define READ_DELAY 10000

typedef union
{
    /* data */
    struct
    {
        /* data */
        float LPG;
        float CO;
        float Smoke;
    }param;
    float rawdata[3];
    
}Mq2_t;

extern float values[3];
extern Mq2_t mq2_value;

// void MQ2(int pin);

/*
    * Initialises the sensor before getting any data.
    * This step is necessary as it needs to be calibrated based on
    * current air and temperature conditions.
    */
void MQ_begin(void);

/*
    * Stops the sensor, calibration and any read data is deleted.
    *
    * Pin information is kept, so just call `begin()` again
    * to read new values, no need to create a new object.
    */
void MQ_close(void);

/*
    * Reads the LPG, CO and smoke data from the sensor and returns and
    * array with the values in this order.
    *
    * The read procedure takes `READ_SAMPLE_TIMES` samples from the sensor
    * every `READ_SAMPLE_INTERVAL` ms and returns the average.
    * 
    * NOTE: do not modify the values of the returned array in place or deallocate it,
    * that could have unexpected consequences. Time to PANIC.
    */
float* MQ_read(bool print);

/*
    * Same as before but only return the data from the specified gas.
    *
    * If the time elapsed since the last measurement is smaller than
    * `READ_DELAY`, the same prior value will be returned.
    */
float readLPG();
float readCO();
float readSmoke();



float MQRead();
float MQGetPercentage(float *pcurve);
float MQCalibration();
float MQResistanceCalculation(int raw_adc);
bool checkCalibration();



void Adc_Init(void);
void read_mq2(int *data);
void read_flame(int *data);

 #ifdef __cplusplus
}
#endif
#endif