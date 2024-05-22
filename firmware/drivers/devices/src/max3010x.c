/***************************************************
  This is a library written for the Maxim MAX3010X Optical Smoke Detector
  It should also work with the MAX30102. However, the MAX30102 does not have a Green LED.
  These sensors use I2C to communicate, as well as a single (optional)
  interrupt line that is not currently supported in this driver.
  Written by Peter Jansen and Nathan Seidle (SparkFun)
  BSD license, all text above must be included in any redistribution.
 *****************************************************/

#include "MAX3010X.h"
#include "i2c_mcu.h"
#include "string.h"
#include "delay_mcu.h"


uint8_t activeLEDs; //Gets set during setup. Allows check() to calculate how many bytes to read from FIFO

uint8_t revisionID;

sense_struct sense;

// Status Registers
static const uint8_t MAX3010X_INTSTAT1 =		0x00;
static const uint8_t MAX3010X_INTSTAT2 =		0x01;
static const uint8_t MAX3010X_INTENABLE1 =		0x02;
static const uint8_t MAX3010X_INTENABLE2 =		0x03;

// FIFO Registers
static const uint8_t MAX3010X_FIFOWRITEPTR = 	0x04;
static const uint8_t MAX3010X_FIFOOVERFLOW = 	0x05;
static const uint8_t MAX3010X_FIFOREADPTR = 	0x06;
static const uint8_t MAX3010X_FIFODATA =		0x07;

// Configuration Registers
static const uint8_t MAX3010X_FIFOCONFIG = 		0x08;
static const uint8_t MAX3010X_MODECONFIG = 		0x09;
static const uint8_t MAX3010X_PARTICLECONFIG = 	0x0A;    // Note, sometimes listed as "SPO2" config in datasheet (pg. 11)
static const uint8_t MAX3010X_LED1_PULSEAMP = 	0x0C;
static const uint8_t MAX3010X_LED2_PULSEAMP = 	0x0D;
static const uint8_t MAX3010X_LED3_PULSEAMP = 	0x0E;
static const uint8_t MAX3010X_LED_PROX_AMP = 	0x10;
static const uint8_t MAX3010X_MULTILEDCONFIG1 = 0x11;
static const uint8_t MAX3010X_MULTILEDCONFIG2 = 0x12;

// Die Temperature Registers
static const uint8_t MAX3010X_DIETEMPINT = 		0x1F;
static const uint8_t MAX3010X_DIETEMPFRAC = 	0x20;
static const uint8_t MAX3010X_DIETEMPCONFIG = 	0x21;

// Proximity Function Registers
static const uint8_t MAX3010X_PROXINTTHRESH = 	0x30;

// Part ID Registers
static const uint8_t MAX3010X_REVISIONID = 		0xFE;
static const uint8_t MAX3010X_PARTID = 			0xFF;    // Should always be 0x15. Identical to MAX30102.

// MAX3010X Commands
// Interrupt configuration (pg 13, 14)
static const uint8_t MAX3010X_INT_A_FULL_MASK =		(uint8_t)~0b10000000;
static const uint8_t MAX3010X_INT_A_FULL_ENABLE = 	0x80;
static const uint8_t MAX3010X_INT_A_FULL_DISABLE = 	0x00;

static const uint8_t MAX3010X_INT_DATA_RDY_MASK = (uint8_t)~0b01000000;
static const uint8_t MAX3010X_INT_DATA_RDY_ENABLE =	0x40;
static const uint8_t MAX3010X_INT_DATA_RDY_DISABLE = 0x00;

static const uint8_t MAX3010X_INT_ALC_OVF_MASK = (uint8_t)~0b00100000;
static const uint8_t MAX3010X_INT_ALC_OVF_ENABLE = 	0x20;
static const uint8_t MAX3010X_INT_ALC_OVF_DISABLE = 0x00;

static const uint8_t MAX3010X_INT_PROX_INT_MASK = (uint8_t)~0b00010000;
static const uint8_t MAX3010X_INT_PROX_INT_ENABLE = 0x10;
static const uint8_t MAX3010X_INT_PROX_INT_DISABLE = 0x00;

static const uint8_t MAX3010X_INT_DIE_TEMP_RDY_MASK = (uint8_t)~0b00000010;
static const uint8_t MAX3010X_INT_DIE_TEMP_RDY_ENABLE = 0x02;
static const uint8_t MAX3010X_INT_DIE_TEMP_RDY_DISABLE = 0x00;

static const uint8_t MAX3010X_SAMPLEAVG_MASK =	(uint8_t)~0b11100000;
static const uint8_t MAX3010X_SAMPLEAVG_1 = 	0x00;
static const uint8_t MAX3010X_SAMPLEAVG_2 = 	0x20;
static const uint8_t MAX3010X_SAMPLEAVG_4 = 	0x40;
static const uint8_t MAX3010X_SAMPLEAVG_8 = 	0x60;
static const uint8_t MAX3010X_SAMPLEAVG_16 = 	0x80;
static const uint8_t MAX3010X_SAMPLEAVG_32 = 	0xA0;

static const uint8_t MAX3010X_ROLLOVER_MASK = 	0xEF;
static const uint8_t MAX3010X_ROLLOVER_ENABLE = 0x10;
static const uint8_t MAX3010X_ROLLOVER_DISABLE = 0x00;

static const uint8_t MAX3010X_A_FULL_MASK = 	0xF0;

// Mode configuration commands (page 19)
static const uint8_t MAX3010X_SHUTDOWN_MASK = 	0x7F;
static const uint8_t MAX3010X_SHUTDOWN = 		0x80;
static const uint8_t MAX3010X_WAKEUP = 			0x00;

static const uint8_t MAX3010X_RESET_MASK = 		0xBF;
static const uint8_t MAX3010X_RESET = 			0x40;

static const uint8_t MAX3010X_MODE_MASK = 		0xF8;
static const uint8_t MAX3010X_MODE_REDONLY = 	0x02;
static const uint8_t MAX3010X_MODE_REDIRONLY = 	0x03;
static const uint8_t MAX3010X_MODE_MULTILED = 	0x07;

// Particle sensing configuration commands (pgs 19-20)
static const uint8_t MAX3010X_ADCRANGE_MASK = 	0x9F;
static const uint8_t MAX3010X_ADCRANGE_2048 = 	0x00;
static const uint8_t MAX3010X_ADCRANGE_4096 = 	0x20;
static const uint8_t MAX3010X_ADCRANGE_8192 = 	0x40;
static const uint8_t MAX3010X_ADCRANGE_16384 = 	0x60;

static const uint8_t MAX3010X_SAMPLERATE_MASK = 0xE3;
static const uint8_t MAX3010X_SAMPLERATE_50 = 	0x00;
static const uint8_t MAX3010X_SAMPLERATE_100 = 	0x04;
static const uint8_t MAX3010X_SAMPLERATE_200 = 	0x08;
static const uint8_t MAX3010X_SAMPLERATE_400 = 	0x0C;
static const uint8_t MAX3010X_SAMPLERATE_800 = 	0x10;
static const uint8_t MAX3010X_SAMPLERATE_1000 = 0x14;
static const uint8_t MAX3010X_SAMPLERATE_1600 = 0x18;
static const uint8_t MAX3010X_SAMPLERATE_3200 = 0x1C;

static const uint8_t MAX3010X_PULSEWIDTH_MASK = 0xFC;
static const uint8_t MAX3010X_PULSEWIDTH_69 = 	0x00;
static const uint8_t MAX3010X_PULSEWIDTH_118 = 	0x01;
static const uint8_t MAX3010X_PULSEWIDTH_215 = 	0x02;
static const uint8_t MAX3010X_PULSEWIDTH_411 = 	0x03;

//Multi-LED Mode configuration (pg 22)
static const uint8_t MAX3010X_SLOT1_MASK = 		0xF8;
static const uint8_t MAX3010X_SLOT2_MASK = 		0x8F;
static const uint8_t MAX3010X_SLOT3_MASK = 		0xF8;
static const uint8_t MAX3010X_SLOT4_MASK = 		0x8F;

#define SLOT_NONE  				  0x00
#define SLOT_RED_LED  			0x01
#define SLOT_IR_LED  				0x02
#define SLOT_GREEN_LED  		0x03
#define SLOT_NONE_PILOT  		0x04
#define SLOT_RED_PILOT 			0x05
#define SLOT_IR_PILOT  			0x06
#define SLOT_GREEN_PILOT    0x07

static const uint8_t MAX_30105_EXPECTEDPARTID = 0x15;



bool MAX3010X_begin(void) {

	I2C_initialize(400000);

  // Step 1: Initial Communication and Verification
  // Check that a MAX3010X is connected
  if (MAX3010X_readPartID() != MAX_30105_EXPECTEDPARTID) {
    // Error -- Part ID read from MAX3010X does not match expected part ID.
    // This may mean there is a physical connectivity problem (broken wire, unpowered, etc).
    return false;
  }

  // Populate revision ID
  MAX3010X_readRevisionID();

  return true;
}

//
// Configuration
//

//Begin Interrupt configuration
uint8_t MAX3010X_getINT1(void) {
  return (readRegister8(MAX3010X_INTSTAT1));
}
uint8_t MAX3010X_getINT2(void) {
  return (readRegister8(MAX3010X_INTSTAT2));
}

void MAX3010X_enableAFULL(void) {
  bitMask(MAX3010X_INTENABLE1, MAX3010X_INT_A_FULL_MASK, MAX3010X_INT_A_FULL_ENABLE);
}
void MAX3010X_disableAFULL(void) {
  bitMask(MAX3010X_INTENABLE1, MAX3010X_INT_A_FULL_MASK, MAX3010X_INT_A_FULL_DISABLE);
}

void MAX3010X_enableDATARDY(void) {
  bitMask(MAX3010X_INTENABLE1, MAX3010X_INT_DATA_RDY_MASK, MAX3010X_INT_DATA_RDY_ENABLE);
}
void MAX3010X_disableDATARDY(void) {
  bitMask(MAX3010X_INTENABLE1, MAX3010X_INT_DATA_RDY_MASK, MAX3010X_INT_DATA_RDY_DISABLE);
}

void MAX3010X_enableALCOVF(void) {
  bitMask(MAX3010X_INTENABLE1, MAX3010X_INT_ALC_OVF_MASK, MAX3010X_INT_ALC_OVF_ENABLE);
}
void MAX3010X_disableALCOVF(void) {
  bitMask(MAX3010X_INTENABLE1, MAX3010X_INT_ALC_OVF_MASK, MAX3010X_INT_ALC_OVF_DISABLE);
}

void MAX3010X_enablePROXINT(void) {
  bitMask(MAX3010X_INTENABLE1, MAX3010X_INT_PROX_INT_MASK, MAX3010X_INT_PROX_INT_ENABLE);
}
void MAX3010X_disablePROXINT(void) {
  bitMask(MAX3010X_INTENABLE1, MAX3010X_INT_PROX_INT_MASK, MAX3010X_INT_PROX_INT_DISABLE);
}

void MAX3010X_enableDIETEMPRDY(void) {
  bitMask(MAX3010X_INTENABLE2, MAX3010X_INT_DIE_TEMP_RDY_MASK, MAX3010X_INT_DIE_TEMP_RDY_ENABLE);
}
void MAX3010X_disableDIETEMPRDY(void) {
  bitMask(MAX3010X_INTENABLE2, MAX3010X_INT_DIE_TEMP_RDY_MASK, MAX3010X_INT_DIE_TEMP_RDY_DISABLE);
}

//End Interrupt configuration

void MAX3010X_softReset(void) {
  bitMask(MAX3010X_MODECONFIG, MAX3010X_RESET_MASK, MAX3010X_RESET);

  // Poll for bit to clear, reset is then complete
  // Timeout after 100ms
 // unsigned long startTime = millis();
  unsigned long Time = 0;
  while (Time < 100)
  {
    uint8_t response = readRegister8(MAX3010X_MODECONFIG);
    if ((response & MAX3010X_RESET) == 0) break; //We're done!
    DelayMs(1); //Let's not over burden the I2C bus
    Time++;
  }
}

void MAX3010X_shutDown(void) {
  // Put IC into low power mode (datasheet pg. 19)
  // During shutdown the IC will continue to respond to I2C commands but will
  // not update with or take new readings (such as temperature)
  bitMask(MAX3010X_MODECONFIG, MAX3010X_SHUTDOWN_MASK, MAX3010X_SHUTDOWN);
}

void MAX3010X_wakeUp(void) {
  // Pull IC out of low power mode (datasheet pg. 19)
  bitMask(MAX3010X_MODECONFIG, MAX3010X_SHUTDOWN_MASK, MAX3010X_WAKEUP);
}

void MAX3010X_setLEDMode(uint8_t mode) {
  // Set which LEDs are used for sampling -- Red only, RED+IR only, or custom.
  // See datasheet, page 19
  bitMask(MAX3010X_MODECONFIG, MAX3010X_MODE_MASK, mode);
}

void MAX3010X_setADCRange(uint8_t adcRange) {
  // adcRange: one of MAX3010X_ADCRANGE_2048, _4096, _8192, _16384
  bitMask(MAX3010X_PARTICLECONFIG, MAX3010X_ADCRANGE_MASK, adcRange);
}

void MAX3010X_setSampleRate(uint8_t sampleRate) {
  // sampleRate: one of MAX3010X_SAMPLERATE_50, _100, _200, _400, _800, _1000, _1600, _3200
  bitMask(MAX3010X_PARTICLECONFIG, MAX3010X_SAMPLERATE_MASK, sampleRate);
}

void MAX3010X_setPulseWidth(uint8_t pulseWidth) {
  // pulseWidth: one of MAX3010X_PULSEWIDTH_69, _188, _215, _411
  bitMask(MAX3010X_PARTICLECONFIG, MAX3010X_PULSEWIDTH_MASK, pulseWidth);
}

// NOTE: Amplitude values: 0x00 = 0mA, 0x7F = 25.4mA, 0xFF = 50mA (typical)
// See datasheet, page 21
void MAX3010X_setPulseAmplitudeRed(uint8_t amplitude) {
  writeRegister8(MAX3010X_LED1_PULSEAMP, amplitude);
}

void MAX3010X_setPulseAmplitudeIR(uint8_t amplitude) {
  writeRegister8(MAX3010X_LED2_PULSEAMP, amplitude);
}

void MAX3010X_setPulseAmplitudeGreen(uint8_t amplitude) {
  writeRegister8(MAX3010X_LED3_PULSEAMP, amplitude);
}

void MAX3010X_setPulseAmplitudeProximity(uint8_t amplitude) {
  writeRegister8(MAX3010X_LED_PROX_AMP, amplitude);
}

void MAX3010X_setProximityThreshold(uint8_t threshMSB) {
  // Set the IR ADC count that will trigger the beginning of particle-sensing mode.
  // The threshMSB signifies only the 8 most significant-bits of the ADC count.
  // See datasheet, page 24.
  writeRegister8(MAX3010X_PROXINTTHRESH, threshMSB);
}

//Given a slot number assign a thing to it
//Devices are SLOT_RED_LED or SLOT_RED_PILOT (proximity)
//Assigning a SLOT_RED_LED will pulse LED
//Assigning a SLOT_RED_PILOT will ??
void MAX3010X_enableSlot(uint8_t slotNumber, uint8_t device) {

  uint8_t originalContents;

  switch (slotNumber) {
    case (1):
      bitMask(MAX3010X_MULTILEDCONFIG1, MAX3010X_SLOT1_MASK, device);
      break;
    case (2):
      bitMask(MAX3010X_MULTILEDCONFIG1, MAX3010X_SLOT2_MASK, device << 4);
      break;
    case (3):
      bitMask(MAX3010X_MULTILEDCONFIG2, MAX3010X_SLOT3_MASK, device);
      break;
    case (4):
      bitMask(MAX3010X_MULTILEDCONFIG2, MAX3010X_SLOT4_MASK, device << 4);
      break;
    default:
      //Shouldn't be here!
      break;
  }
}

//Clears all slot assignments
void MAX3010X_disableSlots(void) {
  writeRegister8(MAX3010X_MULTILEDCONFIG1, 0);
  writeRegister8(MAX3010X_MULTILEDCONFIG2, 0);
}

//
// FIFO Configuration
//

//Set sample average (Table 3, Page 18)
void MAX3010X_setFIFOAverage(uint8_t numberOfSamples) {
  bitMask(MAX3010X_FIFOCONFIG, MAX3010X_SAMPLEAVG_MASK, numberOfSamples);
}

//Resets all points to start in a known state
//Page 15 recommends clearing FIFO before beginning a read
void MAX3010X_clearFIFO(void) {
  writeRegister8(MAX3010X_FIFOWRITEPTR, 0);
  writeRegister8(MAX3010X_FIFOOVERFLOW, 0);
  writeRegister8(MAX3010X_FIFOREADPTR, 0);
}

//Enable roll over if FIFO over flows
void MAX3010X_enableFIFORollover(void) {
  bitMask(MAX3010X_FIFOCONFIG, MAX3010X_ROLLOVER_MASK, MAX3010X_ROLLOVER_ENABLE);
}

//Disable roll over if FIFO over flows
void MAX3010X_disableFIFORollover(void) {
  bitMask(MAX3010X_FIFOCONFIG, MAX3010X_ROLLOVER_MASK, MAX3010X_ROLLOVER_DISABLE);
}

//Set number of samples to trigger the almost full interrupt (Page 18)
//Power on default is 32 samples
//Note it is reverse: 0x00 is 32 samples, 0x0F is 17 samples
void MAX3010X_setFIFOAlmostFull(uint8_t numberOfSamples) {
  bitMask(MAX3010X_FIFOCONFIG, MAX3010X_A_FULL_MASK, numberOfSamples);
}

//Read the FIFO Write Pointer
uint8_t MAX3010X_getWritePointer(void) {
  return (readRegister8(MAX3010X_FIFOWRITEPTR));
}

//Read the FIFO Read Pointer
uint8_t MAX3010X_getReadPointer(void) {
  return (readRegister8(MAX3010X_FIFOREADPTR));
}


// Die Temperature
// Returns temp in C
float MAX3010X_readTemperature() {

  //DIE_TEMP_RDY interrupt must be enabled
  //See issue 19: https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library/issues/19

  // Step 1: Config die temperature register to take 1 temperature sample
  writeRegister8(MAX3010X_DIETEMPCONFIG, 0x01);

  // Poll for bit to clear, reading is then complete
  // Timeout after 100ms
  unsigned long startTime = 0;
  while (startTime < 100)
  {
    //uint8_t response = readRegister8(_i2caddr, MAX3010X_DIETEMPCONFIG); //Original way
    //if ((response & 0x01) == 0) break; //We're done!

	//Check to see if DIE_TEMP_RDY interrupt is set
	uint8_t response = readRegister8(MAX3010X_INTSTAT2);
    if ((response & MAX3010X_INT_DIE_TEMP_RDY_ENABLE) > 0) break; //We're done!
    DelayMs(1); //Let's not over burden the I2C bus
    startTime++;
  }
  //TODO How do we want to fail? With what type of error?
  //? if(millis() - startTime >= 100) return(-999.0);

  // Step 2: Read die temperature register (integer)
  int8_t tempInt = readRegister8(MAX3010X_DIETEMPINT);
  uint8_t tempFrac = readRegister8(MAX3010X_DIETEMPFRAC); //Causes the clearing of the DIE_TEMP_RDY interrupt

  // Step 3: Calculate temperature (datasheet pg. 23)
  return (float)tempInt + ((float)tempFrac * 0.0625);
}

// Returns die temp in F
float MAX3010X_readTemperatureF() {
  float temp = MAX3010X_readTemperature();

  if (temp != -999.0) temp = temp * 1.8 + 32.0;

  return (temp);
}

// Set the PROX_INT_THRESHold
void MAX3010X_setPROXINTTHRESH(uint8_t val) {
  writeRegister8(MAX3010X_PROXINTTHRESH, val);
}


//
// Device ID and Revision
//
uint8_t MAX3010X_readPartID() {
  return readRegister8(MAX3010X_PARTID);
}

void MAX3010X_readRevisionID() {
  revisionID = readRegister8(MAX3010X_REVISIONID);
}

uint8_t MAX3010X_getRevisionID() {
  return revisionID;
}


//Setup the sensor
//The MAX3010X has many settings. By default we select:
// Sample Average = 4
// Mode = MultiLED
// ADC Range = 16384 (62.5pA per LSB)
// Sample rate = 50
//Use the default setup if you are just getting started with the MAX3010X sensor
void MAX3010X_setup(uint8_t powerLevel, uint8_t sampleAverage, uint8_t ledMode, int sampleRate, int pulseWidth, int adcRange) {
	MAX3010X_softReset(); //Reset all configuration, threshold, and data registers to POR values

  //FIFO Configuration
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //The chip will average multiple samples of same type together if you wish
  if (sampleAverage == 1) MAX3010X_setFIFOAverage(MAX3010X_SAMPLEAVG_1); //No averaging per FIFO record
  else if (sampleAverage == 2) MAX3010X_setFIFOAverage(MAX3010X_SAMPLEAVG_2);
  else if (sampleAverage == 4) MAX3010X_setFIFOAverage(MAX3010X_SAMPLEAVG_4);
  else if (sampleAverage == 8) MAX3010X_setFIFOAverage(MAX3010X_SAMPLEAVG_8);
  else if (sampleAverage == 16) MAX3010X_setFIFOAverage(MAX3010X_SAMPLEAVG_16);
  else if (sampleAverage == 32) MAX3010X_setFIFOAverage(MAX3010X_SAMPLEAVG_32);
  else MAX3010X_setFIFOAverage(MAX3010X_SAMPLEAVG_4);

  //setFIFOAlmostFull(2); //Set to 30 samples to trigger an 'Almost Full' interrupt
  MAX3010X_enableFIFORollover(); //Allow FIFO to wrap/roll over
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  //Mode Configuration
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  if (ledMode == 3) MAX3010X_setLEDMode(MAX3010X_MODE_MULTILED); //Watch all three LED channels
  else if (ledMode == 2) MAX3010X_setLEDMode(MAX3010X_MODE_REDIRONLY); //Red and IR
  else MAX3010X_setLEDMode(MAX3010X_MODE_REDONLY); //Red only
  activeLEDs = ledMode; //Used to control how many uint8_ts to read from FIFO buffer
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  //Particle Sensing Configuration
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  if(adcRange < 4096) MAX3010X_setADCRange(MAX3010X_ADCRANGE_2048); //7.81pA per LSB
  else if(adcRange < 8192) MAX3010X_setADCRange(MAX3010X_ADCRANGE_4096); //15.63pA per LSB
  else if(adcRange < 16384) MAX3010X_setADCRange(MAX3010X_ADCRANGE_8192); //31.25pA per LSB
  else if(adcRange == 16384) MAX3010X_setADCRange(MAX3010X_ADCRANGE_16384); //62.5pA per LSB
  else MAX3010X_setADCRange(MAX3010X_ADCRANGE_2048);

  if (sampleRate < 100) MAX3010X_setSampleRate(MAX3010X_SAMPLERATE_50); //Take 50 samples per second
  else if (sampleRate < 200) MAX3010X_setSampleRate(MAX3010X_SAMPLERATE_100);
  else if (sampleRate < 400) MAX3010X_setSampleRate(MAX3010X_SAMPLERATE_200);
  else if (sampleRate < 800) MAX3010X_setSampleRate(MAX3010X_SAMPLERATE_400);
  else if (sampleRate < 1000) MAX3010X_setSampleRate(MAX3010X_SAMPLERATE_800);
  else if (sampleRate < 1600) MAX3010X_setSampleRate(MAX3010X_SAMPLERATE_1000);
  else if (sampleRate < 3200) MAX3010X_setSampleRate(MAX3010X_SAMPLERATE_1600);
  else if (sampleRate == 3200) MAX3010X_setSampleRate(MAX3010X_SAMPLERATE_3200);
  else MAX3010X_setSampleRate(MAX3010X_SAMPLERATE_50);

  //The longer the pulse width the longer range of detection you'll have
  //At 69us and 0.4mA it's about 2 inches
  //At 411us and 0.4mA it's about 6 inches
  if (pulseWidth < 118) MAX3010X_setPulseWidth(MAX3010X_PULSEWIDTH_69); //Page 26, Gets us 15 bit resolution
  else if (pulseWidth < 215) MAX3010X_setPulseWidth(MAX3010X_PULSEWIDTH_118); //16 bit resolution
  else if (pulseWidth < 411) MAX3010X_setPulseWidth(MAX3010X_PULSEWIDTH_215); //17 bit resolution
  else if (pulseWidth == 411) MAX3010X_setPulseWidth(MAX3010X_PULSEWIDTH_411); //18 bit resolution
  else MAX3010X_setPulseWidth(MAX3010X_PULSEWIDTH_69);
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  //LED Pulse Amplitude Configuration
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //Default is 0x1F which gets us 6.4mA
  //powerLevel = 0x02, 0.4mA - Presence detection of ~4 inch
  //powerLevel = 0x1F, 6.4mA - Presence detection of ~8 inch
  //powerLevel = 0x7F, 25.4mA - Presence detection of ~8 inch
  //powerLevel = 0xFF, 50.0mA - Presence detection of ~12 inch

  MAX3010X_setPulseAmplitudeRed(powerLevel);
  MAX3010X_setPulseAmplitudeIR(powerLevel);
  MAX3010X_setPulseAmplitudeGreen(powerLevel);
  MAX3010X_setPulseAmplitudeProximity(powerLevel);
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  //Multi-LED Mode Configuration, Enable the reading of the three LEDs
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  MAX3010X_enableSlot(1, SLOT_RED_LED);
  if (ledMode > 1) MAX3010X_enableSlot(2, SLOT_IR_LED);
  if (ledMode > 2) MAX3010X_enableSlot(3, SLOT_GREEN_LED);
  //enableSlot(1, SLOT_RED_PILOT);
  //enableSlot(2, SLOT_IR_PILOT);
  //enableSlot(3, SLOT_GREEN_PILOT);
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  MAX3010X_clearFIFO(); //Reset the FIFO before we begin checking the sensor
}

//
// Data Collection
//

//Tell caller how many samples are available
uint8_t MAX3010X_available(void)
{
  int8_t numberOfSamples = sense.head - sense.tail;
  if (numberOfSamples < 0) numberOfSamples += STORAGE_SIZE;

  return (numberOfSamples);
}

//Report the most recent red value
uint32_t MAX3010X_getRed(void)
{
  //Check the sensor for new data for 250ms
  if(MAX3010X_safeCheck(250))
    return (sense.red[sense.head]);
  else
    return(0); //Sensor failed to find new data
}

//Report the most recent IR value
uint32_t MAX3010X_getIR(void)
{
  //Check the sensor for new data for 250ms
  if(MAX3010X_safeCheck(250))
    return (sense.IR[sense.head]);
  else
    return(0); //Sensor failed to find new data
}

//Report the most recent Green value
uint32_t MAX3010X_getGreen(void)
{
  //Check the sensor for new data for 250ms
  if(MAX3010X_safeCheck(250))
    return (sense.green[sense.head]);
  else
    return(0); //Sensor failed to find new data
}

//Report the next Red value in the FIFO
uint32_t MAX3010X_getFIFORed(void)
{
  return (sense.red[sense.tail]);
}

//Report the next IR value in the FIFO
uint32_t MAX3010X_getFIFOIR(void)
{
  return (sense.IR[sense.tail]);
}

//Report the next Green value in the FIFO
uint32_t MAX3010X_getFIFOGreen(void)
{
  return (sense.green[sense.tail]);
}

//Advance the tail
void MAX3010X_nextSample(void)
{
  if(MAX3010X_available()) //Only advance the tail if new data is available
  {
    sense.tail++;
    sense.tail %= STORAGE_SIZE; //Wrap condition
  }
}

//Polls the sensor for new data
//Call regularly
//If new data is available, it updates the head and tail in the main struct
//Returns number of new samples obtained
uint16_t MAX3010X_check(void)
{
  //Read register FIDO_DATA in (3-uint8_t * number of active LED) chunks
  //Until FIFO_RD_PTR = FIFO_WR_PTR

  uint8_t readPointer = MAX3010X_getReadPointer();
  uint8_t writePointer = MAX3010X_getWritePointer();

  int numberOfSamples = 0;

  //Do we have new data?
  if (readPointer != writePointer)
  {
    //Calculate the number of readings we need to get from sensor
    numberOfSamples = writePointer - readPointer;
    if (numberOfSamples < 0) numberOfSamples += 32; //Wrap condition

    //We now have the number of readings, now calc uint8_ts to read
    //For this example we are just doing Red and IR (3 uint8_ts each)
    int uint8_tsLeftToRead = numberOfSamples * activeLEDs * 3;

    //Get ready to read a burst of data from the FIFO register
    uint8_t transmit_data_buffer;
    transmit_data_buffer = MAX3010X_FIFODATA;
    I2C_SelectRegister(MAX30105_ADDRESS, transmit_data_buffer);
   // _i2cPort->beginTransmission(MAX3010X_ADDRESS);
   // _i2cPort->write(MAX3010X_FIFODATA);
   // _i2cPort->endTransmission();

    //We may need to read as many as 288 uint8_ts so we read in blocks no larger than I2C_BUFFER_LENGTH
    //I2C_BUFFER_LENGTH changes based on the platform. 64 uint8_ts for SAMD21, 32 uint8_ts for Uno.
    //Wire.requestFrom() is limited to BUFFER_LENGTH which is 32 on the Uno
    while (uint8_tsLeftToRead > 0)
    {
      int toGet = uint8_tsLeftToRead;
      if (toGet > I2C_BUFFER_LENGTH)
      {
        //If toGet is 32 this is bad because we read 6 uint8_ts (Red+IR * 3 = 6) at a time
        //32 % 6 = 2 left over. We don't want to request 32 uint8_ts, we want to request 30.
        //32 % 9 (Red+IR+GREEN) = 5 left over. We want to request 27.

        toGet = I2C_BUFFER_LENGTH - (I2C_BUFFER_LENGTH % (activeLEDs * 3)); //Trim toGet to be a multiple of the samples we need to read
      }

      uint8_tsLeftToRead -= toGet;

      //Request toGet number of uint8_ts from sensor
      //_i2cPort->requestFrom(MAX3010X_ADDRESS, toGet);
      uint8_t data_to_read_buffer[32];
      uint8_t data = 0;
      //data_to_read_buffer = reg;
      I2C_requestBytes(MAX30105_ADDRESS, toGet, data_to_read_buffer, 0);

      while (toGet > 0)
      {
        sense.head++; //Advance the head of the storage struct
        sense.head %= STORAGE_SIZE; //Wrap condition

        uint8_t temp[sizeof(uint32_t)]; //Array of 4 uint8_ts that we will convert into long
        uint32_t tempLong;

        //Burst read three uint8_ts - RED
        temp[3] = 0;
        temp[2] = data_to_read_buffer[0];
        temp[1] = data_to_read_buffer[1];
        temp[0] = data_to_read_buffer[2];

        //Convert array to long
        memcpy(&tempLong, temp, sizeof(tempLong));

		tempLong &= 0x3FFFF; //Zero out all but 18 bits

        sense.red[sense.head] = tempLong; //Store this reading into the sense array

        if (activeLEDs > 1)
        {
          //Burst read three more uint8_ts - IR
          temp[3] = 0;
          temp[2] = data_to_read_buffer[3];
          temp[1] = data_to_read_buffer[4];
          temp[0] = data_to_read_buffer[5];

          //Convert array to long
          memcpy(&tempLong, temp, sizeof(tempLong));

		  tempLong &= 0x3FFFF; //Zero out all but 18 bits

		  sense.IR[sense.head] = tempLong;
        }

        if (activeLEDs > 2)
        {
          //Burst read three more uint8_ts - Green
          temp[3] = 0;
          temp[2] = data_to_read_buffer[6];
          temp[1] = data_to_read_buffer[7];
          temp[0] = data_to_read_buffer[8];

          //Convert array to long
          memcpy(&tempLong, temp, sizeof(tempLong));

		  tempLong &= 0x3FFFF; //Zero out all but 18 bits

          sense.green[sense.head] = tempLong;
        }

        toGet -= activeLEDs * 3;
      }

    } //End while (uint8_tsLeftToRead > 0)

  } //End readPtr != writePtr

  return (numberOfSamples); //Let the world know how much new data we found
}

//Check for new data but give up after a certain amount of time
//Returns true if new data was found
//Returns false if new data was not found
bool MAX3010X_safeCheck(uint8_t maxTimeToCheck)
{
  uint32_t markTime = 0;

  while(1)
  {
	if(markTime > maxTimeToCheck) return(false);

	if(MAX3010X_check() == true) //We found new data!
	  return(true);
	markTime++;
	DelayMs(1);
  }
}

//Given a register, read it, mask it, and then set the thing
void bitMask(uint8_t reg, uint8_t mask, uint8_t thing)
{
  // Grab current register context
  uint8_t originalContents = readRegister8(reg);

  // Zero-out the portions of the register we're interested in
  originalContents = originalContents & mask;

  // Change contents
  writeRegister8(reg, originalContents | thing);
}

//
// Low-level I2C Communication
//
uint8_t readRegister8(uint8_t reg) {

  uint8_t data=0;
 
  I2C_readByte(MAX30105_ADDRESS, reg, &data, 0 );

  return data; //

}

void writeRegister8(uint8_t reg, uint8_t value) {
  I2C_writeByte(MAX30105_ADDRESS, reg, value); //
}


