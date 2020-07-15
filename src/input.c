#include "input.h"
#include <stdio.h>
#include <SDL.h>

// TODO: Remove Gimmick input leftovers

static uint16_t input_comp[2];
static uint16_t input_comp_prev[2];

static const uint8_t *keys;

static SDL_Joystick *joy;

int input_init(void)
{
	joy = NULL;
	if (SDL_NumJoysticks() > 0)
	{
		joy = SDL_JoystickOpen(0);
		if (joy)
		{
			printf("[input] Using Joystick \"%s\"", SDL_JoystickNameForIndex(0));
		}
	}

	input_comp[0] = 0;
	input_comp[1] = 0;
	input_comp_prev[0] = 0;
	input_comp_prev[1] = 0;
	printf("[input] Initialized.\n");
	return 1;
}

void input_poll(void)
{
	keys = SDL_GetKeyboardState(NULL);


	// Compose them into a single input
	input_comp_prev[0] = input_comp[0];
	input_comp[0] = 0;
	input_comp_prev[1] = input_comp[1];
	input_comp[1] = 0;

	// Keyboard.
	// TODO: Disable without special debug enable
	if (keys[SDL_SCANCODE_RIGHT]) 
		input_comp[0] |= SW_RIGHT;
	if (keys[SDL_SCANCODE_LEFT])
		input_comp[0] |= SW_LEFT;
	if (keys[SDL_SCANCODE_UP])
		input_comp[0] |= SW_UP;
	if (keys[SDL_SCANCODE_DOWN])
		input_comp[0] |= SW_DOWN;
	if (keys[SDL_SCANCODE_Z])
		input_comp[0] |= SW_2;
	if (keys[SDL_SCANCODE_X])
		input_comp[0] |= SW_1;
	if (keys[SDL_SCANCODE_A])
		input_comp[0] |= SW_3;
	if (keys[SDL_SCANCODE_S])
		input_comp[0] |= SW_4;
	if (keys[SDL_SCANCODE_1])
		input_comp[0] += SW_START;

	// Joystick.
	if (joy)
	{
		SDL_JoystickUpdate();
		if (SDL_JoystickGetButton(joy, 0)) input_comp[0] |= SW_1;
		if (SDL_JoystickGetButton(joy, 1)) input_comp[0] |= SW_2;
		if (SDL_JoystickGetButton(joy, 2)) input_comp[0] |= SW_3;
		if (SDL_JoystickGetButton(joy, 3)) input_comp[0] |= SW_4;
		if (SDL_JoystickGetButton(joy, 4)) input_comp[0] |= SW_START;
		uint8_t hat_pos = SDL_JoystickGetHat(joy, 0);
		switch (hat_pos)
		{
			default:
				break;
			case SDL_HAT_UP:
				input_comp[0] |= SW_UP;
				break;
			case SDL_HAT_RIGHT:
				input_comp[0] |= SW_RIGHT;
				break;
			case SDL_HAT_DOWN:
				input_comp[0] |= SW_DOWN;
				break;
			case SDL_HAT_LEFT:
				input_comp[0] |= SW_LEFT;
				break;
			case SDL_HAT_RIGHTUP:
				input_comp[0] |= SW_RIGHT | SW_UP;
				break;
			case SDL_HAT_RIGHTDOWN:
				input_comp[0] |= SW_RIGHT | SW_DOWN;
				break;
			case SDL_HAT_LEFTUP:
				input_comp[0] |= SW_LEFT | SW_UP;
				break;
			case SDL_HAT_LEFTDOWN:
				input_comp[0] |= SW_LEFT | SW_DOWN;
				break;

		}
	}

}

void input_shutdown(void)
{
	if (joy && SDL_JoystickGetAttached(joy))
	{
		SDL_JoystickClose(joy);
		joy = NULL;
	}
}
	
#define _IMPULSE_MAIN(kb_code) \
if (!keys) return 0; \
int ret = 0; \
volatile static int prev_sw = 0; \
volatile int sw = keys[kb_code]; \
if (sw && !prev_sw) { ret = 1; } \
prev_sw = sw; \
return ret;

int input_test_button(void)
{
	if (keys[SDL_SCANCODE_F2]) return 1;
	return 0;
}

int input_1p_start_impulse(void)
{
	_IMPULSE_MAIN(SDL_SCANCODE_1);
}

int input_2p_start_impulse(void)
{
	_IMPULSE_MAIN(SDL_SCANCODE_2);
}

int input_1p_coin_impulse(void)
{
	_IMPULSE_MAIN(SDL_SCANCODE_5);
}

int input_2p_coin_impulse(void)
{
	_IMPULSE_MAIN(SDL_SCANCODE_6);
}

int input_service_coin_impulse(void)
{
	_IMPULSE_MAIN(SDL_SCANCODE_9);
}

const uint8_t *input_get_keys(void)
{
	return keys;
}

uint16_t input_get_impulse(int player)
{
	uint16_t ret = 0;
	if (player > 1 || player < 0)
	{
		return 0;
	}

	for (int i = 0; i < 16; i++)
	{
		int val = input_comp[player] & (1 << i);
		int valp = input_comp_prev[player] & (1 << i);
		if (val && !valp)
		{
			ret |= (1 << i);
		}
	}
	return ret;
}

uint16_t input_get_state(int player)
{
	if (player > 1 || player < 0)
	{
		return 0;
	}
	return input_comp[player];
}

uint16_t input_get_state_prev(int player)
{
	if (player > 1 || player < 0)
	{
		return 0;
	}
	return input_comp_prev[player];
}
