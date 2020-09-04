/*
 * Make Like A Tree and Leaf
 * by Freddicus Game Studios, LLC
 * 
 * Author(s): Alfred Sterphone
 * 
 * Repo: https://github.com/Freddicus/blinks-game-01
 * Discussion: https://forum.move38.com/t/new-game-wip-make-like-a-tree-and-leaf/549
 */

#include "colors.h"
#include "globals.h"
#include "states.h"

// -------- global variables --------

byte gameState;
byte blinkState;
byte leafState;
byte branchState;

byte rearFace;
byte headFace;
byte headFaceLeft;
byte headFaceRight;

byte sharedPulseDimness;

// ---- trunk / branch ----

bool isTrunkSplit;
Timer gameTimer;
bool isGameTimerStarted;

bool isFinalBranch;

// --- growth ----

bool growthInitiated;
bool sendingGrowth;
bool receivingGrowth;

bool gotSetupMsg;

Timer soilTimer;

Timer txGrowthTimer;

// ---- branch / bud play ----

byte budFaces[4];
byte activeBudFace;
byte branchHitPoints;

Timer becomeBudCoinFlipTimer;
Timer activeBudSeekingLeafTimer;
Timer tooLateCoolDownTimer;

// leaf play

Timer leafLifeTimer;

// --- initialize ---

void setup() {
  randomize();

  gameState = SETUP;
  blinkState = NONE;
  leafState = NAL;
  branchState = NAB;

  rearFace = -1;
  headFace = -1;
  headFaceLeft = -1;
  headFaceRight = -1;

  isTrunkSplit = false;
  isGameTimerStarted = false;

  isFinalBranch = false;

  growthInitiated = false;
  sendingGrowth = false;
  receivingGrowth = false;

  gotSetupMsg = false;

  activeBudFace = -1;
  branchHitPoints = INITIAL_BRANCH_HIT_POINTS;

  setColor(COLOR_NONE);
}

// --- game loop ---

void loop() {
  if (hasWoken()) {
    setup();
  }

  updateSharedPulseDimness();

  // game instructions will state to the player to start with all blinks together
  // then the player will double click a blink to start the game
  // the player will need to separate all blinks for the game to begin

  if (gameState == SETUP) {
    // button double clicked - need to tell neighbors to start
    if (buttonDoubleClicked()) {
      // we're in setup mode (currently at startup only)
      // indicate to the user to detach all blinks on all blinks
      setValueSentOnAllFaces(PLEASE_DETACH);

      // originator got the setup message
      gotSetupMsg = true;

      // indicate sending setup message
      setColor(GREEN);
    }

    FOREACH_FACE(f) {
      byte faceValue = getLastValueReceivedOnFace(f);
      bool faceValueExpired = isValueReceivedOnFaceExpired(f);

      // receiving the please detach message
      if (faceValue == PLEASE_DETACH && !faceValueExpired) {
        // pass it along to everyone!
        setValueSentOnAllFaces(PLEASE_DETACH);

        // mark received
        setColorOnFace(GREEN, f);
      } else if (faceValue == PLEASE_DETACH && faceValueExpired) {
        // no longer actively receiving detach message - note and indicate
        gotSetupMsg = true;
        pulseColorOnFace(GREEN, f, sharedPulseDimness);
      }
    }  // for each face

    // if i'm alone, then i'm ready to play
    if (gotSetupMsg && isAlone()) {
      gameState = PLAYING;
      // don't send anything
      setValueSentOnAllFaces(QUIET);
    }
  }  // if SETUP

  if (gameState == PLAYING) {
    switch (blinkState) {
      case NONE:
        playingNone();
        break;
      case SOIL:
        playingSoil();
        break;
      case SPROUT:
        playingSprout();
        break;
      case TRUNK:
        playingTrunk();
        break;
      case BRANCH:
        playingBranch();
        break;
      case BUD:
        playingBud();
        break;
      case LEAF:
        playingLeaf();
        break;
    }

    updateColors();
    detectPanic();
  }  // if PLAYING
}  // loop

// -------- Playing methods -------

void playingNone() {
  if (buttonDoubleClicked()) {
    blinkState = SOIL;
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
      blinkState = TRUNK;
      headFace = oppositeFaces[f];
      setValueSentOnFace(faceValue + 1, headFace);
      break;
    } else if (faceValue == TRUNK_5) {
      blinkState = TRUNK;
      isTrunkSplit = true;
      headFaceLeft = oppositeFaces[f] - 1;
      headFaceRight = oppositeFaces[f] + 1;
      setValueSentOnFace(BRANCH_LEFT_1, headFaceLeft);
      setValueSentOnFace(BRANCH_RIGHT_1, headFaceRight);
      break;
    } else if (faceValue >= BRANCH_LEFT_1 || faceValue < BRANCH_LEFT_4) {
      blinkState = BRANCH;
      headFace = oppositeFaces[f];
      setValueSentOnFace(faceValue + 1, headFace);
      updateBudFaces();
    } else if (faceValue >= BRANCH_RIGHT_1 || faceValue < BRANCH_RIGHT_4) {
      blinkState = BRANCH;
      headFace = oppositeFaces[f];
      setValueSentOnFace(faceValue + 1, headFace);
      updateBudFaces();
    } else if (faceValue == BRANCH_RIGHT_4 || faceValue == BRANCH_LEFT_4) {
      isFinalBranch = true;
      blinkState = BRANCH;
      branchState = RANDOMIZING;
      headFace = oppositeFaces[f];  // not used
      setValueSentOnFace(START_BUDDING, rearFace);
    } else if (faceValue == LOOKING_FOR_LEAF) {
      blinkState = LEAF;
      leafState = YOUNG;
      headFace = oppositeFaces[f];                         // not used
      setValueSentOnFace(LOOKING_FOR_LEAF_ACK, rearFace);  // rearFace is the leaf stem
    }
  }
}

void playingSoil() {
  // allow undo switch to soil
  if (buttonDoubleClicked() && isAlone()) {
    blinkState = NONE;
  }

  if (soilTimer.isExpired()) {
    blinkState = SPROUT;
  } else {
    // TODO: animate soil about to sprout
  }
}

void playingSprout() {
  // allow undo sprout for overzealous players
  if (buttonDoubleClicked() && isAlone()) {
    blinkState = NONE;
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
        activeBudSeekingLeafTimer.set(random(ASK_FOR_LEAF_MAX_TIME_MS - ASK_FOR_LEAF_MIN_TIME_MS) + ASK_FOR_LEAF_MIN_TIME_MS);
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
      break;
  }
}

void playingLeaf() {
  switch (leafState) {
    case NAL:
      break;
    case YOUNG:
      break;
    case MATURE:
      break;
    case DYING:
      break;
    case DEAD_LEAF:
      break;
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

// ----- Generic Helpers ---------

void detectPanic() {
  if (isAlone() && buttonMultiClicked() && buttonClickCount() == NUM_PANIC_CLICKS) {
    setup();
    setValueSentOnAllFaces(QUIET);
  }
}

void updateSharedPulseDimness() {
  // get progress from 0 - MAX
  int pulseProgress = millis() % PULSE_LENGTH_MS;

  // transform that progress to a byte (0-255)
  byte pulseMapped = map(pulseProgress, 0, PULSE_LENGTH_MS, 0, 255);

  // transform that byte with sin
  sharedPulseDimness = sin8_C(pulseMapped);
}
