#ifndef PLAYING_H_
#define PLAYING_H_

#include "colors.h"
#include "globals.h"
#include "states.h"
#include "util.h"

void gameStatePlaying();

// -------- Playing methods -------

void playingNone();
void playingSoil();
void playingSprout();
void playingTrunk();
void playingBranch();
void playingBud();
void playingLeaf();

// ----- Game Helpers ------

void ackGrowth();
void sendGrowth();
void sendSplitGrowth();
void updateBudFaces();
void randomizeBudAffinity();

#endif