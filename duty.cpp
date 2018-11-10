
#include <LiquidCrystal.h>
#include <PrintEx.h>

#include "constants.h"

extern LiquidCrystal lcd;
extern PrintEx exSerial;

extern const float dutyMinimum;
extern const unsigned int periodMsec;
extern unsigned int pulseMsec;

void displayDuty() {
  lcd.setCursor(0,1);
  lcd.print("Duty:");
  lcd.print((float)pulseMsec / (float)periodMsec);
}

void setDutyCycle(float dutyCycle) {
  // pump may never completely stop, otherwise 
  // couldn't measure the output of the heat exchanger
  if (dutyCycle < dutyMinimum) {dutyCycle = dutyMinimum;}
  if (dutyCycle > dutyMaximum) {dutyCycle = dutyMaximum;}
  pulseMsec = (int)(periodMsec * dutyCycle);
  exSerial.printf("%.3f pump duty cycle: %.2f (%d of %u msec)\n",
  millis()/1000.0, dutyCycle, pulseMsec, periodMsec);
}

