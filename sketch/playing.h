#ifndef PLAYING_H_
#define PLAYING_H_

#include "colors.h"
#include "game_over.h"
#include "globals.h"
#include "setup.h"
#include "states.h"
#include "util.h"

// --- growth ----

extern bool growthInitiated;
extern bool sendingGrowth;
extern bool receivingGrowth;

extern Timer soilTimer;
extern Timer txGrowthTimer;

// ---- branch / bud play ----

extern byte activeBudFace;
extern byte branchState;

// leaf play

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

void ackGrowth();
void sendGrowth();
void sendSplitGrowth();
void updateBudFaces();
void randomizeBudAffinity();

#endif