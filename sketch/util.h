#ifndef UTIL_H_
#define UTIL_H_

#include <blinklib.h>

#include "globals.h"
#include "states.h"

bool flipCoin();
word random(word min, word max);

void updateSharedPulseDimness();

void detectPanic();

#endif