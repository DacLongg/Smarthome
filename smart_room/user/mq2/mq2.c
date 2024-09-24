#include"mq2.h"
#include <driver/adc.h>

#define millis() timer_get_cnt()
#define delay() delay_us()

extern uint64_t timer_get_cnt(void);
extern void delay_us(uint64_t us);

int _pin;

float LPGCurve[3] = {2.3, 0.18, -0.525}; // {2.3, 0.21, -0.47}; 
float COCurve[3] = {2.3, 0.80, -0.27};   // {2.3, 0.72, -0.34};  (default)
float SmokeCurve[3] = {2.3, 0.28, -0.75};   // {2.3, 0.53, -0.44};                                                    
float Ro = -1.0;

float values[3];  // array with the measured values in the order: lpg, CO and smoke

int lastReadTime = 0;

void Adc_Init(void)
{
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_12);
    adc1_config_width(ADC_WIDTH_BIT_12);
    

}

void read_mq2(int *data)
{

    *data = 0;
        // ret = adc2_get_raw( ADC2_CHANNEL_5, ADC_WIDTH_BIT_12, &adc_value[count]);
    *data = adc1_get_raw( ADC1_CHANNEL_6);
        // printf("adc value %d; ret = %d \n", adc_value[count], ret);
    // printf("adc2 value %d \n", *data);
}



void read_flame(int *data)
{
        *data = 0;
        // ret = adc2_get_raw( ADC2_CHANNEL_5, ADC_WIDTH_BIT_12, &adc_value[count]);
    *data = adc1_get_raw( ADC1_CHANNEL_7);
        // printf("adc value %d; ret = %d \n", adc_value[count], ret);
    // printf("flame sensors %d \n", *data);
}


void MQ_begin(void){
	Ro = MQCalibration();
	// Serial.print("Ro: ");
	// Serial.print(Ro);
	// Serial.println(" kohm");
}

void MQ_close(void){
	Ro = -1.0;
	values[0] = 0.0;
	values[1] = 0.0;
	values[2] = 0.0;
}

bool checkCalibration() {
	if (Ro < 0.0) {
		// Serial.println("Device not calibrated, call MQ2::begin before reading any value.");
		return false;
	}

	return true;
}

float* MQ_read(bool print){
	if (!checkCalibration()) return NULL;

    mq2_value.param.CO = MQGetPercentage(COCurve);
    mq2_value.param.LPG = MQGetPercentage(LPGCurve);
    mq2_value.param.Smoke = MQGetPercentage(SmokeCurve);

	// values[0] = MQGetPercentage(LPGCurve);
	// values[1] = MQGetPercentage(COCurve);
	// values[2] = MQGetPercentage(SmokeCurve);

	lastReadTime = millis();

	if (print){
		// Serial.print(lastReadTime);
		// Serial.print("ms - LPG:");
		// Serial.print(values[0], 5);
		// Serial.print("ppm\t");
		// Serial.print("CO:");
		// Serial.print(values[1], 5);
		// Serial.print("ppm\t");
		// Serial.print("SMOKE:");
		// Serial.print(values[2], 5);
		// Serial.print("ppm\n");
	}

	return values;
}

float MQ_readLPG(){
	if (!checkCalibration()) return 0.0;

    if (millis() < (lastReadTime + READ_DELAY) && values[0] > 0)
        return values[0];
    else
        return (values[0] = MQGetPercentage(LPGCurve));
}

float MQ_readCO(){
	if (!checkCalibration()) return 0.0;

    if (millis() < (lastReadTime + READ_DELAY) && values[1] > 0)
        return values[1];
    else
        return (values[1] = MQGetPercentage(COCurve));
}

float MQ_readSmoke(){
	if (!checkCalibration()) return 0.0;

    if (millis() < (lastReadTime + READ_DELAY) && values[2] > 0)
        return values[2];
    else
        return (values[2] = MQGetPercentage(SmokeCurve));
}

float MQResistanceCalculation(int raw_adc) {
	float flt_adc = (float) raw_adc;
	return RL_VALUE * (4096.0 - flt_adc) / flt_adc;
}

float MQCalibration() {
	float val = 0.0;
    int adcvalue = 0;
    read_mq2(&adcvalue);

	// take multiple samples
	for (int i = 0; i < CALIBARAION_SAMPLE_TIMES; i++) {

		val += MQResistanceCalculation(adcvalue);
		delay_us(CALIBRATION_SAMPLE_INTERVAL);
	}

	//calculate the average value
	val = val / ((float) CALIBARAION_SAMPLE_TIMES);

	//divided by RO_CLEAN_AIR_FACTOR yields the Ro according to the chart in the datasheet 
	val = val / RO_CLEAN_AIR_FACTOR;

	return val; 
}

float MQRead() {
	float rs = 0.0;

    int adcvalue = 0;
    read_mq2(&adcvalue);

	for (int i = 0; i < READ_SAMPLE_TIMES; i++) {
		rs += MQResistanceCalculation(adcvalue);
		delay_us(READ_SAMPLE_INTERVAL);
	}

	return rs / ((float) READ_SAMPLE_TIMES);  // return the average
}

float MQGetPercentage(float *pcurve) {
	float rs_ro_ratio = MQRead() / Ro;
	return pow(10.0, ((log(rs_ro_ratio) - pcurve[1]) / pcurve[2]) + pcurve[0]);
}

