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
  branchAlive = true;

  setColor(COLOR_NONE);
  setValueSentOnAllFaces(Message::SETUP_GAME);
}

// --- game loop ---

void loop() {
  setColor(OFF);

  if (hasWoken()) {
    setup();
  }

  updateSharedPulseDimness();

  // game instructions will state to the player to start with all blinks together
  // then the player will double click a blink to start the game
  // the player will need to separate all blinks for the game to begin

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
      break;
  }

  updateColors();
}
