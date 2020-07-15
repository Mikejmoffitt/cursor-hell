/*

PCM Sample Support

Gimmick! exAct * Mix
(c) Michael Moffitt 2020
------------------------

This file provides structs and functions for interacting with PCM samples.
stereo int16_t data is emitted.

*/

#ifndef SAMPLE_H
#define SAMPLE_H

#include <stdint.h>
#include <SDL_mutex.h>

#define SAMPLE_PLAYBACK_CHANNELS 5

// Wrapper for raw int16_t PCM data.
typedef struct SamplePcmData
{
	int16_t *data;  // Dynamically allocated sample memory.
	int len;
	int loop_point;  // < 0 = does not loop.
	int channels;
} SamplePcmData;

typedef struct SampleChannelState
{
	const SamplePcmData *pcm_data;  // Unoccupied if NULL.
	float gain;
	int pcm_play_index;
	int reserved;
	unsigned int lifetime;  // increments every tick of the engine.
	// TODO: When this code is inevitably reused, a few features to add:
	// * sample priority for interruption
	// * repeat count
	// * panning
} SampleChannelState;

typedef struct SamplePlayerState
{
	SampleChannelState channels[SAMPLE_PLAYBACK_CHANNELS];
	SDL_mutex *mutex;  // Used internally.
} SamplePlayerState;

// Control functions. ---------------------------------------------------------

// Plays the sample in the next available slot, if any.
void sample_play(SamplePlayerState *p, const SamplePcmData *sample);

// Forcibly plays a sample on a particular channel.
void sample_play_on_channel(SamplePlayerState *p, const SamplePcmData *sample,
                            int channel);

// Stops all samples.
void sample_stop_all(SamplePlayerState *p);

// Stops a sample on a particular channel.
void sample_stop_on_channel(SamplePlayerState *p, int channel);

// Reserves a channel to be omitted from automatic selection.
void sample_reserve_channel(SamplePlayerState *p, int channel, int reserved);

// Assign a gain coefficient to a channel.
void sample_set_channel_gain(SamplePlayerState *p, int channel, float gain);

// Playback functions. --------------------------------------------------------

// Runs one tick for the provided channel state, and fills l_out and r_out
// with sample data.
static inline void playback_process(SampleChannelState *s,
                                    int16_t *l_out, int16_t *r_out);

// Generates a single sample by iterating through all channels.
static inline void feed_sample(SamplePlayerState *p, int16_t *sample_out);

// Writes num int16_t stereo samples to data_out.
// If num == 1, four bytes will be written.
void sample_player_fill_buffer(SamplePlayerState *p,
                                  int16_t *data_out, int num);

// Initializes the SamplePlayerState, and allocates needed memory (not data for
// SamplePcmData, though).
void sample_player_init(SamplePlayerState *p);

// Shuts down the SamplePlayerState and frees associated memory (this does NOT
// include SamplePcmData)
void sample_player_shutdown(SamplePlayerState *p);

// Sample loading and destroying. ---------------------------------------------

// Initializes a SamplePcmData struct with PCM data, loaded from a WAV file.
// Allocates memory.
int sample_load_pcm_from_wav(SamplePcmData *s, const char *wav_fname);

// Frees PCM sample data from a SamplePcmData struct.
void sample_unload_pcm(SamplePcmData *s);

#endif  // SAMPLE_H
