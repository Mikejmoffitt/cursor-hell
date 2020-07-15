#ifndef GFX_H
#define GFX_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <SDL.h>
typedef struct GfxColor
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} GfxColor;

#define GFX_MAP_RGBA_INLINE(r, g, b, a) {r, g, b, a}

static inline GfxColor gfx_map_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	GfxColor ret;
	ret.r = r;
	ret.g = g;
	ret.b = b;
	ret.a = a;
	return ret;
}

typedef enum GfxFlag
{
	GFX_FLAG_H_FLIP = SDL_FLIP_HORIZONTAL,
	GFX_FLAG_V_FLIP = SDL_FLIP_VERTICAL,
	GFX_FLAG_BLEND_NONE = 0x0800,
	GFX_FLAG_BLEND_BLEND = 0x1000,
	GFX_FLAG_BLEND_ADD = 0x2000,
	GFX_FLAG_BLEND_MOD = 0x4000,
	GFX_FLAG_NO_DEBUG_ORIGIN = 0x8000
} GfxFlag;

typedef enum ObjExId
{
	GFX_NULL = 0,
	GFX_OPAOPA,
	GFX_BULLET,
	GFX_FISH_X,
} ObjExId;

typedef struct Gfx
{
	// Programmer-provided
	int frame_w;
	int frame_h;
	int x_origin;
	int y_origin;
	int x_flip;
	int y_flip;
	int prio;
	char chr_path[256];
	char mask_path[256];

	// Determined during init
	SDL_Texture *chr;
	SDL_Texture *mask;
	int row_count;
	int loaded;
} Gfx;

int gfx_init(void);
void gfx_shutdown(void);

Gfx *gfx_get(ObjExId obj_id);

void gfx_draw(const Gfx *sprite, int i, int x, int y, GfxFlag flags);
void gfx_draw_mask(const Gfx *sprite, GfxColor c, int i, int x,
                   int y, GfxFlag flags);
void gfx_draw_mask_cutoff(const Gfx *sprite, GfxColor c, int cutoff,
                          int i, int x, int y, GfxFlag flags);
void gfx_draw_tinted(const Gfx *sprite, GfxColor c,
                           int i, int x, int y, GfxFlag flags);
void gfx_draw_cutoff(const Gfx *sprite, int cutoff,
                           int i, int x, int y, GfxFlag flags);

#endif  // GFX_H
