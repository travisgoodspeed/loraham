/* This is a beacon for the LoRaHam protocol by KK4VCZ.
   https://github.com/travisgoodspeed/loraham/

*/


#include <SPI.h>
#include <RH_RF95.h>  //See http://www.airspayce.com/mikem/arduino/RadioHead/

#include "config.h"
#include "platforms.h"

#include "utilities.h"
#include "network.h"

unsigned long lastbeacon = 0; // var for last beacon time

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

  //Beacon once at startup (if we're going to have an active loop)
  if (voltage() > RADIO_CONTINUOUS_VOLTAGE) {
    radioon();
    beacon("POWERED ON");
  }

  // enable periodic beacon task
#ifdef BEACON_PERIODIC
#endif
}


void loop() {
  // if battery is low we turn off the radio and do not digipeat
  if (voltage() < RADIO_CONTINUOUS_VOLTAGE) {
    radiooff();
  }
  else {
    radioon();
    recvpkt();
    handlepackets();
    xmitstack();
  }


  // If battery is above MIN_XMIT_VOLTAGE we beacon periodically.
  // If battery is below RADIO_CONTINUOUS_VOLTAGE we use a reduced beacon period and delay() to reduce battery waste.

#ifdef BEACON_PERIODIC
  if(voltage() > RADIO_CONTINUOUS_VOLTAGE) {
    if(millis() - lastbeacon > BEACON_PERIOD) {
      // radio is already on since voltage is high enough
      beacon("XMIT ONLY");
      xmitstack();
      lastbeacon = millis();
    }
  }
  else if(voltage() > MIN_XMIT_VOLTAGE) {
    // turn on radio
    radioon();
    // transmit a beacon
      beacon("XMIT ONLY");
      xmitstack();
      lastbeacon = millis();
    // since voltage is too low for digipeat, turn radio off and delay by low battery period
    radiooff();
    delay(BEACON_PERIOD_LOWBATT);
  }
#endif // BEACON_PERIODIC

  if(voltage() < MIN_XMIT_VOLTAGE) {
    delay(LOWBATT_WAIT_PERIOD);
  }
}
