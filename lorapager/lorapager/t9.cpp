#include <RH_RF95.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h> // Hardware-specific library

#include "config.h"
#include "touch.h"

#define TIMEOUT 800

Adafruit_ILI9341 *t9tft;

TouchRegion T9Regions[] = {
  { 2, 82, 78, 138, 1 },
  { 82, 82, 158, 138, 2 },
  { 162, 82, 238, 138, 3 },
  { 2, 142, 78, 198, 4 },
  { 82, 142, 158, 198, 5 },
  { 162, 142, 238, 198, 6 },
  { 2, 202, 78, 258, 7 },
  { 82, 202, 158, 258, 8 },
  { 162, 202, 238, 258, 9 },
  { 2, 262, 78, 318, 10 },
  { 82, 262, 158, 318, 11 },
  { 162, 262, 238, 318, 12 },
  { 0, 0, WIDTH, 80, 20 },
  { 0, 0, 0, 0, 0 }
};

#ifndef POCGTFO_T9

char* buttons[] = {
  "",
  "..!!11--@@##$$%%^^&&**(())__++==\"\"''<<>>\0\0",
  "aAbBcC22\0\0",
  "dDeEfF33\0\0",
  "gGhHiI44\0\0",
  "jJkKlL55\0\0",
  "mMnNoO66\0\0",
  "pPqQrRsS77\0\0",
  "tTuUvV88\0\0",
  "wWxXyYzZ99\0\0",
  "\0\0",
  "  00\0\0",
  "\0\0"
};

char* buttonlabels[] = {
  "1", "2ABC", "3DEF", "4GHI", "5JKL", "6MNO", "7PQRS", "8TUV", "9WXYZ", "SHFT", "0 _", "BKSP"
};

#else

char* buttons[] = {
  "",
  "aAvVwWzZ11\0\0",
  "bBeExX22\0\0",
  "cClL33\0\0",
  "dDhHqQ44\0\0",
  "fFnN55\0\0",
  "gGsS66\0\0",
  "iIpP77\0\0",
  "jJmMuUyY88\0\0",
  "kKrR99\0\0",
  "\0\0",
  "oOtT  00..!!--@@##$$%%^^&&**(())__++==\"\"''<<>>\0\0",
  "\0\0"
};

char* buttonlabels[] = {
  "1AVWZ", "2BEX", "3CL", "4DHQ", "5FN", "6GS", "7IP", "8JMUY", "9KR", "SHFT", "0 OT_", "BKSP"
};

#endif

int activeButton, stri, btni;

long timeout;




char strbuffer[RH_RF95_MAX_MESSAGE_LEN + 1];

void t9init(Adafruit_ILI9341 *tft) {
  t9tft = tft;
}

void t9setBuffer(char* newBuffer) {
  strncpy((char*) strbuffer, (char*) newBuffer, sizeof (strbuffer));
  strbuffer[sizeof (strbuffer) - 1] = 0;
  stri = strlen(strbuffer);
  // TODO: look at this
  strbuffer[stri + 1] = 0;
}

char* t9getBuffer() {
  return strbuffer;
}



void printStr() {
  t9tft->fillRect(0, 20, WIDTH, HEIGHT - 240 - 20, ILI9341_BLACK);
  t9tft->setCursor(0, 20);
  t9tft->setTextColor(ILI9341_WHITE);
  t9tft->setTextSize(2);
  t9tft->println(strbuffer);
}
void t9start() {
  int i;
  activeButton = -1;
  btni = 1;
  setTouchRegions(T9Regions);
  t9tft->fillRect(0, 20, WIDTH, HEIGHT - 20, ILI9341_BLACK);
  t9tft->setTextColor(ILI9341_WHITE);
  t9tft->setTextSize(2);
  for (i = 0; T9Regions[i].id != 20; i++) {
    t9tft->fillRect(T9Regions[i].x1, T9Regions[i].y1,
                 T9Regions[i].x2 - T9Regions[i].x1,
                 T9Regions[i].y2 - T9Regions[i].y1, ILI9341_BLUE);

    t9tft->setCursor(T9Regions[i].x1 + 2, T9Regions[i].y1 + 5);
    t9tft->println(buttonlabels[i]);
  }
  timeout = 0;
  printStr();
}

void bksp() {
  if (stri > 0 && strbuffer[stri] == 0) {
    stri--;
  }
  strbuffer[stri] = 0;
  activeButton = -1;
}

void shift() {
  btni ^= 0x01;
  if (activeButton > 0) {
    strbuffer[stri] = buttons[activeButton][btni];
  }
}

void nextPress() {
  btni += 2;
  if (buttons[activeButton][btni] == 0) {
    btni &= 0x01; // preserve shift
  }
  strbuffer[stri] = buttons[activeButton][btni];
}

void nextChar() {
  stri++;
  strbuffer[stri + 1] = 0;
  activeButton = -1;
}

void newPress() {
  btni &= 0x01;
  strbuffer[stri] = buttons[activeButton][btni];
}

bool t9loop() {
  static int btn;
  if (millis() > timeout && activeButton > 0) {
    activeButton = -1;
    nextChar();
  }
  if ((btn = getTouchRegionId()) > 0) {
    if(btn == 20) {
      return true;
    }
    if(btn != activeButton) {
      if(btn == 10) {
        shift();
      } else if (btn == 12) {
        bksp();
      } else {
        if(activeButton != -1) {
          nextChar();
        }
        activeButton = btn;
        newPress();
      }
    } else {
      nextPress();
    }
    printStr();
    timeout = millis() + TIMEOUT;
  }
  return false;
}

