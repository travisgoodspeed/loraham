#include "radio.h"
#include "ui.h"

void setup() {
  Serial.begin(9600);

  delay(10);
  uiSetup();
  radioSetup();
}

void loop() {
  uiLoop();
  radioLoop();
}
