
#include <LiquidCrystal.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <PrintEx.h>

#include "constants.h"

extern DallasTemperature sensors;
extern LiquidCrystal lcd;
extern PrintEx exSerial;

extern float hexInTemp;
extern float hexOutTemp;
extern float hwcTopTemp;
extern float hwcBottomTemp;


void measureTemperatures() {
  sensors.requestTemperatures(); 

  //TODO: use average of last N readings
  
  hexInTemp = sensors.getTempC(HeatExInAddr);
  hexOutTemp = sensors.getTempC(HeatExOutAddr);
  hwcTopTemp = sensors.getTempC(HWC_TopAddr);
  hwcBottomTemp = sensors.getTempC(HWC_BottomAddr);
}

void displayTemperatures() {
  exSerial.printf("Heat Exchanger in/out : %.2f/%.2f Celsius  ", hexInTemp, hexOutTemp);
  exSerial.printf("Hot Water Cylinder top/bottom: %.2f/%.2f Celsius\n", hwcTopTemp, hwcBottomTemp);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("HEX:");
  //lcd.print(hwcBottomTemp); lcd.print(" ");
  //lcd.print(hwcTopTemp); lcd.print(" ");
  lcd.print(hexInTemp); lcd.print(" ");
  lcd.print(hexOutTemp);
}
