#include <RH_RF95.h>  //See http://www.airspayce.com/mikem/arduino/RadioHead/

#include "config.h"
#include "platforms.h"

#include "utilities.h"
#include "network.h"

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// recv/xmit buffers
uint8_t recvbuf[BUFFER_PACKETS][RH_RF95_MAX_MESSAGE_LEN + 1]; // + 1 to accommodate str terminating null byte at end
int recvrssi[BUFFER_PACKETS];

uint8_t xmitbuf[BUFFER_PACKETS][RH_RF95_MAX_MESSAGE_LEN + 1];

uint8_t recvbufi = 0;
int8_t xmitbufi = -1;


// queue a packet for transmission
void queuepkt(uint8_t *buf) {
  if (xmitbufi < BUFFER_PACKETS - 1) {
    xmitbufi++;
  }
  strcpy((char*) xmitbuf[xmitbufi], (char*) buf);
  xmitbuf[xmitbufi][RH_RF95_MAX_MESSAGE_LEN] = 0; // just in case!
}

void radiosetup()
{
  // clear buffers
  memset(recvbuf, 0, (BUFFER_PACKETS*(RH_RF95_MAX_MESSAGE_LEN+1)));
  memset(xmitbuf, 0, (BUFFER_PACKETS*(RH_RF95_MAX_MESSAGE_LEN+1)));
  memset(recvrssi, 0, BUFFER_PACKETS);

  // configure radio pins
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
}


void radioon() {
  Serial.println("Feather LoRa init start!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    //while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    //while (1);
  } else {
    Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  }

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
  Serial.println("Set power to 23.");
  Serial.print("Max packet length: "); Serial.println(RH_RF95_MAX_MESSAGE_LEN);
}

void radiooff() {
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
}

// put a beacon packet in the queue
void beacon(char* msg) {
  static int packetnum = 0;

  char radiopacket[RH_RF95_MAX_MESSAGE_LEN+1];
  snprintf(radiopacket,
           RH_RF95_MAX_MESSAGE_LEN,
           "BEACON %s VCC=%f count=%d uptime=%ld%s%s",
           CALLSIGN,
           (float) voltage(),
           packetnum,
           uptime(),
           msg[0] == 0 ? "" : " ",
           msg);

  radiopacket[RH_RF95_MAX_MESSAGE_LEN] = 0;

  queuepkt((uint8_t*) radiopacket);
  packetnum++;
}

// if packet available, place it in the recv buffer
bool recvpkt() {
  uint8_t len = RH_RF95_MAX_MESSAGE_LEN;
  bool packetrecieved = false;
  while (rf95.available()) {
    recvbufi = recvbufi + 1 % BUFFER_PACKETS;
    if (rf95.recv(recvbuf[recvbufi], &len)) {
      recvbuf[recvbufi][len] = 0;
      recvrssi[recvbufi] = rf95.lastRssi();
      Serial.print("RX ");
      Serial.print(recvrssi[recvbufi]);
      Serial.print(": "); Serial.println((char*) recvbuf[recvbufi]);
      packetrecieved = true;
    }
  }
  return packetrecieved;
}

// looks at all the packets in the recv buffer and takes appropriate action
void handlepackets() {
  for (int i = 0; i < BUFFER_PACKETS; i++) {
    if (strlen((char*) recvbuf[i]) > 0) {
      if (shouldrt(recvbuf[i])) {
        digipeat(recvbuf[i], recvrssi[i]);
      }

      recvbuf[i][0] = 0; // handled!
    }
  }
}

// Add RT lines to recieved packet and queue it for transmission
bool digipeat(uint8_t *pkt, int rssi) {
  uint8_t data[RH_RF95_MAX_MESSAGE_LEN + 2];
  int r = random(MAX_XMIT_WAIT);
  snprintf((char*) data,
           RH_RF95_MAX_MESSAGE_LEN + 1,
           "%s\n" //First line is the original packet.
           "RT %s rssi=%d", //Then we append our call and strength as a repeater.
           (char*) pkt,
           CALLSIGN,  //Repeater's callsign.
           rssi //Signal strength, for routing.
          );
  if (strlen((char*) data) > RH_RF95_MAX_MESSAGE_LEN) {
    return false; // packet too long
  }
  for (int n = 0; n < r / 10; n++) {
    delay(10);
    recvpkt();
  }
  queuepkt(data);
  return true;
}

// transmits all the packets in the xmit stack, while receiving any that come in
void xmitstack() {
  while (xmitbufi > -1) {
    while (rf95.waitCAD() && recvpkt()) {}
    Serial.print("TX: ");
    Serial.println((char*) xmitbuf[xmitbufi]);
#ifdef BLINK_LED
    digitalWrite(LED, HIGH);
#endif
    rf95.send(xmitbuf[xmitbufi], strlen((char*) xmitbuf[xmitbufi]));
    rf95.waitPacketSent();
#ifdef BLINK_LED
    digitalWrite(LED, LOW);
#endif

    xmitbufi--;
  }
}

