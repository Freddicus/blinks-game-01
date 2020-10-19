#ifndef PLAYING_H_
#define PLAYING_H_

#include "colors.h"
#include "game_over.h"
#include "globals.h"
#include "setup.h"
#include "states.h"
#include "util.h"

// ---- trunk / branch ----

extern bool isSplit;

// --- growth ----

extern bool receivingGrowth;

extern Timer soilTimer;

// ---- branch / bud play ----

extern byte activeBudFace;
extern byte branchState;

// ---- leaf play ----

extern byte leafState;
extern bool hasLeafFlashedGreeting;

// ---- setup methods ----

void initPlayVariables();
void playGame();
void gameStatePlaying();

// -------- Playing methods -------

void playingNone();
void playingSoil();
void playingSprout();
void playingTrunk();
void playingBranch();
void playingBud();
void playingBudWithLeaf();
void playingLeaf();

// ----- Game Helpers ------

void updateBudFaces();
void randomizeBudAffinity();

#endif