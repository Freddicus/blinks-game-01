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
  DEAD
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
  PLEASE_DETACH
};

// -------- global constants --------

const static byte oppositeFaces[] = {3, 4, 5, 0, 1, 2};

// -------- global variables --------

byte gameState;
byte blinkState;
byte leafState;

byte rearFace;
byte headFace;
byte headFaceLeft;
byte headFaceRight;

byte pulseDimness;

bool isTrunkSplit;
bool sendingGrowth;
bool receivingGrowth;

bool gotSetupMsg;

Timer soilTimer;

// --- game values ---

#define NUM_BLINKS 18
#define NUM_PANIC_CLICKS 6

#define COLOR_NONE makeColorRGB(1, 1, 1)         // almost off
#define COLOR_SOIL makeColorRGB(170, 145, 134)   // i know there is no brown, but...
#define COLOR_SPROUT makeColorRGB(191, 255, 0)   // lime greenish
#define COLOR_GROWTH makeColorRGB(84, 164, 222)  // water vibes
#define COLOR_TRUNK makeColorRGB(255, 192, 0)    // basically orange

#define FACE_SPROUT 0

#define PULSE_LENGTH 2000

// --- initialize ---

void setup() {
  gameState = SETUP;
  blinkState = NONE;
  leafState = NAL;

  rearFace = -1;
  headFace = -1;
  headFaceLeft = -1;
  headFaceRight = -1;

  isTrunkSplit = false;
  sendingGrowth = false;
  receivingGrowth = false;

  gotSetupMsg = false;

  setColor(dim(WHITE, 40));
}

// --- game loop ---

void loop() {
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
        // setColorOnFace(dim(YELLOW, 40), f);
        pulseColorOnFace(GREEN, f);
      }
    }

    // if i'm alone, then i'm ready to play
    if (gotSetupMsg && isAlone()) {
      gameState = PLAYING;
      // don't send anything
      setValueSentOnAllFaces(QUIET);
    }
  }

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
        break;
      case LEAF:
        break;
    }

    updateColor();
    detectPanic();
  }
}

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
      break;
    case BUD:
      break;
    case LEAF:
      break;
  }
}

void handleGrowthColor() {
  if (sendingGrowth) {
    pulseColorOnFace(COLOR_GROWTH, headFace);
  }

  if (receivingGrowth) {
    pulseColorOnFace(COLOR_GROWTH, rearFace);
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
    } else if (faceValue >= BRANCH_RIGHT_1 || faceValue < BRANCH_RIGHT_4) {
      blinkState = BRANCH;
      headFace = oppositeFaces[f];
      setValueSentOnFace(faceValue + 1, headFace);
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
  headFace = FACE_SPROUT;
  setValueSentOnFace(TRUNK_1, headFace);

  if (buttonSingleClicked()) {
    sendingGrowth = true;
  }

  // send up growth command / lights
  if (sendingGrowth) {
    sendGrowth();
  }

  if (getLastValueReceivedOnFace(headFace) == GROW_ACK) {
    sendingGrowth = false;
  }
}

void playingTrunk() {
  byte rxRear = getLastValueReceivedOnFace(rearFace);
  byte rxHead = getLastValueReceivedOnFace(headFace);

  if (rxRear == GROW) {
    sendingGrowth = true;
    setValueSentOnFace(GROW_ACK, rearFace);
    if (isTrunkSplit) {
      setValueSentOnFace(GROW, headFaceLeft);
      setValueSentOnFace(GROW, headFaceRight);
    } else {
      setValueSentOnFace(GROW, headFace);
    }
  }

  if (rxHead == GROW_ACK) {
    sendingGrowth = false;
  }
}

void playingBranch() {
}

// ----- Helpers ---------

void detectPanic() {
  if (isAlone() && buttonMultiClicked() && buttonClickCount() == NUM_PANIC_CLICKS) {
    setup();
    setValueSentOnAllFaces(QUIET);
  }
}

void sendGrowth() {
  setValueSentOnFace(GROW, headFace);
}

void updatePulseDimness() {
  // get progress from 0 - MAX
  int pulseProgress = millis() % PULSE_LENGTH;

  // transform that progress to a byte (0-255)
  byte pulseMapped = map(pulseProgress, 0, PULSE_LENGTH, 0, 255);

  // transform that byte with sin
  pulseDimness = sin8_C(pulseMapped);
}

void pulseColor(Color color) {
  setColor(dim(color, pulseDimness));
}

void pulseColorOnFace(Color color, byte face) {
  setColorOnFace(dim(color, pulseDimness), face);
}