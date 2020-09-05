#include "playing.h"

// ---- trunk / branch ----

bool isTrunkSplit;
bool isFinalBranch;

// --- growth ----

bool growthInitiated;
bool sendingGrowth;
bool receivingGrowth;

Timer soilTimer;
Timer txGrowthTimer;

// ---- branch / bud play ----

byte budFaces[4];
byte activeBudFace;
byte branchHitPoints;
byte branchState;

bool isLeafSignalTimerStarted;

Timer becomeBudCoinFlipTimer;
Timer activeBudSeekingLeafTimer;
Timer tooLateCoolDownTimer;
Timer leafSignalTimer;

// leaf play

Timer leafLifeTimer;
byte leafState;

bool hasLeafFlashedGreeting;

// ---- setup methods ----

void initPlayVariables() {
  isTrunkSplit = false;
  isFinalBranch = false;

  growthInitiated = false;
  sendingGrowth = false;
  receivingGrowth = false;

  isLeafSignalTimerStarted = false;
  hasLeafFlashedGreeting = false;

  leafState = NAL;
  branchState = NAB;

  soilTimer.set(0);
  txGrowthTimer.set(0);
}

void gameStatePlaying() {
  switch (blinkState) {
    case BlinkState::NONE:
      playingNone();
      break;
    case BlinkState::SOIL:
      playingSoil();
      break;
    case BlinkState::SPROUT:
      playingSprout();
      break;
    case BlinkState::TRUNK:
      playingTrunk();
      break;
    case BlinkState::BRANCH:
      playingBranch();
      break;
    case BlinkState::BUD:
      playingBud();
      break;
    case BlinkState::LEAF:
      playingLeaf();
      break;
  }

  updateColors();
  detectPanic();
}

// -------- Playing methods -------

void playingNone() {
  if (buttonDoubleClicked()) {
    blinkState = BlinkState::SOIL;
    soilTimer.set(5000);
  }

  // let's find out when we become attached!
  FOREACH_FACE(f) {
    byte faceValue = getLastValueReceivedOnFace(f);
    bool faceValueExpired = isValueReceivedOnFaceExpired(f);
    if (faceValueExpired) {
      // continue - nothing to read
      continue;
    } else {
      rearFace = f;
    }

    if (faceValue >= TRUNK_1 || faceValue < TRUNK_5) {
      blinkState = BlinkState::TRUNK;
      headFace = oppositeFaces[f];
      setValueSentOnFace(faceValue + 1, headFace);
      break;
    } else if (faceValue == TRUNK_5) {
      blinkState = BlinkState::TRUNK;
      isTrunkSplit = true;
      headFaceLeft = oppositeFaces[f] - 1;
      headFaceRight = oppositeFaces[f] + 1;
      setValueSentOnFace(BRANCH_LEFT_1, headFaceLeft);
      setValueSentOnFace(BRANCH_RIGHT_1, headFaceRight);
      break;
    } else if (faceValue >= BRANCH_LEFT_1 || faceValue < BRANCH_LEFT_4) {
      blinkState = BlinkState::BRANCH;
      headFace = oppositeFaces[f];
      setValueSentOnFace(faceValue + 1, headFace);
      updateBudFaces();
    } else if (faceValue >= BRANCH_RIGHT_1 || faceValue < BRANCH_RIGHT_4) {
      blinkState = BlinkState::BRANCH;
      headFace = oppositeFaces[f];
      setValueSentOnFace(faceValue + 1, headFace);
      updateBudFaces();
    } else if (faceValue == BRANCH_RIGHT_4 || faceValue == BRANCH_LEFT_4) {
      isFinalBranch = true;
      blinkState = BlinkState::BRANCH;
      branchState = RANDOMIZING;
      headFace = oppositeFaces[f];  // not used
      setValueSentOnFace(START_BUDDING, rearFace);
    } else if (faceValue == LOOKING_FOR_LEAF) {
      blinkState = BlinkState::LEAF;
      leafState = YOUNG;
      headFace = oppositeFaces[f];                         // not used
      setValueSentOnFace(LOOKING_FOR_LEAF_ACK, rearFace);  // rearFace is the leaf stem
    }
  }
}

void playingSoil() {
  // allow undo switch to soil
  if (buttonDoubleClicked() && isAlone()) {
    blinkState = BlinkState::NONE;
  }

  if (soilTimer.isExpired()) {
    blinkState = BlinkState::SPROUT;
  } else {
    // TODO: animate soil about to sprout
  }
}

void playingSprout() {
  // allow undo sprout for overzealous players
  if (buttonDoubleClicked() && isAlone()) {
    blinkState = BlinkState::NONE;
  }

  headFace = FACE_SPROUT;
  setValueSentOnFace(TRUNK_1, headFace);

  if (buttonSingleClicked()) {
    txGrowthTimer.set(GROWTH_DELAY_MS);
    growthInitiated = true;
  }

  if (growthInitiated == true && txGrowthTimer.isExpired()) {
    sendingGrowth = true;
  }

  // send up growth command / lights
  if (sendingGrowth) {
    sendGrowth();
    growthInitiated = false;
  }

  if (getLastValueReceivedOnFace(headFace) == GROW_ACK) {
    sendingGrowth = false;
  }

  if (getLastValueReceivedOnFace(headFace) == START_THE_CLOCK) {
    setValueSentOnFace(START_THE_CLOCK_NOW, headFace);
  }
}

void playingTrunk() {
  byte rxRear = getLastValueReceivedOnFace(rearFace);
  byte rxHead = getLastValueReceivedOnFace(headFace);

  // TODO: add growthTimer and use growthInitiated
  if (rxRear == GROW) {
    receivingGrowth = true;
    sendingGrowth = true;
    ackGrowth();
    if (isTrunkSplit) {
      sendSplitGrowth();
    } else {
      sendGrowth();
    }
  }

  if (rxHead == GROW_ACK) {
    receivingGrowth = false;
    sendingGrowth = false;
  }

  if (isTrunkSplit) {
    byte rxHeadLeft = getLastValueReceivedOnFace(headFaceLeft);
    byte rxHeadRight = getLastValueReceivedOnFace(headFaceRight);

    // both branches are budding - start the trunk timer!
    if (rxHeadLeft == rxHeadRight && rxHeadRight == START_BUDDING) {
      // message down to trunk 1 that it's time
      setValueSentOnFace(START_THE_CLOCK, rearFace);
    }
  }

  if (rxHead == START_THE_CLOCK) {
    setValueSentOnFace(START_THE_CLOCK, rearFace);
  }

  if (rxRear == START_THE_CLOCK_NOW) {
    gameTimer.set(GAME_TIMER_MS);
    isGameTimerStarted = true;
  }

  if (isGameTimerStarted && gameTimer.isExpired()) {
    if (isTrunkSplit) {
      // TODO: game is over! animate!!!
    } else {
      setValueSentOnFace(START_THE_CLOCK_NOW, headFace);
    }
  }
}

void playingBranch() {
  byte rxRear = getLastValueReceivedOnFace(rearFace);
  byte rxHead = getLastValueReceivedOnFace(headFace);

  switch (branchState) {
    case NAB:
      // --- do the growth stuff
      // TODO: add growthTimer and use growthInitiated
      if (rxRear == GROW) {
        setValueSentOnFace(GROW_ACK, rearFace);
      }

      if (rxRear == GROW && !isFinalBranch) {
        sendingGrowth = true;
      }

      if (rxHead == GROW_ACK) {
        sendingGrowth = false;
      }

      // --- do the post-growth stuff
      if (rxHead == START_BUDDING) {
        branchState = RANDOMIZING;
        becomeBudCoinFlipTimer.set(BECOME_BUD_COIN_FLIP_COOLDOWN_MS);
        setValueSentOnFace(START_BUDDING, rearFace);
      }

      break;
    case RANDOMIZING:
      randomizeBudAffinity();
      break;
    default:
      playingBud();
      break;
  }
}

void playingBud() {
  byte rxRear = getLastValueReceivedOnFace(rearFace);
  byte rxHead = getLastValueReceivedOnFace(headFace);

  switch (branchState) {
    case BUDDING:
      if (activeBudFace == -1) {
        activeBudSeekingLeafTimer.set(random(ASK_FOR_LEAF_MIN_TIME_MS, ASK_FOR_LEAF_MAX_TIME_MS));
        activeBudFace = budFaces[random(4)];
        setValueSentOnFace(LOOKING_FOR_LEAF, activeBudFace);
      } else {
        if (activeBudSeekingLeafTimer.isExpired()) {
          setValueSentOnFace(QUIET, activeBudFace);
          activeBudFace = -1;
          branchState = TOO_LATE;
          tooLateCoolDownTimer.set(TOO_LATE_COOL_DOWN_MS);
        } else {
          if (getLastValueReceivedOnFace(activeBudFace) == LOOKING_FOR_LEAF_ACK) {
            branchState = GREW_A_LEAF;
          }
        }
      }
      break;
    case TOO_LATE:
      if (tooLateCoolDownTimer.isExpired()) {
        branchState = RANDOMIZING;
        becomeBudCoinFlipTimer.set(BECOME_BUD_COIN_FLIP_COOLDOWN_MS);
      }
      break;
    case GREW_A_LEAF:
      // TODO: meat and potatos regarding leaf maturity and damaging branch
      playingBudWithLeaf();
      break;
  }
}

void playingBudWithLeaf() {
  if (!isLeafSignalTimerStarted) {
    // send connected
    isLeafSignalTimerStarted = true;
    setValueSentOnFace(BRANCH_GREET_LEAF, activeBudFace);
    leafSignalTimer.set(random(LEAF_PLAY_TIME_MIN_MS, LEAF_PLAY_TIME_MAX_MS));
    return;
  }

  if (leafSignalTimer.isExpired()) {
    setValueSentOnFace(BRANCH_MATURE_LEAF, activeBudFace);
    leafSignalTimer.set(random(LEAF_PLAY_TIME_MIN_MS, LEAF_PLAY_TIME_MAX_MS));
  }
}

void playingLeaf() {
  byte rxRear = getLastValueReceivedOnFace(rearFace);

  if (rxRear == Message::BRANCH_MATURE_LEAF) {
    setValueSentOnFace(Message::BRANCH_MATURE_LEAF_ACK, rearFace);
    switch (leafState) {
      case LeafState::NAL:
        // should not happen
        break;
      case LeafState::YOUNG:
        leafState = LeafState::MATURE;
        break;
      case LeafState::MATURE:
        leafState = LeafState::DYING;
        break;
      case LeafState::DYING:
        leafState = LeafState::DEAD_LEAF;
        break;
      case LeafState::DEAD_LEAF:
        // TODO poisin the tree
        break;
    }
  }
}

// ----- Game Helpers ------

// set GROW_ACK on rearFace
void ackGrowth() {
  setValueSentOnFace(GROW_ACK, rearFace);
}

// set GROW on headFace
void sendGrowth() {
  setValueSentOnFace(GROW, headFace);
}

void sendSplitGrowth() {
  setValueSentOnFace(GROW, headFaceLeft);
  setValueSentOnFace(GROW, headFaceRight);
}

void updateBudFaces() {
  budFaces[0] = (headFace + 1) % FACE_COUNT;
  budFaces[1] = (headFace + 2) % FACE_COUNT;
  budFaces[2] = (headFace + 4) % FACE_COUNT;
  budFaces[3] = (headFace + 5) % FACE_COUNT;
}

void randomizeBudAffinity() {
  bool becomeBud = false;
  if (becomeBudCoinFlipTimer.isExpired()) {
    becomeBud = flipCoin();
  }

  // should i be a bud?
  if (becomeBud) {
    blinkState = BUD;
    branchState = BUDDING;
  } else {
    becomeBudCoinFlipTimer.set(BECOME_BUD_COIN_FLIP_COOLDOWN_MS);
  }
}
