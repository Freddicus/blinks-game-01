#ifndef PLAYING_H_
#define PLAYING_H_

#include "colors.h"
#include "game_over.h"
#include "globals.h"
#include "states.h"
#include "util.h"

// ---- trunk / branch ----

extern bool isSplit;

// --- soil ----

extern Timer soilTimer;

// ---- branch / bud play ----

extern byte activeLeafFace;
extern Color* activeLeafColor;
extern byte branchState;

// ---- leaf play ----

// ---- setup methods ----

void initPlayVariables();
void gameStatePlaying();

// -------- Playing methods -------

void playingNone();
void playingSoil();
void playingSprout();
void playingTrunk();
void playingBranch();
void playingBranchWithLeaf();
void playingCollector();

// ----- Game Helpers ------

void updateLeafFaces();
void randomizeLeafGrowing();
Message getLeafMessageFromLeafColorIndex(byte leafColorIndex);

#endif