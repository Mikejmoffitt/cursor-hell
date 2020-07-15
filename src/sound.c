#include "sound.h"
#include <stdio.h>
#include <SDL.h>
#include <SDL_audio.h>
#include "sample.h"
#include "common.h"

#define SOUND_MASTER_GAIN 1.50

// For balancing SFX and MUS
#define SOUND_SFX_GAIN 0.8
#define SOUND_MUS_GAIN 1.0

// Sound effect samples.
static SamplePcmData sam_sfx[SOUND_INVALID];

// Stream variables for music.

#define AUDIO_RATE 44100
#define AUDIO_FORMAT AUDIO_S16SYS
#define AUDIO_CHANNELS 2
#define AUDIO_SAMPLES 16

#define MUSIC_TRACK_COUNT 1

static SDL_AudioDeviceID audio_device_id;
static SDL_AudioSpec audio_spec;

static int music_current_track_id = -1;
static int mute;

// Data for music cues.
typedef struct MusicSampleList
{
	SamplePcmData track_data;
} MusicSampleList;
static MusicSampleList music_samples[MUSIC_TRACK_COUNT];

static SamplePlayerState sample_player;

static const int music_id_lookup[] =
{
	[SOUND_MUS_BGM] = 0,
};

// Sample feeder routines. ----------------------------------------------------
void audio_callback(void *userdata, uint8_t *stream, int len)
{
	memset(stream, 0, len);
	int16_t *out_data = (int16_t *)stream;
	const int num_samples = (len / (sizeof(int16_t)) / 2);
	sample_player_fill_buffer(&sample_player, out_data, num_samples);
}

static void load_tracks(void)
{
	for (int i = 0; i < ARRAYSIZE(music_samples); i++)
	{
		SamplePcmData *track_data = &music_samples[i].track_data;
		char buffer[256];
		snprintf(buffer, sizeof(buffer), "res/music/%02d.wav", i);
		sample_load_pcm_from_wav(track_data, buffer);
	}
}

static void music_play(int track)
{
	if (track >= ARRAYSIZE(music_samples))
	{
		printf("[sound] Invalid track %d requested\n", track);
		return;
	}
	if (track == music_current_track_id) return;
	music_current_track_id = track;

	sample_play_on_channel(&sample_player,
	                       &music_samples[track].track_data,
	                       0);
}

static void music_stop(void)
{
	sample_stop_on_channel(&sample_player, 0);
	music_current_track_id = -1;
}

static void play_sfx(SoundCue id)
{
	if (id >= SOUND_INVALID)
	{
		printf("[sound] Invalid sound ID $%02X selected\n", id);
		return;
	}

	if (sam_sfx[id].data)
	{
		sample_play(&sample_player, &sam_sfx[id]);
	}
}

void sound_play_sfx(SoundCue id)
{
	if (mute) return;
	play_sfx(id);
}

void sound_play_music(SoundCue id)
{
	if (id == SOUND_MUS_STOP) music_stop();
	else music_play(music_id_lookup[id]);
}

void sound_stop(void)
{
	music_stop();
	sample_stop_all(&sample_player);
}

static void load_sfx(void)
{
	for (int i = 0; i < SOUND_INVALID; i++)
	{
		char fpath[256];
		snprintf(fpath, 256, "res/sfx/%02x.wav", i);
		sample_load_pcm_from_wav(&sam_sfx[i], fpath);
	}
}

static void unload_sfx(void)
{
	for (int i = 0; i < SOUND_INVALID; i++)
	{
		if (sam_sfx[i].data)
		{
			sample_unload_pcm(&sam_sfx[i]);
		}
	}
}

int sound_init(void)
{
	audio_spec.freq = AUDIO_RATE;
	audio_spec.format = AUDIO_FORMAT;
	audio_spec.channels = AUDIO_CHANNELS;
	audio_spec.samples = AUDIO_SAMPLES;
	audio_spec.callback = audio_callback;
	audio_spec.userdata = NULL;

	audio_device_id = SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);
	if (audio_device_id == 0)
	{
		printf("[sound] Couldn't open audio device\n");
		return 0;
	}

	load_sfx();
	load_tracks();

	SDL_PauseAudioDevice(audio_device_id, 0);

	music_current_track_id = -1;

	sample_player_init(&sample_player);

	sample_reserve_channel(&sample_player, 0, 1);  // Don't use 0 for SFX.
	sample_set_channel_gain(&sample_player, 0, SOUND_MUS_GAIN * SOUND_MASTER_GAIN);
	for (int i = 1; i < SAMPLE_PLAYBACK_CHANNELS; i++)
	{
		sample_set_channel_gain(&sample_player, i, SOUND_SFX_GAIN * SOUND_MASTER_GAIN);
	}
	return 1;
}

void sound_shutdown(void)
{
	sample_player_shutdown(&sample_player);
	SDL_PauseAudioDevice(audio_device_id, 1);
	SDL_CloseAudioDevice(audio_device_id);

	unload_sfx();

	for (int i = 0; i < ARRAYSIZE(music_samples); i++)
	{
		SamplePcmData *track_data = &music_samples[i].track_data;
		sample_unload_pcm(track_data);
	}
}

void sound_set_mute(int muted)
{
	mute = muted;
}

int sound_get_mute(void)
{
	return mute;
}
