
#include "config.h"
#include "radio.h"

#include "ui.h"

#include <RH_RF95.h>  //See http://www.airspayce.com/mikem/arduino/RadioHead/

RH_RF95 rf95(RFM95_CS, RFM95_INT);

void radioSetup() {
  // radioinit
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    delay(10000);
  }
  if (!rf95.setFrequency(FREQ)) {
    while (1);
  }

  rf95.setTxPower(23, false);
  rf95.setModeRx();
  noInterrupts();
}

void sendPacket(Packet p) {
  interrupts();
  rf95.send(p.data, strlen((char*) p.data));
  noInterrupts();
}

void radioLoop() {
  interrupts();
  uint8_t len = RH_RF95_MAX_MESSAGE_LEN;
  static Packet packet;
  
  static uint8_t data[RH_RF95_MAX_MESSAGE_LEN + 1];
  while(rf95.available()) {
    Serial.println("packet available");
    if (rf95.recv(packet.data, &len)) {
      Serial.println("got packet");
      packet.data[len] = 0;
      packet.rssi = rf95.lastRssi();
      Serial.println("calling onrecvmsg");
      noInterrupts();
      onRecvMsg(packet);
      interrupts();
    }
  }
  noInterrupts();
}


