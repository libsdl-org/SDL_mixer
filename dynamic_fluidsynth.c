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

#include "SDL_loadso.h"
#include "dynamic_fluidsynth.h"

fluidsynth_loader fluidsynth = {
	0, NULL
};

#ifdef FLUIDSYNTH_DYNAMIC
#define FLUIDSYNTH_LOADER(FUNC, SIG) \
	fluidsynth.FUNC = (SIG) SDL_LoadFunction(fluidsynth.handle, #FUNC); \
	if (fluidsynth.FUNC == NULL) { SDL_UnloadObject(fluidsynth.handle); return -1; }
#else
#define FLUIDSYNTH_LOADER(FUNC, SIG) \
	fluidsynth.FUNC = FUNC;
#endif

int Mix_InitFluidSynth()
{
	if ( fluidsynth.loaded == 0 ) {
#ifdef FLUIDSYNTH_DYNAMIC
		fluidsynth.handle = SDL_LoadObject(FLUIDSYNTH_DYNAMIC);
		if ( fluidsynth.handle == NULL ) return -1;
#endif

		FLUIDSYNTH_LOADER(delete_fluid_player, int (*)(fluid_player_t*));
		FLUIDSYNTH_LOADER(delete_fluid_settings, void (*)(fluid_settings_t*));
		FLUIDSYNTH_LOADER(delete_fluid_synth, int (*)(fluid_synth_t*));
		FLUIDSYNTH_LOADER(fluid_player_add, int (*)(fluid_player_t*, const char*));
		FLUIDSYNTH_LOADER(fluid_player_add_mem, int (*)(fluid_player_t*, const void*, size_t));
		FLUIDSYNTH_LOADER(fluid_player_get_status, int (*)(fluid_player_t*));
		FLUIDSYNTH_LOADER(fluid_player_play, int (*)(fluid_player_t*));
		FLUIDSYNTH_LOADER(fluid_player_set_loop, int (*)(fluid_player_t*, int));
		FLUIDSYNTH_LOADER(fluid_player_stop, int (*)(fluid_player_t*));
		FLUIDSYNTH_LOADER(fluid_settings_setnum, int (*)(fluid_settings_t*, const char*, double));
		FLUIDSYNTH_LOADER(fluid_synth_get_settings, fluid_settings_t* (*)(fluid_synth_t*));
		FLUIDSYNTH_LOADER(fluid_synth_set_gain, void (*)(fluid_synth_t*, float));
		FLUIDSYNTH_LOADER(fluid_synth_sfload, int(*)(fluid_synth_t*, const char*, int));
		FLUIDSYNTH_LOADER(fluid_synth_write_s16, int(*)(fluid_synth_t*, int, void*, int, int, void*, int, int));
		FLUIDSYNTH_LOADER(new_fluid_player, fluid_player_t* (*)(fluid_synth_t*));
		FLUIDSYNTH_LOADER(new_fluid_settings, fluid_settings_t* (*)(void));
		FLUIDSYNTH_LOADER(new_fluid_synth, fluid_synth_t* (*)(fluid_settings_t*));
	}
	++fluidsynth.loaded;

	return 0;
}

void Mix_QuitFluidSynth()
{
	if ( fluidsynth.loaded == 0 ) {
		return;
	}
	if ( fluidsynth.loaded == 1 ) {
#ifdef FLUIDSYNTH_DYNAMIC
		SDL_UnloadObject(fluidsynth.handle);
#endif
	}
	--fluidsynth.loaded;
}

#endif /* USE_FLUIDSYNTH_MIDI */
