

#include <RH_RF95.h>

#include <Arduino.h>
  
#include "packet.h"
#include "config.h"

bool getSender(Packet p, char* dst, unsigned int len) {
  char strbuf[RH_RF95_MAX_MESSAGE_LEN + 1];
  char* strptr;
  strncpy(strbuf, (char*) p.data, RH_RF95_MAX_MESSAGE_LEN);
  strptr = strtok(strbuf, " "); // 
  if (strptr == NULL) { return false; }
  strptr = strtok(NULL, " ");
  if (strptr == NULL) { return false; }
  strncpy(dst, strptr, len);
}

bool getDest(Packet p, char* dst, unsigned int len) {
  char strbuf[RH_RF95_MAX_MESSAGE_LEN + 1];
  char* strptr;
  strncpy(strbuf, (char*) p.data, RH_RF95_MAX_MESSAGE_LEN);
  strptr = strtok(strbuf, " "); // 
  if (strptr == NULL) { return false; }
  strncpy(dst, strptr, len);
}

//Returns the battery voltage as a float.
float voltage() {
  return 3.3;
  float measuredvbat;
  interrupts();
  delay(2000);
  measuredvbat = analogRead(VBATPIN);
  delay(2000);
  noInterrupts();
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  return measuredvbat;
}
