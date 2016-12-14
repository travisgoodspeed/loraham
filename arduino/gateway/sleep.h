#ifndef SLEEP_H_
#define SLEEP_H_

uint16_t getepoch();
void sleepsetup();
void sleepreset(char timerslot);
bool sleep(unsigned int seconds, char timerslot);

#endif // SLEEP_H_

