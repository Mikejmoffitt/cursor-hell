#include "bullet.h"
#include "gfx.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "common.h"
#include "player.h"
#include "obj.h"
static Bullet bullets[BULLET_COUNT];

void bullets_clear(void)
{
	memset(bullets, 0, sizeof(bullets));
}

static inline void bullet_maybe_set_vec(Bullet *b)
{
	if (!b->vec_dirty) return;

	b->dx = b->speed * cos(b->angle);
	b->dy = b->speed * sin(b->angle);

	b->vec_dirty = 0;
}

Bullet *bullet_spawn(BulletType type, float x, float y, float angle, float speed)
{
	for (int i = 0; i < ARRAYSIZE(bullets); i++)
	{
		Bullet *b = &bullets[i];
		if (b->active) continue;
		memset(b, 0, sizeof(*b));
		b->type = type;
		b->x = x;
		b->y = y;
		b->angle = angle;
		b->speed = speed;
		b->vec_dirty = 1;
		b->active = 1;
		bullet_maybe_set_vec(b);
		return b;
	}
	return NULL;
}

void bullets_run(void)
{
	for (int i = 0; i < ARRAYSIZE(bullets); i++)
	{
		Bullet *b = &bullets[i];
		if (!b->active) continue;
		// Animate
		b->anim_cnt++;
		if (b->anim_cnt >= BULLET_ANIM_SPEED)
		{
			b->anim_cnt = 0;
			b->anim_frame++;
			if (b->anim_frame >= BULLET_ANIM_LEN)
			{
				b->anim_frame = 0;
			}
		}
		// Move
		bullet_maybe_set_vec(b);
		b->x += b->dx;
		b->y += b->dy;

		// Draw
		const Gfx *gfx = gfx_get(GFX_BULLET);
		gfx_draw(gfx, b->anim_frame + 4 * b->type, (int)b->x, (int)b->y, 0);

		// Bounds
		if (b->x > 320 + 32) b->active = 0;
		else if (b->x < -32) b->active = 0;
		if (b->y > 240 + 32) b->active = 0;
		else if (b->y < -32) b->active = 0;

		// Scan for objects
		for (int i = 0; i < OBJ_COUNT; i++)
		{
			Obj *o = obj_get(i);
			if (o->type == OBJ_TYPE_NULL) continue;
			if (o->x + o->left > b->x) continue;
			if (o->x + o->right < b->x) continue;
			if (o->y + o->top > b->y) continue;
			if (o->y + o->bottom < b->y) continue;
			obj_touch_bullet(o, b);
		}

		// Collide
		Player *p = player_get();
		if (p->death > 0) continue;
		if (p->x + p->left > b->x) continue;
		if (p->x + p->right < b->x) continue;
		if (p->y + p->top > b->y) continue;
		if (p->y + p->bottom < b->y) continue;
		player_touch_bullet(p, b);
	}
}

Bullet *bullet_get(int index)
{
	if (index >= ARRAYSIZE(bullets))
	{
		fprintf(stderr, "Bullet %d NG!\n", index);
		return NULL;
	}
	return &bullets[index];
}

void bullet_erase(Bullet *b)
{
	b->active = 0;
}
