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
