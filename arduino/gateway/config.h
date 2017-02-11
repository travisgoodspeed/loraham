// this file contains user defines (callsign, platform, etc).

#ifndef CONFIG_H_
#define CONFIG_H_

//Uncomment these and change them to match your hardware.
//#define CALLSIGN "MYCALLSIGN-5"
//#define BANNER "SomeRadio solar digipeater mAh=1234"


// Define your hardware type both here and in Toos/Board.
#define FEATHER_M0


#if !defined(CALLSIGN)
#error Please set your callsign and node ID above!
#endif

#if !defined(BANNER)
#error Please set a banner
#endif
// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 434.0

// LED to use.
// TODO Move this to platforms.h
#define LED 13

// LED debug options
// #define DEBUG_LED_XMIT
#define DEBUG_LED_RTC

// use RTC.  This helps power consumption, but totally bricks USB.
//#define RTC_ENABLED

// Repeat incoming packets.
// Only use this for nodes with good antenna placement.
#define REPEATER_ENABLED


// Periodic beaconing.
#define BEACON_PERIODIC
#define BEACON_PERIOD (60 * 10 * 1000) // ms
#define BEACON_PERIOD_LOWBATT (60 * 20 * 1000) // ms

#define LOWBATT_WAIT_PERIOD (60 * 20 * 1000) // ms

// ring buffer size in packets
#define BUFFER_PACKETS 10

// max xmit wait - we'll wait between 0 and n milliseconds before transmitting to avoid collision
#define MAX_XMIT_WAIT 10000

// These voltage thresholds are used for power management.
#define RADIO_CONTINUOUS_VOLTAGE 3.9  // Begin repeating above this voltage.
#define MIN_XMIT_VOLTAGE 3.75         // Beneath this voltage, we only beacon.
#define ONLY_CHARGE_VOLTAGE 3.65      // Beneath this voltage, we only charge to conserve all possible power.

#endif // CONFIG_H_

