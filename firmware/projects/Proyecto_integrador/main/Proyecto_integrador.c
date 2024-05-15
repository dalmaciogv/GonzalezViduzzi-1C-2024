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
/*==================[macros and definitions]=================================*/
#define MEASURE_PERIOD 1000000

TaskHandle_t medir_TaskHandle = NULL;
TaskHandle_t procesamiento_TaskHandle = NULL;
TaskHandle_t mostrar_TaskHandle = NULL;

bool measure_flag = false;
bool process_flag = false;
bool view_flag = false;
/*==================[internal data definition]===============================*/

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
	// MEDICION DEL MAX30102
}

static void Mostrar(void *pvParameter)
{
	// VISUALIZACION VIA BLUETOOTH
}

static void Procesamiento(void *pvParameter)
{
	// PROCESAMIENTO DE DATOS
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	// MAX30102 INIT

	timer_config_t timer_Medir = {
		.timer = TIMER_A,
		.period = MEASURE_PERIOD,
		.func_p = FuncTimer_Medir,
		.param_p = NULL};
	TimerInit(&timer_Medir);

	timer_config_t timer_Mostrar = {
		.timer = TIMER_B,
		.period = MEASURE_PERIOD,
		.func_p = FuncTimer_Mostrar,
		.param_p = NULL};
	TimerInit(&timer_Mostrar);

	timer_config_t timer_Procesamiento = {
		.timer = TIMER_C,
		.period = MEASURE_PERIOD,
		.func_p = FuncTimer_Procesamiento,
		.param_p = NULL};
	TimerInit(&timer_Procesamiento);

	xTaskCreate(&Medir, "Measure_MAX30102", 512, NULL, 5, &medir_TaskHandle);
	xTaskCreate(&Mostrar, "Measure_MAX30102", 512, NULL, 5, &mostrar_TaskHandle);
	xTaskCreate(&Procesamiento, "Measure_MAX30102", 512, NULL, 5, &procesamiento_TaskHandle);
}
/*==================[end of file]============================================*/