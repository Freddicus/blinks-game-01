#ifndef GAME_STATES_H_
#define GAME_STATES_H_

#include <blinklib.h>

enum GameState {
  PLAYING,
  GAME_OVER
};

enum BlinkState {
  NONE,
  SOIL,
  SPROUT,
  TRUNK,
  BRANCH,
  COLLECTOR
};

enum BranchState {
  NAB,  // Not A Branch
  RANDOMIZING,
  GREW_A_LEAF
};

enum CollectorState {
  DETACHED,
  COLLECTING,
  COLLECTED
};

// --- simple messages ---

enum Message : byte {
  QUIET,
  SETUP_TRUNK,
  SETUP_BRANCH,
  START_THE_GAME,
  START_THE_CLOCK_NOW,
  LEAF_GREEN,
  LEAF_YELLOW,
  LEAF_ORANGE,
  LEAF_RED,
  END_GAME,
  RESET_GAME
};

#endif