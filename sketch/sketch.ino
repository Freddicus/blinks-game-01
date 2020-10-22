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
#include "game_over.h"
#include "globals.h"
#include "playing.h"
#include "setup.h"
#include "states.h"

// -------- global variables --------

byte gameState;
byte blinkState;

byte rearFace;
byte headFace;
byte headFaceLeft;
byte headFaceRight;

byte sharedPulseDimness;

Timer gameTimer;
bool isGameTimerStarted;

// --- initialize ---

// initializing unsigned 8-bit ints to -1 is a little sketchy, but shouldn't be a problem
// it's effectively initializing them to 255 or similar, which doesn't have collision with
// the current code's use cases

void setup() {
  randomize();

  initPlayVariables();
  initSetupVariables();

  gameState = GameState::SETUP;
  blinkState = BlinkState::NONE;

  rearFace = -1;
  headFace = -1;
  headFaceLeft = -1;
  headFaceRight = -1;

  isGameTimerStarted = false;

  activeBudFace = -1;

  setColor(COLOR_NONE);
  setValueSentOnAllFaces(Message::SETUP_GAME);
}

// --- game loop ---

void loop() {
  setColor(OFF);

  if (hasWoken()) {
    // game can't continue from sleep
    setup();
  }

  updateSharedPulseDimness();

  // game instructions will state to the player to start with all blinks together
  // then the player will double click a blink to start the game
  // the player will need to separate all blinks for the game to begin

  playGame();
  updateColors();
}