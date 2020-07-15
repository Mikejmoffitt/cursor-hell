/*

Display Creation and Event Handling

Gimmick! exAct * Mix
(c) Michael Moffitt 2016 - 2020
------------------------

*/

#ifndef DISP_H
#define DISP_H

#include <SDL.h>
#include "common.h"

// Enum for the desired display mode.
typedef enum DispMode
{
	MODE_WINDOW,
	MODE_FULLSCREEN,
	MODE_FULLSCREEN_WINDOW
} DispMode;

typedef enum DispScale
{
	SCALE_FIT_HEIGHT,
	SCALE_FIT_WIDTH,
} DispScale;

// Enum for common aspect ratios. Sorted from "wideness" low to high.
typedef enum DispRatio
{
	// Tate
	RATIO_9_32,
	RATIO_9_21,
	RATIO_9_16,
	RATIO_10_16,
	RATIO_2_3,
	RATIO_3_4,
	RATIO_4_5,
	RATIO_7_8,
	RATIO_15_16,

	RATIO_1_1,

	// Yoko
	RATIO_16_15,
	RATIO_8_7,
	RATIO_5_4,
	RATIO_4_3,
	RATIO_3_2,
	RATIO_16_10,
	RATIO_16_9,
	RATIO_21_9,
	RATIO_32_9,

	RATIO_INVALID,
} DispRatio;

// A description of the internal rendering buffer.
typedef struct DispBufferDescriptor
{
	int valid;
	int w, h;  // Internal rendering dimensions.
	int prescale;  // == 1 to disable prescaling.
	int filter;  // Enables linear filtering when scaling.
	float par;  // Coefficient applied to width when projecting.
	DispScale scale;

} DispBufferDescriptor;

// A configuration struct that is passed during init.
typedef struct DispConfig
{
	// Dimensions of monitor / window. Will be overwritten if in fullscreen.
	int w, h;
	DispMode mode;

	// Buffer descriptors are created for each aspect ratio.
	DispBufferDescriptor descriptors[RATIO_INVALID];

} DispConfig;

#define DISP_REFRESH 60.0f

// -----------------------------------------------------
int disp_init(const DispConfig *config);
void disp_shutdown(void);

void disp_flip(void);
int disp_get_w(void);
int disp_get_h(void);

int disp_get_center_x(void);
int disp_get_center_y(void);

SDL_Texture *disp_get_buffer(void);
SDL_Renderer *disp_get_renderer(void);
SDL_Window *disp_get_window(void);
SDL_Rect *disp_get_main_rect(void);

DispRatio disp_get_ratio(void);
float disp_get_ratio_raw(void);

#endif
