/*! @mainpage Proyecto Integrador
 *
 * @section genDesc Proyecto Integrador
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 03/04/2024 | Document creation		                         |
 *
 * @author Dalmacio Gonzalez Viduzzi (dgonzalezviduzzi@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "timer_mcu.h"
#include "led.h"
#include <iir_filter.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "heartRate.h"
#include "max3010x.h"
#include "spo2_algorithm.h"
#include "string.h"
#include "ble_mcu.h"
#include "analog_io_mcu.h"
// #include "mcp9700.h"

/*==================[macros and definitions]=================================*/
#define MEASURE_PERIOD_A 2000000
#define MEASURE_PERIOD_B 1000000
#define MEASURE_PERIOD_C 1000000

TaskHandle_t medir_TaskHandle = NULL;
TaskHandle_t procesamiento_TaskHandle = NULL;
TaskHandle_t mostrar_TaskHandle = NULL;

bool measure_flag = false;
bool process_flag = false;
bool view_flag = false;
#define LED_BT LED_1
// MAX30102
#define BUFFER_SIZE 256
#define SAMPLE_FREQ 100
#define CONFIG_BLINK_PERIOD 100
/*==================[internal data definition]===============================*/
float dato_filt;
float dato;

uint32_t irBuffer[100];		// infrared LED sensor data
uint32_t redBuffer[100];	// red LED sensor data
int32_t bufferLength = 100; // data length
int32_t spo2;				// SPO2 value
int8_t validSPO2;			// indicator to show if the SPO2 calculation is valid
int32_t heartRate;			// heart rate value
int8_t validHeartRate;		// indicator to show if the heart rate calculation is valid
uint16_t temp;				// temperature value
char str1[100], str2[100], str3[100], str4[100], str5[100], str6[100];

typedef struct
{
	float redValues[BUFFER_SIZE];
	float irValues[BUFFER_SIZE];
	float temperatures[BUFFER_SIZE];
	int index;
} Measurements_t;

Measurements_t measurements;

/*==================[internal functions declaration]=========================*/
void FuncTimer_Medir(void *param)
{
	//xTaskNotifyGive(medir_TaskHandle);
}

void FuncTimer_Mostrar(void *param)
{
	xTaskNotifyGive(mostrar_TaskHandle);
}

void FuncTimer_Procesamiento(void *param)
{
	xTaskNotifyGive(procesamiento_TaskHandle);
}


float CalculateAverage(float *values, int size)
{
	float sum = 0.0;
	for (int i = 0; i < size; i++)
	{
		sum += values[i];
	}
	return sum / size;
}

float CalculateMax(float *values, int size)
{
	float max = values[0];
	for (int i = 1; i < size; i++)
	{
		if (values[i] > max)
		{
			max = values[i];
		}
	}
	return max;
}

float CalculateMin(float *values, int size)
{
	float min = values[0];
	for (int i = 1; i < size; i++)
	{
		if (values[i] < min)
		{
			min = values[i];
		}
	}
	return min;
}

void MCP9700Init()
{
	// Configuracion del AD Convert
	analog_input_config_t config_ADC =
		{
			.input = CH1,		/*!< Inputs: CH0, CH1, CH2, CH3 */
			.mode = ADC_SINGLE, /*!< Mode: single read or continuous read */
			.func_p = NULL,		/*!< Pointer to callback function for convertion end (only for continuous mode) */
			.param_p = NULL,	/*!< Pointer to callback function parameters (only for continuous mode) */
			.sample_frec = 0	/*!< Sample frequency min: 20kHz - max: 2MHz (only for continuous mode)  */
		};

	AnalogInputInit(&config_ADC);
}

uint16_t LeerTemp()
{
	uint16_t aux = 200, temperatura;
	//AnalogInputReadSingle(CH1, &aux); // se lee la medicion en una variable auxiliar porque luego hay que hacer una conversion
									  // MCP9700 mide 10mV por grado Celcius
	temperatura = aux / 10;
	return temperatura;
}

static void Medir(void *pvParameter)
{
	/* Se imprimen por consola los valores de frequencia y magnitud correspondiente */
	printf("****MAX30102 Test****\n");
	measurements.index = 0;
	while (1)
	{
		uint8_t i;
		for (i = 25; i < 100; i++)
		{
			redBuffer[i - 25] = redBuffer[i];
			irBuffer[i - 25] = irBuffer[i];
		}

		// take 25 sets of samples before calculating the heart rate.
		for (i = 75; i < 100; i++)
		{

			while (MAX3010X_available() == false) // do we have new data?
				MAX3010X_check();				  // Check the sensor for new data

			redBuffer[i] = MAX3010X_getRed();
			irBuffer[i] = MAX3010X_getIR();
			MAX3010X_nextSample(); // We're finished with this sample so move to next sample

			// send samples and calculation result to terminal program through UART
			dato = (float)redBuffer[i];
			HiPassFilter(&dato, &dato_filt, 1);
		}

		temp = LeerTemp();
		// ALMACENAMIENTO DE DATOS PARA SU POSTERIOR PROCESAMIENTO
		if (measurements.index < BUFFER_SIZE)
		{
			measurements.redValues[measurements.index] = (float)redBuffer[i];
			measurements.irValues[measurements.index] = (float)irBuffer[i];
			measurements.temperatures[measurements.index] = (float)temp;
			measurements.index++;
		}
		else
		{
			// Si el buffer está lleno, reiniciamos el índice (esto puede ser mejorado con un buffer circular)
			measurements.index = 0;
		}
		

		// After gathering 25 new samples recalculate HR and SP02
		maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

		sprintf(str1,"*F%ld*", heartRate);
		sprintf(str2,"*QSPO2= %ld %%.\n*", spo2);
		sprintf(str3,"*T%d*", temp);
		BleSendString(str1);
		BleSendString(str2);
		BleSendString(str3);

		if (process_flag)
		{
			process_flag = false;

			float avgRed = CalculateAverage(measurements.redValues, measurements.index);
			float maxRed = CalculateMax(measurements.redValues, measurements.index);
			float minRed = CalculateMin(measurements.redValues, measurements.index);

			float avgIr = CalculateAverage(measurements.irValues, measurements.index);
			float maxIr = CalculateMax(measurements.irValues, measurements.index);
			float minIr = CalculateMin(measurements.irValues, measurements.index);

			float avgTemp = CalculateAverage(measurements.temperatures, measurements.index);
			float maxTemp = CalculateMax(measurements.temperatures, measurements.index);
			float minTemp = CalculateMin(measurements.temperatures, measurements.index);

			sprintf(str4,"Average Red: %f, Max Red: %f, Min Red: %f\n", avgRed, maxRed, minRed);
			sprintf(str5,"Average IR: %f, Max IR: %f, Min IR: %f\n", avgIr, maxIr, minIr);
			sprintf(str6, "*WAverage Temp: %f, Max Temp: %f, Min Temp: %f\n*", avgTemp, maxTemp, minTemp);
			BleSendString(str4);
			BleSendString(str5);
			BleSendString(str6);
		}

		LedToggle(LED_2);

		switch (BleStatus())
		{
		case BLE_OFF:
			LedOff(LED_BT);
			break;
		case BLE_DISCONNECTED:
			LedToggle(LED_BT);
			break;
		case BLE_CONNECTED:
			LedOn(LED_BT);
			break;
		}

		//ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
	}
}

static void Mostrar(void *pvParameter)
{
	// VISUALIZACION VIA BLUETOOTH
	while (1)
	{
		printf("HR= %ld BPM.\n", heartRate);
		printf("SPO2= %ld por ciento.\n", spo2);
		printf("Temperature= %d ºC.\n", temp);
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
	}
}

static void Procesamiento(void *pvParameter)
{
	// PROCESAMIENTO DE DATOS
	while (1)
	{
		if (process_flag)
		{
			process_flag = false;

			float avgRed = CalculateAverage(measurements.redValues, measurements.index);
			float maxRed = CalculateMax(measurements.redValues, measurements.index);
			float minRed = CalculateMin(measurements.redValues, measurements.index);

			float avgIr = CalculateAverage(measurements.irValues, measurements.index);
			float maxIr = CalculateMax(measurements.irValues, measurements.index);
			float minIr = CalculateMin(measurements.irValues, measurements.index);

			float avgTemp = CalculateAverage(measurements.temperatures, measurements.index);
			float maxTemp = CalculateMax(measurements.temperatures, measurements.index);
			float minTemp = CalculateMin(measurements.temperatures, measurements.index);

			printf("Average Red: %f, Max Red: %f, Min Red: %f\n", avgRed, maxRed, minRed);
			printf("Average IR: %f, Max IR: %f, Min IR: %f\n", avgIr, maxIr, minIr);
			printf("Average Temp: %f, Max Temp: %f, Min Temp: %f\n", avgTemp, maxTemp, minTemp);
		}
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

void Max30102Init()
{
	HiPassInit(SAMPLE_FREQ, 1, ORDER_4);
	MAX3010X_begin();
	MAX3010X_setup(30, 1, 2, SAMPLE_FREQ, 69, 4096);
}

void RecibirDato(uint8_t *data, uint8_t length)
{
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	// MAX30102 INIT
	Max30102Init();
	LedsInit();
	ble_config_t ble_configuration = {
		"Sistema de monitoreo",
		RecibirDato};
	BleInit(&ble_configuration);

	timer_config_t timer_Medir = {
		.timer = TIMER_A,
		.period = MEASURE_PERIOD_A,
		.func_p = FuncTimer_Medir,
		.param_p = NULL};
	//TimerInit(&timer_Medir);

	timer_config_t timer_Mostrar = {
		.timer = TIMER_B,
		.period = MEASURE_PERIOD_B,
		.func_p = FuncTimer_Mostrar,
		.param_p = NULL};
	//TimerInit(&timer_Mostrar);

	/*	timer_config_t timer_Procesamiento = {
			.timer = TIMER_C,
			.period = MEASURE_PERIOD_C,
			.func_p = FuncTimer_Procesamiento,
			.param_p = NULL};
		TimerInit(&timer_Procesamiento);
	*/
	xTaskCreate(&Medir, "Measure_MAX30102", 2048, NULL, 5, &medir_TaskHandle);
	//xTaskCreate(&Mostrar, "Mostrar", 2048, NULL, 5, &mostrar_TaskHandle);

	//TimerStart(timer_Medir.timer);
	//TimerStart(timer_Mostrar.timer);
}
/*==================[end of file]============================================*/