/*! @mainpage Guia1_Ej3
 *
 * \section DescripcionGeneral Proyecto 1 - Ejercicio 3
 *
 * Mediante una funcion se recibe un puntero a una estructura LED y se controla su funcionamiento
 * con los modos ON, OFF y TOGGLE -encendido, apagado y titileo (a cierta frecuencia)-
 *
 * Para mas informacion consultar el siguiente diagrama de flujo que se utilizo como guia para la implementacion
 * Diagrama de flujo --- ver imagen en la consigna 3
 * https://docs.google.com/document/d/1f4OtorkZ1hOFu-jOo0Uhmz-cJnUvV05Z-eNhs19IVZM/edit
 *
 * @section changelog Changelog
 *
 * |   Fecha    | Descripcion                                    |
 * |:----------:|:-----------------------------------------------|
 * | 13/03/2024 | Creacion del documento                         |
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
/** @def ON
 * @brief Enciende el LED
*/
#define ON 1

/** @def OFF
 * @brief Apaga el LED
*/
#define OFF 0

/** @def TOGGLE
 * @brief Hace titilar el LED
*/
#define TOGGLE 2

/** @def CONFIG_BLINK_PERIOD
 * @brief Establece un periodo con el que titila el LED
*/
#define CONFIG_BLINK_PERIOD 100

/*==================[internal data definition]===============================*/
/** @fn my_leds 
 * @brief Estructura LED para controlar los mismos  
 * @param mode modo del led, que puede ser ON, OFF o TOGGLE 
 * @param n_led indica el número de led a controlar
 * @param n_ciclos indica la cantidad de ciclos de encendido/apagado
 * @param periodo //indica el tiempo de cada ciclo
 */
struct leds
{
    uint8_t mode;
	uint8_t n_led;
	uint8_t n_ciclos;
	uint16_t periodo;
} my_leds;

/*==================[internal functions declaration]=========================*/
/** @fn void controlLed( struct leds * LED )
 * @brief Metodo por el cual se controla 3 leds diferentes, cada uno con 3 modos de funcionamiento
 * @param LED puntero a una estructura LED
 * @return
*/
void controlLed( struct leds * LED )
{
	uint8_t i = 0;

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

		case TOGGLE:							//si MODE TOGGLE titila el led que corresponda hasta que i sea mayor que n_ciclos
		{										
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

			i++;								//incrementa i

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