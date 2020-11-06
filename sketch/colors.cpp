#include "colors.h"

void updateColors() {
  // initialize color each loop so we can see if we miss anything
  setColor(COLOR_NONE);

  updateSharedPulseDimness();

  switch (gameState) {
    case GameState::PLAYING:
      handlePlayingColors();
      break;
    case GameState::GAME_OVER:
      handleGameOverColors();
      break;
  }
}

void handleGameOverColors() {
  sparkle();
}

void handlePlayingColors() {
  switch (blinkState) {
    case BlinkState::NONE:
      setColor(COLOR_NONE);
      break;
    case BlinkState::SOIL:
      handleSoilColor();
      break;
    case BlinkState::SPROUT:
      setColorOnFace(COLOR_SPROUT, headFace);
      if (isGameTimerStarted) {
        setColor(COLOR_SPROUT);
      }
      break;
    case BlinkState::TRUNK:
      if (!isGameStarted) {
        pulseColorOnFace(COLOR_TRUNK, rearFace, sharedPulseDimness);
        if (isSplit) {
          pulseColorOnFace(COLOR_BRANCH, headFaceLeft, sharedPulseDimness);
          pulseColorOnFace(COLOR_BRANCH, headFaceRight, sharedPulseDimness);
        } else {
          pulseColorOnFace(COLOR_TRUNK, headFace, sharedPulseDimness);
        }
      } else {
        setColor(COLOR_TRUNK);
      }
      handleGameTimerColor();
      break;
    case BlinkState::BRANCH:
      if (!isGameStarted) {
        pulseColorOnFace(COLOR_BRANCH, rearFace, sharedPulseDimness);
        if (isSplit) {
          pulseColorOnFace(COLOR_BRANCH, headFaceLeft, sharedPulseDimness);
          pulseColorOnFace(COLOR_BRANCH, headFaceRight, sharedPulseDimness);
        } else {
          pulseColorOnFace(COLOR_BRANCH, headFace, sharedPulseDimness);
        }
      } else {
        setColor(COLOR_BRANCH);
      }
      handleBranchColor();
      break;
    case BlinkState::COLLECTOR:
      handleCollectorColor();
      break;
  }
}

void handleSoilColor() {
  if (!soilTimer.isExpired()) {
    spinColor(COLOR_SOIL, SPIN_SPEED_FAST_MS);
  }
}

void handleGameTimerColor() {
  if (isGameTimerStarted && !gameTimer.isExpired()) {
    spinColor(COLOR_TRUNK, SPIN_SPEED_MEDIUM_MS);
  }
}

void handleBranchColor() {
  switch (branchState) {
    case BranchState::NAB:
      // don't override potential growth color
      break;
    case BranchState::RANDOMIZING:
      // sparkle();
      break;
    case BranchState::GREW_A_LEAF:
      // eventually different colors here
      if (activeLeafFace != NOT_SET) {
        setColorOnFace(GREEN, activeLeafFace);
      }
      break;
  }
}

void handleCollectorColor() {
  setColor(GREEN);
}

void pulseColor(Color color, byte pulseDimness) {
  setColor(dim(color, pulseDimness));
}

void pulseColorOnFace(Color color, byte face, byte pulseDimness) {
  setColorOnFace(dim(color, pulseDimness), face);
}

void sparkle() {
  FOREACH_FACE(f) {
    byte randomR = random(32);
    byte randomG = random(32);
    byte randomB = random(32);
    Color randomColor = MAKECOLOR_5BIT_RGB(randomR, randomG, randomB);
    setColorOnFace(randomColor, f);
  }
}

void spinColor(Color color, long revolutionMs) {
  int spinProgress = millis() % revolutionMs;
  byte spinMapped = map(spinProgress, 0, revolutionMs, 0, 6);
  setColorOnFace(color, spinMapped);
  setColorOnFace(dim(color, 200), CCW_FROM_FACE(spinMapped, 1));
  setColorOnFace(dim(color, 160), CCW_FROM_FACE(spinMapped, 2));
  setColorOnFace(dim(color, 120), CCW_FROM_FACE(spinMapped, 3));
  setColorOnFace(dim(color, 80), CCW_FROM_FACE(spinMapped, 4));
  setColorOnFace(dim(color, 40), CCW_FROM_FACE(spinMapped, 5));
}