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
#include "uart_mcu.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "timer_mcu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"

/*==================[macros and definitions]=================================*/
/** @def AD_SAMPLE_PERIOD
 * @brief Establece un periodo de medicion, expresado en milisegundos
 * se mide cada 3 segundos
 */
#define AD_SAMPLE_PERIOD 3000 // el sensor de PH mide cada 3s
#define SHOW_PERIOD 5000	  // se muestra por el puerto serie el estado de las variables cada 5s
uint32_t pH;				  // variable donde se almacena el valor actual del pH
bool humedad;
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

TaskHandle_t ADC_TaskHandle = NULL;
TaskHandle_t Mostrar_TaskHandle = NULL;

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

static void ADConvert(void *pvParameter)
{
	uint32_t aux;
	while (true)
	{
		AnalogInputReadSingle(CH1, &aux);
		pH = getpH(aux);
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
	}
}

void Mostrar (void *pvParameter){
	string hum_str;
	if(humedad){
		hum_str = "humedad correcta.";
	}
	else{
		hum_str = "humedad incorrecta.";
	}
	UartSendString(UART_PC, "pH: ");
	UartSendString(UART_PC, (char*)UartItoa(pH, 10));
	UartSendString(UART_PC, ", ");
	UartSendString(UART_PC, hum_str);
}

void FuncTimerADC(void *param)
{
	xTaskNotifyGive(ADC_TaskHandle); /*Envia una notificacion*/
}

void FuncTimerMostrar(void *param)
{
	xTaskNotifyGive(Mostrar_TaskHandle); /*Envia una notificacion*/
}

/** @fn getpH
 * @brief A partir de un voltaje dado, devuelve el valor del pH correspondiente
 * @param volt voltaje que corresponde a una medicion de pH
 * @return pH_aux
 */
uint32_t getpH(uint16_t volt)
{
	uint32_t aux = 14 / 3000; // Factor de conversion, segun la relacion 0V-3V con pH de 0 a 14
							  // se utiliza el valor de voltaje en milivotls (mV)
	uint32_t pH_aux = volt * aux;
	return pH_aux;
}


/*==================[external functions definition]==========================*/
void app_main(void)
{

	// Creo la estructura del tipo gpioConf_t que contienen el puerto GPIO_20
	// con sus correspondiente direccion(entrada/salida)
	gpioConf_t pin =
		{GPIO_20, GPIO_INPUT};

	// Inicializo el puertos GPIO

	GPIOInit(pin.pin, pin.dir);

	timer_config_t timer_ADC = {// Inicializacion del timer
								.timer = TIMER_A,
								.period = AD_SAMPLE_PERIOD,
								.func_p = FuncTimerADC,
								.param_p = NULL};
	TimerInit(&timer_ADC);

	timer_config_t timer_Mostrar = {// Inicializacion del timer
									.timer = TIMER_B,
									.period = SHOW_PERIOD,
									.func_p = FuncTimerMostrar,
									.param_p = NULL};
	TimerInit(&timer_Mostrar);

	serial_config_t puerto_serie = {// Inicializacion del puerto serie
									.baud_rate = 115200,
									.port = UART_PC,
									.func_p = NULL,
									.param_p = NULL};
	// Inicializacion del puerto serie
	UartInit(&puerto_serie);

	// Configuracion del AD Convert
	analog_input_config_t config_ADC =
		{
			.input = CH1,		/*!< Inputs: CH0, CH1, CH2, CH3 */
			.mode = ADC_SINGLE, /*!< Mode: single read or continuous read */
			.func_p = NULL,		/*!< Pointer to callback function for convertion end (only for continuous mode) */
			.param_p = NULL,	/*!< Pointer to callback function parameters (only for continuous mode) */
			.sample_frec = 0	/*!< Sample frequency min: 20kHz - max: 2MHz (only for continuous mode)  */
		};
	// Inicializacion del AD Convert
	AnalogInputInit(&config_ADC);

	xTaskCreate(&ADConvert, "conversor AD", 4096, NULL, 5, &ADC_TaskHandle);
	xTaskCreate(&Mostrar, "Mostrar", 4096, NULL, 5, &Mostrar_TaskHandle);

	TimerStart(timer_ADC.timer);
	TimerStart(timer_Mostrar.timer);
}
/*==================[end of file]============================================*/