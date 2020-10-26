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

// -------- global variables (used in colors and playing) --------

byte gameState;
byte blinkState;

byte rearFace;
byte headFace;
byte headFaceLeft;
byte headFaceRight;

byte sharedPulseDimness;

Timer gameTimer;
bool isGameTimerStarted;

void setup() {
  randomize();
  initPlayVariables();
}

// --- game loop ---

void loop() {
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
