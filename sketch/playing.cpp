#include "playing.h"

// ------------------------
// ---- trunk / branch ----
// ------------------------

// set to true if the growth path is split left/right
bool isSplit;

// ---------------
// --- growth ----
// ---------------

// set to true when rear face is getting GROW message
bool receivingGrowth;

// used briefly to countdown the transition from soil to sprout
Timer soilTimer;

// ---------------------------
// ---- branch / bud play ----
// ---------------------------

// holds face indexes for potential buds
byte budFaces[5];

// the face that's actively budding or -1 if not budding
byte activeBudFace;

// track the current state of the branch / bud
byte branchState;

// ---------------------------
// ---- bud <-> leaf play ----
// ---------------------------

// true if the leaf signal timer has started
bool isLeafSignalTimerStarted;

// timer that controls whether or not bud is elegible to randomize to receive a leaf
Timer becomeBudCoinFlipTimer;

// timer that controls how long to seek a leaf
Timer activeBudSeekingLeafTimer;

// timer that controls how long to show that the player was too late in attaching a leaf
Timer tooLateCoolDownTimer;

// timer that binds the leaf to the bud
Timer leafSignalTimer;

// -------------------
// ---- leaf play ----
// -------------------

// TODO: not implemented
Timer leafLifeTimer;

// tracks the state of maturity for the leaf
byte leafState;

bool hasLeafFlashedGreeting;

// ---- setup methods ----

void initPlayVariables() {
  isSplit = false;

  receivingGrowth = false;

  isLeafSignalTimerStarted = false;
  hasLeafFlashedGreeting = false;

  leafState = LeafState::NAL;
  branchState = BranchBudState::NAB;

  soilTimer.set(0);
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
  // double click turns blink into soil
  if (isAlone() && buttonDoubleClicked()) {
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
      // setup trunk message comes from other trunk blinks or the sprout
      case Message::SETUP_TRUNK:
        blinkState = BlinkState::TRUNK;
        headFace = OPPOSITE_FACE(f);
        break;
      // setup branch message comes from other branches or the split trunk
      case Message::SETUP_BRANCH:
        blinkState = BlinkState::BRANCH;
        headFace = OPPOSITE_FACE(f);

        // determine where buds can go initially
        updateBudFaces();
        break;
      // a bud is looking for a leaf - so we become a leaf
      case Message::LOOKING_FOR_LEAF:
        blinkState = BlinkState::LEAF;
        leafState = LeafState::NEW;

        // this head face index is not currently used, but let's track it in case
        headFace = OPPOSITE_FACE(f);

        // tell the branch, it got a leaf (rearFace is the leaf stem)
        setValueSentOnFace(Message::LOOKING_FOR_LEAF_ACK, rearFace);
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

    // only calculate this once per loop when the change is made
    headFaceLeft = isSpilt ? CW_FROM_FACE(rearFace, 2) : -1;
    headFaceRight = isSplit ? CCW_FROM_FACE(rearFace, 2) : -1;
  }

  // always send message to the rear
  setValueSentOnFace(Message::SETUP_TRUNK, rearFace);  // tell the previous guy i'm a trunk

  if (isSplit) {
    // if i'm a split trunk, i'm making branches
    setValueSentOnFace(Message::SETUP_BRANCH, headFaceLeft);
    setValueSentOnFace(Message::SETUP_BRANCH, headFaceRight);
  } else {
    // tell the next guy i'm a trunk, so you will be too
    setValueSentOnFace(Message::SETUP_TRUNK, headFace);
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

    // only calculate this once per loop when the change is made
    headFaceLeft = isSpilt ? CW_FROM_FACE(rearFace, 2) : -1;
    headFaceRight = isSplit ? CCW_FROM_FACE(rearFace, 2) : -1;

    // determine where buds can go after a split
    updateBudFaces();
  }

  // always send message to the rear
  setValueSentOnFace(Message::SETUP_BRANCH, rearFace);  // tell the previous guy i'm a trunk

  if (isSplit) {
    // if i'm a split branch, i'm making branches
    setValueSentOnFace(Message::SETUP_BRANCH, headFaceLeft);
    setValueSentOnFace(Message::SETUP_BRANCH, headFaceRight);
  } else {
    // tell the next guy i'm a branch, so you will be too
    setValueSentOnFace(Message::SETUP_TRUNK, headFace);
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
    case BranchBudState::DEAD_BRANCH:
      return;
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
        // reset leaf signal timer for current future leaves
        isLeafSignalTimerStarted = false;
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

  // leaf signal timer never started, so let's kick off the leaf maturity advancement signaling
  if (!isLeafSignalTimerStarted) {
    // send connected
    isLeafSignalTimerStarted = true;
    setValueSentOnFace(Message::BRANCH_GREET_LEAF, activeBudFace);
    leafSignalTimer.set(random(LEAF_PLAY_TIME_MIN_MS, LEAF_PLAY_TIME_MAX_MS));
    // don't advance maturity on initial leaf signal timer reset
    return;
  }

  // the leaf signal timer is expired... tell the leaf to mature by one, then set the timer
  if (leafSignalTimer.isExpired()) {
    setValueSentOnFace(Message::BRANCH_MATURE_LEAF, activeBudFace);
    leafSignalTimer.set(random(LEAF_PLAY_TIME_MIN_MS, LEAF_PLAY_TIME_MAX_MS));
  }

  if (rxBud == Message::BRANCH_MATURE_LEAF_ACK) {
    setValueSentOnFace(Message::QUIET, activeBudFace);
  } else if (rxBud == Message::SEND_POISON) {
    branchState = BranchBudState::DEAD_BRANCH;
  }
}

void playingLeaf() {
  byte rxRear = getLastValueReceivedOnFace(rearFace);
  bool isRearValueExpired = isValueReceivedOnFaceExpired(rearFace);

  // every time i get the message to mature, i acknowledge and advance my state by one
  if (rxRear == Message::BRANCH_MATURE_LEAF && !isRearValueExpired) {
    setValueSentOnFace(Message::BRANCH_MATURE_LEAF_ACK, rearFace);
    if (leafState < LeafState::DEAD_LEAF) {
      ++leafState;
    }
  } else {
    setValueSentOnFace(Message::QUIET, rearFace);
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
  if (isSplit) {
    budFaces[0] = CW_FROM_FACE(headFace, 1);
    budFaces[1] = CW_FROM_FACE(headFace, 1);
    budFaces[2] = CCW_FROM_FACE(headFace, 1);
    budFaces[3] = CCW_FROM_FACE(headFace, 1);
    budFaces[4] = OPPOSITE_FACE(rearFace);
  } else {
    budFaces[0] = CW_FROM_FACE(headFace, 1);
    budFaces[1] = CW_FROM_FACE(headFace, 2);
    budFaces[2] = CCW_FROM_FACE(headFace, 1);
    budFaces[3] = CCW_FROM_FACE(headFace, 2);
    budFaces[4] = OPPOSITE_FACE(rearFace);
  }
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
