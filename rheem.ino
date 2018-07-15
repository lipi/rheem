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

#define REVISION 1

PrintEx exSerial = Serial;
LiquidCrystal lcd( 8, 9, 4, 5, 6, 7 );
OneWire ow(2);
DallasTemperature sensors(&ow);

DeviceAddress HeatExInAddr = { 0x28, 0xFF, 0xA7, 0x67, 0x70, 0x17, 0x03, 0xB3 }; // A1-2
DeviceAddress HeatExOutAddr = { 0x28, 0xFF, 0x8C, 0x25, 0x71, 0x17, 0x03, 0x77 }; // A1-4
DeviceAddress HWC_TopAddr = { 0x28, 0xFF, 0x56, 0xE9, 0x70, 0x17, 0x03, 0x14 }; // A1-1
DeviceAddress HWC_BottomAddr = { 0x28, 0xFF, 0x70, 0x20, 0x71, 0x17, 0x03, 0x71 }; // A1-3

// outputs
const int compressorPin = 10;
const int ledPin = 13;
const int heaterPin = 3;

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
  testOutput(compressorPin, "compressor relay");
  testOutput(heaterPin, "heater relay");
  testOutput(ledPin, "LED");
}
  
void setup() {
  lcd.begin(16, 2);

  Serial.begin(9600);

  pinMode(compressorPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(heaterPin, OUTPUT);  
  
  selfTest();

  sensors.begin();
  sensors.setResolution(HeatExInAddr, 10);
  sensors.setResolution(HeatExOutAddr, 10);
  sensors.setResolution(HWC_TopAddr, 10);
  sensors.setResolution(HWC_BottomAddr, 10);
}

void printTemperatures(DallasTemperature sensors) {
  sensors.requestTemperatures(); 
  float hexIn = sensors.getTempC(HeatExInAddr);
  float hexOut = sensors.getTempC(HeatExOutAddr);
  float hwcTop = sensors.getTempC(HWC_TopAddr);
  float hwcBottom = sensors.getTempC(HWC_BottomAddr);
  exSerial.printf("Heat Exchanger in/out : %.2f/%.2f Celsius  ", hexIn, hexOut);
  exSerial.printf("Hot Water Cylinder top/bottom: %.2f/%.2f Celsius\n", hwcTop, hwcBottom);
}

void loop() {
    printTemperatures(sensors);  
    delay(1000);
}
