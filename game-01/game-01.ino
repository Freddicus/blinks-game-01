/*
 * Make Like A Tree and Leaf
 * by Freddicus Game Studios, LLC
 * 
 * Author(s): Alfred Sterphone
 * 
 * Repo: https://github.com/Freddicus/blinks-game-01
 * Discussion: https://forum.move38.com/t/new-game-wip-make-like-a-tree-and-leaf/549
 */

// might be overkill -  moving to regular globals for now
//// ---- flash mem data ----
//// #include <avr/pgmspace.h>
//// const static byte oppositeFaces[] PROGMEM = {3, 4, 5, 0, 1, 2};

// ---- enums ----

enum GameState {
  SETUP,
  PLAYING
};

enum BlinkState {
  NONE,
  SOIL,
  SPROUT,
  TRUNK,
  BRANCH,
  BUD,
  LEAF
};

enum LeafState {
  NAL,  // Not A Leaf
  YOUNG,
  MATURE,
  DYING,
  DEAD_LEAF
};

enum BranchBudState {
  NAB,  // Not A Branch/Bud
  RANDOMIZING,
  BUDDING,
  TOO_LATE,
  DEAD_BRANCH
};

// --- simple messages ---

enum Message : byte {
  QUIET,
  TRUNK_1,
  TRUNK_2,
  TRUNK_3,
  TRUNK_4,
  TRUNK_5,
  BRANCH_LEFT_1,
  BRANCH_LEFT_2,
  BRANCH_LEFT_3,
  BRANCH_LEFT_4,
  BRANCH_RIGHT_1,
  BRANCH_RIGHT_2,
  BRANCH_RIGHT_3,
  BRANCH_RIGHT_4,
  GROW,
  GROW_ACK,
  START_BUDDING,
  PLEASE_DETACH
};

// -------- global constants --------

const static byte oppositeFaces[] = {3, 4, 5, 0, 1, 2};

// -------- global variables --------

byte gameState;
byte blinkState;
byte leafState;
byte branchState;

byte rearFace;
byte headFace;
byte headFaceLeft;
byte headFaceRight;

byte pulseDimness;

bool isTrunkSplit;
bool isFinalBranch;

// --- growth ----
bool growthInitiated;
bool sendingGrowth;
bool receivingGrowth;

bool gotSetupMsg;

Timer soilTimer;

Timer txGrowthTimer;

// branch / bud play
byte budFaces[4];
byte activeBudFace;
byte branchHitPoints;

Timer becomeBudCoinFlipTimer;
Timer activeBudSeekingLeafTimer;
Timer tooLateCoolDownTimer;

// leaf play

// --- game values ---

#define NUM_BLINKS 18
#define NUM_PANIC_CLICKS 6

#define COLOR_NONE makeColorRGB(1, 1, 1)         // almost off
#define COLOR_SOIL makeColorRGB(170, 145, 134)   // i know there is no brown, but...
#define COLOR_SPROUT makeColorRGB(191, 255, 0)   // lime greenish
#define COLOR_GROWTH makeColorRGB(84, 164, 222)  // water vibes
#define COLOR_TRUNK makeColorRGB(255, 192, 0)    // basically orange
#define COLOR_BUD COLOR_SPROUT                   // lime greenish
#define COLOR_BRANCH COLOR_TRUNK                 // basically orange

#define FACE_SPROUT 0

#define PULSE_LENGTH_MS 2000
#define GROWTH_DELAY_MS 1250
#define BECOME_BUD_COIN_FLIP_COOLDOWN_MS 7500
#define ASK_FOR_LEAF_MAX_TIME_MS 5000
#define ASK_FOR_LEAF_MIN_TIME_MS 1250
#define TOO_LATE_COOL_DOWN_MS 4000

#define INITIAL_BRANCH_HIT_POINTS 4

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
  isFinalBranch = false;

  growthInitiated = false;
  sendingGrowth = false;
  receivingGrowth = false;

  gotSetupMsg = false;

  activeBudFace = -1;
  branchHitPoints = INITIAL_BRANCH_HIT_POINTS;

  setColor(dim(WHITE, 40));
}

// --- game loop ---

void loop() {
  if (hasWoken()) {
    setup();
  }

  updatePulseDimness();

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
        pulseColorOnFace(GREEN, f);
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

    updateColor();
    detectPanic();
  }  // if PLAYING
}  // loop

// --------- color section ------------

void updateColor() {
  switch (blinkState) {
    case NONE:
      pulseColor(WHITE);
      break;
    case SOIL:
      setColor(COLOR_SOIL);
      break;
    case SPROUT:
      setColorOnFace(COLOR_SPROUT, FACE_SPROUT);
      handleGrowthColor();
      break;
    case TRUNK:
      setColor(COLOR_TRUNK);
      handleGrowthColor();
      break;
    case BRANCH:
      setColor(COLOR_TRUNK);
      handleGrowthColor();
      handleBranchBudColor();
      break;
    case BUD:
      handleBranchBudColor();
      break;
    case LEAF:
      break;
  }
}

void handleGrowthColor() {
  if (sendingGrowth || (growthInitiated == true && !txGrowthTimer.isExpired())) {
    pulseColorOnFace(COLOR_GROWTH, headFace);
  }

  if (receivingGrowth) {
    pulseColorOnFace(COLOR_GROWTH, rearFace);
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
      pulseColorOnFace(COLOR_BUD, activeBudFace);
      break;
    case TOO_LATE:
      pulseColor(RED);
      break;
  }
}

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
      blinkState = BRANCH;
      branchState = RANDOMIZING;
      headFace = oppositeFaces[f];  // not used
      setValueSentOnFace(START_BUDDING, rearFace);
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
}

void playingTrunk() {
  byte rxRear = getLastValueReceivedOnFace(rearFace);
  byte rxHead = getLastValueReceivedOnFace(headFace);

  // TODO: add growthTimer and use growthInitiated
  if (rxRear == GROW) {
    sendingGrowth = true;
    ackGrowth();
    if (isTrunkSplit) {
      sendSplitGrowth();
    } else {
      sendGrowth();
    }
  }

  if (rxHead == GROW_ACK) {
    sendingGrowth = false;
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
      } else {
        if (activeBudSeekingLeafTimer.isExpired()) {
          activeBudFace = -1;
          branchState = TOO_LATE;
          tooLateCoolDownTimer.set(TOO_LATE_COOL_DOWN_MS);
        }
      }
      break;
    case TOO_LATE:
      if (tooLateCoolDownTimer.isExpired()) {
        branchState = RANDOMIZING;
        becomeBudCoinFlipTimer.set(BECOME_BUD_COIN_FLIP_COOLDOWN_MS);
      }
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
void ackGrowth() {
  setValueSentOnFace(GROW_ACK, rearFace);
}

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

bool flipCoin() {
  return random(1);  // random(1000) % 2 better? need to test...
}

void updatePulseDimness() {
  // get progress from 0 - MAX
  int pulseProgress = millis() % PULSE_LENGTH_MS;

  // transform that progress to a byte (0-255)
  byte pulseMapped = map(pulseProgress, 0, PULSE_LENGTH_MS, 0, 255);

  // transform that byte with sin
  pulseDimness = sin8_C(pulseMapped);
}

void pulseColor(Color color) {
  setColor(dim(color, pulseDimness));
}

void pulseColorOnFace(Color color, byte face) {
  setColorOnFace(dim(color, pulseDimness), face);
}

void sparkle() {
  FOREACH_FACE(f) {
    byte randomH = random(millis()) % 255;
    byte randomS = random(millis()) % 255;
    byte randomB = random(40);
    Color randomColor = makeColorHSB(randomH, randomS, randomB);
    setColorOnFace(randomColor, f);
  }
}