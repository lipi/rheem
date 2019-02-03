
#include <OneWire.h>
#include <LiquidCrystal.h>
#include <PrintEx.h>

#include "constants.h"

extern LiquidCrystal lcd;
extern PrintEx exSerial;
extern OneWire ow;
extern DallasTemperature sensors;

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
  digitalWrite(pin, LOW);
  delay(1000);
  exSerial.printf("Testing %s: off for 1 second\n", name);
  digitalWrite(pin, HIGH);
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
  testOutput(selectorPin, "selector relay");
}
