#include "playing.h"

// ------------------------
// ---- trunk / branch ----
// ------------------------

// set to true if the growth path is split left/right
bool isSplit;

// set to true if the segment did something when the game timer expired
bool hasExpiredGameTimerActed;

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

// the face that's actively budding or NOT_SET if not budding
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

// --- initialize ---

// initializing unsigned 8-bit ints to NOT_SET is a little sketchy, but shouldn't be a problem
// it's effectively initializing them to 255, which doesn't have collision with
// the current code's use cases

void initPlayVariables() {
  isSplit = false;

  receivingGrowth = false;

  isLeafSignalTimerStarted = false;
  hasLeafFlashedGreeting = false;

  rearFace = NOT_SET;
  headFace = NOT_SET;
  headFaceLeft = NOT_SET;
  headFaceRight = NOT_SET;

  isGameTimerStarted = false;
  hasExpiredGameTimerActed = false;

  activeBudFace = NOT_SET;

  gameState = GameState::PLAYING;
  blinkState = BlinkState::NONE;
  leafState = LeafState::NAL;
  branchState = BranchBudState::NAB;

  soilTimer.set(0);
  leafLifeTimer.set(0);
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
      rearFace = NOT_SET;
      continue;
    } else {
      // globally track our rear face position
      // rear - what the blink connects to / receives value from
      // head - empty and ready for broadcast / more connections
      // NOTE: this only works in this else-block for reading / tracking one rear face at a time
      // if the game requires to track multiple connection points, then this paradigm will need to change (array?)
      rearFace = f;
    }

    switch (curFaceValue) {
      // setup trunk message comes from other trunk blinks or the sprout
      case Message::SETUP_TRUNK:
        blinkState = BlinkState::TRUNK;
        headFace = OPPOSITE_FACE(f);
        return;  // for now, only read one message from NONE state
      // setup branch message comes from other branches or the split trunk
      case Message::SETUP_BRANCH:
        blinkState = BlinkState::BRANCH;
        headFace = OPPOSITE_FACE(f);

        // determine where buds can go initially
        updateBudFaces();
        return;  // for now, only read one message from NONE state
      // a bud is looking for a leaf - so we become a leaf
      case Message::LOOKING_FOR_LEAF:
        blinkState = BlinkState::LEAF;
        leafState = LeafState::NEW;

        // this head face index is not currently used, but let's track it in case
        headFace = OPPOSITE_FACE(f);

        // tell the branch, it got a leaf (rearFace is the leaf stem)
        setValueSentOnFace(Message::LOOKING_FOR_LEAF_ACK, rearFace);
        return;  // for now, only read one message from NONE state
      default:
        continue;
    }
  }
}

// right now soil just exists to be a buffer transition to sprout
// we'll animate something here
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

// starts the game timer and initiates growth up the tree with single clicks
void playingSprout() {
  // allow undo sprout for overzealous players
  if (!isGameTimerStarted && isAlone() && buttonDoubleClicked()) {
    blinkState = BlinkState::NONE;
    return;
  }

  // the program determines the head face orientation
  if (headFace == NOT_SET) {
    headFace = FACE_SPROUT;
    rearFace = OPPOSITE_FACE(headFace);
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
}

void playingTrunk() {
  byte rxRear = getLastValueReceivedOnFace(rearFace);
  bool isHeadClear = isSplit ? isValueReceivedOnFaceExpired(headFaceLeft) && isValueReceivedOnFaceExpired(headFaceRight) : isValueReceivedOnFaceExpired(headFace);

  // if the game was started AND the timer expired in this segment AND we haven't acted, yet
  // then either pass the message along to start the next timer segment or end the game.
  // this extra check is necessary so that expired trunk segments can still transmit GROW message
  // in below switch statement on rxRear
  if (isGameTimerStarted && !hasExpiredGameTimerActed && gameTimer.isExpired()) {
    if (isSplit) {
      // game time reached the top - game over!
      setValueSentOnAllFaces(Message::END_GAME);
    } else {
      // start the next timer in the next trunk segment
      setValueSentOnFace(Message::START_THE_CLOCK_NOW, headFace);
    }
    hasExpiredGameTimerActed = true;
    return;
  }

  switch (rxRear) {
    case Message::SETUP_TRUNK:
      // if our head is not receiving anything, and we're double-clicked, then we split
      if (isHeadClear && buttonDoubleClicked()) {
        // allow undo - use not operator
        isSplit = !isSplit;

        // only calculate this once per loop when the change is made
        headFaceLeft = isSplit ? CW_FROM_FACE(rearFace, 2) : NOT_SET;
        headFaceRight = isSplit ? CCW_FROM_FACE(rearFace, 2) : NOT_SET;
      }

      if (isSplit) {
        // if i'm a split trunk, i'm making branches
        setValueSentOnFace(Message::SETUP_BRANCH, headFaceLeft);
        setValueSentOnFace(Message::SETUP_BRANCH, headFaceRight);
      } else {
        // tell the next guy i'm a trunk, so you will be too
        setValueSentOnFace(Message::SETUP_TRUNK, headFace);
      }  // isSplit
      return;
    case Message::START_THE_CLOCK_NOW:
      // time to start!
      if (!isGameTimerStarted) {
        gameTimer.set(GAME_TIMER_MS);
        isGameTimerStarted = true;  // trunk's accounting
      }
      return;
    case Message::GROW:
      // we heard the grow message from the sprout
      receivingGrowth = true;

      // send GROW message along
      if (isSplit) {
        setValueSentOnFace(Message::GROW, headFaceLeft);
        setValueSentOnFace(Message::GROW, headFaceRight);
      } else {
        setValueSentOnFace(Message::GROW, headFace);
      }  // isSplit
      return;
    case Message::QUIET:
      receivingGrowth = false;
      setValueSentOnAllFaces(Message::QUIET);
      return;
    default:
      return;
  }
}  // playingTrunk

void playingBranch() {
  byte rxRear = getLastValueReceivedOnFace(rearFace);
  bool isHeadClear = isSplit ? isValueReceivedOnFaceExpired(headFaceLeft) && isValueReceivedOnFaceExpired(headFaceRight) : isValueReceivedOnFaceExpired(headFace);

  switch (rxRear) {
    case Message::SETUP_BRANCH:
      // if our head is not receiving anything, and we're double-clicked, then we split
      if (isHeadClear && buttonDoubleClicked()) {
        // allow undo - use not operator
        isSplit = !isSplit;

        // only calculate this once per loop when the change is made
        headFaceLeft = isSplit ? CW_FROM_FACE(rearFace, 2) : NOT_SET;
        headFaceRight = isSplit ? CCW_FROM_FACE(rearFace, 2) : NOT_SET;

        // determine where buds can go after a split
        updateBudFaces();
      }

      if (isSplit) {
        // if i'm a split branch, i'm making branches
        setValueSentOnFace(Message::SETUP_BRANCH, headFaceLeft);
        setValueSentOnFace(Message::SETUP_BRANCH, headFaceRight);
      } else {
        // tell the next guy i'm a branch, so you will be too
        setValueSentOnFace(Message::SETUP_TRUNK, headFace);
      }
      break;
    case Message::GROW:
      // received grow, so let's send it along
      receivingGrowth = true;
      if (isSplit) {
        setValueSentOnFace(Message::GROW, headFaceLeft);
        setValueSentOnFace(Message::GROW, headFaceRight);
      } else {
        setValueSentOnFace(Message::GROW, headFace);
      }  // isSplit

      // let's also figure out if we're going to bud
      if (becomeBudCoinFlipTimer.isExpired()) {
        branchState = BranchBudState::RANDOMIZING;
      }
      break;
    case Message::QUIET:
      receivingGrowth = false;
      setValueSentOnAllFaces(Message::QUIET);
      return;
  }  // switch (rxRear)

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
  }  // switch (branchState)
}  //playingBranch

void playingBud() {
  bool isFinalBranch = isSplit ? isValueReceivedOnFaceExpired(headFaceLeft) && isValueReceivedOnFaceExpired(headFaceRight) : isValueReceivedOnFaceExpired(headFace);

  switch (branchState) {
    case BranchBudState::BUDDING:
      if (activeBudFace == NOT_SET) {
        // reset leaf signal timer for current future leaves
        isLeafSignalTimerStarted = false;
        activeBudSeekingLeafTimer.set(random(ASK_FOR_LEAF_MIN_TIME_MS, ASK_FOR_LEAF_MAX_TIME_MS));
        activeBudFace = isFinalBranch ? budFaces[random(5)] : budFaces[random(4)];
        setValueSentOnFace(Message::LOOKING_FOR_LEAF, activeBudFace);
      } else {
        if (activeBudSeekingLeafTimer.isExpired()) {
          setValueSentOnFace(Message::QUIET, activeBudFace);
          activeBudFace = NOT_SET;
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
