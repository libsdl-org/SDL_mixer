/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2017 Sam Lantinga <slouken@libsdl.org>

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

#ifdef MUSIC_MID_FLUIDSYNTH

#include <stdio.h>

#include "SDL_loadso.h"

#include "music_fluidsynth.h"

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

static fluidsynth_loader fluidsynth = {
    0, NULL
};

#ifdef FLUIDSYNTH_DYNAMIC
#define FUNCTION_LOADER(FUNC, SIG) \
    fluidsynth.FUNC = (SIG) SDL_LoadFunction(fluidsynth.handle, #FUNC); \
    if (fluidsynth.FUNC == NULL) { SDL_UnloadObject(fluidsynth.handle); return -1; }
#else
#define FUNCTION_LOADER(FUNC, SIG) \
    fluidsynth.FUNC = FUNC;
#endif

static int FLUIDSYNTH_Load()
{
    if (fluidsynth.loaded == 0) {
#ifdef FLUIDSYNTH_DYNAMIC
        fluidsynth.handle = SDL_LoadObject(FLUIDSYNTH_DYNAMIC);
        if (fluidsynth.handle == NULL) {
            return -1;
        }
#endif

        FUNCTION_LOADER(delete_fluid_player, int (*)(fluid_player_t*))
        FUNCTION_LOADER(delete_fluid_settings, void (*)(fluid_settings_t*))
        FUNCTION_LOADER(delete_fluid_synth, int (*)(fluid_synth_t*))
        FUNCTION_LOADER(fluid_player_add, int (*)(fluid_player_t*, const char*))
        FUNCTION_LOADER(fluid_player_add_mem, int (*)(fluid_player_t*, const void*, size_t))
        FUNCTION_LOADER(fluid_player_get_status, int (*)(fluid_player_t*))
        FUNCTION_LOADER(fluid_player_play, int (*)(fluid_player_t*))
        FUNCTION_LOADER(fluid_player_set_loop, int (*)(fluid_player_t*, int))
        FUNCTION_LOADER(fluid_player_stop, int (*)(fluid_player_t*))
        FUNCTION_LOADER(fluid_settings_setnum, int (*)(fluid_settings_t*, const char*, double))
        FUNCTION_LOADER(fluid_synth_get_settings, fluid_settings_t* (*)(fluid_synth_t*))
        FUNCTION_LOADER(fluid_synth_set_gain, void (*)(fluid_synth_t*, float))
        FUNCTION_LOADER(fluid_synth_sfload, int(*)(fluid_synth_t*, const char*, int))
        FUNCTION_LOADER(fluid_synth_write_s16, int(*)(fluid_synth_t*, int, void*, int, int, void*, int, int))
        FUNCTION_LOADER(new_fluid_player, fluid_player_t* (*)(fluid_synth_t*))
        FUNCTION_LOADER(new_fluid_settings, fluid_settings_t* (*)(void))
        FUNCTION_LOADER(new_fluid_synth, fluid_synth_t* (*)(fluid_settings_t*))
    }
    ++fluidsynth.loaded;

    return 0;
}

static void FLUIDSYNTH_Unload()
{
    if (fluidsynth.loaded == 0) {
        return;
    }
    if (fluidsynth.loaded == 1) {
#ifdef FLUIDSYNTH_DYNAMIC
        SDL_UnloadObject(fluidsynth.handle);
#endif
    }
    --fluidsynth.loaded;
}


typedef struct {
    SDL_AudioCVT convert;
    fluid_synth_t *synth;
    fluid_player_t* player;
} FluidSynthMidiSong;

static Uint16 format;
static Uint8 channels;
static int freq;

static int fluidsynth_check_soundfont(const char *path, void *data)
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

static int fluidsynth_load_soundfont(const char *path, void *data)
{
    /* If this fails, it's too late to try Timidity so pray that at least one works. */
    fluidsynth.fluid_synth_sfload((fluid_synth_t*) data, path, 1);
    return 1;
}

static int FLUIDSYNTH_Open(const SDL_AudioSpec *spec)
{
    if (!Mix_EachSoundFont(fluidsynth_check_soundfont, NULL)) {
        return -1;
    }

    format = spec->format;
    channels = spec->channels;
    freq = spec->freq;

    return 0;
}

static FluidSynthMidiSong *fluidsynth_loadsong_common(int (*function)(FluidSynthMidiSong*, void*), void *data)
{
    FluidSynthMidiSong *song;
    fluid_settings_t *settings = NULL;

    if ((song = SDL_calloc(1, sizeof(FluidSynthMidiSong)))) {
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
        SDL_free(song);
    } else {
        Mix_SetError("Insufficient memory for song");
    }
    return NULL;
}

static int fluidsynth_loadsong_RW_internal(FluidSynthMidiSong *song, void *data)
{
    Sint64 offset;
    size_t size;
    char *buffer;
    SDL_RWops *src = (SDL_RWops*) data;

    offset = SDL_RWtell(src);
    SDL_RWseek(src, 0, RW_SEEK_END);
    size = (size_t)(SDL_RWtell(src) - offset);
    SDL_RWseek(src, offset, RW_SEEK_SET);

    if ((buffer = (char*) SDL_malloc(size))) {
        if(SDL_RWread(src, buffer, size, 1) == 1) {
            if (fluidsynth.fluid_player_add_mem(song->player, buffer, size) == FLUID_OK) {
                SDL_free(buffer);
                return 1;
            } else {
                Mix_SetError("FluidSynth failed to load in-memory song");
            }
        } else {
            Mix_SetError("Failed to read in-memory song");
        }
        SDL_free(buffer);
    } else {
        Mix_SetError("Insufficient memory for song");
    }
    return 0;
}

static void *FLUIDSYNTH_CreateFromRW(SDL_RWops *src, int freesrc)
{
    FluidSynthMidiSong *song;

    song = fluidsynth_loadsong_common(fluidsynth_loadsong_RW_internal, (void*) src);
    if (song && freesrc) {
        SDL_RWclose(src);
    }
    return song;
}

static void FLUIDSYNTH_SetVolume(void *context, int volume)
{
    FluidSynthMidiSong *song = (FluidSynthMidiSong *)context;
    /* FluidSynth's default is 0.2. Make 1.2 the maximum. */
    fluidsynth.fluid_synth_set_gain(song->synth, (float) (volume * 1.2 / MIX_MAX_VOLUME));
}

static int FLUIDSYNTH_Play(void *context)
{
    FluidSynthMidiSong *song = (FluidSynthMidiSong *)context;
    fluidsynth.fluid_player_set_loop(song->player, 1);
    fluidsynth.fluid_player_play(song->player);
    return 0;
}

static SDL_bool FLUIDSYNTH_IsPlaying(void *context)
{
    FluidSynthMidiSong *song = (FluidSynthMidiSong *)context;
    return fluidsynth.fluid_player_get_status(song->player) == FLUID_PLAYER_PLAYING ? SDL_TRUE : SDL_FALSE;
}

static int FLUIDSYNTH_GetAudio(void *context, void *data, int bytes)
{
    int result = -1;
    int frames = bytes / channels / ((format & 0xFF) / 8);
    int src_len = frames * 4; /* 16-bit stereo */
    void *src = dest;

    if (bytes < src_len) {
        if (!(src = SDL_malloc(src_len))) {
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
        SDL_memcpy(dest, src, bytes);

    result = 0;

finish:
    if (src != dest)
        SDL_free(src);

    return result;
}

static void FLUIDSYNTH_Stop(void *context)
{
    FluidSynthMidiSong *song = (FluidSynthMidiSong *)context;
    fluidsynth.fluid_player_stop(song->player);
}

static void FLUIDSYNTH_Delete(void *context)
{
    FluidSynthMidiSong *song = (FluidSynthMidiSong *)context;
    fluidsynth.delete_fluid_player(song->player);
    fluidsynth.delete_fluid_settings(fluidsynth.fluid_synth_get_settings(song->synth));
    fluidsynth.delete_fluid_synth(song->synth);
    SDL_free(song);
}

Mix_MusicInterface Mix_MusicInterface_FLUIDSYNTH =
{
    "FLUIDSYNTH",
    MIX_MUSIC_FLUIDSYNTH,
    MUS_MID,
    SDL_FALSE,
    SDL_FALSE,

    FLUIDSYNTH_Load,
    FLUIDSYNTH_Open,
    FLUIDSYNTH_CreateFromRW,
    NULL,   /* CreateFromFile */
    FLUIDSYNTH_SetVolume,
    FLUIDSYNTH_Play,
    FLUIDSYNTH_IsPlaying,
    FLUIDSYNTH_GetAudio,
    NULL,   /* Seek */
    NULL,   /* Pause */
    NULL,   /* Resume */
    FLUIDSYNTH_Stop,
    FLUIDSYNTH_Delete,
    NULL,   /* Close */
    FLUIDSYNTH_Unload,
};

#endif /* MUSIC_MID_FLUIDSYNTH */

/* vi: set ts=4 sw=4 expandtab: */
