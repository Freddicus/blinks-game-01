#include "colors.h"

void updateColors() {
  switch (blinkState) {
    case BlinkState::NONE:
      pulseColor(WHITE, sharedPulseDimness);
      break;
    case BlinkState::SOIL:
      setColor(COLOR_SOIL);
      break;
    case BlinkState::SPROUT:
      setColorOnFace(COLOR_SPROUT, FACE_SPROUT);
      handleGrowthColor();
      break;
    case BlinkState::TRUNK:
      setColor(COLOR_TRUNK);
      handleGrowthColor();
      handleGameTimerColor();
      break;
    case BlinkState::BRANCH:
      setColor(COLOR_TRUNK);
      handleGrowthColor();
      handleBranchBudColor();
      break;
    case BlinkState::BUD:
      handleBranchBudColor();
      break;
    case BlinkState::LEAF:
      handleLeafColor();
      break;
  }
}

void handleGrowthColor() {
  if (sendingGrowth || (growthInitiated == true && !txGrowthTimer.isExpired())) {
    pulseColorOnFace(COLOR_GROWTH, headFace, sharedPulseDimness);
  }

  if (receivingGrowth) {
    pulseColorOnFace(COLOR_GROWTH, rearFace, sharedPulseDimness);
  }
}

void handleGameTimerColor() {
  if (isGameTimerStarted && !gameTimer.isExpired()) {
    spinColor(COLOR_TRUNK, SPIN_SPEED_MEDIUM_MS);
  }
}

void handleBranchBudColor() {
  switch (branchState) {
    case NAB:
      setColor(COLOR_BRANCH);
      break;
    case RANDOMIZING:
      sparkle();
      break;
    case BUDDING:
      pulseColorOnFace(COLOR_BUD, activeBudFace, sharedPulseDimness);
      break;
    case TOO_LATE:
      pulseColor(RED, sharedPulseDimness);
      break;
  }
}

void handleLeafColor() {
  switch (leafState) {
    case NAL:
    case DETACHED:
      setColor(COLOR_NONE);
      break;
    case YOUNG:
      spinColor(COLOR_YOUNG_LEAF, SPIN_SPEED_FAST_MS);
      break;
    case MATURE:
      spinColor(COLOR_MATURE_LEAF, SPIN_SPEED_MEDIUM_MS);
      break;
    case DYING:
      spinColor(COLOR_DYING_LEAF, SPIN_SPEED_SLOW_MS);
      break;
    case DEAD_LEAF:
      setColor(COLOR_DEAD_LEAF);
      break;
  }
}

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