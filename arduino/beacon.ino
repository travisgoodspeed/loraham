/* This is a beacon for the LoRaHam protocol by KK4VCZ.
 * https://github.com/travisgoodspeed/loraham/
 *  
 */

#define CALLSIGN "KK4VCZ-10"
 
#include <SPI.h>
#include <RH_RF95.h>
 
/* for feather32u4 
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7*/
 
/* for feather m0  */
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

 
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
  #define VBATPIN A7

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
    while (1);
  }
  Serial.println("LoRa radio init OK!");
 
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
 
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

void setup() 
{
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
}

//Transmits one beacon and returns.
void beacon(){
  static int packetnum=0;
  
  //Serial.println("Transmitting..."); // Send a message to rf95_server
  
  char radiopacket[RH_RF95_MAX_MESSAGE_LEN];
  snprintf(radiopacket,
           RH_RF95_MAX_MESSAGE_LEN,
           "BEACON %s VCC=%f count=%d  http://github.com/travisgoodspeed/loraham/ RT",
            CALLSIGN,
            (float) voltage(),
            packetnum);
  //itoa(packetnum++, radiopacket+16, 10);
  Serial.print("TX "); Serial.print(packetnum); Serial.print(": "); Serial.println(radiopacket);
  radiopacket[sizeof(radiopacket)] = 0;
  
  //Serial.println("Sending..."); delay(10);
  rf95.send((uint8_t *)radiopacket, sizeof(radiopacket));
 
  //Serial.println("Waiting for packet to complete..."); delay(10);
  rf95.waitPacketSent();
  packetnum++;
}

//If a packet is available, digipeat it.  Otherwise, wait.
void digipeat(){
  //Try to receive a reply.
  if (rf95.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    //Serial.println("Waiting on a packet.");
    if (rf95.recv(buf, &len))
    {
      digitalWrite(LED, HIGH);
      //RH_RF95::printBuffer("Received: ", buf, len);
      Serial.print("Got: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
      
      // Send a reply
      uint8_t data[] = "And hello from KK4VCZ";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      //Serial.println("Sent a reply");
      digitalWrite(LED, LOW);
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
}

void loop(){
  if(1 || !Serial){
    //Turn the radio on.
    radioon();
    //Transmit a beacon once every five minutes.
    beacon();
    //Then turn the radio off to save power.
    radiooff();
    //Five minute delay between beacons.
    delay(5*60000);
  }else{
    digipeat();
  }
}

