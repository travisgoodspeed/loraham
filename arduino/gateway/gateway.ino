/* This is a beacon for the LoRaHam protocol by KK4VCZ.
   https://github.com/travisgoodspeed/loraham/

*/


#include <SPI.h>
#include <RH_RF95.h>  //See http://www.airspayce.com/mikem/arduino/RadioHead/

#include "config.h"
#include "platforms.h"

#include "utilities.h"
#include "network.h"

#include "sleep.h"

#define MODE_OFF 0
#define MODE_CONTINUOUS 1
#define MODE_XMIT_ONLY 2
#define MODE_LOWBATT 3

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
  sleepsetup();
}

void loop() {
  // mode changes
  switch(mode) {
    case MODE_OFF:
      sleepreset(0);
      if(voltage() < MIN_XMIT_VOLTAGE) {
        radiooff();
        mode = MODE_LOWBATT;
      } else {
        radioon();
        if(voltage() > RADIO_CONTINUOUS_VOLTAGE) {
          beacon("Powered on!");
          xmitstack();
          mode = MODE_CONTINUOUS;
        } else {
          beacon("Powered on! Transmit only!");
          xmitstack();
          radiooff();
          mode = MODE_XMIT_ONLY;
        }
      }
      break;
    case MODE_CONTINUOUS:
      if(voltage() < MIN_XMIT_VOLTAGE) {
        beacon("Entering transmit only mode.");
        xmitstack();
        radiooff();
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
      recvpkt();
      while(handlepackets()) {
        xmitstack();
        recvpkt();
      }
#ifdef BEACON_PERIODIC
      if(sleep(BEACON_PERIOD / 1000, 0)) {
        beacon("");
        xmitstack();
        sleepreset(0);
      }
#endif
      delay(20);
      break;
    case MODE_XMIT_ONLY:
      if(sleep(BEACON_PERIOD_LOWBATT / 1000, 0)) {
#ifdef BEACON_PERIODIC
        beacon("Transmit only!");
        radioon();
        xmitstack();
        radiooff();
        sleepreset(0);
#endif
      }
      delay(20);
      break;
    case MODE_LOWBATT:
      while(!sleep(LOWBATT_WAIT_PERIOD / 1000, 0)) {}
      sleepreset(0);
      break;
  }
}

