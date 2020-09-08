#ifndef GAME_STATES_H_
#define GAME_STATES_H_

#include <blinklib.h>

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
  LOOKING_FOR_LEAF,
  LOOKING_FOR_LEAF_ACK,
  START_THE_CLOCK,
  START_THE_CLOCK_NOW,
  BRANCH_GREET_LEAF,
  BRANCH_MATURE_LEAF,
  BRANCH_MATURE_LEAF_ACK,
  PLEASE_DETACH
};

#endif