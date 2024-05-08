/*! @mainpage Template
 *
 * @section genDesc General Description
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
 * | 28/03/2024 | Document creation		                         |
 *
 * @author Dalmacio Gonzalez Viduzzi (dgonzalezviduzzi@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	uint32_t var;
	uint32_t A;

	var |=0<<13;
	var |=0<<14;
	var |=1<<3;
	if(var & 0x00006000)
		{ printf("bits 13 y 14 en 1\n"); }
	else{ printf("bits 13 y 14 en 0\n"); }

	if(var & 0x00000008)
		{ printf("bits 3 en 1\n"); }
	else{ printf("bits 3 en 0\n"); }

	if(var & 0x00000010)
		{ A = 0; }
	else{ A = 0xaa; }

}
/*==================[end of file]============================================*/