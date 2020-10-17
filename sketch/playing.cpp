#include "playing.h"

// ---- trunk / branch ----

bool isSplit;

// --- growth ----

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
  isSplit = false;

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
      detectEndGame();
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
    soilTimer.set(SOIL_PLAY_TIME_MS);
    return;
  }

  // we start off alone so...
  // let's find out when we become attached!
  // check for signals - can be from any stage of the game play
  FOREACH_FACE(f) {
    byte curFaceValue = getLastValueReceivedOnFace(f);
    bool curFaceValueExpired = isValueReceivedOnFaceExpired(f);
    if (curFaceValueExpired || curFaceValue == Message::QUIET) {
      // continue - nothing to read
      // also reset rearFace
      rearFace = -1;
      continue;
    } else {
      // globally track our rear face position
      // rear - what the blink connects to / receives value from
      // head - empty and ready for broadcast / more connections
      rearFace = f;
    }

    switch (curFaceValue) {
      case Message::SETUP_TRUNK:
        blinkState = BlinkState::TRUNK;
        headFace = OPPOSITE_FACE(f);
        setValueSentOnFace(Message::SETUP_TRUNK, headFace);  // tell the next guy i'm a trunk, so you will be too
        setValueSentOnFace(Message::SETUP_TRUNK, rearFace);  // tell the previous guy i'm a trunk
        break;
      case Message::SETUP_BRANCH:
        blinkState = BlinkState::BRANCH;
        headFace = OPPOSITE_FACE(f);
        setValueSentOnFace(Message::SETUP_BRANCH, headFace);  // tell the next guy i'm a branch, so you will be too
        setValueSentOnFace(Message::SETUP_BRANCH, rearFace);  // tell the previous guy i'm a branch
        updateBudFaces();
        break;
      case Message::LOOKING_FOR_LEAF:
        blinkState = BlinkState::LEAF;
        leafState = LeafState::NEW;
        headFace = OPPOSITE_FACE(f);                                  // not used...yet :)
        setValueSentOnFace(Message::LOOKING_FOR_LEAF_ACK, rearFace);  // tell the branch, it got a leaf (rearFace is the leaf stem)
        break;
      default:
        continue;
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
  if (isAlone() && buttonDoubleClicked()) {
    blinkState = BlinkState::NONE;
    return;
  }

  // the program determines the head face orientation
  if (headFace = -1) {
    headFace = FACE_SPROUT;
    setValueSentOnFace(Message::SETUP_TRUNK, headFace);
    return;
  }

  // long press to start the game
  if (!isGameTimerStarted && buttonLongPressed()) {
    setValueSentOnFace(Message::START_THE_CLOCK_NOW, headFace);
    isGameTimerStarted = true;  // sprout's accounting
    return;
  }

  // initiate growth cycle from the sprout
  // once growth hits branches, it should possibly grow leaves
  if (isGameTimerStarted) {
    if (buttonSingleClicked()) {
      setValueSentOnFace(Message::GROW, headFace);
      return;
    }
    setValueSentOnFace(Message::QUIET, headFace);
  }

  // TODO: make sure sprout is doing some fun pulsing in the color routines
}

void playingTrunk() {
  byte rxRear = getLastValueReceivedOnFace(rearFace);
  byte rxHead = getLastValueReceivedOnFace(headFace);
  bool isHeadClear = isValueReceivedOnFaceExpired(headFace);

  // if our head is not receiving anything, and we're double-clicked, then we split
  if (buttonDoubleClicked() && isHeadClear) {
    // allow undo - use not operator
    isSplit = !isSplit;
    headFaceLeft = CW_FROM_FACE(rearFace, 2);
    headFaceRight = CCW_FROM_FACE(rearFace, 2);
    setValueSentOnFace(Message::SETUP_BRANCH, headFaceLeft);
    setValueSentOnFace(Message::SETUP_BRANCH, headFaceRight);
    return;
  }

  if (!isGameTimerStarted && rxRear == START_THE_CLOCK_NOW) {
    gameTimer.set(GAME_TIMER_MS);
    isGameTimerStarted = true;  // trunk's accounting
    return;
  }

  if (rxRear == Message::GROW) {
    receivingGrowth = true;
    if (isSplit) {
      setValueSentOnFace(Message::GROW, headFaceLeft);
      setValueSentOnFace(Message::GROW, headFaceRight);
    } else {
      setValueSentOnFace(Message::GROW, headFace);
    }
    return;
  } else if (rxRear == Message::QUIET) {
    receivingGrowth = false;
    setValueSentOnAllFaces(Message::QUIET);
    return;
  }

  if (isGameTimerStarted && gameTimer.isExpired()) {
    if (isSplit) {
      // game time reached the top - game over!
      setValueSentOnAllFaces(Message::END_GAME);
    } else {
      // start the next timer in the next trunk segment
      setValueSentOnFace(Message::START_THE_CLOCK_NOW, headFace);
    }
  }
}

void playingBranch() {
  byte rxRear = getLastValueReceivedOnFace(rearFace);
  byte rxHead = getLastValueReceivedOnFace(headFace);
  bool isHeadClear = isValueReceivedOnFaceExpired(headFace);

  // if our head is not receiving anything, and we're double-clicked, then we split
  if (buttonDoubleClicked() && isHeadClear) {
    // allow undo - use not operator
    isSplit = !isSplit;
    headFaceLeft = CW_FROM_FACE(rearFace, 2);
    headFaceRight = CCW_FROM_FACE(rearFace, 2);
    setValueSentOnFace(Message::SETUP_BRANCH, headFaceLeft);
    setValueSentOnFace(Message::SETUP_BRANCH, headFaceRight);
    return;
  }

  // received grow, so let's send it along
  if (rxRear == Message::GROW) {
    receivingGrowth = true;
    if (isSplit) {
      setValueSentOnFace(Message::GROW, headFaceLeft);
      setValueSentOnFace(Message::GROW, headFaceRight);
    } else {
      setValueSentOnFace(Message::GROW, headFace);
    }

    // let's also figure out if we're going to bud
    if (becomeBudCoinFlipTimer.isExpired()) {
      branchState = BranchBudState::RANDOMIZING;
    }
  } else if (rxRear == Message::QUIET) {
    receivingGrowth = false;
    setValueSentOnAllFaces(Message::QUIET);
    return;
  }

  switch (branchState) {
    case BranchBudState::NAB:
      // do nothing
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
  bool isFinalBranch = isValueReceivedOnFaceExpired(headFace);

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
        blinkState = BlinkState::BRANCH;
        branchState = BranchBudState::NAB;
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
  bool becomeBud = flipCoin() & flipCoin();

  // should i be a bud?
  if (becomeBud) {
    blinkState = BlinkState::BUD;
    branchState = BranchBudState::BUDDING;
  } else {
    becomeBudCoinFlipTimer.set(BECOME_BUD_COIN_FLIP_COOLDOWN_MS);
    blinkState = BlinkState::BRANCH;
    branchState = BranchBudState::NAB;
  }
}
