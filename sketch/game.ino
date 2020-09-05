/*
 * Make Like A Tree and Leaf
 * by Freddicus Game Studios, LLC
 * 
 * Author(s): Alfred Sterphone
 * 
 * Repo: https://github.com/Freddicus/blinks-game-01
 * Discussion: https://forum.move38.com/t/new-game-wip-make-like-a-tree-and-leaf/549
 */

#include <blinklib.h>

#include "colors.h"
#include "globals.h"
#include "playing.h"
#include "setup.h"
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

Timer gameTimer;
bool isGameTimerStarted;

// --- initialize ---

void setup() {
  randomize();

  initPlayVariables();
  initSetupVariables();

  gameState = SETUP;
  blinkState = NONE;
  leafState = NAL;
  branchState = NAB;

  rearFace = -1;
  headFace = -1;
  headFaceLeft = -1;
  headFaceRight = -1;

  isGameTimerStarted = false;

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

  switch (gameState) {
    case SETUP:
      gameStateSetup();
      break;
    case PLAYING:
      gameStatePlaying();
      break;
  }
}
