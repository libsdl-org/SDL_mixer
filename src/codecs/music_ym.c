/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2023 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/*
	This file supports playing YM files ( Atari ST music file format )
	StSound written by Arnaud Carré (aka Leonard/Oxygene)
	Added implementation by Aleksandr Andreev (aka yozka)
 */

#ifdef MUSIC_YM

#include "music_ym.h"

#include <YmTypes.h>
#include <StSoundLibrary.h>

typedef struct
{
	int play_count;
	int frequency;
	int channels;
	int samples;
	int volume;
	SDL_bool looped;

	SDL_AudioStream *stream;
	void *buffer;
	Sint32 buffer_size;
	
	YMMUSIC *song;
	ymMusicInfo_t yminfo;
} YM_Music;

static int YM_Seek(void *context, double position);
static void YM_Delete(void *context);

void *YM_CreateFromRW(SDL_RWops *src, int freesrc)
{
	YM_Music *music;

	music = (YM_Music *)SDL_calloc(1, sizeof(*music));
	if (!music) {
		SDL_OutOfMemory();
		return NULL;
	}

	music->volume 		= MIX_MAX_VOLUME;
	music->channels		= 1;
	music->frequency	= 44100;
	music->samples 		= music->frequency * YM_SAMPLE_LENGTH_SEC;
	
	music->song = ymMusicCreate();
	if (!music->song) {
		YM_Delete(music);
		return NULL;
	}
	
	const Sint64 size = SDL_RWsize(src);
	void* pBlock = SDL_malloc(size);
	size_t read = SDL_RWread(src, pBlock, 1, size);
	ymbool load = ymMusicLoadMemory(music->song, pBlock, SDL_static_cast(ymu32, size));
	SDL_free(pBlock);
	if (load == YMFALSE) {
		YM_Delete(music);
		return NULL;
	}
	ymMusicGetInfo(music->song, &music->yminfo);
	
	music->stream = SDL_NewAudioStream(AUDIO_S16SYS, music->channels, music->frequency,
									   music_spec.format, music_spec.channels, music_spec.freq);
	if (!music->stream) {
		YM_Delete(music);
		return NULL;
	}

	music->buffer_size = music->samples * music->channels * sizeof(ymsample);
	music->buffer = SDL_malloc((size_t)music->buffer_size);
	if (!music->buffer) {
		SDL_OutOfMemory();
		YM_Delete(music);
		return NULL;
	}

	if (freesrc) {
		SDL_RWclose(src);
	}
	return music;
}

static void YM_SetVolume(void *context, int volume)
{
	YM_Music *music = (YM_Music *)context;
	music->volume = volume;
}

static int YM_GetVolume(void *context)
{
	YM_Music *music = (YM_Music *)context;
	return music->volume;
}

static int YM_Play(void *context, int play_count)
{
	YM_Music *music = (YM_Music *)context;
	music->play_count = play_count;
	music->looped = play_count < 0 ? SDL_TRUE : SDL_FALSE;
	ymMusicSetLoopMode(music->song, music->looped == SDL_TRUE ? YMTRUE : YMFALSE);
	ymMusicPlay(music->song);
	return YM_Seek(music, 0.0);
}

static int YM_GetSome(void *context, void *data, int bytes, SDL_bool *done)
{
	YM_Music *music = (YM_Music *)context;
	int filled, amount, currentMs, remainingMs, duration, nbSample;

	filled = SDL_AudioStreamGet(music->stream, data, bytes);
	if (filled != 0) {
		return filled;
	}

	if (music->play_count == 0) {
		/* All done */
		*done = SDL_TRUE;
		return 0;
	}
	
	currentMs = ymMusicGetPos(music->song);
	remainingMs = music->yminfo.musicTimeInMs - currentMs;
	if (remainingMs < 0) {
		return -1;
	}
	if (music->looped && remainingMs == 0) {
		remainingMs = music->yminfo.musicTimeInMs;
	}
	
	duration = music->samples * 1000 / music->frequency;
	if (remainingMs > duration) {
		remainingMs = duration;
	}
	
	nbSample = (remainingMs * music->frequency) / 1000;
	amount = nbSample * sizeof(ymsample);
	if (amount > music->buffer_size) {
		return -1;
	}

	if (amount > 0 && nbSample > 0) {
		ymbool ret = ymMusicCompute(music->song, (ymsample*)music->buffer, nbSample);
		if (ret == YMFALSE) {
			return -1;
		}
		if (SDL_AudioStreamPut(music->stream, music->buffer, amount) < 0) {
			return -1;
		}
	} else 
		if (music->play_count == 1) {
			music->play_count = 0;
			SDL_AudioStreamFlush(music->stream);
		} else {
			int play_count = -1;
			if (music->play_count > 0) {
				play_count = (music->play_count - 1);
			}
			if (YM_Play(music, play_count) < 0) {
				return -1;
			}
			
		}
	return 0;
}

static int YM_GetAudio(void *context, void *data, int bytes)
{
	YM_Music *music = (YM_Music *)context;
	return music_pcm_getaudio(context, data, bytes, music->volume, YM_GetSome);
}

static SDL_bool YM_IsPlaying(void *context)
{
	YM_Music *music = (YM_Music *)context;
	return music->play_count == 0 ? SDL_FALSE : SDL_TRUE;
}

static int YM_Seek(void *context, double position)
{
	YM_Music *music = (YM_Music *)context;
	if (ymMusicIsSeekable(music->song) == YMFALSE) {
		return -1;
	}
	ymMusicSeek(music->song, (ymu32)(position * 1000));
    return 0;
}

static double YM_Tell(void *context)
{
	YM_Music *music = (YM_Music *)context;
	double pos = ymMusicGetPos(music->song) / 1000.0;
	return pos;
}

static double YM_Duration(void *context)
{
	YM_Music *music = (YM_Music *)context;
	double duration = music->yminfo.musicTimeInMs / 1000.0;
	return duration;
}

static void YM_Delete(void *context)
{
	YM_Music *music = (YM_Music *)context;
	if (music->song) {
		ymMusicDestroy(music->song);
	}
	if (music->stream) {
		SDL_FreeAudioStream(music->stream);
	}
	if (music->buffer) {
		SDL_free(music->buffer);
	}
	SDL_free(music);
}

static void YM_Stop(void *context)
{
	YM_Music *music = (YM_Music *)context;
	music->play_count = 0;
	if (ymMusicIsOver(music->song)) {
		ymMusicStop(music->song);
	}
	SDL_AudioStreamClear(music->stream);
}

Mix_MusicInterface Mix_MusicInterface_YM =
{
	"YM",
	MIX_MUSIC_YM,
	MUS_YM,
	SDL_FALSE,
	SDL_FALSE,

	NULL,	/* Load */
	NULL,	/* Open */
	YM_CreateFromRW,
	NULL,	/* CreateFromFile */
	YM_SetVolume,
	YM_GetVolume,
	YM_Play,
	YM_IsPlaying,
	YM_GetAudio,
	NULL,	/* Jump */
	YM_Seek,
	YM_Tell,
	YM_Duration,
	NULL,	/* LoopStart */
	NULL,	/* LoopEnd */
	NULL,	/* LoopLength */
	NULL,	/* GetMetaTag */
	NULL,	/* GetNumTracks */
	NULL,	/* StartTrack */
	NULL,	/* Pause */
	NULL,	/* Resume */
	YM_Stop,
	YM_Delete,
	NULL,	/* Close */
	NULL	/* Unload */
};

#endif /* MUSIC_YM */

/* vi: set ts=4 sw=4 expandtab: */
