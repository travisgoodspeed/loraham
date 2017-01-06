#ifndef NETWORK_H_
#define NETWORK_H_

void radiosetup();
void beacon(char* msg);
void radioon();
void radiooff();

void queuepkt(uint8_t *buf, bool delay);
bool recvpkt();
bool handlepackets();
bool digipeat(uint8_t *pkt, int rssi);
void xmitstack();

#endif // NETWORK_H_
