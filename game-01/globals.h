#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <blinklib.h>

// -------- global constants --------

const static byte oppositeFaces[] = {3, 4, 5, 0, 1, 2};

// --- game values ---

#define NUM_BLINKS 18
#define NUM_PANIC_CLICKS 6

#define FACE_SPROUT 0

#define PULSE_LENGTH_MS 2000
#define GROWTH_DELAY_MS 1250
#define BECOME_BUD_COIN_FLIP_COOLDOWN_MS 7500
#define ASK_FOR_LEAF_MAX_TIME_MS 5000
#define ASK_FOR_LEAF_MIN_TIME_MS 1250
#define TOO_LATE_COOL_DOWN_MS 4000
#define GAME_TIMER_MS 45000
#define SPIN_SPEED_FAST_MS 250
#define SPIN_SPEED_MEDIUM_MS 500  // tick-tock
#define SPIN_SPEED_SLOW_MS 1000

#define INITIAL_BRANCH_HIT_POINTS 4

// -------- global variables --------

extern byte gameState;
extern byte blinkState;
extern byte leafState;
extern byte branchState;

extern byte rearFace;
extern byte headFace;
extern byte headFaceLeft;
extern byte headFaceRight;

extern byte sharedPulseDimness;

// ---- trunk / branch ----

extern bool isTrunkSplit;
extern Timer gameTimer;
extern bool isGameTimerStarted;

extern bool isFinalBranch;

// --- growth ----

extern bool growthInitiated;
extern bool sendingGrowth;
extern bool receivingGrowth;

extern bool gotSetupMsg;

extern Timer soilTimer;

extern Timer txGrowthTimer;

// ---- branch / bud play ----

extern byte budFaces[4];
extern byte activeBudFace;
extern byte branchHitPoints;

extern Timer becomeBudCoinFlipTimer;
extern Timer activeBudSeekingLeafTimer;
extern Timer tooLateCoolDownTimer;

// leaf play

extern Timer leafLifeTimer;

#endif