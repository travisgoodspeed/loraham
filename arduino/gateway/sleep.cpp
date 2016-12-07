#include <RTCZero.h>
#include "config.h"
#include "platforms.h"

// we want to not deep sleep for this amount of time
// or we risk not being able to use serial
#define SAFETY_MILLIS 120000

#ifdef RTC_ENABLED
RTCZero rtc;

bool wokebyrtc = false;
#endif

int lastmillis;

void sleepsetup() {
#ifdef RTC_ENABLED
  rtc.begin();

  // https://github.com/arduino/ArduinoCore-samd/issues/142

  // Enable the DFLL48M clock in standby mode
  SYSCTRL->VREG.bit.RUNSTDBY = 1;
  SYSCTRL->DFLLCTRL.bit.RUNSTDBY = 1;

  USBDevice.detach();
#else
  lastmillis = millis();
#endif
}

#ifdef RTC_ENABLED
void setwokebyrtc() {
  wokebyrtc = true;
}
#endif

void sleepreset() {
#ifndef RTC_ENABLED
  // no RTC
  lastmillis = millis();
#else
  // RTC
  rtc.setTime(0,0,0);
  /* yeah, this is kind of a hack at the moment. one day it could be
   * cool to use the RTC to keep current time.
   */
#endif
}

// returns true when seconds is elapsed
// returns false if we're not using RTC or if some other interrupt
// woke up the processor
bool sleep(unsigned int seconds) {
#ifndef RTC_ENABLED
  if (millis()-lastmillis > seconds * 1000) {
    return true;
  }
  return false;
#else
  int h, m, s;
  h = seconds / 3600;
  m = (seconds - h * 3600) / 60;
  s = seconds - h * 3600 - m * 60;
  rtc.setAlarmTime(h, m, s);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);
  rtc.attachInterrupt(setwokebyrtc);
  wokebyrtc = false;
#ifdef DEBUG_LED_RTC
  digitalWrite(LED, LOW);
#endif
  rtc.standbyMode();
#ifdef DEBUG_LED_RTC
  digitalWrite(LED, HIGH);
#endif
  return wokebyrtc;
#endif
}




