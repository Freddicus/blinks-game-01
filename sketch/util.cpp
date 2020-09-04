#include "util.h"

bool flipCoin() {
  return random(1);
}

byte nextFace(byte face) {
  return nextFace(face, 1);
}

byte nextFace(byte face, byte amount) {
  return (face + amount) % FACE_COUNT;
}

byte prevFace(byte face) {
  return prevFace(face, 1);
}

byte prevFace(byte face, byte amount) {
  return (face + (FACE_COUNT - (amount % FACE_COUNT))) % FACE_COUNT;
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
