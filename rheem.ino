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
#include <PID_v1.h>

// project files
#include "constants.h"
#include "selftest.h"
#include "temperature.h"
#include "duty.h"

#define REVISION 1


PrintEx exSerial = Serial;
LiquidCrystal lcd( 8, 9, 4, 5, 6, 7 );
OneWire ow(2);
DallasTemperature sensors(&ow);
Timer timer;

unsigned int pulseMsec = 0;

double hexInTemp;
double hexOutTemp;
double hwcTopTemp;
double hwcBottomTemp;

bool pumpOn;
bool heaterOn;

// helpers to implement recycle time for compressor
unsigned long compressorStopTime = millis();
int recycleEvent = NO_TIMER_AVAILABLE;
enum State {
  stopped,
  starting,
  started
};
State compressorState = stopped;

// PID variables
double dutyCycle = dutyMinimum;
double targetTemp = targetTemperature;
double kP = 0.1; // 0.0 disables the component
double kI = 0.01;
double kD = 0.5;

PID pid(&hexOutTemp, // input
        &dutyCycle, // output
        &targetTemp, // setpoint
        kP, kI, kD,
        P_ON_E,
        REVERSE);

void onEvery(void* context);
void recycleCallback(void* context);

void setup() {
  lcd.begin(16, 2);

  Serial.begin(115200);

  pinMode(pumpPin, OUTPUT);
  pinMode(compressorPin, OUTPUT);
  pinMode(heaterPin, OUTPUT);  

  // uncomment following line to find sensor addresses and test outputs
  //selfTest();

  sensors.begin();
  sensors.setResolution(HeatExInAddr, 10);
  sensors.setResolution(HeatExOutAddr, 10);
  sensors.setResolution(HWC_TopAddr, 10);
  sensors.setResolution(HWC_BottomAddr, 10);
  sensors.setWaitForConversion(false);

  timer.every(periodMsec, onEvery, NULL);

  pid.SetOutputLimits(dutyMinimum, dutyMaximum);
  pid.SetSampleTime(periodMsec);
  pid.SetMode(AUTOMATIC);

  stopPump();
  stopCompressor();
  stopHeater();
}

void startCompressor() {
  if (stopped == compressorState) {
    if (compressorStopTime + recycleTimeMsec < millis() ) {
      digitalWrite(compressorPin, HIGH);
      exSerial.printf("Compressor started\n");
      compressorState = started;
    }
    else {
      unsigned long holdoff = recycleTimeMsec - (millis() - compressorStopTime);
      recycleEvent = timer.after(holdoff, recycleCallback, NULL);
      compressorState = starting;
      exSerial.printf("Compressor starting\n");
    }
  }
  else {
    // nothing to do
  }
}

void recycleCallback(void* context) {
  digitalWrite(compressorPin, HIGH);
  compressorState = started; 
  exSerial.printf("Compressor started\n");
}

void stopCompressor() {
  if (stopped != compressorState) {
    digitalWrite(compressorPin, LOW);
    compressorState = stopped;
    compressorStopTime = millis();
    if (recycleEvent >= 0) {
      recycleEvent = timer.stop(recycleEvent);
    }
    exSerial.printf("Compressor stopped\n");
  }
}
  
void startHeater() {digitalWrite(heaterPin, LOW); heaterOn = true;}
void stopHeater() {digitalWrite(heaterPin, HIGH); heaterOn = false;}
void startPump() {setDutyCycle(dutyMinimum); pumpOn = true;}
void stopPump() {digitalWrite(pumpPin, HIGH); pumpOn = false; pulseMsec = 0;}

void stopAllHeaters() {
      exSerial.printf("Stopping all heaters\n");
      stopCompressor();
      stopHeater();
      stopPump();
}

void calculateOutputs() {
    
  if (hexInTemp >= hexInTempMax || hwcBottomTemp >= hwcBottomTempMax ) {
    exSerial.printf("Cylinder water too hot\n");
    if ( compressorState != stopped || heaterOn) {
      stopAllHeaters();
    }

    // too hot, nothing to do
    return;
  }

  // safety override
  if (hexOutTemp > hexOutTempMax ) {
    exSerial.printf("Heat exchanger output too hot\n");
    if (compressorState != stopped || heaterOn) {
      stopAllHeaters();
    }
  
    // too hot, nothing to do
    return;
  }

  if (hwcBottomTemp < hwcBottomTempMin && stopped == compressorState) {
    exSerial.printf("Hot-water cylinder bottom too cold, starting compressor\n");
    startCompressor(); // will take a while to start heating...
    //TODO: start heater if ambient temperature low
    startPump();
    return;
  }

  pid.Compute(); // where all the magic happens...
  
  if (started == compressorState) {
    setDutyCycle(dutyCycle);
  }
}

void onEvery(void* context) {

  // first start the pulse, to avoid being affected 
  // by time taken by temperature measurement, printing, etc.
  if (pumpOn) {
    timer.pulseImmediate(pumpPin, pulseMsec, LOW);
  }

  measureTemperatures();
  displayTemperatures();
#if 0
  if ( hexInTemp < 0 || hexOutTemp < 0 || hwcBottomTemp < 0 || hwcTopTemp < 0) {
    // likely sensor error (-127C), so stop everything
    // TODO: display error code
    stopAllHeaters();
    return;
  }
#endif
  calculateOutputs();

  displayDuty();
}

void getParameters() {
  char type = Serial.read();
  float value = Serial.parseFloat();
  exSerial.printf("%c = %.3f\n", type, value);
  switch (type) {
    case 'p': kP = (double)value; break;
    case 'i': kI = (double)value; break;
    case 'd': kD = (double)value; break;
    case 't': targetTemp = (double)value; break;
    case '?': break; // print current values
    case 'h': break; // print current values
    default:
      exSerial.printf("Error: %c is not a valid parameter\n", type);
      return;
  }
  pid.SetTunings(kP, kI, kD);
  exSerial.printf("kP = %.3f kI = %.3f kD = %.3f target: %.1f C\n", kP, kI, kD, targetTemp);
}

void loop() {
  timer.update();

  if (Serial.available() > 0) {
    getParameters();
  }
}

