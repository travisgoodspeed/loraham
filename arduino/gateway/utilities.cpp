
#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <Arduino.h>

#include "config.h"
#include "platforms.h"
#include "utilities.h"
#include "sleep.h"

//Returns the battery voltage as a float.
float voltage() {
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  return measuredvbat;
}

//! Uptime in seconds, correcting for rollover.
long int uptime() {
#ifdef RTC_ENABLED
  return getepoch();
#else
  static unsigned long rollover = 0;
  static unsigned long lastmillis = millis();

  //Account for rollovers, every ~50 days or so.
  if (lastmillis > millis()) {
    rollover += (lastmillis >> 10);
    lastmillis = millis();
  }

  return (rollover + (millis() >> 10));
#endif
}

// returns true if supplied packet should be retransmitted
bool shouldrt(uint8_t *buf) {
#ifdef REPEATER_ENABLED
  //Don't RT any packet containing our own callsign.
  if (strcasestr((char*) buf, CALLSIGN)) {
    //Serial.println("I've already retransmitted this one.\n");
    return false;
  }
  //No objections.  RT it!
  return true;
#else
  //Repeater is disabled, so we RT nothing.
  return false;
#endif
}

#endif // UTILITIES_H_
