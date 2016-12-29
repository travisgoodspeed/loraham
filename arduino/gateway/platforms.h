// this file contains platform defines for different platforms that can run the gateway.

#ifndef PLATFORMS_H_
#define PLATFORMS_H_

// for feather32u4
#if defined(FEATHER_32U4)
  #define RFM95_CS 8
  #define RFM95_RST 4
  #define RFM95_INT 7
  #define VBATPIN A9

// for feather m0
#elif defined(FEATHER_M0)
  #define RFM95_CS 8
  #define RFM95_RST 4
  #define RFM95_INT 3
  #define VBATPIN A7

// for shield
#elif defined(SHIELD)
  #define RFM95_CS 10
  #define RFM95_RST 9
  #define RFM95_INT 7

// for ESP w/featherwing
#elif defined(ESP_FEATHERWING)
  #define RFM95_CS  2    // "E"
  #define RFM95_RST 16   // "D"
  #define RFM95_INT 15   // "B"

// Feather 32u4 w/wing
#elif defined(FEATHER32U4_WING)
  #define RFM95_RST     11   // "A"
  #define RFM95_CS      10   // "B"
  #define RFM95_INT     2    // "SDA" (only SDA/SCL/RX/TX have IRQ!)

// Feather m0 w/wing
#elif defined(FEATHERM0_WING)
  #define RFM95_RST     11   // "A"
  #define RFM95_CS      10   // "B"
  #define RFM95_INT     6    // "D"

// Teensy 3.x w/wing
#elif defined(TEENSY3_WING)
  #define RFM95_RST     9   // "A"
  #define RFM95_CS      10   // "B"
  #define RFM95_INT     4    // "C"

#else
  #error "Platform not defined. Please specify a platform in config.h."
#endif

#endif // PLATFORMS_H_
