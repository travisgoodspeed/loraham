/* This is a beacon for the LoRaHam protocol by KK4VCZ.
   https://github.com/travisgoodspeed/loraham/

*/


#include <SPI.h>
#include <RH_RF95.h>  //See http://www.airspayce.com/mikem/arduino/RadioHead/

#include "config.h"
#include "platforms.h"

#include "utilities.h"
#include "network.h"

#define MODE_OFF 0
#define MODE_CONTINUOUS 1
#define MODE_XMIT_ONLY 2
#define MODE_LOWBATT 3

unsigned long lastbeacon = 0; // var for last beacon time
unsigned char mode = MODE_OFF;

void setup() {
  // configure LED and pins
  pinMode(LED, OUTPUT);

  // configure serial
  delay(1000);
  Serial.begin(9600);
  Serial.setTimeout(10);
  delay(100);
  digitalWrite(LED, LOW);

  // set up the radio
  radiosetup(); // network.cpp
}


void loop() {
  // mode changes
  switch(mode) {
    case MODE_OFF: // turn radio on, probably
      if(voltage() < ONLY_CHARGE_VOLTAGE) {
        mode = MODE_LOWBATT;
      } else {
        radioon();
        if(voltage() > RADIO_CONTINUOUS_VOLTAGE) {
          beacon("Powered on!");
          mode = MODE_CONTINUOUS;
        } else {
          beacon("Powered on! Transmit only!");
          xmitstack();
          radiooff();
          mode = MODE_XMIT_ONLY;
        }
        lastbeacon = millis();
      }
      break;
    case MODE_CONTINUOUS:
      if(voltage() < MIN_XMIT_VOLTAGE) {
        beacon("Entering transmit only mode.");
        xmitstack();
        radiooff();
        lastbeacon = millis();
        mode = MODE_XMIT_ONLY;
      }
      break;
    case MODE_XMIT_ONLY:
      if(voltage() > RADIO_CONTINUOUS_VOLTAGE) {
        radioon();
        mode = MODE_CONTINUOUS;
      } else if (voltage() < ONLY_CHARGE_VOLTAGE) {
        beacon("Low battery! Turning radio off.");
        xmitstack();
        radiooff();
        mode = MODE_LOWBATT;
      }
      break;
    case MODE_LOWBATT:
      if(voltage() > MIN_XMIT_VOLTAGE) {
        radioon();
        beacon("Waking from charge mode. Transmit only!");
        xmitstack();
        radiooff();
        mode = MODE_XMIT_ONLY;
      }
      break;
  }

  switch (mode) {
    case MODE_CONTINUOUS:
#ifdef BEACON_PERIODIC
      if(millis()-lastbeacon>BEACON_PERIOD) {
        beacon("");
        lastbeacon = millis();
      }
#endif
      recvpkt();
      handlepackets();
      xmitstack();
      delay(20);
      break;
    case MODE_XMIT_ONLY:
      delay(BEACON_PERIOD_LOWBATT);
#ifdef BEACON_PERIODIC
      radioon();
      beacon("Transmit only");
      xmitstack();
      radiooff();
#endif
      break;
    case MODE_LOWBATT:
      delay(LOWBATT_WAIT_PERIOD);
      break;
  }
}
