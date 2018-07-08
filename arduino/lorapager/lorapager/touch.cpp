

#include "config.h"
#include "touch.h"

#include <SPI.h>
#include <Wire.h>      // this is needed even tho we aren't using it
#include <Adafruit_STMPE610.h>

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 3800
#define TS_MAXX 250
#define TS_MINY 180
#define TS_MAXY 3640


Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

TouchRegion *regions;

#define REGISTER_TIME 30
#define DEBOUNCE_TIME 50

boolean touching = false;
boolean registered = false;
long resetTime = 0, registerTime = 0;


void touchInit() {
  if (!ts.begin()) {
    Serial.println("Couldn't start touchscreen controller");
    while (1);
  }
}

void setTouchRegions(TouchRegion *newRegions) {
  regions = newRegions;
}

int getTouchRegionId() {

  TouchRegion *region = regions;
  unsigned char i;
  int id = -1;
  static uint16_t tx, ty, x, y;
  static uint8_t tz;

  while(!ts.bufferEmpty()) {
    ts.readData(&tx, &ty, &tz);
  }
  x = map(tx, TS_MINX, TS_MAXX, 0, WIDTH);
  y = map(ty, TS_MINY, TS_MAXY, 0, HEIGHT);

  if (touching && !registered && millis() > registerTime) {
    id = 0;

    Serial.print(x);
    Serial.print(" ");
    Serial.println(y);
    for(i = 0; region[i].id != 0; i++) {
      if(x >= region[i].x1 && x <= region[i].x2 &&
           y >= region[i].y1 && y <= region[i].y2) {
        id = region[i].id;
        Serial.print("Selected ");
        Serial.println(id);
        break;
      }
    }
    registered = true;
  }
  if (ts.touched()) {
    resetTime = millis() + DEBOUNCE_TIME;

    if (!touching) {
      Serial.println("touch on");
      touching = true;
      registered = false;
      registerTime = millis() + REGISTER_TIME;
    }
    ts.writeRegister8(STMPE_INT_STA, 0xFF); // reset all ints
  } else  if (touching && millis() > resetTime) {

      Serial.println("touch off");

    touching = false;
  }
  return id;
}

