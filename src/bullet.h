#ifndef BULLET_H
#define BULLET_H

#define BULLET_COUNT 256

#define BULLET_ANIM_SPEED 3
#define BULLET_ANIM_LEN 4

typedef enum BulletType
{
	BULLET_TYPE_PINK,
	BULLET_TYPE_BLUE,
	BULLET_TYPE_OPA_SHOT,
	BULLET_TYPE_OPA_LASER,
} BulletType;

typedef struct Bullet
{
	int active;
	int type;
	float x, y;
	float dx, dy;
	float angle, speed;
	int vec_dirty;
	int anim_frame;
	int anim_cnt;
} Bullet;

void bullets_clear(void);
void bullet_erase(Bullet *b);
void bullets_run(void);
Bullet *bullet_spawn(BulletType type, float x, float y, float angle, float speed);
Bullet *bullet_get(int index);

#endif  // BULLET_H
