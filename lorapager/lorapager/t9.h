#ifndef T9_H_
#define T9_H_

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h> // Hardware-specific library

void t9init(Adafruit_ILI9341 *tft);
void t9setBuffer(char* newBuffer);
void t9start();
bool t9loop();
char* t9getBuffer();

#endif // T9_H_
