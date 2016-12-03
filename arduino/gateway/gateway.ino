/* This is a beacon for the LoRaHam protocol by KK4VCZ.
   https://github.com/travisgoodspeed/loraham/

*/


#include <SPI.h>
#include <Scheduler.h>
#include <RH_RF95.h>  //See http://www.airspayce.com/mikem/arduino/RadioHead/

#include "config.h"
#include "platforms.h"

#include "utilities.h"
#include "network.h"


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
  if (voltage() > LISTEN_VOLTAGE) {
    beacon("POWERED ON");
  }

  // enable periodic beacon task
#ifdef BEACON_PERIODIC
  Scheduler.startLoop(beaconloop);
#endif
}


void loop() {
  // FSM for digipeater
  // if battery is low we do not digipeat
  if (voltage() > LISTEN_VOLTAGE) {
    radioon();
    recvpkt();
    handlepackets();
    xmitstack();
    delay(1000);
  }
}

void beaconloop()
{
  if(voltage() > ONLY_BEACON_VOLTAGE) {
    // xmit a beacon
    radioon();
    beacon("XMIT ONLY");
    xmitstack();
    radiooff();
  }
  delay(BEACON_PERIOD);
}

