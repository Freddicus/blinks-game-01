#ifndef PLAYING_H_
#define PLAYING_H_

#include "colors.h"
#include "game_over.h"
#include "globals.h"
#include "setup.h"
#include "states.h"
#include "util.h"

// ---- trunk / branch ----

extern bool isTrunkSplit;
extern bool isFinalBranch;

// --- growth ----

extern bool growthInitiated;
extern bool sendingGrowth;
extern bool receivingGrowth;

extern Timer soilTimer;
extern Timer txGrowthTimer;

// ---- branch / bud play ----

extern byte budFaces[4];
extern byte activeBudFace;
extern bool branchAlive;
extern byte branchState;

extern Timer becomeBudCoinFlipTimer;
extern Timer activeBudSeekingLeafTimer;
extern Timer tooLateCoolDownTimer;

// leaf play

extern Timer leafLifeTimer;
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