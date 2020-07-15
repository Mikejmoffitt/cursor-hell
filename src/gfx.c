#include "gfx.h"
#include <SDL_image.h>
#include "common.h"
#include <stdio.h>
#include "disp.h"

// Static HD Object definitions.
#define GFX_DECL(w, h, x_o, y_o, x_f, y_f, prio, c_path, m_path) \
{ (w), (h), (x_o), (y_o), (x_f), (y_f), prio, c_path, m_path, NULL, NULL, 0, 0 }

#define GFX(name, prefix, w, h, x_o, y_o, x_f, y_f, prio) \
static Gfx o_##name = GFX_DECL(w, h, x_o, y_o, x_f, y_f, \
                                  prio, "res/gfx/"#prefix"/"#name".png", \
                                  "res/gfx/"#prefix"/mask/"#name".png" )

// This is a magic number to indicate top-left zero origin positioning.
#define ZO -68000

// ============ These objects need vertical offsets defined.

// Name                     Prefix   W  H  Xo  Yo  Xf Yf P

// Special sprites.
GFX(opaopa,                   obj, 32, 32, 0, 11, 0, 0, 0);
GFX(bullet,                   obj, 16, 16, 0, 8, 0, 0, 0);
GFX(fish_x,                   obj, 160, 160, 0, 64, 0, 0, 0);

// Special objects that don't show up directly in the object list.
static Gfx *objlist_ex[] =
{
	[GFX_NULL] = NULL,
	[GFX_OPAOPA] = &o_opaopa,
	[GFX_BULLET] = &o_bullet,
	[GFX_FISH_X] = &o_fish_x,
};

#undef GFX_DECL
#undef GFX

// ============================================================================

static int shutdown = 0;

static inline void load_sprite(Gfx *obj)
{
	if (!obj || obj->loaded) return;

	SAFELOAD_BMP(obj->chr, obj->chr_path);
	SAFELOAD_BMP(obj->mask, obj->mask_path);

	// Zero-origin sprites adjust offset so that the top-left is 0, 0.
	if (obj->x_origin == ZO) obj->x_origin = obj->frame_w / 2;
	if (obj->y_origin == ZO) obj->y_origin = obj->frame_h;

	uint32_t format;
	int access;
	int tex_w, tex_h;
	SDL_QueryTexture(obj->chr, &format, &access, &tex_w, &tex_h);

	obj->row_count = tex_w / obj->frame_w;
	obj->loaded = 1;
}

int gfx_init(void)
{
	// objlist_ex covers sprites that are called upon outside of the normal list.
	for (int i = 0; i < ARRAYSIZE(objlist_ex); i++)
	{
		load_sprite(objlist_ex[i]);
	}

	shutdown = 0;

	printf("[gfx] Initialized.\n");
	return 1;
}

void gfx_shutdown(void)
{
	shutdown = 1;
	int i = 0;
	Gfx *obj;
	do
	{
		obj = objlist_ex[i];
		if (!obj) break;

		if (obj->loaded)
		{
			if (obj->chr) SDL_DestroyTexture(obj->chr);
			if (obj->mask) SDL_DestroyTexture(obj->mask);
			obj->chr = NULL;
			obj->mask = NULL;
			obj->loaded = 0;
		}
		i++;
	}
	while (obj);
}

Gfx *gfx_get(ObjExId obj_id)
{
	if (obj_id >= ARRAYSIZE(objlist_ex))
	{
		printf("[gfx] obj_ex_id $%02X out of range\n", obj_id);
		return NULL;
	};
	return objlist_ex[obj_id];
}


// Read data from sprite, and configure atlas source.
static inline void set_up_draw_geometry(const Gfx *sprite, int i, int *src_x, int *src_y,
                                        int *x, int *y, int flags)
{
	if (shutdown) return;
	if (sprite->row_count <= 0)
	{
		printf("[gfx] Bad geometry for %d: Gfx %p\n", i, sprite);
		*x = -512;
		*y = -512;
		*src_x = 0;
		*src_y = 0;
		return;
	}
	const int idx_x = i % sprite->row_count;
	const int idx_y = i / sprite->row_count;

	*src_x = idx_x * sprite->frame_w;
	*src_y = idx_y * sprite->frame_h;

	*x -= sprite->frame_w / 2;
	*y -= sprite->frame_h;
	if (flags & GFX_FLAG_H_FLIP)
	{
		*x -= sprite->x_origin;
	}
	else
	{
		*x += sprite->x_origin;
	}
	*y += sprite->y_origin;
}

void gfx_draw_internal(const Gfx *sprite, int i, int x, int y, int use_mask,
                       uint8_t r, uint8_t g, uint8_t b, uint8_t a, GfxFlag flags)
{
	if (shutdown) return;
	SDL_Rect src_rect;
	set_up_draw_geometry(sprite, i, &src_rect.x, &src_rect.y, &x, &y, flags);
	src_rect.w = sprite->frame_w;
	src_rect.h = sprite->frame_h;

	SDL_Rect dest_rect;
	dest_rect.x = x;
	dest_rect.y = y;
	dest_rect.w = sprite->frame_w;
	dest_rect.h = sprite->frame_h;

	SDL_RendererFlip flip = flags & (SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
	SDL_Texture *tex = (use_mask ? sprite->mask : sprite->chr);
	SDL_SetTextureColorMod(tex, r, g, b);
	SDL_SetTextureAlphaMod(tex, a);

	if (flags & GFX_FLAG_BLEND_ADD)
	{
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_ADD);
	}
	else if (flags & GFX_FLAG_BLEND_MOD)
	{
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_MOD);
	}
	else if (flags & GFX_FLAG_BLEND_BLEND)
	{
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
	}
	else if (flags & GFX_FLAG_BLEND_NONE)
	{
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_NONE);
	}
	else
	{
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
	}

	SDL_RenderCopyEx(disp_get_renderer(), tex, &src_rect, &dest_rect,
	                 0.0f, NULL, flip);
}

// Draw a sprite's frame i at given coords.
void gfx_draw(const Gfx *sprite, int i, int x, int y, GfxFlag flags)
{
	gfx_draw_internal(sprite, i, x, y, 0, 0xFF, 0xFF, 0xFF, 0xFF, flags);
}

// Draw a sprite using its alpha mask only.
void gfx_draw_mask(const Gfx *sprite, GfxColor c, int i, int x,
                   int y, GfxFlag flags)
{
	gfx_draw_internal(sprite, i, x, y, 1, c.r, c.g, c.b, c.a, flags);
}

// Draw a sprite's frame i at given coords, with a tint.
void gfx_draw_tinted(const Gfx *sprite, GfxColor c, int i,
                     int x, int y, GfxFlag flags)
{
	gfx_draw_internal(sprite, i, x, y, 0, c.r, c.g, c.b, c.a, flags);
}

static void cutoff_internal(const Gfx *sprite, GfxColor c, int use_mask,
                            int cutoff, int i, int x, int y, GfxFlag flags)
{
	// TODO: This is like 90% the same as draw_internal, refactor please.
	SDL_Rect src_rect;
	set_up_draw_geometry(sprite, i, &src_rect.x, &src_rect.y, &x, &y, flags);
	src_rect.w = sprite->frame_w;
	src_rect.h = sprite->frame_h;

	SDL_Rect dest_rect;
	dest_rect.x = x;
	dest_rect.y = y;
	dest_rect.w = sprite->frame_w;
	dest_rect.h = sprite->frame_h;

	SDL_Rect clipping_rect = dest_rect;

	// if height dips below cutoff, reduce it, and draw what remains.
	if (y + clipping_rect.h > cutoff) clipping_rect.h = cutoff - y;
	if (clipping_rect.h <= 0) return;

	SDL_RendererFlip flip = flags & (SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
	SDL_Texture *tex = (use_mask ? sprite->mask : sprite->chr);
	SDL_SetTextureColorMod(tex, c.r, c.g, c.b);
	SDL_SetTextureAlphaMod(tex, c.a);

	SDL_RenderSetClipRect(disp_get_renderer(), &clipping_rect);
	SDL_RenderCopyEx(disp_get_renderer(), tex, &src_rect, &dest_rect,
	                 0.0f, NULL, flip);
	SDL_RenderSetClipRect(disp_get_renderer(), NULL);
}

// Draw a sprite's frame i at given coords excluding image data beneath cutoff.
void gfx_draw_cutoff(const Gfx *sprite, int cutoff, int i, int x,
                           int y, GfxFlag flags)
{
	cutoff_internal(sprite, gfx_map_rgba(255, 255, 255, 0xFF), 0, cutoff, i, x, y, flags);
}

// Draw a sprite's frame i at given coords excluding image data beneath cutoff.
void gfx_draw_mask_cutoff(const Gfx *sprite, GfxColor c, int cutoff,
                          int i, int x, int y, GfxFlag flags)
{
	cutoff_internal(sprite, c, 1, cutoff, i, x, y, flags);
}
