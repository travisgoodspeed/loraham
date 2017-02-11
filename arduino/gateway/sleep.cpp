
//Install this with the Library Manager, then include it with the menu item.
#include <RTCZero.h> 

#include "config.h"
#include "platforms.h"

#define TIMER_SLOTS 2

#ifdef RTC_ENABLED
RTCZero rtc;
bool wokebyrtc = false;
uint32_t sleeptime[TIMER_SLOTS];
#else
uint32_t lastmillis[TIMER_SLOTS];
#endif

// "Timerslots" are ways for us to keep track of multiple durations of time.
// For example, we can use one timerslot for sleeping during a transmit cycle,
// and not have to worry about it resetting the timer for the beacon

void sleepsetup() {
#ifdef RTC_ENABLED
  rtc.begin();

  // https://github.com/arduino/ArduinoCore-samd/issues/142

  // Enable the DFLL48M clock in standby mode
  SYSCTRL->VREG.bit.RUNSTDBY = 1;
  SYSCTRL->DFLLCTRL.bit.RUNSTDBY = 1;

  USBDevice.detach();

  rtc.setY2kEpoch(0);
#endif
}

#ifdef RTC_ENABLED
void setwokebyrtc() {
  wokebyrtc = true;
}

uint32_t getepoch() {
  return rtc.getY2kEpoch();
}
#endif

void sleepreset(char timerslot) {
#ifndef RTC_ENABLED
  // no RTC
  lastmillis[timerslot] = millis();
#else
  // RTC
  sleeptime[timerslot] = rtc.getEpoch();
#endif
}

// returns true when seconds is elapsed
// returns false if we're not using RTC or if some other interrupt
// woke up the processor
bool sleep(unsigned int seconds, char timerslot) {
#ifndef RTC_ENABLED
  if (millis()-lastmillis[timerslot] > seconds * 1000) {
    return true;
  }
  return false;
#else
  if (rtc.getEpoch() >= sleeptime[timerslot] + seconds) {
    return true;
  }
  rtc.setAlarmEpoch(sleeptime[timerslot] + seconds + 1); // + 1, to prevent race conditions just in case
  wokebyrtc = false;
  rtc.attachInterrupt(setwokebyrtc);
  rtc.enableAlarm(rtc.MATCH_MMSS); // if something goes wrong, recover in 1 hr
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
