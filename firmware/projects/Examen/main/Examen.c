/*! @mainpage Template
 *
 * @section genDesc Examen promocional - EProgramable
 *
 * Se pretende diseñar un dispositivo basado en la ESP-EDU
 * que permita controlar el riego y el pH de una plantera.
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
 * | 11/06/2024 | Document creation		                         |
 *
 * @author Dalmacio Gonzalez Viduzzi (dgonzalezviduzzi@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/** @fn gpioConf_t
 * @brief Estructura que representa un puerto GPIO
 * @param pin numero de pin del GPIO
 * @param dir direccion del GPIO; '0' entrada ;  '1' salida
 */
typedef struct
{
	gpio_t pin; /*!< GPIO pin number */
	io_t dir;	/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/*==================[external functions definition]==========================*/
void app_main(void){

// Creo la estructura del tipo gpioConf_t que contienen el puerto GPIO_20
//con sus correspondiente direccion(entrada/salida)
	gpioConf_t pin =
		{GPIO_20, GPIO_INPUT};

	// Inicializo el puertos GPIO

		GPIOInit(pin.pin, pin.dir);


}
/*==================[end of file]============================================*/