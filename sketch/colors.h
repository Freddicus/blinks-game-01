#ifndef COLORS_H_
#define COLORS_H_

#include <blinklib.h>

#include "globals.h"
#include "playing.h"
#include "states.h"
#include "util.h"

#define COLOR_NONE MAKECOLOR_5BIT_RGB(1, 1, 1)       // almost off
#define COLOR_SOIL makeColorRGB(170, 145, 134)       // i know there is no brown, but...
#define COLOR_SPROUT makeColorRGB(191, 255, 0)       // lime greenish
#define COLOR_GROWTH makeColorRGB(84, 164, 222)      // water vibes
#define COLOR_TRUNK makeColorRGB(255, 192, 0)        // basically orange
#define COLOR_BUD COLOR_SPROUT                       // lime greenish
#define COLOR_BRANCH COLOR_TRUNK                     // basically orange
#define COLOR_NEW_LEAF makeColorRGB(119, 179, 89)    // another light green
#define COLOR_YOUNG_LEAF makeColorRGB(71, 179, 73)   // light green
#define COLOR_MATURE_LEAF makeColorRGB(51, 128, 52)  // deep green
#define COLOR_DYING_LEAF makeColorRGB(230, 230, 92)  // pale yellow
#define COLOR_DEAD_LEAF makeColorRGB(153, 107, 61)   // "brown"

void updateColors();

void handleSetupColors();
void handlePlayingColors();
void handleGameOverColors();

void pulseColor(Color color, byte pulseDimness);
void pulseColorOnFace(Color color, byte face, byte pulseDimness);
void sparkle();
void spinColor(Color color, long revolutionMs);

#endif