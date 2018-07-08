
#include <SPI.h>
#include <Wire.h>      // this is needed even tho we aren't using it

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h> // Hardware-specific library
#include <Adafruit_STMPE610.h>
#include "config.h"
#include "ui.h"
#include "touch.h"
#include "packet.h"
#include "t9.h"
#include "util.h"

#include "radio.h"

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define PREV 1
#define NEXT 2
#define NEW 3
#define NEW_W_TO 4
#define COMPOSE_TO 5
#define COMPOSE_MSG 6
#define COMPOSE_CANCEL 7
#define COMPOSE_SEND 8

uint8_t state;

#define UI_STATE_MESSAGES_SETUP 0
#define UI_STATE_MESSAGES_LOOP 1
#define UI_STATE_COMPOSE_SETUP 2
#define UI_STATE_COMPOSE_LOOP 3
#define UI_STATE_COMPOSE_TO_SETUP 4
#define UI_STATE_COMPOSE_TO_LOOP 5
#define UI_STATE_COMPOSE_MSG_SETUP 6
#define UI_STATE_COMPOSE_MSG_LOOP 7


void (*sendFromRadio)(Packet msg);

Packet pktbuffer[BACKLOG];
unsigned int buffersize = 0, bufferpos = 0, showbuffer = 0;

char msgTo[256], msgBody[256];

TouchRegion defRegions[] = {
  { 2, 246, 78, 318, PREV },
  { 82, 246, 158, 318, NEXT },
  { 162, 246, 238, 318, NEW },
  { 0, 20, WIDTH, 70, NEW_W_TO },
  { 0, 0, 0, 0, 0 }
};

TouchRegion composeRegions[] = {
  { 0, 25, WIDTH, 75, COMPOSE_TO },
  { 0, 100, WIDTH, 280, COMPOSE_MSG },
  { 0, 282, WIDTH/2-2, HEIGHT, COMPOSE_CANCEL },
  { WIDTH/2+2, 282, WIDTH, HEIGHT, COMPOSE_SEND },
  { 0, 0, 0, 0, 0}
};

void statusBar() {
  static char statusLine[20];
  float vcc;
  tft.fillRect(0, 0, tft.width(), 20, ILI9341_BLACK);
  tft.setCursor(0,0);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("LoRaHam");
  vcc = voltage();
  tft.setCursor(WIDTH / 2, 0);
  snprintf(statusLine, 19, "%d.%03dV", (int) vcc, (int) (vcc*1000)%1000);
  tft.println(statusLine);
  
}

void showMsg() {
  Serial.println("show msg");
  Packet *packet;
  packet = &pktbuffer[showbuffer];
  tft.fillRect(0, 20, tft.width(), 226, ILI9341_BLACK);
  tft.setCursor(0, 20);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.println((char*) packet->data);
}

void drawDefaultUI() {
  unsigned char i;
  char* labels[] = {"<", ">", "N"};
  tft.fillRect(0, 20, WIDTH, HEIGHT-20, ILI9341_BLACK);
  tft.setTextSize(4);
  tft.setTextColor(ILI9341_WHITE);
  for (i = 0; defRegions[i].id != NEW_W_TO; i++) {
    tft.fillRect(defRegions[i].x1, defRegions[i].y1,
                 defRegions[i].x2 - defRegions[i].x1,
                 defRegions[i].y2 - defRegions[i].y1, ILI9341_BLUE);

    tft.setCursor(defRegions[i].x1 + 10, defRegions[i].y1 + 5);
    tft.println(labels[i]);
  }
  if (buffersize > 0) {
    showMsg();
  }
}

void drawComposeUI() {
  unsigned char i;
  char* composelabels[] = {"", "", "Cancel", "Send!"};
  tft.fillRect(0, 20, WIDTH, HEIGHT-20, ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(0, 25);
  tft.println("To:");
  tft.setCursor(24, 25);
  tft.setTextSize(2);
  tft.println(msgTo);
  tft.setCursor(0, 100);
  tft.setTextSize(2);
  tft.println(msgBody);
  
  for (i = 2; composeRegions[i].id != 0; i++) {
    tft.fillRect(composeRegions[i].x1, composeRegions[i].y1,
                 composeRegions[i].x2 - composeRegions[i].x1,
                 composeRegions[i].y2 - composeRegions[i].y1, ILI9341_YELLOW);
    tft.setTextColor(ILI9341_BLACK);
    tft.setCursor(composeRegions[i].x1 + 10, composeRegions[i].y1 + 5);
    tft.println(composelabels[i]);
  }
  setTouchRegions(composeRegions);
}

void newMsg(Packet packet) {
  Serial.println("new msg");
  memcpy(&pktbuffer[bufferpos], &packet, sizeof (Packet));
  Serial.println((char*) packet.data);
  bufferpos = (bufferpos + 1) % BACKLOG;
  showbuffer = (BACKLOG + bufferpos - 1) % BACKLOG;
  if (buffersize < BACKLOG) {
    buffersize++;
  }
  if (state == UI_STATE_MESSAGES_LOOP) {
    showMsg();
  }
}

void onRecvMsg(Packet packet) {
  newMsg(packet);
}

void messageScreenLoop() {
  switch (getTouchRegionId()) {
    case PREV:
      if (showbuffer > 0 || buffersize == BACKLOG) {
        showbuffer = (BACKLOG + showbuffer - 1) % BACKLOG;
        showMsg();
      }
      break;
    case NEXT:
      if (BACKLOG+showbuffer < (BACKLOG+bufferpos-1) || buffersize == BACKLOG) {
        showbuffer = (BACKLOG + showbuffer + 1) % BACKLOG;
        showMsg();
      }
      break;
    case NEW:
      msgTo[0] = 0;
      msgBody[0] = 0;
      state = UI_STATE_COMPOSE_SETUP;
      break;
    case NEW_W_TO:
      msgBody[0] = 0;
      if (getSender(pktbuffer[showbuffer], msgTo, 256)) {
        state = UI_STATE_COMPOSE_SETUP;
      }
      break;
    default:
    break;
  }
}

void uiLoop() {
  static char msgToSend[RH_RF95_MAX_MESSAGE_LEN + 1];
  Packet packetToSend;
  switch(state) {
    case UI_STATE_MESSAGES_SETUP:
      drawDefaultUI();
      setTouchRegions(defRegions);
      state = UI_STATE_MESSAGES_LOOP;
    case UI_STATE_MESSAGES_LOOP:
      messageScreenLoop();
      break;
    case UI_STATE_COMPOSE_SETUP:
      drawComposeUI();
      state = UI_STATE_COMPOSE_LOOP;
    case UI_STATE_COMPOSE_LOOP:
      switch(getTouchRegionId()) {
        case COMPOSE_TO:
          state = UI_STATE_COMPOSE_TO_SETUP; break;
        case COMPOSE_MSG:
          state = UI_STATE_COMPOSE_MSG_SETUP; break;
        case COMPOSE_CANCEL:
          state = UI_STATE_MESSAGES_SETUP; break;
        case COMPOSE_SEND:
          if(msgTo[0] != 0 && msgBody[0] != 0) {
            snprintf((char*)packetToSend.data, RH_RF95_MAX_MESSAGE_LEN, "%s %s %s", msgTo, CALLSIGN, msgBody);
            packetToSend.rssi = -1;
            newMsg(packetToSend); // register packet in ui
            sendPacket(packetToSend); // send to air
            state = UI_STATE_MESSAGES_SETUP;
          }
          break;
        default: break;    
      }
      break;
    case UI_STATE_COMPOSE_TO_SETUP:
      t9setBuffer(msgTo);
      t9start();
      state = UI_STATE_COMPOSE_TO_LOOP;
    case UI_STATE_COMPOSE_TO_LOOP:
      if(t9loop()) {
        strncpy(msgTo, t9getBuffer(), sizeof (msgTo) - 1);
        state = UI_STATE_COMPOSE_SETUP;
      }
      break;
    case UI_STATE_COMPOSE_MSG_SETUP:
      t9setBuffer(msgBody);
      t9start();
      state = UI_STATE_COMPOSE_MSG_LOOP;
    case UI_STATE_COMPOSE_MSG_LOOP:
      if(t9loop()) {
        strncpy(msgBody, t9getBuffer(), sizeof (msgBody) - 1);
        state = UI_STATE_COMPOSE_SETUP;
      }
      break;
    default: break;
  }
}

void uiSetup() {
  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
  t9init(&tft);
  statusBar();
  touchInit();

  state = UI_STATE_MESSAGES_SETUP;
  uiLoop();
}


  
