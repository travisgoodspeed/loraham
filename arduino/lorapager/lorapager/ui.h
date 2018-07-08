#ifndef UI_H_
#define UI_H_

#include "packet.h"

void uiSetup();
void uiLoop();
void newMsg(Packet msg);
void onRecvMsg(Packet packet);

#endif // UI_H_
