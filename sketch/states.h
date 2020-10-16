#ifndef GAME_STATES_H_
#define GAME_STATES_H_

#include <blinklib.h>

enum GameState {
  SETUP,
  PLAYING,
  GAME_OVER
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
  NEW,
  YOUNG,
  MATURE,
  DYING,
  DEAD_LEAF,
  DETACHED
};

enum BranchBudState {
  NAB,  // Not A Branch/Bud
  RANDOMIZING,
  BUDDING,
  GREW_A_LEAF,
  TOO_LATE,
  DEAD_BRANCH
};

// --- simple messages ---

enum Message : byte {
  QUIET,
  SETUP_GAME,
  SETUP_TRUNK,
  SETUP_BRANCH,
  GROW,
  LOOKING_FOR_LEAF,
  LOOKING_FOR_LEAF_ACK,
  START_THE_CLOCK_NOW,
  BRANCH_GREET_LEAF,
  BRANCH_MATURE_LEAF,
  BRANCH_MATURE_LEAF_ACK,
  SEND_POISON,
  END_GAME,
  PLEASE_DETACH
};

#endif