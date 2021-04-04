/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2012 Sam Lantinga <slouken@libsdl.org>

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

  James Le Cuirot
  chewi@aura-online.co.uk
*/

#ifdef USE_FLUIDSYNTH_MIDI

#include <stdio.h>
#include <sys/types.h>

#include "SDL_mixer.h"
#include "music_fluidsynth.h"

static Uint16 format;
static Uint8 channels;
static int freq;

#define CVT_BUFSIZE 4096

int SDLCALL fluidsynth_check_soundfont(const char *path, void *data)
{
	FILE *file = fopen(path, "r");

	if (file) {
		fclose(file);
		return 1;
	} else {
		Mix_SetError("Failed to access the SoundFont %s", path);
		return 0;
	}
}

int SDLCALL fluidsynth_load_soundfont(const char *path, void *data)
{
	/* If this fails, it's too late to try Timidity so pray that at least one works. */
	fluidsynth.fluid_synth_sfload((fluid_synth_t*) data, path, 1);
	return 1;
}

int fluidsynth_init(SDL_AudioSpec *mixer)
{
	if (!Mix_EachSoundFont(fluidsynth_check_soundfont, NULL))
		return -1;

	format = mixer->format;
	channels = mixer->channels;
	freq = mixer->freq;

	return 0;
}

static FluidSynthMidiSong *fluidsynth_loadsong_common(SDL_RWops *rw)
{
	FluidSynthMidiSong *song;
	off_t rw_offset;
	size_t rw_size;
	char *rw_mem;
	int src_freq;
	int ret;

	if (!Mix_Init(MIX_INIT_FLUIDSYNTH)) {
		return NULL;
	}

	if (!(song = SDL_malloc(sizeof(FluidSynthMidiSong)))) {
		Mix_SetError("Insufficient memory for song");
		return NULL;
	}

	SDL_memset(song, 0, sizeof(FluidSynthMidiSong));

	/* fluidsynth limits: */
	src_freq = freq;
	if (src_freq < 8000) {
		src_freq = 8000;
	}
	else if (src_freq > 96000) {
		src_freq = 48000;
	}
	if (SDL_BuildAudioCVT(&song->convert, AUDIO_S16SYS, 2, src_freq, format, channels, freq) < 0) {
		Mix_SetError("Failed to set up audio conversion");
		goto fail;
	}
	song->convert.buf = SDL_malloc(CVT_BUFSIZE * song->convert.len_mult);
	if (!song->convert.buf) {
		Mix_SetError("Insufficient memory for song");
		goto fail;
	}

	if (!(song->settings = fluidsynth.new_fluid_settings())) {
		Mix_SetError("Failed to create FluidSynth settings");
		goto fail;
	}

	fluidsynth.fluid_settings_setnum(song->settings, "synth.sample-rate", (double)src_freq);

	if (!(song->synth = fluidsynth.new_fluid_synth(song->settings))) {
		Mix_SetError("Failed to create FluidSynth synthesizer");
		goto fail;
	}

	if (!Mix_EachSoundFont(fluidsynth_load_soundfont, song->synth)) {
		goto fail;
	}

	if (!(song->player = fluidsynth.new_fluid_player(song->synth))) {
		Mix_SetError("Failed to create FluidSynth player");
		goto fail;
	}

	rw_offset = SDL_RWtell(rw);
	SDL_RWseek(rw, 0, RW_SEEK_END);
	rw_size = SDL_RWtell(rw) - rw_offset;
	SDL_RWseek(rw, rw_offset, RW_SEEK_SET);

	if (!(rw_mem = (char*) SDL_malloc(rw_size))) {
		Mix_SetError("Insufficient memory for song");
		goto fail;
	}

	if(SDL_RWread(rw, rw_mem, rw_size, 1) != 1) {
		SDL_free(rw_mem);
		Mix_SetError("Failed to read in-memory song");
		goto fail;
	}

	ret = fluidsynth.fluid_player_add_mem(song->player, rw_mem, rw_size);
	SDL_free(rw_mem);

	if (ret != FLUID_OK) {
		Mix_SetError("FluidSynth failed to load in-memory song");
		goto fail;
	}

	return song;

fail:
	fluidsynth_freesong(song);
	return NULL;
}

FluidSynthMidiSong *fluidsynth_loadsong_RW(SDL_RWops *rw, int freerw)
{
	FluidSynthMidiSong *song;

	song = fluidsynth_loadsong_common(rw);
	if (freerw) {
		SDL_RWclose(rw);
	}
	return song;
}

void fluidsynth_freesong(FluidSynthMidiSong *song)
{
	if (!song) {
		return;
	}
	if (song->player) {
		fluidsynth.delete_fluid_player(song->player);
	}
	if (song->synth) {
		fluidsynth.delete_fluid_synth(song->synth);
	}
	if (song->settings) {
		fluidsynth.delete_fluid_settings(song->settings);
	}
	SDL_free(song->convert.buf);
	SDL_free(song);
}

void fluidsynth_start(FluidSynthMidiSong *song)
{
	fluidsynth.fluid_player_set_loop(song->player, 1);
	fluidsynth.fluid_player_play(song->player);
	song->playing = 1;
}

void fluidsynth_stop(FluidSynthMidiSong *song)
{
	fluidsynth.fluid_player_stop(song->player);
	song->playing = 0;
}

int fluidsynth_active(FluidSynthMidiSong *song)
{
	return fluidsynth.fluid_player_get_status(song->player) == FLUID_PLAYER_PLAYING ? 1 : 0;
}

void fluidsynth_setvolume(FluidSynthMidiSong *song, int volume)
{
	/* FluidSynth's default is 0.2. Make 1.2 the maximum. */
	fluidsynth.fluid_synth_set_gain(song->synth, (float) (volume * 1.2 / MIX_MAX_VOLUME));
}

static void fluid_get_samples(FluidSynthMidiSong *song)
{
	Uint8 data[CVT_BUFSIZE];
	SDL_AudioCVT *cvt;

	if (fluidsynth.fluid_synth_write_s16(song->synth, CVT_BUFSIZE / 4, data, 0, 2, data, 1, 2) != FLUID_OK) {
		fluidsynth_stop(song);
		Mix_SetError("Error generating FluidSynth audio");
		return;
	}

	cvt = &song->convert;
	memcpy(cvt->buf, data, CVT_BUFSIZE);
	if (cvt->needed) {
		cvt->len = CVT_BUFSIZE;
		SDL_ConvertAudio(cvt);
	} else {
		cvt->len_cvt = CVT_BUFSIZE;
	}
	song->len_available = cvt->len_cvt;
	song->snd_available = cvt->buf;
}

int fluidsynth_playsome(FluidSynthMidiSong *song, Uint8 *dest, int len)
{
	int mixable;

	while (len > 0 && song->playing) {
		if (!song->len_available) {
			fluid_get_samples(song);
		}
		mixable = len;
		if (mixable > song->len_available) {
			mixable = song->len_available;
		}
		memcpy(dest, song->snd_available, mixable);
		song->len_available -= mixable;
		song->snd_available += mixable;
		len -= mixable;
		dest += mixable;
	}

	return len;
}

#endif /* USE_FLUIDSYNTH_MIDI */
