//
// Arduino sketch to control Rheem Heat Pump Water Heater
//
// Hardware requirements:
// * Uno board
// * Dallas 1-Wire temperature sensors
// * relays
// * LCD
//
// Software libraries used:
// * OneWire
// * DallasTemperature
// * PrintEx

// Arduino libraries
#include <LiquidCrystal.h>

// third party libraries
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PrintEx.h>
#include <Timer.h>

#define REVISION 1

// heat exchanger inpu and output tempreature sensors
DeviceAddress HeatExInAddr = { 0x28, 0xFF, 0xA7, 0x67, 0x70, 0x17, 0x03, 0xB3 }; // A1-2
DeviceAddress HeatExOutAddr = { 0x28, 0xFF, 0x8C, 0x25, 0x71, 0x17, 0x03, 0x77 }; // A1-4

// hot-water cylinder top and bottom temperature sensors
DeviceAddress HWC_TopAddr = { 0x28, 0xFF, 0x56, 0xE9, 0x70, 0x17, 0x03, 0x14 }; // A1-1
DeviceAddress HWC_BottomAddr = { 0x28, 0xFF, 0x70, 0x20, 0x71, 0x17, 0x03, 0x71 }; // A1-3

// outputs
const int heaterPin = 3;      // relay
const int compressorPin = 10; // relay
const int pumpPin = 13;       // solid-state relay

PrintEx exSerial = Serial;
LiquidCrystal lcd( 8, 9, 4, 5, 6, 7 );
OneWire ow(2);
DallasTemperature sensors(&ow);
Timer timer;
const float dutyMinimum = 0.01;
const int periodMsec = 5000;
int pulseMsec = 0;

float hexInTemp;
float hexOutTemp;
float hwcTopTemp;
float hwcBottomTemp;

bool pumpOn;
bool compressorOn;
bool heaterOn;

uint8_t findDevices(OneWire ow) {
  uint8_t address[8];
  uint8_t count = 0;

  Serial.println("OneWire devices found:");
  if (ow.search(address)) {
    do {
      count++;
      Serial.print("{ ");
      for (uint8_t i = 0; i < 8; i++) {
        Serial.print("0x");
        if (address[i] < 0x10) Serial.print("0");
        Serial.print(address[i], HEX);
        if (i < 7) Serial.print(", ");
      }
      Serial.println(" }");
    } while (ow.search(address));
  }
  return count;
}

void testOutput(int pin, const char* name) {
  exSerial.printf("Testing %s: on for 1 second\n", name);
  digitalWrite(pin, HIGH);
  delay(1000);
  exSerial.printf("Testing %s: off for 1 second\n", name);
  digitalWrite(pin, LOW);
  delay(1000);
}

void selfTest() {
  // test 0: serial port
  exSerial.printf("========================\n");
  exSerial.printf("RHEEM Controller rev %d\n", REVISION);
  exSerial.printf("========================\n");

  // test 1: LCD
  lcd.print("RHEEM Controller");
  lcd.setCursor(5,1);
  lcd.print("rev:");
  lcd.print(REVISION);
    
  // test 2: temperature sensors
  int count = findDevices(ow);
  exSerial.printf("%d of %d devices found: %s\n",  count, 4, count == 4 ? "OK" : "FAIL");

  // test 3: relays
  testOutput(pumpPin, "LED");
  testOutput(compressorPin, "compressor relay");
  testOutput(heaterPin, "heater relay");
}

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

void measureTemperatures() {
  sensors.requestTemperatures(); 

  //TODO: use average of last N readings
  
  hexInTemp = sensors.getTempC(HeatExInAddr);
  hexOutTemp = sensors.getTempC(HeatExOutAddr);
  hwcTopTemp = sensors.getTempC(HWC_TopAddr);
  hwcBottomTemp = sensors.getTempC(HWC_BottomAddr);
  
  exSerial.printf("Heat Exchanger in/out : %.2f/%.2f Celsius  ", hexInTemp, hexOutTemp);
  exSerial.printf("Hot Water Cylinder top/bottom: %.2f/%.2f Celsius\n", hwcTopTemp, hwcBottomTemp);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("HEX:");
  //lcd.print(hwcBottomTemp); lcd.print(" ");
  //lcd.print(hwcTopTemp); lcd.print(" ");
  lcd.print(hexInTemp); lcd.print(" ");
  lcd.print(hexOutTemp);

  lcd.setCursor(0,1);
  lcd.print("Duty:");
  lcd.print((float)pulseMsec / (float)periodMsec);
}


void setDutyCycle(float dutyCycle) {
  // pump may never completely stop, otherwise 
  // couldn't measure the output of the heat exchanger
  if (dutyCycle < dutyMinimum) {dutyCycle = dutyMinimum;}
  if (dutyCycle > 1.0) {dutyCycle = 1.0;}
  pulseMsec = periodMsec * dutyCycle;
  exSerial.printf("pump duty cycle: %.2f (%d of %d msec)\n", dutyCycle, pulseMsec, periodMsec);
}

void incrementDutyCycle(float step) {
  if (step < 0.0) {step = 0.0;}
  if (step > 1.0) {step = 1.0;}
  float dutyCycle = (float)pulseMsec / (float)periodMsec;
  dutyCycle = dutyCycle + step;
  setDutyCycle(dutyCycle);
}

void decrementDutyCycle(float step) {
  if (step < 0.0) {step = 0.0;}
  if (step > 1.0) {step = 1.0;}
  float dutyCycle = (float)pulseMsec / (float)periodMsec;
  dutyCycle = dutyCycle - step;
  setDutyCycle(dutyCycle);
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
  calculateOutputs();
}

void loop() {
  timer.update();
}

