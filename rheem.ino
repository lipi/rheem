//
// Arduino sketch to control Rheem Heat Pump Water Heater
//
// Hardware requirements:
// * Uno board
// * Dallas 1-Wire temperature sensors
// * relays
// * LCD
//

// Arduino libraries
#include <LiquidCrystal.h>

// third party libraries
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PrintEx.h>
#include <Timer.h>

// project files
#include "constants.h"
#include "parameters.h"
#include "selftest.h"
#include "temperature.h"
#include "duty.h"


PrintEx exSerial = Serial;
LiquidCrystal lcd( 8, 9, 4, 5, 6, 7 );
OneWire ow(2);
DallasTemperature sensors(&ow);
Timer timer;
int pulseMsec = 0;

float hexInTemp;
float hexOutTemp;
float hwcTopTemp;
float hwcBottomTemp;

bool pumpOn;
bool compressorOn;
bool heaterOn;

void setup() {
  lcd.begin(16, 2);

  Serial.begin(115200);

  pinMode(pumpPin, OUTPUT);
  pinMode(compressorPin, OUTPUT);
  pinMode(heaterPin, OUTPUT);  
  
  //selfTest();

  sensors.begin();
  sensors.setResolution(HeatExInAddr, 10);
  sensors.setResolution(HeatExOutAddr, 10);
  sensors.setResolution(HWC_TopAddr, 10);
  sensors.setResolution(HWC_BottomAddr, 10);

  timer.every(periodMsec, onEvery, NULL);
}


void startCompressor() {digitalWrite(compressorPin, HIGH); compressorOn = true;}
void stopCompressor() {digitalWrite(compressorPin, LOW); compressorOn = false;}
void startHeater() {digitalWrite(heaterPin, HIGH); heaterOn = true;}
void stopHeater() {digitalWrite(heaterPin, LOW); heaterOn = false;}
void startPump() {setDutyCycle(dutyMinimum); pumpOn = true;}
void stopPump() {digitalWrite(pumpPin, LOW); pumpOn = false;}

void calculateOutputs() {
    
  if (hexInTemp >= 45.0 || hwcBottomTemp >= 50.0) {
    if ( compressorOn || heaterOn) {
      exSerial.printf("Water too hot, stopping all heaters\n");
      stopCompressor();
      stopHeater();
      stopPump();
    }

    // too hot, nothing to do
    return;
  }

  if (hwcBottomTemp < 45.0 && !compressorOn) {
    exSerial.printf("Hot-water cylinder bottom too cold, starting compressor\n");
    startCompressor(); // will take a while to start heating...
    //TODO: start heater if ambient temperature low
    startPump();
    return;
  }

  if (hexOutTemp < 62.0 ) {
    exSerial.printf("Heat exchanger output too cold, reducing pump speed\n");
    // shorter bursts of pumping --> less water to heat --> higher temperature 
    float tempDiff = 62.0 - hexOutTemp;
    float dutyStep = tempDiff * 0.01; // 50 Celsius --> 0.5 step
    decrementDutyCycle(dutyStep);
  }
   
  // if temperature is exactly 62.0 Celsius: do not change 
  
  if (hexOutTemp > 62.0) {
    exSerial.printf("Heat exchanger output too hot, increasing pump speed\n");
    // longer bursts of pumping --> more water to heat --> lower temperature
    float tempDiff = hexOutTemp - 62.0;
    float dutyStep = tempDiff * 0.01; // 50 Celsius --> 0.5 step
    incrementDutyCycle(dutyStep);
    if (hexOutTemp > 70.0) {
      exSerial.printf("Heat exchanger output too hot, stopping all heaters\n");
      stopCompressor();
      stopHeater();
      stopPump();
    }
  }

  //setDutyCycle(0.4);
}

void onEvery(void* context) {
  // first start the pulse, to avoid being affected 
  // by time taken by temperature measurement, printing, etc.
  if (pumpOn) {
    timer.pulseImmediate(pumpPin, pulseMsec, LOW);
  }
  
  measureTemperatures();
  displayTemperatures();
  displayDuty();

  calculateOutputs();
}

void loop() {
  timer.update();
}

