#include "obj.h"
#include "gfx.h"
#include "bullet.h"
#include "common.h"
#include "palette.h"
static Obj obj[OBJ_COUNT];

static int osc = 0;

void obj_clear(void)
{
	memset(obj, 0, sizeof(obj));
}

void fishx_func(Obj *o, int init)
{
	if (init)
	{
		o->gfx = gfx_get(GFX_FISH_X);
		o->hp = 2560;
		o->top = -30;
		o->bottom = 30;
		o->left = -40;
		o->right = 40;
	}
	o->data[0]++;
	o->gfx_frame = (o->data[0] / 16) % 2;
	if (o->data[0] > 127)
	{
		o->y += 0.5;
	}
	else
	{
		o->y -= 0.5;
	}
	if (o->data[0] == 0xFF) o->data[1]++;

	const Player *p = player_get();
	const float towards_pl = 3.14159 + atan((o->y - p->y) / (o->x - p->x));

	switch (o->data[1] % 4)
	{
		case 0:
			if (o->data[0] % 8 == 0)
			{
				bullet_spawn(BULLET_TYPE_PINK, o->x, o->y, 3.14159 - (0.11 * o->data[0]), 2.0f);
				bullet_spawn(BULLET_TYPE_PINK, o->x, o->y, 3.14159 + (0.11 * o->data[0]), 2.0f);
			}
			break;
		case 1:
			if (o->data[0] % 8 == 0)
			{
				bullet_spawn(BULLET_TYPE_BLUE, o->x, o->y, 0.1 * o->data[0], 2.0f);
			}
			if (o->data[0] % 64 < 48)
			{
				bullet_spawn(BULLET_TYPE_PINK, o->x, o->y, (0.3 * o->data[0]), 1.0f);
			}
			break;
		case 2:
			if (o->data[0] % 16 == 0)
			{
				bullet_spawn(BULLET_TYPE_PINK, o->x, o->y, towards_pl, 2.0f);
			}
			if (o->data[0] % 32 < 16)
			{
				bullet_spawn(BULLET_TYPE_BLUE, o->x, o->y, (0.5 * o->data[0]), 0.5f);
			}
			break;
		case 3:
			if (o->data[0] % 32 == 0)
			{
				bullet_spawn(BULLET_TYPE_PINK, o->x, o->y, towards_pl + 0.3, 2.5f);
				bullet_spawn(BULLET_TYPE_PINK, o->x, o->y, towards_pl - 0.3, 2.5f);
			}
			else if (o->data[0] % 32 == 16)
			{
				bullet_spawn(BULLET_TYPE_PINK, o->x, o->y, towards_pl, 2.5f);
				bullet_spawn(BULLET_TYPE_PINK, o->x, o->y, towards_pl + 0.15, 2.5f);
				bullet_spawn(BULLET_TYPE_PINK, o->x, o->y, towards_pl - 0.15, 2.5f);
			}
			break;
	}


}

static void (*obj_funcs[OBJ_COUNT])(Obj *o, int init) =
{
	[OBJ_TYPE_FISH_X] = fishx_func,
};

static void (*obj_bullet_funcs[OBJ_COUNT])(Obj *o, Bullet *b) =
{

};

static void (*obj_player_funcs[OBJ_COUNT])(Obj *o, Player *p) =
{

};

static void standard_bullet_func(Obj *o, Bullet *b)
{
	if (b->type >= BULLET_TYPE_OPA_SHOT)
	{
		b->active = 0;
		o->damage_flash = 19 + osc;
		o->hp--;
		if (o->hp <= 0)
		{
			// TODO: Spawn explosion.
			o->type = OBJ_TYPE_NULL;
		}
	}
}

static void standard_player_func(Obj *o, Player *p)
{
	player_kill(p);
	o->damage_flash = 19 + osc;
}

void obj_run(void)
{
	osc ^= 1;
	for (int i = 0; i < ARRAYSIZE(obj); i++)
	{
		Obj *o = &obj[i];
		if (o->type == OBJ_TYPE_NULL) continue;

		if (o->damage_flash > 0) o->damage_flash--;

		if (obj_funcs[o->type]) obj_funcs[o->type](o,/*init=*/0);
		if (o->gfx)
		{
			if (o->damage_flash % 2 == 0)
			{
				gfx_draw(o->gfx, o->gfx_frame, o->x, o->y, o->gfx_flags);
			}
			else
			{
				gfx_draw_mask(o->gfx, COLOR_WHITE, o->gfx_frame, o->x, o->y, o->gfx_flags);
			}
		}

		// Scan for player
		Player *p = player_get();
		if (o->x + o->right < p->x + p->left) continue;
		if (o->x + o->left > p->x + p->right) continue;
		if (o->y + o->bottom < p->y + p->top) continue;
		if (o->y + o->top > p->y + p->bottom) continue;
		obj_touch_player(o, p);
	}
}

Obj *obj_spawn(ObjType type, float x, float y)
{
	for (int i = 0; i < ARRAYSIZE(obj); i++)
	{
		Obj *o = &obj[i];
		if (o->type != OBJ_TYPE_NULL) continue;
		memset(o, 0, sizeof(*o));
		o->type = type;
		o->x = x;
		o->y = y;
		if (obj_funcs[type]) obj_funcs[type](o,/*init=*/1);
		return o;
	}
	return NULL;
}

void obj_erase(Obj *o)
{
	o->type = OBJ_TYPE_NULL;
}

void obj_touch_bullet(Obj *o, Bullet *b)
{
	if (obj_bullet_funcs[o->type])
	{
		obj_bullet_funcs[o->type](o, b);
	}
	else
	{
		standard_bullet_func(o, b);
	}
}

void obj_touch_player(Obj *o, Player *p)
{
	if (obj_player_funcs[o->type])
	{
		obj_player_funcs[o->type](o, p);
	}
	else
	{
		standard_player_func(o, p);
	}
}

Obj *obj_get(int index)
{
	if (index >= ARRAYSIZE(obj))
	{
		fprintf(stderr, "Obj %d NG!\n", index);
		return NULL;
	}
	return &obj[index];
}
