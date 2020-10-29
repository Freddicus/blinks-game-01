/*
 * Make Like A Tree and Leaf
 * by Freddicus Game Studios, LLC
 * 
 * Author(s): Alfred Sterphone
 * 
 * Repo: https://github.com/Freddicus/blinks-game-01
 * Discussion: https://forum.move38.com/t/new-game-wip-make-like-a-tree-and-leaf/549
 */
#define SERIAL_LOGGING 1

#ifdef SERIAL_LOGGING
#include <Serial.h>
#endif

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

#ifdef SERIAL_LOGGING
ServicePortSerial sp;
#endif

void setup() {
#ifdef SERIAL_LOGGING
  sp.begin();
#endif
  randomize();
  initPlayVariables();
}

// --- game loop ---

void loop() {
#ifdef SERIAL_LOGGING
  if (sp.available()) {
    sp.print("Blink State: ");
    sp.println(blinkState);

    sp.print("Rear Face: ");
    sp.println(rearFace);

    // if (!isSplit) {
    sp.print("Head Face: ");
    sp.println(headFace);
    // } else {
    // sp.print("Head Face Left: ");
    // sp.println(headFaceLeft);

    // sp.print("Head Face Right: ");
    // sp.println(headFaceRight);
    // }

    // sp.println("--------------------------");

    // debugPrintTimer.set(1500);
  }
#endif

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
