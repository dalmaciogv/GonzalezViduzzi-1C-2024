/*! @mainpage Blinking
 *
 * \section genDesc General Description
 *
 * This example makes LED_1, LED_2 and LED_3 blink at different rates, using FreeRTOS tasks.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Dalmacio Gonzalez Viduzzi (dgonzalezviduzzi@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "switch.h"
#include "lcditse0803.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"

/*==================[macros and definitions]=================================*/
#define SAMPLE_PERIOD 2000
/*==================[internal data definition]===============================*/

TaskHandle_t convert_TaskHandle = NULL;

/*==================[internal functions declaration]=========================*/
void FuncTimerA(void *param)
{
	xTaskNotifyGive(convert_TaskHandle); /* Envía una notificación a la tarea asociada al LED_1 */
}

static void ADConvert(void *pvParameter)
{
	uint16_t aux;
	while (true)
	{
		AnalogInputReadSingle(CH1, &aux);
		UartSendString(UART_PC, (char*)UartItoa(aux, 10));
		UartSendString(UART_PC, "\r");
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{



	timer_config_t timer_interrupciones = {
		.timer = TIMER_A,
		.period = SAMPLE_PERIOD,
		.func_p = FuncTimerA,
		.param_p = NULL};

	TimerInit(&timer_interrupciones);

	xTaskCreate(&ADConvert, "Measure_HCSR04", 512, NULL, 5, &convert_TaskHandle);
	TimerStart(timer_interrupciones.timer);

	analog_input_config_t config_ADC =
	{
		.input = CH1,			/*!< Inputs: CH0, CH1, CH2, CH3 */
		.mode = ADC_SINGLE, /*!< Mode: single read or continuous read */
		.func_p = NULL,		/*!< Pointer to callback function for convertion end (only for continuous mode) */
		.param_p = NULL,	/*!< Pointer to callback function parameters (only for continuous mode) */
		.sample_frec = 0 /*!< Sample frequency min: 20kHz - max: 2MHz (only for continuous mode)  */
	};

	AnalogInputInit(&config_ADC);

	serial_config_t puerto_serie = {
		.baud_rate = 115200,
		.port = UART_PC,
		.func_p = NULL,
		.param_p = NULL};

	UartInit(&puerto_serie);
}

/*==================[end of file]============================================*/