#ifndef PACKET_H_
#define PACKET_H_


#include <RH_RF95.h>

typedef struct {
  uint8_t data[RH_RF95_MAX_MESSAGE_LEN + 1];
  int rssi;
} Packet;


#endif // PACKET_H_
