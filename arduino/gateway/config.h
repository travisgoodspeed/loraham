// this file contains user defines (callsign, platform, etc).

#ifndef CONFIG_H_
#define CONFIG_H_

#define CALLSIGN "AB3TL-5"
#define FEATHER_M0 // hardware type

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 434.0

// LED to use
#define LED 13

// Periodic beacon enable
#define BEACON_PERIODIC
#define BEACON_PERIOD 60 * 60 // ms

// ring buffer size in packets
#define BUFFER_PACKETS 10

// max xmit wait - we'll wait between 0 and n milliseconds before transmitting to avoid collision
#define MAX_XMIT_WAIT 10000

// minimum voltages for functions
#define LISTEN_VOLTAGE 3.85  // hysteresis
#define ONLY_BEACON_VOLTAGE 3.75
#define ONLY_CHARGE_VOLTAGE 3.6

#endif // CONFIG_H_
