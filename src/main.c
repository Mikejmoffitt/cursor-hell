#include <stdio.h>
#include "common.h"
#include "disp.h"
#include "mainloop.h"
#include <SDL.h>

#define INT_W 320
#define INT_H 240

int configure_display(int w, int h, int full)
{
	DispConfig disp_config;
	memset(&disp_config, 0, sizeof(disp_config));

	disp_config.w = w;
	disp_config.h = h;
	switch (full)
	{
		case 0:
			disp_config.mode = MODE_WINDOW;
			break;
		default:
		case 1:
			disp_config.mode = MODE_FULLSCREEN_WINDOW;
			break;
		case 2:
			disp_config.mode = MODE_FULLSCREEN;
			break;
	}

	// 4:3 and taller displays.
	disp_config.descriptors[RATIO_4_3].valid = 1;
	disp_config.descriptors[RATIO_4_3].w = INT_W;
	disp_config.descriptors[RATIO_4_3].h = INT_H;
	disp_config.descriptors[RATIO_4_3].prescale = 0;
	disp_config.descriptors[RATIO_4_3].filter = 1;
	disp_config.descriptors[RATIO_4_3].par = 1.0f;
	disp_config.descriptors[RATIO_4_3].scale = SCALE_FIT_HEIGHT;

	disp_config.descriptors[RATIO_5_4] = disp_config.descriptors[RATIO_4_3];
	disp_config.descriptors[RATIO_5_4].scale = SCALE_FIT_WIDTH;
	disp_config.descriptors[RATIO_5_4].par = 1.0f;

	disp_config.descriptors[RATIO_8_7] = disp_config.descriptors[RATIO_5_4];
	disp_config.descriptors[RATIO_5_4] = disp_config.descriptors[RATIO_5_4];
	disp_config.descriptors[RATIO_15_16] = disp_config.descriptors[RATIO_5_4];
	disp_config.descriptors[RATIO_7_8] = disp_config.descriptors[RATIO_5_4];
	disp_config.descriptors[RATIO_4_5] = disp_config.descriptors[RATIO_5_4];
	disp_config.descriptors[RATIO_3_4] = disp_config.descriptors[RATIO_5_4];
	disp_config.descriptors[RATIO_2_3] = disp_config.descriptors[RATIO_5_4];
	disp_config.descriptors[RATIO_10_16] = disp_config.descriptors[RATIO_5_4];
	disp_config.descriptors[RATIO_9_16] = disp_config.descriptors[RATIO_5_4];
	disp_config.descriptors[RATIO_9_21] = disp_config.descriptors[RATIO_5_4];
	disp_config.descriptors[RATIO_9_32] = disp_config.descriptors[RATIO_5_4];
	disp_config.descriptors[RATIO_1_1] = disp_config.descriptors[RATIO_5_4];

	// Special case for famicom-perfect.
	disp_config.descriptors[RATIO_16_15].valid = 1;
	disp_config.descriptors[RATIO_16_15].w = INT_W;
	disp_config.descriptors[RATIO_16_15].h = INT_H;
	disp_config.descriptors[RATIO_16_15].prescale = 0;
	disp_config.descriptors[RATIO_16_15].filter = 0;
	disp_config.descriptors[RATIO_16_15].par = 1.0f;
	disp_config.descriptors[RATIO_16_15].scale = SCALE_FIT_HEIGHT;

	// Widescreen resolutions that have cropping between 4:3 and 16:9.
	disp_config.descriptors[RATIO_3_2].valid = 1;
	disp_config.descriptors[RATIO_3_2].w = INT_W;
	disp_config.descriptors[RATIO_3_2].h = 192;
	disp_config.descriptors[RATIO_3_2].prescale = 4;
	disp_config.descriptors[RATIO_3_2].filter = 1;
	disp_config.descriptors[RATIO_3_2].par = 1.0f;
	disp_config.descriptors[RATIO_3_2].scale = SCALE_FIT_HEIGHT;

	disp_config.descriptors[RATIO_16_10].valid = 1;
	disp_config.descriptors[RATIO_16_10].w = INT_W;
	disp_config.descriptors[RATIO_16_10].h = 178;
	disp_config.descriptors[RATIO_16_10].prescale = 4;
	disp_config.descriptors[RATIO_16_10].filter = 1;
	disp_config.descriptors[RATIO_16_10].par = 1.0f; 
	disp_config.descriptors[RATIO_16_10].scale = SCALE_FIT_HEIGHT;

	disp_config.descriptors[RATIO_16_9].valid = 1;
	disp_config.descriptors[RATIO_16_9].w = INT_W;
	disp_config.descriptors[RATIO_16_9].h = 160;
	disp_config.descriptors[RATIO_16_9].prescale = 4;
	disp_config.descriptors[RATIO_16_9].filter = 1;
	disp_config.descriptors[RATIO_16_9].par = 1.0f;
	disp_config.descriptors[RATIO_16_9].scale = SCALE_FIT_HEIGHT;

	// Hopeless widescreen resolutions shorter than 16:9.
	disp_config.descriptors[RATIO_21_9] = disp_config.descriptors[RATIO_16_9];
	disp_config.descriptors[RATIO_32_9] = disp_config.descriptors[RATIO_16_9];

	return disp_init(&disp_config);
}

int main(int argc, char **argv)
{
	printf("CURSOR ZONE III : THE TEARS OF WINDOWS 10\n");
	printf("Ver. "__DATE__" SDL-VER\n");
	printf("(c) 2020 Michael Moffitt\n");

	int display_w = 640;
	int display_h = 480;
	int full = 0;

	if (argc >= 4)
	{
		display_w = atoi(argv[1]);
		display_h = atoi(argv[2]);
		full = atoi(argv[3]);
	}
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0)
	{
		printf("Couldn't initialize SDL.\n");
		return -1;
	}

	if (!configure_display(display_w, display_h, full))
	{
		printf("Couldn't initialize display.\n");
		return 1;
	}

	mainloop_run();
	disp_shutdown();
	return 0;
}
