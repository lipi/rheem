
#include <LiquidCrystal.h>
#include <PrintEx.h>

#include "parameters.h"

extern LiquidCrystal lcd;
extern PrintEx exSerial;

extern const float dutyMinimum;
extern float dutyCycle;
extern const int periodMsec;
extern int pulseMsec;

void displayDuty() {
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
