#include "disp.h"

#define DISP_RATIO_TOLERANCE 0.025f

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *main_buffer;
static SDL_Texture *interstitial_buffer;
static SDL_Surface *test_surface;

static SDL_Rect window_rect;
static SDL_Rect interstitial_rect;
static SDL_Rect main_rect;

static float window_ratio_raw;
static DispRatio window_ratio;

static int prescale_factor;
static int perfect_line_count;

static DispConfig disp_config;

#define WANT_CURSOR

static const float ratio_lut[] =
{
	[RATIO_16_15] = 16.0 / 15.0,
	[RATIO_8_7] = 8.0 / 7.0,
	[RATIO_5_4] = 5.0 / 4.0,
	[RATIO_3_2] = 3.0 / 2.0,
	[RATIO_4_3] = 4.0 / 3.0,
	[RATIO_16_10] = 16.0 / 10.0,
	[RATIO_16_9] = 16.0 / 9.0,
	[RATIO_21_9] = 21.0 / 9.0,
	[RATIO_32_9] = 32.0 / 9.0,

	[RATIO_15_16] = 15.0 / 16.0,
	[RATIO_7_8] = 7.0 / 8.0,
	[RATIO_4_5] = 4.0 / 5.0,
	[RATIO_2_3] = 2.0 / 3.0,
	[RATIO_3_4] = 3.0 / 4.0,
	[RATIO_10_16] = 10.0 / 16.0,
	[RATIO_9_16] = 9.0 / 16.0,
	[RATIO_9_21] = 9.0 / 21.0,
	[RATIO_9_32] = 9.0 / 32.0,

	[RATIO_1_1] = 1.0f,
};

// Flip display and handle display events.
void disp_flip(void)
{
	// 1) Draw integer scaled main_buffer contents onto interstitial_buffer.
	SDL_SetRenderTarget(renderer, interstitial_buffer);

	SDL_Rect dest_rect;
	dest_rect.x = 0;
	dest_rect.y = (window_ratio <= RATIO_4_3) ? (8 * prescale_factor) : 0;
	dest_rect.w = interstitial_rect.w;
	dest_rect.h = interstitial_rect.h;

	SDL_RenderCopy(renderer, main_buffer, &main_rect, &dest_rect);

	// 2) Draw interstitial buffer, with linear filtering, to window.
	const DispBufferDescriptor *descriptor = &disp_config.descriptors[window_ratio];
	const float adjusted_w = interstitial_rect.w * descriptor->par;
	float base_scale;

	// Choose which dimension should fill the screen w/h.
	switch (descriptor->scale)
	{
		default:
		case SCALE_FIT_WIDTH:
			base_scale = (float)window_rect.w / adjusted_w;
			break;
		case SCALE_FIT_HEIGHT:
			base_scale = (float)window_rect.h / interstitial_rect.h;
			break;
	}

	SDL_SetRenderTarget(renderer, NULL);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	dest_rect.w = adjusted_w * base_scale;
	dest_rect.h = interstitial_rect.h * base_scale;
	dest_rect.x = (window_rect.w - dest_rect.w) / 2;
	dest_rect.y = (window_rect.h - dest_rect.h) / 2;

	SDL_RenderFillRect(renderer, &window_rect);
	SDL_RenderCopy(renderer, interstitial_buffer,
	               &interstitial_rect, &dest_rect);

#ifdef WANT_CURSOR
	// Dump the FB into a software surface.
	SDL_RenderReadPixels(renderer, &window_rect, SDL_PIXELFORMAT_ABGR8888, test_surface->pixels, test_surface->pitch);
	SDL_Cursor *cursor_new = SDL_CreateColorCursor(test_surface, 0, 0);
	SDL_SetCursor(cursor_new);
#endif

	// 3) Delay for strict frame limiting.
	SDL_RenderPresent(renderer);

#ifdef WANT_CURSOR
	static SDL_Cursor *cursor_prev;
	if (cursor_prev) SDL_FreeCursor(cursor_prev);
	cursor_prev = cursor_new;
#endif

	// 4) Swap buffers.

	// 5) Restore main_buffer as the target surface.
	SDL_SetRenderTarget(renderer, main_buffer);

	// 6) Wipe main buffer.
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &main_rect);
}

int disp_init(const DispConfig *new_config)
{
	memcpy(&disp_config, new_config, sizeof(disp_config));

	// 1) Set up window
	SDL_WindowFlags win_flags = SDL_WINDOW_ALLOW_HIGHDPI |
	                            SDL_WINDOW_ALWAYS_ON_TOP |
	                            SDL_WINDOW_INPUT_FOCUS |
	                            SDL_WINDOW_OPENGL;
	window_rect.x = 0;
	window_rect.y = 0;
	window_rect.w = disp_config.w;
	window_rect.h = disp_config.h;
	switch (disp_config.mode)
	{
		default:
			fprintf(stderr, "[disp] Invalid mode %d\n", disp_config.mode);
			break;
		case MODE_WINDOW:
			break;
		case MODE_FULLSCREEN_WINDOW:
			win_flags |= SDL_WINDOW_FULLSCREEN;
			win_flags |= SDL_WINDOW_BORDERLESS;
			{
				SDL_DisplayMode current_mode;
				SDL_GetCurrentDisplayMode(0, &current_mode);
				window_rect.w = current_mode.w;
				window_rect.h = current_mode.h;
			}
			break;
		case MODE_FULLSCREEN:
			win_flags |= SDL_WINDOW_FULLSCREEN;
			win_flags |= SDL_WINDOW_BORDERLESS;
			break;
	}

	window = SDL_CreateWindow("RESPONSIVE STG TEST 2020 MIKE MOFFITT | Z, X = SHOOT, ARROWS = MOVE",
	                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                          window_rect.w, window_rect.h, win_flags);
	if (!window)
	{
		fprintf(stderr, "[disp] Couldn't create window.\n");
		return 0;
	}

	SDL_ShowCursor(SDL_ENABLE);

	SDL_GetWindowSize(window, &window_rect.w, &window_rect.h);
	window_ratio_raw = (window_rect.w / (float)window_rect.h);
	perfect_line_count = (disp_get_ratio() <= RATIO_4_3);
	
	// 2) Set up renderer.
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED |
	                                          SDL_RENDERER_PRESENTVSYNC |
	                                          SDL_RENDERER_TARGETTEXTURE);
	if (!renderer)
	{
		fprintf(stderr, "[disp] Couldn't create rendering context.\n");
		SDL_DestroyWindow(window);
		return 0;
	}

	// 3) Main buffer, chosen by aspect ratio.
	window_ratio = RATIO_INVALID;
	for (DispRatio r = 0; r < RATIO_INVALID; r++)
	{
		const float mode_ratio = ratio_lut[r];
		if (window_ratio_raw > mode_ratio - DISP_RATIO_TOLERANCE &&
		    window_ratio_raw < mode_ratio + DISP_RATIO_TOLERANCE)
		{
			window_ratio = r;
			break;
		}
	}

	if (window_ratio == RATIO_INVALID)
	{
		fprintf(stderr, "[disp] Unhandled aspect ratio %f\n", window_ratio_raw);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		return 0;
	}

	printf("[disp] Using ratio %d = %f\n", window_ratio, ratio_lut[window_ratio]);

	const DispBufferDescriptor *descriptor = &disp_config.descriptors[window_ratio];
	if (!descriptor->valid)
	{
		fprintf(stderr, "[disp] Invalid descriptor for ratio %f\n", window_ratio_raw);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		return 0;
	}

	SDL_ClearHints();
	if (descriptor->prescale > 1) SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);
	main_rect.x = 0;
	main_rect.y = 0;
	main_rect.w = descriptor->w;
	main_rect.h = descriptor->h;
	main_buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
	                                SDL_TEXTUREACCESS_TARGET,
	                                main_rect.w, main_rect.h);
	if (!main_buffer)
	{
		fprintf(stderr, "[disp] Couldn't create main buffer.\n");
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		return 0;
	}

	test_surface = SDL_CreateRGBSurface(0, 640, 480, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

	// 4) Interstitial buffer.
	SDL_ClearHints();
	if (descriptor->filter) SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	if (descriptor->prescale == 0) prescale_factor = window_rect.w / main_rect.w;
	else prescale_factor = descriptor->prescale;
	interstitial_rect.x = 0;
	interstitial_rect.y = 0;
	interstitial_rect.w = main_rect.w * prescale_factor;
	interstitial_rect.h = main_rect.h * prescale_factor;
	interstitial_buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
	                                        SDL_TEXTUREACCESS_TARGET,
	                                        interstitial_rect.w,
	                                        interstitial_rect.h);
	if (!interstitial_buffer)
	{
		fprintf(stderr, "[disp] Couldn't create interstitial buffer.\n");
		SDL_DestroyTexture(main_buffer);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		return 0;
	}

	// 5) Small configuration steps.
	printf("[disp] Init complete\n");
	return 1;
}

void disp_shutdown(void)
{
	SDL_DestroyTexture(interstitial_buffer);
	SDL_DestroyTexture(main_buffer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}

int disp_get_w(void)
{
	return main_rect.w;
}

int disp_get_h(void)
{
	return main_rect.h;
}

int disp_get_display_w(void)
{
	return window_rect.w;
}

int disp_get_display_h(void)
{
	return window_rect.h;
}

int disp_get_center_x(void)
{
	return main_rect.w / 2;
}

int disp_get_center_y(void)
{
	return main_rect.h / 2;
}

SDL_Texture *disp_get_buffer(void)
{
	return main_buffer;
}

SDL_Renderer *disp_get_renderer(void)
{
	return renderer;
}

SDL_Window *disp_get_window(void)
{
	return window;
}

SDL_Rect *disp_get_main_rect(void)
{
	return &main_rect;
}

DispRatio disp_get_ratio(void)
{
	return window_ratio;
}

float disp_get_ratio_raw(void)
{
	return window_ratio_raw;
}
