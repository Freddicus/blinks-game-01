#ifndef UTIL_H_
#define UTIL_H_

#include <blinklib.h>

bool flipCoin();
byte nextFace(byte face);
byte nextFace(byte face, byte amount);
byte prevFace(byte face);
byte prevFace(byte face, byte amount);

#endif