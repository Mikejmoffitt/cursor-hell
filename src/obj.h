#ifndef OBJ_H
#define OBJ_H

#include "gfx.h"
#include "bullet.h"
#include "player.h"

#define OBJ_COUNT 128

typedef enum ObjType
{
	OBJ_TYPE_NULL,
	OBJ_TYPE_FISH_X,
	OBJ_TYPE_INVALID_COUNT,
} ObjType;

typedef struct Obj
{
	ObjType type;
	float x;
	float y;
	int hp;
	int damage_flash;

	// Hitbox
	int top, bottom, left, right;

	// Graphics
	const Gfx *gfx;
	int gfx_frame;
	int gfx_flags;


	uint8_t data[256];  // Generic storage for subtypes
} Obj;

void obj_clear(void);
void obj_run(void);
void obj_erase(Obj *o);
void obj_touch_bullet(Obj *o, Bullet *b);
void obj_touch_player(Obj *o, Player *p);
Obj *obj_spawn(ObjType type, float x, float y);
Obj *obj_get(int index);

#endif  // OBJ_H
