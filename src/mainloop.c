#include "mainloop.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "palette.h"
#include <SDL.h>
#include <math.h>

#include "common.h"
#include "gfx.h"
#include "input.h"
#include "disp.h"
#include "sound.h"
#include "bullet.h"
#include "player.h"
#include "obj.h"

#define FIRST_EXEC SE_MAIN

#ifndef M_PI
#define M_PI 3.14159
#endif
typedef enum ScreenExec
{
	// Any can transition to SE_CONFIG.
	SE_INIT = 0, // --> FIRST_EXEC
	SE_SHUTDOWN,
	SE_MAIN,

	SE_LIST_INVALID,
} ScreenExec;

// Chickenshit variables

static ScreenExec current_exec;
static int app_alive = 1;
static int should_exit = 0;

// Counts upwards, indicating elapsed frames. When the state changes, this is
// reset to zero.
static unsigned int elapsed = 0;

static void exec_change(ScreenExec new_state)
{
	current_exec = new_state;
}

static void exec_next()
{
	current_exec++;
}

// Each component is initialized one at a time, in this order. During shutdown,
// the shutdown functions are run in the reverse order.
typedef struct InitFunc
{
	char name[32];
	int (*init_func)(void);
	void (*shutdown_func)(void);
	int status;
} InitFunc;

static InitFunc init_funcs[] =
{
	{"sound", sound_init, sound_shutdown},
	{"gfx", gfx_init, gfx_shutdown},
	{"input", input_init, NULL},
};

// States. ====================================================================

static void se_init(int first)
{
	for (int i = 0; i < ARRAYSIZE(init_funcs); i++)
	{
		InitFunc *f = &init_funcs[i];
		if (!f->init_func) continue;
		const int status = f->init_func();
		f->status = status;
		if (status != 1)
		{
			printf("[mainloop] Error init \"%s\"\n",
			        f->name);
			exec_change(SE_SHUTDOWN);
			return;
		}
	}

	exec_change(FIRST_EXEC);
}

static void se_shutdown(int first)
{
	for (int i = ARRAYSIZE(init_funcs) - 1; i >= 0; i--)
	{
		InitFunc *f = &init_funcs[i];
		if (!f->shutdown_func) continue;
		printf("[mainloop] Shutting down \"%s\"\n", f->name);
		if (f->status == 1) f->shutdown_func();
	}

	app_alive = 0;
}

static void se_main(int first)
{
	if (first)
	{
		sound_play_music(SOUND_MUS_BGM);
		bullets_clear();
		player_clear(player_get());
		obj_clear();

		obj_spawn(OBJ_TYPE_FISH_X, 200, 120);
	}

	player_run(player_get());
	obj_run();
	bullets_run();
}

// ----------------------------------------------------------------------------

static void (*dispatch_funcs[])(int first) =
{
	[SE_INIT] = se_init,
	[SE_SHUTDOWN] = se_shutdown,
	[SE_MAIN] = se_main,
	[SE_LIST_INVALID] = NULL
};

static void state_dispatch(ScreenExec state)
{
	static ScreenExec prev_exec = SE_LIST_INVALID;
	int first = (state != prev_exec);
	if (first) elapsed = 0;
	prev_exec = state;
	if (!dispatch_funcs[state])
	{
		printf("[mainloop] State undefined for %d!!\n", state);
		exec_change(SE_SHUTDOWN);
	}
	dispatch_funcs[state](first);
	elapsed++;
}

static void maybe_exit_application(void)
{
	if (should_exit) exec_change(SE_SHUTDOWN);
}

// ============================================================================
static void handle_sdl_events(void)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			default:
				break;
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_CLOSE)
				{
					should_exit = 1;
				}
				break;
		}
	}
}

void mainloop_run(void)
{
	app_alive = 1;
	current_exec = SE_INIT;

	SDL_WarpMouseInWindow(disp_get_window(), 0, 0);
	while (app_alive)
	{
		handle_sdl_events();
		input_poll();
		state_dispatch(current_exec);
		maybe_exit_application();
		disp_flip();
	}
}
