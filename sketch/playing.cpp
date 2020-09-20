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

byte budFaces[5];
byte activeBudFace;
bool branchAlive;
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

  branchAlive = true;

  leafState = LeafState::NAL;
  branchState = BranchBudState::NAB;

  soilTimer.set(0);
  txGrowthTimer.set(0);
}

void playGame() {
  switch (gameState) {
    case GameState::SETUP:
      gameStateSetup();
      break;
    case GameState::PLAYING:
      gameStatePlaying();
      detectResetGame();
      break;
    case GameState::GAME_OVER:
      gameStateGameOver();
      detectResetGame();
      break;
  }
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

  if (isAlone()) {
    blinkState = BlinkState::NONE;
  }
}

// -------- Playing methods -------

void playingNone() {
  if (buttonDoubleClicked()) {
    blinkState = BlinkState::SOIL;
    soilTimer.set(5000);
    return;
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

    if (faceValue == Message::SETUP_TRUNK) {
      blinkState = BlinkState::TRUNK;
      headFace = OPPOSITE_FACE(f);
      setValueSentOnFace(Message::SETUP_TRUNK, headFace);
      break;
    } else if (faceValue == Message::SETUP_TRUNK && buttonDoubleClicked()) {
      blinkState = BlinkState::TRUNK;
      isTrunkSplit = true;
      headFaceLeft = CW_FROM_FACE(f, 2);
      headFaceRight = CCW_FROM_FACE(f, 2);
      setValueSentOnFace(Message::SETUP_BRANCH, headFaceLeft);
      setValueSentOnFace(Message::SETUP_BRANCH, headFaceRight);
      break;
    } else if (faceValue == Message::SETUP_BRANCH) {
      blinkState = BlinkState::BRANCH;
      headFace = OPPOSITE_FACE(f);
      setValueSentOnFace(SETUP_BRANCH, headFace);
      updateBudFaces();
      break;
    } else if (faceValue == Message::SETUP_BRANCH && buttonDoubleClicked()) {
      isFinalBranch = true;
      blinkState = BlinkState::BRANCH;
      branchState = BranchBudState::RANDOMIZING;
      headFace = OPPOSITE_FACE(f);  // not used
      setValueSentOnFace(Message::START_BUDDING, rearFace);
      break;
    } else if (faceValue == Message::LOOKING_FOR_LEAF) {
      blinkState = BlinkState::LEAF;
      leafState = LeafState::NEW;
      headFace = OPPOSITE_FACE(f);                                  // not used
      setValueSentOnFace(Message::LOOKING_FOR_LEAF_ACK, rearFace);  // rearFace is the leaf stem
      break;
    }
  }
}

void playingSoil() {
  // allow undo switch to soil
  if (buttonDoubleClicked() && isAlone()) {
    blinkState = BlinkState::NONE;
    return;
  }

  if (soilTimer.isExpired()) {
    blinkState = BlinkState::SPROUT;
  }
}

void playingSprout() {
  // allow undo sprout for overzealous players
  if (buttonDoubleClicked() && isAlone()) {
    blinkState = BlinkState::NONE;
    return;
  }

  headFace = FACE_SPROUT;
  setValueSentOnFace(Message::SETUP_TRUNK, headFace);

  if (buttonSingleClicked()) {
    txGrowthTimer.set(GROWTH_DELAY_MS);
    growthInitiated = true;
    return;
  }

  if (growthInitiated == true && txGrowthTimer.isExpired()) {
    sendingGrowth = true;
  }

  // send up growth command / lights
  if (sendingGrowth) {
    sendGrowth();
    growthInitiated = false;
  }

  if (getLastValueReceivedOnFace(headFace) == Message::GROW_ACK) {
    sendingGrowth = false;
  }

  if (getLastValueReceivedOnFace(headFace) == Message::START_THE_CLOCK) {
    setValueSentOnFace(Message::START_THE_CLOCK_NOW, headFace);
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
    if (rxHeadLeft == rxHeadRight && rxHeadRight == Message::START_BUDDING) {
      // message down to trunk 1 that it's time
      setValueSentOnFace(Message::START_THE_CLOCK, rearFace);
    }
  }

  if (rxHead == START_THE_CLOCK) {
    setValueSentOnFace(Message::START_THE_CLOCK, rearFace);
  }

  if (rxRear == START_THE_CLOCK_NOW) {
    gameTimer.set(GAME_TIMER_MS);
    isGameTimerStarted = true;
  }

  if (isGameTimerStarted && gameTimer.isExpired()) {
    if (isTrunkSplit) {
      // game time reached the top - game over!
      setValueSentOnAllFaces(Message::END_GAME);
    } else {
      setValueSentOnFace(Message::START_THE_CLOCK_NOW, headFace);
    }
  }
}

void playingBranch() {
  byte rxRear = getLastValueReceivedOnFace(rearFace);
  byte rxHead = getLastValueReceivedOnFace(headFace);

  switch (branchState) {
    case BranchBudState::NAB:
      // --- do the growth stuff
      // TODO: add growthTimer and use growthInitiated
      if (rxRear == Message::GROW) {
        setValueSentOnFace(Message::GROW_ACK, rearFace);
      }

      if (rxRear == Message::GROW && !isFinalBranch) {
        sendingGrowth = true;
      }

      if (rxHead == Message::GROW_ACK) {
        sendingGrowth = false;
      }

      // --- do the post-growth stuff
      if (rxHead == Message::START_BUDDING) {
        branchState = BranchBudState::RANDOMIZING;
        becomeBudCoinFlipTimer.set(BECOME_BUD_COIN_FLIP_COOLDOWN_MS);
        setValueSentOnFace(Message::START_BUDDING, rearFace);
      }

      break;
    case BranchBudState::RANDOMIZING:
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
    case BranchBudState::BUDDING:
      if (activeBudFace == -1) {
        activeBudSeekingLeafTimer.set(random(ASK_FOR_LEAF_MIN_TIME_MS, ASK_FOR_LEAF_MAX_TIME_MS));
        activeBudFace = isFinalBranch ? budFaces[random(5)] : budFaces[random(4)];
        setValueSentOnFace(Message::LOOKING_FOR_LEAF, activeBudFace);
      } else {
        if (activeBudSeekingLeafTimer.isExpired()) {
          setValueSentOnFace(Message::QUIET, activeBudFace);
          activeBudFace = -1;
          branchState = BranchBudState::TOO_LATE;
          tooLateCoolDownTimer.set(TOO_LATE_COOL_DOWN_MS);
        } else {
          if (getLastValueReceivedOnFace(activeBudFace) == LOOKING_FOR_LEAF_ACK) {
            branchState = BranchBudState::GREW_A_LEAF;
          }
        }
      }
      break;
    case BranchBudState::TOO_LATE:
      if (tooLateCoolDownTimer.isExpired()) {
        branchState = BranchBudState::RANDOMIZING;
        becomeBudCoinFlipTimer.set(BECOME_BUD_COIN_FLIP_COOLDOWN_MS);
      }
      break;
    case BranchBudState::GREW_A_LEAF:
      playingBudWithLeaf();
      break;
  }
}

void playingBudWithLeaf() {
  byte rxBud = getLastValueReceivedOnFace(activeBudFace);

  if (!isLeafSignalTimerStarted) {
    // send connected
    isLeafSignalTimerStarted = true;
    setValueSentOnFace(Message::BRANCH_GREET_LEAF, activeBudFace);
    leafSignalTimer.set(random(LEAF_PLAY_TIME_MIN_MS, LEAF_PLAY_TIME_MAX_MS));
    // don't advance maturity on initial leaf signal timer reset
    return;
  }

  if (leafSignalTimer.isExpired()) {
    setValueSentOnFace(Message::BRANCH_MATURE_LEAF, activeBudFace);
    leafSignalTimer.set(random(LEAF_PLAY_TIME_MIN_MS, LEAF_PLAY_TIME_MAX_MS));
  }

  if (rxBud == Message::BRANCH_MATURE_LEAF_ACK) {
    setValueSentOnFace(Message::QUIET, activeBudFace);
  } else if (rxBud == Message::SEND_POISON) {
    branchAlive = false;
  }
}

void playingLeaf() {
  byte rxRear = getLastValueReceivedOnFace(rearFace);
  bool isRearValueExpired = isValueReceivedOnFaceExpired(rearFace);

  if (rxRear == Message::BRANCH_MATURE_LEAF && !isRearValueExpired) {
    setValueSentOnFace(Message::BRANCH_MATURE_LEAF_ACK, rearFace);
    if (leafState < LeafState::DEAD_LEAF) {
      ++leafState;
    }
  }

  switch (leafState) {
    case LeafState::NAL:
    case LeafState::NEW:
    case LeafState::YOUNG:
    case LeafState::MATURE:
    case LeafState::DYING:
      break;
    case LeafState::DEAD_LEAF:
      setValueSentOnFace(Message::SEND_POISON, rearFace);
      break;
  }
}

// ----- Game Helpers ------

// set GROW_ACK on rearFace
void ackGrowth() {
  setValueSentOnFace(Message::GROW_ACK, rearFace);
}

// set GROW on headFace
void sendGrowth() {
  setValueSentOnFace(Message::GROW, headFace);
}

void sendSplitGrowth() {
  setValueSentOnFace(Message::GROW, headFaceLeft);
  setValueSentOnFace(Message::GROW, headFaceRight);
}

void updateBudFaces() {
  budFaces[0] = CW_FROM_FACE(headFace, 1);
  budFaces[1] = CW_FROM_FACE(headFace, 2);
  budFaces[2] = CCW_FROM_FACE(headFace, 1);
  budFaces[3] = CCW_FROM_FACE(headFace, 2);
  budFaces[4] = OPPOSITE_FACE(rearFace);
}

void randomizeBudAffinity() {
  bool becomeBud = false;
  if (becomeBudCoinFlipTimer.isExpired()) {
    becomeBud = flipCoin();
  }

  // should i be a bud?
  if (becomeBud) {
    blinkState = BlinkState::BUD;
    branchState = BranchBudState::BUDDING;
  } else {
    becomeBudCoinFlipTimer.set(BECOME_BUD_COIN_FLIP_COOLDOWN_MS);
  }
}
