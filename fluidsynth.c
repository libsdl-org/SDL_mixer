/*
    SDL_mixer:  An audio mixer library based on the SDL library
    Copyright (C) 1997-2011 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    James Le Cuirot
    chewi@aura-online.co.uk
*/

#include "fluidsynth.h"
#include "SDL_mixer.h"
#include <stdio.h>
#include <sys/types.h>

static Uint16 format;
static Uint8 channels;
static int freq;

int fluidsynth_check_soundfont(const char *path, void *data)
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

int fluidsynth_load_soundfont(const char *path, void *data)
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

FluidSynthMidiSong *fluidsynth_loadsong_common(int (*function)(FluidSynthMidiSong*, void*), void *data)
{
	FluidSynthMidiSong *song;
	fluid_settings_t *settings = NULL;

	if ((song = malloc(sizeof(FluidSynthMidiSong)))) {
		memset(song, 0, sizeof(FluidSynthMidiSong));

		if (SDL_BuildAudioCVT(&song->convert, AUDIO_S16, 2, freq, format, channels, freq) >= 0) {
			if ((settings = fluidsynth.new_fluid_settings())) {
				fluidsynth.fluid_settings_setnum(settings, "synth.sample-rate", (double) freq);

				if ((song->synth = fluidsynth.new_fluid_synth(settings))) {
					if (Mix_EachSoundFont(fluidsynth_load_soundfont, (void*) song->synth)) {
						if ((song->player = fluidsynth.new_fluid_player(song->synth))) {
							if (function(song, data)) return song;
							fluidsynth.delete_fluid_player(song->player);
						} else {
							Mix_SetError("Failed to create FluidSynth player");
						}
					}
					fluidsynth.delete_fluid_synth(song->synth);
				} else {
					Mix_SetError("Failed to create FluidSynth synthesizer");
				}
				fluidsynth.delete_fluid_settings(settings);
			} else {
				Mix_SetError("Failed to create FluidSynth settings");
			}
		} else {
			Mix_SetError("Failed to set up audio conversion");
		}
		free(song);
	} else {
		Mix_SetError("Insufficient memory for song");
	}
	return NULL;
}

int fluidsynth_loadsong_internal(FluidSynthMidiSong *song, void *data)
{
	const char* path = (const char*) data;

	if (fluidsynth.fluid_player_add(song->player, path) == FLUID_OK) {
		return 1;
	} else {
		Mix_SetError("FluidSynth failed to load %s", path);
		return 0;
	}
}

int fluidsynth_loadsong_RW_internal(FluidSynthMidiSong *song, void *data)
{
	off_t offset;
	size_t size;
	char *buffer;
	SDL_RWops *rw = (SDL_RWops*) data;

	offset = SDL_RWtell(rw);
	SDL_RWseek(rw, 0, RW_SEEK_END);
	size = SDL_RWtell(rw) - offset;
	SDL_RWseek(rw, offset, RW_SEEK_SET);

	if ((buffer = (char*) malloc(size))) {
		if(SDL_RWread(rw, buffer, size, 1) == 1) {
			if (fluidsynth.fluid_player_add_mem(song->player, buffer, size) == FLUID_OK) {
				return 1;
			} else {
				Mix_SetError("FluidSynth failed to load in-memory song");
			}
		} else {
			Mix_SetError("Failed to read in-memory song");
		}
		free(buffer);
	} else {
		Mix_SetError("Insufficient memory for song");
	}
	return 0;
}

FluidSynthMidiSong *fluidsynth_loadsong(const char *midifile)
{
	return fluidsynth_loadsong_common(fluidsynth_loadsong_internal, (void*) midifile);
}

FluidSynthMidiSong *fluidsynth_loadsong_RW(SDL_RWops *rw)
{
	return fluidsynth_loadsong_common(fluidsynth_loadsong_RW_internal, (void*) rw);
}

void fluidsynth_freesong(FluidSynthMidiSong *song)
{
	if (!song) return;
	fluidsynth.delete_fluid_player(song->player);
	fluidsynth.delete_fluid_settings(fluidsynth.fluid_synth_get_settings(song->synth));
	fluidsynth.delete_fluid_synth(song->synth);
	free(song);
}

void fluidsynth_start(FluidSynthMidiSong *song)
{
	fluidsynth.fluid_player_set_loop(song->player, 1);
	fluidsynth.fluid_player_play(song->player);
}

void fluidsynth_stop(FluidSynthMidiSong *song)
{
	fluidsynth.fluid_player_stop(song->player);
}

int fluidsynth_active(FluidSynthMidiSong *song)
{
	return fluidsynth.fluid_player_get_status(song->player) == FLUID_PLAYER_PLAYING ? 1 : 0;
}

void fluidsynth_setvolume(FluidSynthMidiSong *song, int volume)
{
	/* FluidSynth's default is 0.2. Make 0.8 the maximum. */
	fluidsynth.fluid_synth_set_gain(song->synth, (float) (volume * 0.00625));
}

int fluidsynth_playsome(FluidSynthMidiSong *song, void *dest, int dest_len)
{
	int result = -1;
	int frames = dest_len / channels / ((format & 0xFF) / 8);
	int src_len = frames * 4; /* 16-bit stereo */
	void *src = dest;

	if (dest_len < src_len) {
		if (!(src = malloc(src_len))) {
			Mix_SetError("Insufficient memory for audio conversion");
			return result;
		}
	}

	if (fluidsynth.fluid_synth_write_s16(song->synth, frames, src, 0, 2, src, 1, 2) != FLUID_OK) {
		Mix_SetError("Error generating FluidSynth audio");
		goto finish;
	}

	song->convert.buf = src;
	song->convert.len = src_len;

	if (SDL_ConvertAudio(&song->convert) < 0) {
		Mix_SetError("Error during audio conversion");
		goto finish;
	}

	if (src != dest)
		memcpy(dest, src, dest_len);

	result = 0;

finish:
	if (src != dest)
		free(src);

	return result;
}
