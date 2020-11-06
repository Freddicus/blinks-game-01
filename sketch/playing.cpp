#include "playing.h"

// --------------
// ---- soil ----
// --------------

// used briefly to countdown the transition from soil to sprout
Timer soilTimer;

// ------------------------
// ---- trunk / branch ----
// ------------------------

// set to true if the growth path is split left/right
bool isSplit;

// set to true if the segment did something when the game timer expired
bool hasExpiredGameTimerActed;

// ---------------------------
// ---- branch / bud play ----
// ---------------------------

// holds face indexes for potential buds
byte budFaces[5];

// the face that's actively budding or NOT_SET if not budding
byte activeLeafFace;

// track the current state of the branch / bud
byte branchState;

// ---------------------------
// ---- bud <-> leaf play ----
// ---------------------------

// true if the leaf signal timer has started
bool isLeafSignalTimerStarted;

// timer that controls whether or not to flip a coin to grow a leaf
Timer growLeafCoinFlipTimer;

// timer that controls how long to seek a leaf
Timer leafTimer;

// -------------------
// ---- leaf play ----
// -------------------

// --- misc ---

Timer messageSpacer;

// --- initialize ---

// initializing unsigned 8-bit ints to NOT_SET is a little sketchy, but shouldn't be a problem
// it's effectively initializing them to 255, which doesn't have collision with
// the current code's use cases

void initPlayVariables() {
  isSplit = false;

  isLeafSignalTimerStarted = false;

  isGameStarted = false;

  rearFace = NOT_SET;
  headFace = NOT_SET;
  headFaceLeft = NOT_SET;
  headFaceRight = NOT_SET;

  isGameTimerStarted = false;
  hasExpiredGameTimerActed = false;

  activeLeafFace = NOT_SET;

  gameState = GameState::PLAYING;
  blinkState = BlinkState::NONE;
  branchState = BranchState::NAB;

  soilTimer.set(0);
  leafTimer.set(0);
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
    case BlinkState::COLLECTOR:
      playingCollector();
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
    if (curFaceValueExpired) {
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
      default:
        continue;
    }
  }
}

// right now soil just exists to be a buffer transition to sprout
// we'll animate something here
void playingSoil() {
  // allow undo switch to soil
  if (isAlone() && buttonDoubleClicked()) {
    blinkState = BlinkState::NONE;
    return;
  }

  if (soilTimer.isExpired()) {
    blinkState = BlinkState::SPROUT;
  }
}

// starts the game timer
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
    isGameStarted = true;  // sprout's accounting
    setValueSentOnFace(Message::START_THE_GAME, headFace);
    messageSpacer.set(500);
    return;
  }

  if (!isGameTimerStarted && isGameStarted && messageSpacer.isExpired()) {
    setValueSentOnFace(Message::START_THE_CLOCK_NOW, headFace);
    isGameTimerStarted = true;  // sprout's accounting
    return;
  }
}

void playingTrunk() {
  byte rxRear = getLastValueReceivedOnFace(rearFace);
  bool isHeadClear = isSplit ? isValueReceivedOnFaceExpired(headFaceLeft) && isValueReceivedOnFaceExpired(headFaceRight) : isValueReceivedOnFaceExpired(headFace);

  // if the game was started AND the timer expired in this segment AND we haven't acted, yet
  // then either pass the message along to start the next timer segment or end the game.
  // this extra check is necessary so that expired trunk segments can still transmit message
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
    case Message::START_THE_GAME:
      isGameStarted = true;  // trunk's accounting
      if (isSplit) {
        setValueSentOnFace(Message::START_THE_GAME, headFaceLeft);
        setValueSentOnFace(Message::START_THE_GAME, headFaceRight);
      } else {
        setValueSentOnFace(Message::START_THE_GAME, headFace);
      }  // isSplit
      return;
    case Message::START_THE_CLOCK_NOW:
      // time to start!
      if (!isGameTimerStarted) {
        gameTimer.set(GAME_TIMER_MS);
        isGameTimerStarted = true;  // trunk's accounting
      }
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
        setValueSentOnFace(Message::SETUP_BRANCH, headFace);
      }
      break;
    case Message::START_THE_GAME:
      isGameStarted = true;  // branch's accounting (not used)
      if (isSplit) {
        setValueSentOnFace(Message::START_THE_GAME, headFaceLeft);
        setValueSentOnFace(Message::START_THE_GAME, headFaceRight);
      } else {
        setValueSentOnFace(Message::START_THE_GAME, headFace);
      }  // isSplit

      // start randomize immediately
      branchState = BranchState::RANDOMIZING;
      break;
    default:
      break;
  }  // switch (rxRear)

  switch (branchState) {
    case BranchState::NAB:
      // do nothing
      break;
    case BranchState::RANDOMIZING:
      randomizeLeafGrowing();
      break;
    case BranchState::GREW_A_LEAF:
      playingBranchWithLeaf();
      break;
  }  // switch (branchState)
}  //playingBranch

void playingBranchWithLeaf() {
  bool isFinalBranch = isSplit ? isValueReceivedOnFaceExpired(headFaceLeft) && isValueReceivedOnFaceExpired(headFaceRight) : isValueReceivedOnFaceExpired(headFace);

  if (activeLeafFace == NOT_SET) {
    // reset leaf signal timer for current future leaves
    isLeafSignalTimerStarted = false;
    leafTimer.set(random(ASK_FOR_LEAF_MIN_TIME_MS, ASK_FOR_LEAF_MAX_TIME_MS));
    activeLeafFace = isFinalBranch ? budFaces[random(5)] : budFaces[random(4)];
    setValueSentOnFace(Message::LEAF_GREEN, activeLeafFace);
    // TODO: curr color / message_color
  } else {
    if (leafTimer.isExpired()) {
      setValueSentOnFace(Message::QUIET, activeLeafFace);
      activeLeafFace = NOT_SET;
    }
  }
}

void playingCollector() {
}

// ----- Game Helpers ------

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

void randomizeLeafGrowing() {
  if (!growLeafCoinFlipTimer.isExpired()) {
    return;
  }

  bool becomeBud = flipCoin();

  // should i be a bud?
  if (becomeBud) {
    branchState = BranchState::GREW_A_LEAF;
  } else {
    growLeafCoinFlipTimer.set(BECOME_BUD_COIN_FLIP_COOLDOWN_MS);
    branchState = BranchState::NAB;
  }
}
