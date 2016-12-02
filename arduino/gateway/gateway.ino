/* This is a beacon for the LoRaHam protocol by KK4VCZ.
 * https://github.com/travisgoodspeed/loraham/
 *  
 */

#define CALLSIGN "KB3RGT-10"

#include <SPI.h>
#include <RH_RF95.h>  //See http://www.airspayce.com/mikem/arduino/RadioHead/
 
/* for feather32u4 
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
#define VBATPIN A9  
 
/* for feather m0  */
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
#define VBATPIN A7
 
/* for shield 
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 7
*/
 
 
/* for ESP w/featherwing 
#define RFM95_CS  2    // "E"
#define RFM95_RST 16   // "D"
#define RFM95_INT 15   // "B"
*/
 
/* Feather 32u4 w/wing
#define RFM95_RST     11   // "A"
#define RFM95_CS      10   // "B"
#define RFM95_INT     2    // "SDA" (only SDA/SCL/RX/TX have IRQ!)
*/
 
/* Feather m0 w/wing 
#define RFM95_RST     11   // "A"
#define RFM95_CS      10   // "B"
#define RFM95_INT     6    // "D"
*/
 
/* Teensy 3.x w/wing 
#define RFM95_RST     9   // "A"
#define RFM95_CS      10   // "B"
#define RFM95_INT     4    // "C"
*/

 
// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 434.0
 
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);
 
// Blinky on receipt
#define LED 13

// ring buffer size in packets
#define BUFFER_PACKETS 10

// max xmit wait - we'll wait between 0 and n milliseconds before transmitting to avoid collision
#define MAX_XMIT_WAIT 10000


uint8_t recvbuf[BUFFER_PACKETS][RH_RF95_MAX_MESSAGE_LEN + 1]; // + 1 to accommodate str terminating null byte at end
int recvrssi[BUFFER_PACKETS];

uint8_t xmitbuf[BUFFER_PACKETS][RH_RF95_MAX_MESSAGE_LEN + 1];

uint8_t recvbufi = 0;
int8_t xmitbufi = -1;


//Returns the battery voltage as a float.
float voltage(){
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  return measuredvbat;
}

void radioon(){
  Serial.println("Feather LoRa RX Test!");
  
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
  }else{
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

void radiooff(){
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
}

void setup() {
  pinMode(LED, OUTPUT);     
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  delay(1000);
  //while (!Serial);
  Serial.begin(9600);
  Serial.setTimeout(10);
  delay(100);
 
  radioon();
  digitalWrite(LED, LOW);

  for(int i = 0; i < BUFFER_PACKETS; i++) {
    recvbuf[i][0] = xmitbuf[i][0] = 0;  // clear buffers
    recvbuf[i][RH_RF95_MAX_MESSAGE_LEN] = 0;
  }

  //Beacon once at startup.
  beacon();
}

//! Uptime in seconds, correcting for rollover.
long int uptime(){
  static unsigned long rollover=0;
  static unsigned long lastmillis=millis();

  //Account for rollovers, every ~50 days or so.
  if(lastmillis>millis()){
    rollover+=(lastmillis>>10);
    lastmillis=millis();
  }

  return(rollover+(millis()>>10));
}


// queue a packet for transmission
void queuepkt(uint8_t *buf) {
  if (xmitbufi < BUFFER_PACKETS - 1) {
    xmitbufi++;
  }
  strcpy((char*) xmitbuf[xmitbufi], (char*) buf);
  xmitbuf[xmitbufi][RH_RF95_MAX_MESSAGE_LEN] = 0; // just in case!
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

// transmits all the packets in the xmit stack, while receiving any that come in
void xmitstack() {
  while (xmitbufi > -1) {
    while(rf95.waitCAD() && recvpkt()) {}
    Serial.print("TX: ");
    Serial.println((char*) xmitbuf[xmitbufi]);
    digitalWrite(LED, HIGH);
    rf95.send(xmitbuf[xmitbufi], strlen((char*) xmitbuf[xmitbufi]));
    rf95.waitPacketSent();
    digitalWrite(LED, LOW);

    xmitbufi--;
  }
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

// put a beacon packet in the queue
void beacon(){
  static int packetnum=0;

  char radiopacket[RH_RF95_MAX_MESSAGE_LEN];
  snprintf(radiopacket,
           RH_RF95_MAX_MESSAGE_LEN,
           "BEACON %s VCC=%f count=%d uptime=%ld",
           CALLSIGN,
           (float) voltage(),
           packetnum,
           uptime());

  radiopacket[sizeof(radiopacket)] = 0;

  queuepkt((uint8_t*) radiopacket);
  packetnum++;
}


// returns true if supplied packet should be retransmitted
bool shouldrt(uint8_t *buf){
  //Don't RT any packet containing our own callsign.
  if(strcasestr((char*) buf, CALLSIGN)){
    //Serial.println("I've already retransmitted this one.\n");
    return false;
  }
  //Don't RT if the packet is too long.
  if(strlen((char*) buf)>128){
    //Serial.println("Length is too long.\n");
    return false;
  }
  //No objections.  RT it!
  return true;
}

// Add RT lines to recieved packet and queue it for transmission
void digipeat(uint8_t *pkt, int rssi){
  uint8_t data[RH_RF95_MAX_MESSAGE_LEN+1];
  int r = random(MAX_XMIT_WAIT);
  snprintf((char*) data,
           RH_RF95_MAX_MESSAGE_LEN,
           "%s\n" //First line is the original packet.
           "RT %s rssi=%d VCC=%f uptime=%ld", //Then we append our call and strength as a repeater.
           (char*) pkt,
           CALLSIGN,  //Repeater's callsign.
           rssi, //Signal strength, for routing.
           voltage(), //Repeater's voltage
           uptime()
           );
  for (int n = 0; n < r / 10; n++) {
    delay(10);
    recvpkt();
  }
  queuepkt(data);
}

void loop(){
  static unsigned long lastbeacon=millis();
  int n;
  
  //Only digipeat if the battery is in good shape.
  if(voltage()>3.5){
    //Only digipeat when battery is high.
    digitalWrite(LED, HIGH);
    delay(10);
    digitalWrite(LED, LOW);
    for(n = 0; n < 1000; n++) {
      recvpkt();
      handlepackets();
      xmitstack();
      delay(20);

      //Every ten minutes, we beacon just in case.
      if(millis()-lastbeacon>10*60000){
        beacon();
        lastbeacon=millis();
      }
    }
  }else{
    //Transmit a beacon every ten minutes when battery is low.
    radiooff();
    delay(10*60000);
    radioon();
    beacon();
    xmitstack();
  };
}

