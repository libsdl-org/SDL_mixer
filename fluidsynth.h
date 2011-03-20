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

#ifndef _FLUIDSYNTH_H_
#define _FLUIDSYNTH_H_

#include "dynamic_fluidsynth.h"
#include <SDL_rwops.h>
#include <SDL_audio.h>

typedef struct {
	SDL_AudioCVT convert;
	fluid_synth_t *synth;
	fluid_player_t* player;
} FluidSynthMidiSong;

int fluidsynth_init(SDL_AudioSpec *mixer);
FluidSynthMidiSong *fluidsynth_loadsong(const char *midifile);
FluidSynthMidiSong *fluidsynth_loadsong_RW(SDL_RWops *rw);
void fluidsynth_freesong(FluidSynthMidiSong *song);
void fluidsynth_start(FluidSynthMidiSong *song);
void fluidsynth_stop(FluidSynthMidiSong *song);
int fluidsynth_active(FluidSynthMidiSong *song);
void fluidsynth_setvolume(FluidSynthMidiSong *song, int volume);
int fluidsynth_playsome(FluidSynthMidiSong *song, void *stream, int len);

#endif /* _FLUIDSYNTH_H_ */
