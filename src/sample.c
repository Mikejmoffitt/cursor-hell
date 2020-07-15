#include "sample.h"
#include "dr_wav.h"
#include "common.h"

// Control functions. ---------------------------------------------------------

static void associate_pcm(SampleChannelState *s, const SamplePcmData *sample)
{
	s->pcm_data = sample;
	s->pcm_play_index = 0;
	s->lifetime = 0;
}

// Plays the sample in the next available slot, if any.
void sample_play(SamplePlayerState *p, const SamplePcmData *sample)
{
	SDL_LockMutex(p->mutex);
	if (sample->data == NULL)
	{
		fprintf(stderr, "[sample] $%p->data == NULL\n", p);
		SDL_UnlockMutex(p->mutex);
		return;
	}
	// First look for a sample of the same type to retrigger.
	for (int i = 0; i < ARRAYSIZE(p->channels); i++)
	{
		if (p->channels[i].pcm_data == sample)
		{
			associate_pcm(&p->channels[i], sample);
			SDL_UnlockMutex(p->mutex);
			return;
		}
	}

	// If no interruption was found, then look for an empty slot.
	for (int i = 0; i < ARRAYSIZE(p->channels); i++)
	{
		if (!p->channels[i].pcm_data && !p->channels[i].reserved)
		{
			associate_pcm(&p->channels[i], sample);
			SDL_UnlockMutex(p->mutex);
			return;
		}
	}

	// Finally, replace the sample with the highest lifetime.
	int lifetime_highest = 0;
	int lifetime_highest_index = -1;
	for (int i = 0; i < ARRAYSIZE(p->channels); i++)
	{
		if (p->channels[i].reserved) continue;
		if (p->channels[i].lifetime > lifetime_highest)
		{
			lifetime_highest = p->channels[i].lifetime;
			lifetime_highest_index = i;
		}
	}

	if (lifetime_highest_index < 0)
	{
		fprintf(stderr, "[sample] No channel for $%p! Fuck!\n", p);
		SDL_UnlockMutex(p->mutex);
		return;
	}

	associate_pcm(&p->channels[lifetime_highest_index], sample);
	SDL_UnlockMutex(p->mutex);
}

// Forcibly plays a sample on a particular channel.
void sample_play_on_channel(SamplePlayerState *p, const SamplePcmData *sample,
                            int channel)
{
	SDL_LockMutex(p->mutex);
	if (sample->data == NULL)
	{
		fprintf(stderr, "[sample] $%p->data == NULL\n", p);
		SDL_UnlockMutex(p->mutex);
		return;
	}
	if (channel >= ARRAYSIZE(p->channels))
	{
		fprintf(stderr, "[sample] Invalid channel %d\n", channel);
		SDL_UnlockMutex(p->mutex);
		return;
	}

	associate_pcm(&p->channels[channel], sample);
	SDL_UnlockMutex(p->mutex);
}

// Stops all samples.
void sample_stop_all(SamplePlayerState *p)
{
	SDL_LockMutex(p->mutex);
	for (int i = 0; i < ARRAYSIZE(p->channels); i++)
	{
		p->channels[i].pcm_data = NULL;
	}
	SDL_UnlockMutex(p->mutex);

}

// Stops a sample on a particular channel.
void sample_stop_on_channel(SamplePlayerState *p, int channel)
{
	SDL_LockMutex(p->mutex);
	if (channel >= ARRAYSIZE(p->channels))
	{
		fprintf(stderr, "[sample] Invalid channel %d\n", channel);
		SDL_UnlockMutex(p->mutex);
		return;
	}

	p->channels[channel].pcm_data = NULL;
	SDL_UnlockMutex(p->mutex);
}

// Reserves a channel to be omitted from automatic selection.
void sample_reserve_channel(SamplePlayerState *p, int channel, int reserved)
{
	SDL_LockMutex(p->mutex);
	if (channel >= ARRAYSIZE(p->channels))
	{
		fprintf(stderr, "[sample] Invalid channel %d\n", channel);
		SDL_UnlockMutex(p->mutex);
		return;
	}

	p->channels[channel].reserved = reserved;
	SDL_UnlockMutex(p->mutex);
}

void sample_set_channel_gain(SamplePlayerState *p, int channel, float gain)
{
	SDL_LockMutex(p->mutex);
	if (channel >= ARRAYSIZE(p->channels))
	{
		fprintf(stderr, "[sample] Invalid channel %d\n", channel);
		SDL_UnlockMutex(p->mutex);
		return;
	}
	p->channels[channel].gain = gain;
	SDL_UnlockMutex(p->mutex);
}

// Playback functions. --------------------------------------------------------

// Runs one tick for the provided channel state, and fills l_out and r_out
// with sample data.
static inline void playback_process(SampleChannelState *s,
                                    int16_t *l_out, int16_t *r_out)
{
	if (!s->pcm_data) return;
	s->lifetime++;

	const SamplePcmData *pcm_data = s->pcm_data;

	int64_t pcm_idx = s->pcm_play_index * pcm_data->channels;

	if (pcm_idx >= pcm_data->len)
	{
		if (pcm_data->loop_point < 0)
		{
			s->pcm_data = NULL;
			s->pcm_play_index = 0;
			return;
		}
		else
		{
			s->pcm_play_index = pcm_data->loop_point;
			pcm_idx = s->pcm_play_index *
			                 pcm_data->channels;
		}
	}

	// Pull sample; if the source data is mono, simply double up.
	const int16_t sample_l = s->gain * (pcm_data->data[pcm_idx]);
	const int16_t sample_r = s->gain * ((pcm_data->channels == 1) ?
	                         pcm_data->data[pcm_idx] : pcm_data->data[pcm_idx + 1]);

	*l_out += sample_l;
	*r_out += sample_r;

	s->pcm_play_index++;
}

// Generates a single sample by iterating through all channels.
static inline void feed_sample(SamplePlayerState *p, int16_t *sample_out)
{
	sample_out[0] = 0;
	sample_out[1] = 0;
	// Accumulate data for all channels.
	for (int i = 0; i < ARRAYSIZE(p->channels); i++)
	{
		playback_process(&p->channels[i],
		                 &sample_out[0], &sample_out[1]);
	}
}

// Writes num int16_t stereo samples to data_out.
// If num == 1, four bytes will be written.
void sample_player_fill_buffer(SamplePlayerState *p,
                                  int16_t *data_out, int num)
{
	SDL_LockMutex(p->mutex);
	for (int i = 0; i < num; i++)
	{
		feed_sample(p, data_out);
		data_out += 2;
	}
	SDL_UnlockMutex(p->mutex);
}

void sample_player_init(SamplePlayerState *p)
{
	p->mutex = SDL_CreateMutex();
	memset(p->channels, 0, sizeof(p->channels));
	for (int i = 0; i < ARRAYSIZE(p->channels); i++)
	{
		sample_set_channel_gain(p, i, 1.0f);
	}
}

void sample_player_shutdown(SamplePlayerState *p)
{
	SDL_DestroyMutex(p->mutex);
}

// Sample loading and destroying. ---------------------------------------------
int sample_load_pcm_from_wav(SamplePcmData *s, const char *wav_fname)
{
	// Load WAV file into drwav
	drwav wav;

	if (!drwav_init_file(&wav, wav_fname, NULL))
	{
		printf("[sample] Error loading %s\n", wav_fname);
		return 0;
	}
	if (wav.channels > 2)
	{
		printf("[sample] Can't use %d channel WAVs\n", wav.channels);
		return 0;
	}

	// Load PCM data from wav
	s->len = wav.totalPCMFrameCount * wav.channels;
	s->data = (int16_t *)malloc(s->len * sizeof(int16_t));
	s->loop_point = wav.smpl.numSampleLoops > 0 ?
	                wav.smpl.loops[0].start : -1;
	s->channels = wav.channels;

	if (!drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount,
				       s->data))
	{
		printf("[sample] Couldn't load WAV data.\n");
		sample_unload_pcm(s);
		return 0;
	}

	drwav_uninit(&wav);
	return 1;
}

void sample_unload_pcm(SamplePcmData *s)
{
	if (!s->data) return;
	free(s->data);
	s->data = NULL;
}
