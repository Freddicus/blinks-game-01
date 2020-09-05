#ifndef UTIL_H_
#define UTIL_H_

#include <blinklib.h>

#include "globals.h"
#include "states.h"

bool flipCoin();
word random(word min, word max);

byte nextFace(byte face);
byte nextFace(byte face, byte amount);
byte prevFace(byte face);
byte prevFace(byte face, byte amount);
void updateSharedPulseDimness();

void detectPanic();

#endif