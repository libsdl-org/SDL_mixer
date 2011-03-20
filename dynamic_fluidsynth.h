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

#ifdef USE_FLUIDSYNTH_MIDI

#include <fluidsynth.h>

typedef struct {
	int loaded;
	void *handle;

	int (*delete_fluid_player)(fluid_player_t*);
	void (*delete_fluid_settings)(fluid_settings_t*);
	int (*delete_fluid_synth)(fluid_synth_t*);
	int (*fluid_player_add)(fluid_player_t*, const char*);
	int (*fluid_player_add_mem)(fluid_player_t*, const void*, size_t);
	int (*fluid_player_get_status)(fluid_player_t*);
	int (*fluid_player_play)(fluid_player_t*);
	int (*fluid_player_set_loop)(fluid_player_t*, int);
	int (*fluid_player_stop)(fluid_player_t*);
	int (*fluid_settings_setnum)(fluid_settings_t*, const char*, double);
	fluid_settings_t* (*fluid_synth_get_settings)(fluid_synth_t*);
	void (*fluid_synth_set_gain)(fluid_synth_t*, float);
	int (*fluid_synth_sfload)(fluid_synth_t*, const char*, int);
	int (*fluid_synth_write_s16)(fluid_synth_t*, int, void*, int, int, void*, int, int);
	fluid_player_t* (*new_fluid_player)(fluid_synth_t*);
	fluid_settings_t* (*new_fluid_settings)(void);
	fluid_synth_t* (*new_fluid_synth)(fluid_settings_t*);
} fluidsynth_loader;

extern fluidsynth_loader fluidsynth;

#endif /* USE_FLUIDSYNTH_MIDI */

extern int Mix_InitFluidSynth();
extern void Mix_QuitFluidSynth();
