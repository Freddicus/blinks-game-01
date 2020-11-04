#include "util.h"
#include "playing.h"

Timer resetGameTimer;
bool didResetGame = false;

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

void detectResetGame() {
  // detect the initiator of game reset
  if (!isAlone() && buttonMultiClicked() && buttonClickCount() == NUM_RESET_GAME_CLICKS) {
    initPlayVariables();
    setValueSentOnAllFaces(Message::RESET_GAME);
    didResetGame = true;
    resetGameTimer.set(1000);
    return;
  }

  // detect propagation of game reset
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == Message::RESET_GAME) {
      initPlayVariables();
      setValueSentOnAllFaces(Message::RESET_GAME);
      didResetGame = true;
      resetGameTimer.set(1000);
      return;
    }
  }

  if (didResetGame && resetGameTimer.isExpired()) {
    didResetGame = false;
    setValueSentOnAllFaces(Message::QUIET);
  }
}

void detectEndGame() {
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == Message::END_GAME) {
      gameState = GameState::GAME_OVER;
      return;
    }
  }
}