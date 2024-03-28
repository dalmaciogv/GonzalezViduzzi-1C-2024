/*! @mainpage Guia1_Ej4
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
 * | 20/03/2024 | Document creation		                         |
 *
 * @author Dalmacio González Viduzzi (dgonzalezviduzzi@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <led.h>
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

//uint8_t cantidad_digitos(uint32_t numero);

//void  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number);


//Implementacion

/*uint8_t cantidad_digitos(uint32_t numero)
{
	uint8_t digitos = 1;
	uint8_t aux = numero;
	while ((aux/10)=!0)
	{
		digitos++;
		aux = numero%10;
	}
	return digitos;
}*/

void  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
	uint32_t aux = data;
	for(int i=digits-1; i>=0; i--)
	{
		bcd_number[i] = aux % 10;
		aux = aux / 10;
	}
}


typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;


void confPin (uint8_t *bcd_number, gpioConf_t *array)
{
	for (uint8_t i = 0; i < 4; i++)
	{
		if(bcd_number[i]==1)
		{
			GPIOOn(array[i].pin);
		}
		else
		{
			GPIOOff(array[i].pin);
		}
	}
	
}


/*==================[external functions definition]==========================*/
void app_main(void)
{
	uint32_t num = 1234;
	uint8_t array[4];
	uint8_t digito = 4;
	
	convertToBcdArray(num, digito, array);
	for (uint8_t i = 0; i < 4; i++)
	{
		printf("%d\n",array[i]);
	}
	
	gpioConf_t pines[4] = {{GPIO_20,GPIO_OUTPUT} , {GPIO_21,GPIO_OUTPUT} , {GPIO_22,GPIO_OUTPUT} , {GPIO_23,GPIO_OUTPUT}};
	for(u_int8_t i=0; i<4; i++)
	{
		GPIOInit(pines[i].pin, pines[i].dir);
		i++;
	}
	
	
	confPin(array, pines);
	
	for (uint8_t i = 0; i < 4; i++)
	{
		printf("%d\n",pines[i].pin);
	}
	

}
/*==================[end of file]============================================*/