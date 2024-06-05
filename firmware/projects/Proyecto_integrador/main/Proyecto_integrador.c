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
#define LED_BT	LED_1
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
/*==================[internal functions declaration]=========================*/
void FuncTimer_Medir(void *param)
{
	xTaskNotifyGive(medir_TaskHandle);
}

void FuncTimer_Mostrar(void *param)
{
	xTaskNotifyGive(mostrar_TaskHandle);
}

void FuncTimer_Procesamiento(void *param)
{
	xTaskNotifyGive(procesamiento_TaskHandle);
}

static void Medir(void *pvParameter)
{
	/* Se imprimen por consola los valores de frequencia y magnitud correspondiente */
	printf("****MAX30102 Test****\n");
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
			// printf("%ld,%2.2f,%ld\n", redBuffer[i], dato_filt, heartRate);
		}

		// After gathering 25 new samples recalculate HR and SP02
		maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
		printf("HR= %ld, HRvalid= %d \n", heartRate, validHeartRate);
		printf("SPO2= %ld, SPO2Valid= %d \n", spo2, validSPO2);
		LedToggle(LED_2);
		// vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
		// ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
	}
}

static void Mostrar(void *pvParameter)
{
	// VISUALIZACION VIA BLUETOOTH
}

static void Procesamiento(void *pvParameter)
{
	// PROCESAMIENTO DE DATOS
}

void Max30102Init()
{
	HiPassInit(SAMPLE_FREQ, 1, ORDER_4);
	MAX3010X_begin();
	MAX3010X_setup(30, 1, 2, SAMPLE_FREQ, 69, 4096);
}

void RecibirDato(){

}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	// MAX30102 INIT
	// Max30102Init();
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
	TimerInit(&timer_Medir);
	/*
		timer_config_t timer_Mostrar = {
			.timer = TIMER_B,
			.period = MEASURE_PERIOD_B,
			.func_p = FuncTimer_Mostrar,
			.param_p = NULL};
		TimerInit(&timer_Mostrar);

		timer_config_t timer_Procesamiento = {
			.timer = TIMER_C,
			.period = MEASURE_PERIOD_C,
			.func_p = FuncTimer_Procesamiento,
			.param_p = NULL};
		TimerInit(&timer_Procesamiento);
	*/
	// xTaskCreate(&Medir, "Measure_MAX30102", 512, NULL, 5, &medir_TaskHandle);
	// TimerStart(timer_Medir.timer);

	// xTaskCreate(&Mostrar, "Measure_MAX30102", 512, NULL, 5, &mostrar_TaskHandle);
	// xTaskCreate(&Procesamiento, "Measure_MAX30102", 512, NULL, 5, &procesamiento_TaskHandle);
	MAX3010X_begin();
	MAX3010X_setup(30, 1, 2, SAMPLE_FREQ, 69, 4096);
	printf("****MAX30102 Test****\n");
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
			// HiPassFilter(&dato, &dato_filt, 1);
			//  printf("%ld,%2.2f,%ld\n", redBuffer[i], dato_filt, heartRate);
		}

		// After gathering 25 new samples recalculate HR and SP02
		maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
		printf("HR= %ld, HRvalid= %d \n", heartRate, validHeartRate);
		printf("SPO2= %ld, SPO2Valid= %d \n", spo2, validSPO2);
		LedToggle(LED_2);

		  switch(BleStatus()){
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

	char data[50];
	sprintf(data, "SPO2: %ld, HeartRate: %ld\n", spo2, heartRate);
	BleSendString(data);
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	}
  

}
/*==================[end of file]============================================*/