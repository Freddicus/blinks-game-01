#include "util.h"

bool flipCoin() {
  return random(1);
}

word random(word min, word max) {
  return random(max - min) + min;
}

void updateSharedPulseDimness() {
  // get progress from 0 - MAX
  int pulseProgress = millis() % PULSE_LENGTH_MS;

  // transform that progress to a byte (0-255)
  byte pulseMapped = map(pulseProgress, 0, PULSE_LENGTH_MS, 0, 255);

  // transform that byte with sin
  sharedPulseDimness = sin8_C(pulseMapped);
}

void detectPanic() {
  if (isAlone() && buttonMultiClicked() && buttonClickCount() == NUM_PANIC_CLICKS) {
    setup();
    setValueSentOnAllFaces(QUIET);
  }
}
