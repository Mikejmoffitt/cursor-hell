#ifndef PLAYER_H
#define PLAYER_H

#include "bullet.h"

typedef struct Player
{
	float x;
	float y;
	int dir;
	int death;
	int muteki;
	int shots_pending;
	int shot_cooldown;
	int laser_hold;
	int anim_cnt;
	int anim_frame;
	int top, bottom, left, right;
} Player;

void player_touch_bullet(Player *p, Bullet *b);
void player_run(Player *p);
void player_clear(Player *p);
void player_kill(Player *p);

Player *player_get(void);

#endif  // PLAYER_H
