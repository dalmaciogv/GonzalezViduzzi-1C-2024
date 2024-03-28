/*! @mainpage Guia1_Ej3
 *
 * \section genDesc General Description
 *
 * This project makes the exercise 3 of the guia 1.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 13/03/2024 | Document creation		                         |
 *
 * @author Gonzalez Viduzzi Dalmacio (dgonzalezviduzzi@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <led.h>
/*==================[macros and definitions]=================================*/

#define ON 1
#define OFF 0
#define TOGGLE 2
#define CONFIG_BLINK_PERIOD 100

/*==================[internal data definition]===============================*/

struct leds
{
    uint8_t mode;       //ON, OFF, TOGGLE
	uint8_t n_led;      //indica el número de led a controlar
	uint8_t n_ciclos;   //indica la cantidad de ciclos de ncendido/apagado
	uint16_t periodo;    //indica el tiempo de cada ciclo
} my_leds;

/*==================[internal functions declaration]=========================*/

void controlLed( struct leds * LED );

//Implementacion

void controlLed( struct leds * LED )
{
	uint8_t i = 0;
	//uint8_t j = 0;

	switch(LED->mode)
	{
		case ON:							//si MODE ON entonces prende el led que corresponda
		{									
			switch (LED->n_led)
			{
				case 1:
				{
					LedOn(LED_1);
					break;
				}
				
				case 2:
				{
					LedOn(LED_2);
					break;
				}

				case 3:
				{
					LedOn(LED_3);
					break;
				}

				default:
					break;
			}
		break;
		}
	
		case OFF:							//si MODE OFF apaga el led que corresponda
		{
			switch (LED->n_led)
			{
				case 1:
				{
					LedOff(LED_1);
					break;
				}
				
				case 2:
				{
					LedOff(LED_2);
					break;
				}

				case 3:
				{
					LedOff(LED_3);
					break;
				}

				default:
					break;
			}
		break;
		}

		case TOGGLE:							//si MODE TOGGLE
		{										//titila el led que corresponda hasta que i sea mayor que n_ciclos
			while(i<(LED->n_ciclos))
			{
				switch (LED->n_led)
				{
				case 1:
				{	
					LedToggle(LED_1);
					break;
				}

				case 2:
				{	
					LedToggle(LED_2);
					break;
				}

				case 3:
				{	
					LedToggle(LED_3);
					break;
				}

				default:
				break;
				}	//fin del switch

			i++;								//y luego incrementa i

			uint16_t delay = LED->periodo/CONFIG_BLINK_PERIOD; //asigno periodo a la variable delay

				while(delay>0)									//mientras delay no sea cero, lo decrementa
				{
					delay--;
					vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
				}
		}														//fin del while
		break;													//fin del CASE TOGGLE
		}

		default:
		break;
	}
}



void app_main(void)
{
	//uint8_t teclas;   //inicializacion
	LedsInit();
	my_leds.mode = TOGGLE;
	my_leds.n_led = 1;
	my_leds.periodo = 500;
	my_leds.n_ciclos = 10;
	controlLed(&my_leds);
    while(1)
	{
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	}
}

/*==================[external functions definition]==========================*/








/*==================[end of file]============================================*/