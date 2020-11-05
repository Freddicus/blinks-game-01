#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <blinklib.h>

// -------- global constants --------

const static byte faceOffsetArray[] = {0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5};
#define CW_FROM_FACE(f, amt) faceOffsetArray[(f) + (amt)]
#define CCW_FROM_FACE(f, amt) faceOffsetArray[6 + (f) - (amt)]
#define OPPOSITE_FACE(f) CW_FROM_FACE((f), 3)
#define NOT_SET 255

// --- game values ---

#define NUM_RESET_GAME_CLICKS 5

#define FACE_SPROUT 0

#define PULSE_LENGTH_MS 2000
#define GROWTH_DELAY_MS 1250
#define GROWTH_COOLDOWN_MS 1250
#define BECOME_BUD_COIN_FLIP_COOLDOWN_MS 7500
#define ASK_FOR_LEAF_MAX_TIME_MS 10000
#define ASK_FOR_LEAF_MIN_TIME_MS 5000
#define TOO_LATE_COOL_DOWN_MS 4000
#define GAME_TIMER_MS 45000
#define SPIN_SPEED_FAST_MS 250
#define SPIN_SPEED_MEDIUM_MS 500  // tick-tock
#define SPIN_SPEED_SLOW_MS 1000

#define SOIL_PLAY_TIME_MS 2000
#define LEAF_PLAY_TIME_MIN_MS 500
#define LEAF_PLAY_TIME_MAX_MS 2000

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

extern bool isGameStarted;
extern bool isGameTimerStarted;
extern Timer gameTimer;

extern bool gotSetupMsg;

#endif