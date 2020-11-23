#include "util.h"

#include "playing.h"

bool flipCoin() {
  return random(100) >= 50;
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

void detectResetGame() {
  // detect the initiator of game reset
  if (wasButtonTripleClicked) {
    gotResetSignalTime = millis();
    gameState = GameState::RESET;
    return;
  }

  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == Message::RESET_GAME) {
      gotResetSignalTime = millis();
      gameState = GameState::RESET;
      return;
    }
  }
}

void detectEndGame() {
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == Message::END_GAME) {
      blinkState = BlinkState::NONE;
      gameState = GameState::GAME_OVER;
      return;
    }
  }
}