
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

bool sane(double temperature) {
  return (temperature > 0.0 && temperature < 100.0);
}

void measureTemperatures() {
  int samples = 5;
  
  hexInTemp = 0.0;
  hexOutTemp = 0.0;
  hwcTopTemp = 0.0;
  hwcBottomTemp = 0.0;

  for (int i = 0; i < samples; ) {
    sensors.requestTemperatures();

    double hexIn = sensors.getTempC(HeatExInAddr); 
    double hexOut = sensors.getTempC(HeatExOutAddr);
    double hwcTop = sensors.getTempC(HWC_TopAddr);
    double hwcBot = sensors.getTempC(HWC_BottomAddr);
    if (sane(hexIn) && sane(hexOut) && sane(hwcTop) && sane(hwcBot)) {
      hexInTemp += hexIn;
      hexOutTemp += hexOut;
      hwcTopTemp += hwcTop;
      hwcBottomTemp += hwcBot;
      i++;
    }
    else {
      continue;
    }
  }
  
  hexInTemp /= (double)samples;
  hexOutTemp /= (double)samples;
  hwcTopTemp /= (double)samples;
  hwcBottomTemp /= (double)samples;
}

void displayTemperatures() {
  exSerial.printf("HEX in/out: %.2f/%.2f C ", hexInTemp, hexOutTemp);
  exSerial.printf("HWC top/bottom: %.2f/%.2f C\n", hwcTopTemp, hwcBottomTemp);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("HEX:");
  //lcd.print(hwcBottomTemp); lcd.print(" ");
  //lcd.print(hwcTopTemp); lcd.print(" ");
  lcd.print(hexInTemp); lcd.print(" ");
  lcd.print(hexOutTemp);
}
