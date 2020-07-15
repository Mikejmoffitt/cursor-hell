#include "player.h"
#include "input.h"
#include "gfx.h"
#include "common.h"
#include "sound.h"

#define PLAYER_SHOT_RATE_LIMIT 2
#define PLAYER_SHOT_BURST 4
#define PLAYER_LASER_CUTOFF 30
#define PLAYER_SPEED_NORMAL 2.0f
#define PLAYER_SPEED_FOCUS 1.5f

static Player player;

Player *player_get(void)
{
	return &player;
}

void player_reset(Player *p)
{
	p->x = 32;
	p->y = 120;
	p->dir = 0;
}

void player_clear(Player *p)
{
	memset(p, 0, sizeof(*p));
	p->top = -3;
	p->bottom = 3;
	p->left = -5;
	p->right = 5;
	player_reset(p);
}

static void shot_logic(Player *p)
{

	if ((input_get_impulse(0) & (SW_2)) || (input_get_state(0) & SW_1))
	{
		p->shots_pending = PLAYER_SHOT_BURST;
	}
	if (input_get_state(0) & SW_2)
	{
		p->laser_hold++;
	}
	else
	{
		p->laser_hold = 0;
	}

	if (p->laser_hold < PLAYER_LASER_CUTOFF)
	{
		if (p->shots_pending)
		{
			if (p->shot_cooldown == 0)
			{
				bullet_spawn(BULLET_TYPE_OPA_SHOT, p->x, p->y, p->dir ? 3.14159 : 0, 6);
				sound_play_sfx(2);
				p->shot_cooldown = PLAYER_SHOT_RATE_LIMIT;
				p->shots_pending--;
			}
			else
			{
				p->shot_cooldown--;
			}
		}
	}
	else
	{
		bullet_spawn(BULLET_TYPE_OPA_LASER, p->x, p->y, p->dir ? 3.14159 : 0, 12);
		sound_play_sfx(3);
	}
}

void player_run(Player *p)
{
	if (p->muteki > 0) p->muteki--;
	if (p->death > 0)
	{
		p->death--;
		if (p->death == 0)
		{
			player_reset(p);
		}
		else
		{
			return;
		}
	}

	// Animate
	p->anim_cnt++;
	if (p->anim_cnt >= 12)
	{
		p->anim_cnt = 0;
		p->anim_frame++;
		if (p->anim_frame >= 2)
		{
			p->anim_frame = 0;
		}
	}
	shot_logic(p);


	// Move
	const int speed = (p->laser_hold < PLAYER_LASER_CUTOFF ? PLAYER_SPEED_NORMAL : PLAYER_SPEED_FOCUS);
	if (input_get_state(0) & SW_RIGHT)
	{
		p->x += speed;
		p->dir = 0;
	}
	else if (input_get_state(0) & SW_LEFT)
	{
		p->x -= speed;
//		p->dir = 1;
	}
	if (input_get_state(0) & SW_DOWN)
	{
		p->y += speed;
	}
	else if (input_get_state(0) & SW_UP)
	{
		p->y -= speed;
	}

	if (p->x < 0) p->x = 0;
	if (p->x > 320) p->x = 320;
	if (p->y < 0) p->y = 0;
	if (p->y > 240) p->y = 240;


	// Render
	if (p->muteki % 4 == 0)
	{
		gfx_draw(gfx_get(GFX_OPAOPA), p->anim_frame, p->x, p->y, p->dir ? GFX_FLAG_H_FLIP : 0);
	}
	else
	{
		gfx_draw_mask(gfx_get(GFX_OPAOPA), gfx_map_rgba(128, 192, 255, 255), p->anim_frame, p->x, p->y, p->dir ? GFX_FLAG_H_FLIP : 0);
	}
}

void player_touch_bullet(Player *p, Bullet *b)
{
	if (p->death > 0 || p->muteki > 0) return;
	if (b->type <= BULLET_TYPE_BLUE)
	{
		player_kill(p);
		bullet_erase(b);
	}
}

void player_kill(Player *p)
{
	p->death = 120;
	p->muteki = 160;
}
