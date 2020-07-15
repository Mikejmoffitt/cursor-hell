#ifndef _SOUND_H
#define _SOUND_H

typedef enum SoundCue
{
	SOUND_MUS_STOP = 0x00,
	SOUND_MUS_BGM = 0x01,
	SOUND_SHOT = 0x02,
	SOUND_HIT = 0x03,
	SOUND_INVALID,
} SoundCue;

int sound_init(void);
void sound_shutdown(void);
void sound_play_sfx(SoundCue id);
void sound_play_music(SoundCue id);
void sound_stop(void);
void sound_set_mute(int muted);
int sound_get_mute(void);

#endif // _SOUND_H
