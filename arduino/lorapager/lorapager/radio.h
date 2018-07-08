#ifndef RADIO_H_
#define RADIO_H_

#include "packet.h"

void radioSetup();
void radioLoop();
void sendPacket(Packet p);

#endif // RADIO_H_
