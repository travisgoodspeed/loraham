
#include <RTCZero.h>


#include "config.h"
#include "platforms.h"

#ifdef RTC_ENABLED
RTCZero rtc;
bool wokebyrtc = false;
uint32_t sleeptime;
#else
int lastmillis;
#endif

void sleepsetup() {
#ifdef RTC_ENABLED
  rtc.begin();

  // https://github.com/arduino/ArduinoCore-samd/issues/142

  // Enable the DFLL48M clock in standby mode
  SYSCTRL->VREG.bit.RUNSTDBY = 1;
  SYSCTRL->DFLLCTRL.bit.RUNSTDBY = 1;

  USBDevice.detach();

  rtc.setY2kEpoch(0);
  sleeptime = 0;
#else
  lastmillis = millis();
#endif
}

#ifdef RTC_ENABLED
void setwokebyrtc() {
  wokebyrtc = true;
}

uint16_t getepoch() {
  return rtc.getY2kEpoch();
}
#endif

void sleepreset() {
#ifndef RTC_ENABLED
  // no RTC
  lastmillis = millis();
#else
  // RTC
  sleeptime = rtc.getEpoch();
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
  if (rtc.getEpoch() >= sleeptime + seconds) {
    return true;
  }
  rtc.setAlarmEpoch(sleeptime + seconds + 1); // + 1, to prevent race conditions just in case
  wokebyrtc = false;
  rtc.attachInterrupt(setwokebyrtc);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);
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




