/*! @mainpage Guia2_Ej2
 *
 * \section genDesc Proyecto 2 - Ejercicio 2
 *
 * This example makes LED_1, LED_2 and LED_3 blink at different rates, using FreeRTOS tasks.
 *
 *		Rangos 
 *      Distancia (cm)        LEDs
 *      [0-10]               Ninguno
 *      [10-20]              LED_1
 *      [20-30]              LED_2
 *       >30                 LED_3
 *
 * @section hardConn Hardware Connection
 *
 * |    Periferico  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO_3		|
 * | 	TRIGGER	 	| 	GPIO_2		|
 * |    Vcc= +5V    |   Vcc= +5V    |
 * |    GND         |   GND         |
 * 
 * @section changelog Changelog
 *
 * |   Fecha    | Descripcion                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/04/2024 | Creacion del documento                         |
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

/*==================[macros and definitions]=================================*/
/** @def MEASURE_PERIOD
 * @brief Establece un periodo de medicion
*/
#define MEASURE_PERIOD 1000000

/*==================[internal data definition]===============================*/
/** @param hold_flag
 * @brief Indica si se debe mantener la lectura en el display o no
*/
bool hold_flag = false;

/** @param measure_flag
 * @brief Indica si se debe medir o no
*/
bool measure_flag = false;

/** @param teclas
 * @brief Representa una tecla de la placa utilizada
*/
uint8_t teclas;

/** @param distancia
 * @brief Representa la distancia que se va a medir
*/
uint16_t distancia = 0;

/** @param mostrar_TaskHandle
*/
TaskHandle_t mostrar_TaskHandle = NULL;

/** @param medir_TaskHandle
*/
TaskHandle_t medir_TaskHandle = NULL;

/*==================[internal functions declaration]=========================*/

/** @fn void FuncTimerA(void *param)
 * @brief Función invocada en la interrupción del timer
 * @param param
 */
void FuncTimerA(void *param)
{
	xTaskNotifyGive(mostrar_TaskHandle); /* Envía una notificación a la tarea asociada al LED_1 */
	xTaskNotifyGive(medir_TaskHandle);
}

/** @fn static void Mostrar(void *pvParameter)
 * @brief Muestar por display LCD el valor medido y enciende el led correspondiente a la medicion 
 * @param pvParameter
 * @return
*/
static void Mostrar(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
		if (measure_flag)
		{
			if (distancia < 10)
			{
				LedOff(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
			}

			if (distancia >= 10 && distancia < 20)
			{
				LedOn(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
			}

			if (distancia >= 20 && distancia < 30)
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOff(LED_3);
			}

			if (distancia >= 30)
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOn(LED_3);
			}

			if (!hold_flag)
			{
				LcdItsE0803Write(distancia);
			}
		}
		else
		{
			LedOff(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
			LcdItsE0803Off();
		}
	}
}

/** @fn static void Medir(void *pvParameter)
 * @brief Mide la distancia con el periferico HC-SR04
 * @param pvParameter
 * @return
*/
static void Medir(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
		distancia = HcSr04ReadDistanceInCentimeters();
	}
}

/** @fn void Tecla1()
 * @brief Cambia el estado de la bandera para la tecla 1
*/
void Tecla1()
{
	measure_flag = !measure_flag;
}

/** @fn void Tecla2()
 * @brief Cambia el estado de la bandera para la tecla 2
*/
void Tecla2()
{
	hold_flag = !hold_flag;
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	//Inicializacion
	LedsInit();
	SwitchesInit();
	LcdItsE0803Init();
	HcSr04Init(GPIO_3, GPIO_2);

	SwitchActivInt(SWITCH_1, &Tecla1, NULL);
	SwitchActivInt(SWITCH_2, &Tecla2, NULL);

	//Inicializacion del timer
	timer_config_t timer_interrupciones = {
		.timer = TIMER_A,
		.period = MEASURE_PERIOD,
		.func_p = FuncTimerA,
		.param_p = NULL};

	TimerInit(&timer_interrupciones);

	/* Creación de tareas */
	xTaskCreate(&Medir, "Measure_HCSR04", 512, NULL, 5, &medir_TaskHandle);
	xTaskCreate(&Mostrar, "Display", 512, NULL, 5, &mostrar_TaskHandle);
	TimerStart(timer_interrupciones.timer);
}

/*==================[end of file]============================================*/