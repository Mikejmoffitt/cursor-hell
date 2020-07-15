#ifndef PALETTE_H
#define PALETTE_H

#include "gfx.h"

// A few colors that keep being referenced all over.
#define COLOR_SKY gfx_map_rgba(0x4C, 0xAC, 0xFC, 0xFF)
#define COLOR_SKY_ALT gfx_map_rgba(0x2E, 0xC2, 0xBA, 0xFF)
#define COLOR_BLACK gfx_map_rgba(0x00, 0x00, 0x00, 0xFF)
#define COLOR_WHITE gfx_map_rgba(0xFF, 0xFF, 0xFF, 0xFF)
#define COLOR_YELLOW gfx_map_rgba(0xFF, 0xEE, 0x23, 0xFF)
#define COLOR_RED gfx_map_rgba(0xFF, 0x22, 0x22, 0xFF)

#define COLOR_FADE_BACKING gfx_map_rgba(0x00, 0x00, 0x00, 0x7F)

#define COLOR_UNDER_TREES gfx_map_rgba(0x07, 0x05, 0x16, 0xFF)

// Color for boss blinking at low HP.
#define COLOR_DAMAGE COLOR_RED

// Color for boss or enemy recoiling from damage.
#define COLOR_RECOIL gfx_map_rgba(0xFF, 0xFF, 0xFF, 0xFF)

#endif  // PALETTE_H
