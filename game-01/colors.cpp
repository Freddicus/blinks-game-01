#include "colors.h"

void pulseColor(Color color, byte pulseDimness) {
  setColor(dim(color, pulseDimness));
}

void pulseColorOnFace(Color color, byte face, byte pulseDimness) {
  setColorOnFace(dim(color, pulseDimness), face);
}

void sparkle() {
  FOREACH_FACE(f) {
    byte randomH = random(360);
    byte randomS = random(100);
    byte randomB = random(40);  // 40% max brightness
    Color randomColor = makeColorHSBMapped(randomH, randomS, randomB);
    setColorOnFace(randomColor, f);
  }
}

void spinColor(Color color, long revolutionMs) {
  int spinProgress = millis() % revolutionMs;
  byte spinMapped = map(spinProgress, 0, revolutionMs, 0, 6);
  setColorOnFace(color, spinMapped);
  setColorOnFace(dim(color, 200), prevFace(spinMapped));
  setColorOnFace(dim(color, 160), prevFace(spinMapped, 2));
  setColorOnFace(dim(color, 120), prevFace(spinMapped, 3));
  setColorOnFace(dim(color, 80), prevFace(spinMapped, 4));
  setColorOnFace(dim(color, 40), prevFace(spinMapped, 5));
}

Color makeColorHSBMapped(word h, word s, word b) {
  byte mappedH = map(h, 0, 360, 0, 255);
  byte mappedS = map(s, 0, 100, 0, 255);
  byte mappedB = map(b, 0, 100, 0, 255);
  return makeColorHSB(mappedH, mappedS, mappedB);
}