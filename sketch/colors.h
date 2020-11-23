#ifndef COLORS_H_
#define COLORS_H_

#include <blinklib.h>

#include "globals.h"
#include "playing.h"
#include "states.h"
#include "util.h"

#define COLOR_NONE MAKECOLOR_5BIT_RGB(1, 1, 1)         // almost off
#define COLOR_WHITE_25 MAKECOLOR_5BIT_RGB(8, 8, 8)     // 25% white
#define COLOR_WHITE_50 MAKECOLOR_5BIT_RGB(16, 16, 16)  // 50% white
#define COLOR_WHITE_75 MAKECOLOR_5BIT_RGB(24, 24, 24)  // 75% white
#define COLOR_SOIL makeColorRGB(117, 94, 84)           // i know there is no brown, but...
#define COLOR_SPROUT makeColorRGB(117, 199, 10)        // lime greenish
#define COLOR_TRUNK makeColorRGB(166, 114, 17)         // basically orange
#define COLOR_BRANCH makeColorRGB(143, 90, 47)         // basically orange
#define EMPTY_DIMNESS 100

void updateColors();

void handlePlayingColors();
void handleGameOverColors();
void handleResetColors();
void handleSoilColor();
void handleTrunkColor();
void handleGameTimerColor();
void handleBranchColor();
void handleCollectorColor();

void pulseColor(Color color, byte pulseDimness);
void pulseColorOnFace(Color color, byte face, byte pulseDimness);
void sparkle();
void spinColor(Color color, long revolutionMs);

#endif