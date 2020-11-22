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
#include "states.h"
#include "debug.h"

// -------- global variables (used in colors and playing) --------

byte gameState;
byte blinkState;

byte rearFace;
byte headFace;
byte headFaceLeft;
byte headFaceRight;

byte sharedPulseDimness;

bool isGameStarted;
bool isGameTimerStarted;
Timer gameTimer;

byte collectorColorIndex;
byte numLeavesCollected;

bool wasButtonLongPressed;
bool wasButtonSingleClicked;
bool wasButtonDoubledClicked;
bool wasButtonTripleClicked;

void setup() {
  randomize();
  initPlayVariables();
}

// --- game loop ---

void loop() {
  LOGLN(blinkState);

  wasButtonLongPressed = buttonLongPressed();
  wasButtonSingleClicked = buttonSingleClicked();
  wasButtonDoubledClicked = buttonDoubleClicked();
  wasButtonTripleClicked = buttonMultiClicked() && buttonClickCount() == 3;

  switch (gameState) {
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

  updateColors();
}
