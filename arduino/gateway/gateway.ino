/* This is a beacon for the LoRaHam protocol by KK4VCZ.
 * https://github.com/travisgoodspeed/loraham/
 *  
 */

#define CALLSIGN "KK4VCZ-14"

#include <SPI.h>
#include <RH_RF95.h>  //See http://www.airspayce.com/mikem/arduino/RadioHead/
 
/* for feather32u4 
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
#define VBATPIN A9  /**/
 
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

//Transmits one beacon and returns.
void beacon(){
  static int packetnum=0;
  
  //Serial.println("Transmitting..."); // Send a message to rf95_server
  
  char radiopacket[RH_RF95_MAX_MESSAGE_LEN];
  snprintf(radiopacket,
           RH_RF95_MAX_MESSAGE_LEN,
           "BEACON %s VCC=%f count=%d uptime=%ld",
           CALLSIGN,
           (float) voltage(),
           packetnum,
           uptime());

  Serial.print("TX "); Serial.print(packetnum); Serial.print(": "); Serial.println(radiopacket);
  radiopacket[sizeof(radiopacket)] = 0;
  
  //Serial.println("Sending..."); delay(10);
  rf95.send((uint8_t *)radiopacket, strlen((char*) radiopacket));
 
  //Serial.println("Waiting for packet to complete..."); delay(10);
  rf95.waitPacketSent();
  packetnum++;
}


//Handles retransmission of the packet.
bool shouldirt(uint8_t *buf, uint8_t len){
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
  
  //Random backoff if we might RT it.
  delay(random(10000));
  //Don't RT if we've gotten an incoming packet in that time.
  if(rf95.available()){
    //Serial.println("Interrupted by another packet.");
    return false;
  }

  //No objections.  RT it!
  return true;
}

//If a packet is available, digipeat it.  Otherwise, wait.
void digipeat(){
  //digitalWrite(LED, LOW);
  //Try to receive a reply.
  if (rf95.available()){
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    int rssi=0;
    /*
     * When we receive a packet, we repeat it after a random
     * delay if:
     * 1. It asks to be repeated.
     * 2. We've not yet received a different packet.
     * 3. We've waited a random amount of time.
     * 4. The first word is not RT.
     */
    if (rf95.recv(buf, &len)){
      rssi=rf95.lastRssi();
      //digitalWrite(LED, HIGH);
      //RH_RF95::printBuffer("Received: ", buf, len);
      //Serial.print("Got: ");
      buf[len]=0;
      Serial.println((char*)buf);
      Serial.println("");

      if(shouldirt(buf,len)){
        // Retransmit.
        uint8_t data[RH_RF95_MAX_MESSAGE_LEN];
        snprintf((char*) data,
                 RH_RF95_MAX_MESSAGE_LEN,
                 "%s\n" //First line is the original packet.
                 "RT %s rssi=%d VCC=%f uptime=%ld", //Then we append our call and strength as a repeater.
                 (char*) buf,
                 CALLSIGN,  //Repeater's callsign.
                 (int) rssi, //Signal strength, for routing.
                 voltage(), //Repeater's voltage
                 uptime()
                 );
        rf95.send(data, strlen((char*) data));
        rf95.waitPacketSent();
        Serial.println((char*) data);
        //digitalWrite(LED, LOW);
        Serial.println("");
      }else{
        //Serial.println("Declining to retransmit.\n");
      }
    }else{
      Serial.println("Receive failed");
    }
  }else{
    delay(10);
  }
}

void loop(){
  static unsigned long lastbeacon=millis();
  
  //Only digipeat if the battery is in good shape.
  if(voltage()>3.5){
    //Only digipeat when battery is high.
    digipeat();

    //Every ten minutes, we beacon just in case.
    if(millis()-lastbeacon>10*60000){
      beacon();
      lastbeacon=millis();
    }
  }else{
    //Transmit a beacon every ten minutes when battery is low.
    radiooff();
    delay(10*60000);
    radioon();
    beacon();
  };
}

