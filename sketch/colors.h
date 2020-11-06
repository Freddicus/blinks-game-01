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
#define COLOR_BUD COLOR_SPROUT                         // sprout color
#define COLOR_BRANCH makeColorRGB(255, 192, 0)         // basically orange
#define COLOR_NEW_LEAF makeColorRGB(119, 179, 89)      // another light green
#define COLOR_YOUNG_LEAF makeColorRGB(71, 179, 73)     // light green
#define COLOR_MATURE_LEAF makeColorRGB(51, 128, 52)    // deep green
#define COLOR_DYING_LEAF makeColorRGB(230, 230, 92)    // pale yellow
#define COLOR_DEAD_LEAF makeColorRGB(153, 107, 61)     // "brown"

void updateColors();

void handlePlayingColors();
void handleGameOverColors();
void handleSoilColor();
void handleGameTimerColor();
void handleBranchColor();
void handleCollectorColor();

void pulseColor(Color color, byte pulseDimness);
void pulseColorOnFace(Color color, byte face, byte pulseDimness);
void sparkle();
void spinColor(Color color, long revolutionMs);

#endif