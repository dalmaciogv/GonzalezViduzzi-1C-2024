/*! @mainpage Template
 *
 * @section genDesc Examen promocional - EProgramable
 *
 * Se pretende diseñar un dispositivo basado en la ESP-EDU
 * que permita controlar el riego y el pH de una plantera.
 *
 * @section hardConn Hardware Connection
 *
 * |    Periferico  |   ESP32   	|
 * |:--------------:|:--------------|
 * | Sensor Humedad	| 	GPIO_20		|
 * |Bomba agua		| 	GPIO_21		|
 * |Bomba pH basico | 	GPIO_22		|
 * |Bomba pH acido	|	GPIO_19		|
 * |    Vcc= +5V    |   Vcc= +5V    |
 * |    GND         |   GND         |
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
#include "switch.h"

/*==================[macros and definitions]=================================*/
/** @def AD_SAMPLE_PERIOD
 * @brief Establece un periodo de medicion, expresado en milisegundos
 * se mide cada 3 segundos
 */
#define AD_SAMPLE_PERIOD 3000 // el sensor de PH mide cada 3s

/** @def SHOW_PERIOD
 * @brief Establece un periodo con el que se muestran los datos, expresado en milisegundos
 * se muestran cada 5 segundos
 */
#define SHOW_PERIOD 5000	  // se muestra por el puerto serie el estado de las variables cada 5s

/** @def CONTTROL_PERIOD
 * @brief Establece un periodo con el que se controlan los dispositivos dependiendo del estado actual
 * se controlan cada 5 segundos
 */
#define CONTROL_PERIOD 3000

/** @def pH
 * variable donde se almacena el valor actual del pH
*/
uint32_t pH;

/** @def pH
 * variable tipo BOOL donde se almacena el estado de la humedad en la planta
*/
bool humedad;	//
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

TaskHandle_t ADC_TaskHandle = NULL;
TaskHandle_t Mostrar_TaskHandle = NULL;
TaskHandle_t Control_TaskHandle = NULL;

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

/** @fn Control
 * @brief Esta funcion controla los dispositivos
 * @param pvParameter
 * @param pin vector de estructuras del tipo gpioConf_t
 */
static void Control(void *pvParameter, gpioConf_t pin[]){
	bool humedad_anterior=humedad;
	humedad = GPIORead(pin[0].pin);
	if(humedad!=humedad_anterior){
		if(humedad)
		{GPIOOn(pin[1].pin);
		UartSendString(UART_PC, "Bomba de agua encendida.");}
		else{GPIOOff(pin[1].pin);
		UartSendString(UART_PC, "Bomba de agua apagada.");}
	}
	if(pH<6){
		if(GPIORead(pin[2].pin) == false)
		{GPIOOn(pin[2].pin);
		UartSendString(UART_PC, "Bomba de pH basico encendida.");}
	}
	else
	{
		if(GPIORead(pin[2].pin)){
		GPIOOff(pin[2].pin);
		UartSendString(UART_PC, "Bomba de pH basico apagada.");
	}


	if(pH>6.7){
		if(GPIORead(pin[3].pin) == false)
		{GPIOOn(pin[3].pin);
		UartSendString(UART_PC, "Bomba de pH acido encendida.");}
	}
	else
	{
		if(GPIORead(pin[3].pin)){
		GPIOOff(pin[2].pin);
		UartSendString(UART_PC, "Bomba de pH acido apagada.");
	}
}

/** @fn ADConvert
 * @brief Conversor AD
 * @param pvParameter
 */
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
/** @fn ADConvert
 * @brief Muestra por el puerto serie el estado de las variables
 * @param pvParameter
 */
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
/** @fn FuncTimerADC
 * @brief Funcion del timer del conversor AD
 * @param param
 */
void FuncTimerADC(void *param)
{
	xTaskNotifyGive(ADC_TaskHandle); /*Envia una notificacion*/
}

/** @fn FuncTimerMostrar
 * @brief Funcion del timer utilizado para mostrar por el puerto serie
 * @param param
 */
void FuncTimerMostrar(void *param)
{
	xTaskNotifyGive(Mostrar_TaskHandle); /*Envia una notificacion*/
}

/** @fn FuncTimerControl
 * @brief Funcion del timer para controlar los perifericos
 * @param param
 */
void FuncTimerControl(void *param)
{
	xTaskNotifyGive(Control_TaskHandle); /*Envia una notificacion*/
}

/** @fn getpH
 * @brief A partir de un voltaje dado, devuelve el valor del pH correspondiente
 * @param volt voltaje que corresponde a una medicion de pH
 * @return pH_aux
 */
uint32_t getpH(uint32_t volt)
{
	uint32_t aux = 14 / 3000; // Factor de conversion, segun la relacion 0V-3V con pH de 0 a 14
							  // se utiliza el valor de voltaje en milivotls (mV)
	uint32_t pH_aux = volt * aux;
	return pH_aux;
}

/** @fn Tecla1
 * @brief Funcion para la tecla 1
 */
void Tecla1(timer_config_t timer_ADC, timer_config_t timer_Mostrar, timer_config_t timer_Control)
{
	TimerStart(timer_ADC.timer);
	TimerStart(timer_Mostrar.timer);
	TimerStart(timer_Control.timer);
	UartSendString(UART_PC, "Iniciando el sistema.");
}

/** @fn Tecla1
 * @brief Funcion para la tecla 2
 */
void Tecla2(timer_config_t timer_ADC, timer_config_t timer_Mostrar, timer_config_t timer_Control){
	GPIODeinit();
	TimerReset(timer_ADC.timer);
	TimerReset(timer_Mostrar.timer);
	TimerReset(timer_Control.timer);
	UartSendString(UART_PC, "Deteniendo el sistema.");
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	// Creo el vector de estructuras del tipo gpioConf_t que contienen los puertos
	// con sus correspondiente direccion(entrada/salida)
	gpioConf_t pin[4] =
		{{GPIO_20, GPIO_INPUT},{GPIO_21, GPIO_OUTPUT},{GPIO_22, GPIO_OUTPUT},{GPIO_19, GPIO_OUTPUT}};
	//GPIO20-sensor de humedad
	//GPIO21- bomba de agua
	//GPIO22- bomba de pH basico
	//GPIO23- bomba de pH acido

	timer_config_t timer_ADC = {// Configuracion del timer
								.timer = TIMER_A,
								.period = AD_SAMPLE_PERIOD,
								.func_p = FuncTimerADC,
								.param_p = NULL};

	timer_config_t timer_Mostrar = {// Configuracion del timer
									.timer = TIMER_B,
									.period = SHOW_PERIOD,
									.func_p = FuncTimerMostrar,
									.param_p = NULL};

	timer_config_t timer_Control = {// Configuracion del timer
									.timer = TIMER_C,
									.period = CONTROL_PERIOD,
									.func_p = FuncTimerControl,
									.param_p = NULL};

	serial_config_t puerto_serie = {// Configuracion del puerto serie
									.baud_rate = 115200,
									.port = UART_PC,
									.func_p = NULL,
									.param_p = NULL};


	// Configuracion del AD Convert
	analog_input_config_t config_ADC =
		{
			.input = CH1,		/*!< Inputs: CH0, CH1, CH2, CH3 */
			.mode = ADC_SINGLE, /*!< Mode: single read or continuous read */
			.func_p = NULL,		/*!< Pointer to callback function for convertion end (only for continuous mode) */
			.param_p = NULL,	/*!< Pointer to callback function parameters (only for continuous mode) */
			.sample_frec = 0	/*!< Sample frequency min: 20kHz - max: 2MHz (only for continuous mode)  */
		};
	//Inicializo los switches
	SwitchesInit();

	// Inicializo los puertos GPIO
	for(int i=0; i<4; i++){
	GPIOInit(pin[i].pin, pin[i].dir);}
	//Inicializo los timers
	TimerInit(&timer_ADC);
	TimerInit(&timer_Mostrar);
	TimerInit(&timer_Control);
	// Inicializacion del puerto serie
	UartInit(&puerto_serie);
	// Inicializacion del AD Convert
	AnalogInputInit(&config_ADC);

	SwitchActivInt(SWITCH_1, &Tecla1, NULL);
	SwitchActivInt(SWITCH_2, &Tecla2, NULL);

	xTaskCreate(&ADConvert, "conversor DA", 512, NULL, 5, &ADC_TaskHandle);
    xTaskCreate(&Mostrar, "Mostrar", 4096, NULL, 5, &Mostrar_TaskHandle);
    xTaskCreate(&Control, "Control", 4096, NULL, 5, &Control_TaskHandle);

}
/*==================[end of file]============================================*/