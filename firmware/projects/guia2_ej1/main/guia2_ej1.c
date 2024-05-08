/*! @mainpage Blinking
 *
 * \section genDesc Proyecto 2 - Ejercicio 1
 *
 * Este programa mide una distancia utilizando ultrasonido (a traves del periferico HC-SR04)
 * mostrando la distancia medida (en centimetros) por un display de 7 segmentos
 * y a traves de LEDs segun en que rango se encuentra la medicion
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
 * |   Fecha	| Descripcion                                    |
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

/*==================[macros and definitions]=================================*/
/** @def MEASURE_PERIOD
 * @brief Establece un periodo de medicion
*/
#define MEASURE_PERIOD 1000

/** @def CONFIG_BLINK_PERIOD
 * @brief Establece un periodo de duracion del pulso en el switch
*/
#define SWITCH_PERIOD 100

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

/*==================[internal functions declaration]=========================*/
/** @fn static void Mostrar(void *pvParameter)
 * @brief Muestar por display LCD el valor medido y enciende el led correspondiente a la medicion 
 * @param pvParameter
 * @return
*/
static void Mostrar(void *pvParameter)
{
	while (true)
	{
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

		vTaskDelay(MEASURE_PERIOD / portTICK_PERIOD_MS);
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
		distancia = HcSr04ReadDistanceInCentimeters();
		vTaskDelay(MEASURE_PERIOD / portTICK_PERIOD_MS);
	}
}

/** @fn static void Teclas(void *pvParameter)
 * @brief Lee el estado de las teclas y cambia el estado de las banderas que correspondan
 * @param pvParameter
 * @return
*/
static void Teclas(void *pvParameter)
{
	while (true)
	{
		teclas = SwitchesRead();
		switch (teclas)
		{
		case SWITCH_1:
			measure_flag = !measure_flag;
			break;

		case SWITCH_2:
			hold_flag = !hold_flag;
			break;

			vTaskDelay(SWITCH_PERIOD / portTICK_PERIOD_MS);
		}
	}
}


/*==================[external functions definition]==========================*/
void app_main(void)
{
	LedsInit();
	SwitchesInit();
	LcdItsE0803Init();
	HcSr04Init(GPIO_3, GPIO_2);
	xTaskCreate(&Medir, "Measure_HCSR04", 512, NULL, 5, NULL);
	xTaskCreate(&Mostrar, "Display", 512, NULL, 5, NULL);
	xTaskCreate(&Teclas, "Switchs", 512, NULL, 5, NULL);
}

/*==================[end of file]============================================*/