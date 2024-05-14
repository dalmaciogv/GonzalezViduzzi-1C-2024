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
#define AD_SAMPLE_PERIOD 2000
#define DA_SAMPLE_PERIOD 4000
#define BUFFER_SIZE 231
/*==================[internal data definition]===============================*/

TaskHandle_t ADC_TaskHandle = NULL;
TaskHandle_t DAC_TaskHandle = NULL;

const char ECG[BUFFER_SIZE] = {    76, 77, 78, 77, 79, 86, 81, 76, 84, 93,
	85, 80, 89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91, 99, 105, 101,
	96, 102, 106, 101, 96, 100, 107, 101, 94, 100, 104, 100, 91, 99, 103,
	98, 91, 96, 105, 95, 88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87,
	80, 83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82, 79, 69, 80,
	82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72, 80, 87, 79, 76, 85, 95, 87,
	81, 88, 93, 88, 84, 87, 94, 86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92,
	99, 91, 88, 94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89, 89, 90, 98,
	107, 104, 98, 104, 110, 102, 98, 103, 111, 101, 94, 103, 108, 102, 95,
	97, 106, 100, 92, 101, 103, 100, 94, 98, 103, 96, 90, 98, 103, 97, 90,
	99, 104, 95, 90, 99, 104, 100, 93, 100, 106, 101, 93, 101, 105, 103,
	96, 105, 112, 105, 99, 103, 108, 99, 96, 102, 106, 99, 90, 92, 100, 87,
	80, 82, 88, 77, 69, 75, 79, 74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75,
	84, 79, 77, 77, 76, 76,		};

/*==================[internal functions declaration]=========================*/
void FuncTimerADC(void *param)
{
	xTaskNotifyGive(ADC_TaskHandle); /* Envía una notificación a la tarea asociada al LED_1 */
}

void FuncTimerDAC(void *param)
{
	xTaskNotifyGive(DAC_TaskHandle); /* Envía una notificación a la tarea asociada al LED_1 */
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

static void DAConvert(void *pvParameter)
{
	uint8_t i = 0;
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // enviar dato
        AnalogOutputWrite(ecg[i]);
        i++;

        if (i == (BUFFER_SIZE-1))
        {
            i = 0;
        }
    }
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	serial_config_t puerto_serie = {	//Inicializacion del puerto serie
		.baud_rate = 115200,
		.port = UART_PC,
		.func_p = NULL,
		.param_p = NULL};

	UartInit(&puerto_serie);

	timer_config_t timer_ADC = {		//Inicializacion del timer
		.timer = TIMER_A,
		.period = AD_SAMPLE_PERIOD,
		.func_p = FuncTimerADC,
		.param_p = NULL};
	TimerInit(&timer_ADC);

	timer_config_t timer_DAC = {		//Inicializacion del timer
		.timer = TIMER_A,
		.period = DA_SAMPLE_PERIOD,
		.func_p = FuncTimerDAC,
		.param_p = NULL};
	TimerInit(&timer_DAC);

	analog_input_config_t config_ADC =		
	{
		.input = CH1,			/*!< Inputs: CH0, CH1, CH2, CH3 */
		.mode = ADC_SINGLE, /*!< Mode: single read or continuous read */
		.func_p = NULL,		/*!< Pointer to callback function for convertion end (only for continuous mode) */
		.param_p = NULL,	/*!< Pointer to callback function parameters (only for continuous mode) */
		.sample_frec = 0 /*!< Sample frequency min: 20kHz - max: 2MHz (only for continuous mode)  */
	};
	AnalogInputInit(&config_ADC);

	AnalogOuputInit();

	/*creacion de tareas*/
    xTaskCreate(&DAC_TaskHandle, "conversor DA", 512, NULL, 5, &ConversorDA_task_handle);
    xTaskCreate(&ADC_TaskHandle, "conversor AD", 4096, NULL, 5, &ConversorAD_task_handle);

    /*Inicio del conteo de timers*/
    TimerStart(timer_ADC.timer);
    TimerStart(timer_DAC.timer);
}

/*==================[end of file]============================================*/