/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2024 Sam Lantinga <slouken@libsdl.org>

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

#include <SDL3/SDL.h>

#include "music_fluidsynth.h"

#include <fluidsynth.h>


typedef struct {
    int loaded;
    void *handle;

#if (FLUIDSYNTH_VERSION_MAJOR >= 2)
    void (*delete_fluid_player)(fluid_player_t*);
    void (*delete_fluid_synth)(fluid_synth_t*);
    int (*fluid_player_seek)(fluid_player_t *, int);
#else
    int (*delete_fluid_player)(fluid_player_t*);
    int (*delete_fluid_synth)(fluid_synth_t*);
#endif
    void (*delete_fluid_settings)(fluid_settings_t*);
    int (*fluid_player_add)(fluid_player_t*, const char*);
    int (*fluid_player_add_mem)(fluid_player_t*, const void*, size_t);
    int (*fluid_player_get_status)(fluid_player_t*);
    int (*fluid_player_play)(fluid_player_t*);
    int (*fluid_player_set_loop)(fluid_player_t*, int);
    int (*fluid_player_stop)(fluid_player_t*);
    int (*fluid_settings_setnum)(fluid_settings_t*, const char*, double);
    int (*fluid_settings_getnum)(fluid_settings_t*, const char*, double*);
    fluid_settings_t* (*fluid_synth_get_settings)(fluid_synth_t*);
    void (*fluid_synth_set_gain)(fluid_synth_t*, float);
    int (*fluid_synth_sfload)(fluid_synth_t*, const char*, int);
    int (*fluid_synth_write_s16)(fluid_synth_t*, int, void*, int, int, void*, int, int);
    int (*fluid_synth_write_float)(fluid_synth_t*, int, void*, int, int, void*, int, int);
    fluid_player_t* (*new_fluid_player)(fluid_synth_t*);
    fluid_settings_t* (*new_fluid_settings)(void);
    fluid_synth_t* (*new_fluid_synth)(fluid_settings_t*);
} fluidsynth_loader;

static fluidsynth_loader fluidsynth;

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
#if (FLUIDSYNTH_VERSION_MAJOR >= 2)
        FUNCTION_LOADER(delete_fluid_player, void (*)(fluid_player_t*))
        FUNCTION_LOADER(delete_fluid_synth, void (*)(fluid_synth_t*))
        FUNCTION_LOADER(fluid_player_seek, int (*)(fluid_player_t *, int))
#else
        FUNCTION_LOADER(delete_fluid_player, int (*)(fluid_player_t*))
        FUNCTION_LOADER(delete_fluid_synth, int (*)(fluid_synth_t*))
#endif
        FUNCTION_LOADER(delete_fluid_settings, void (*)(fluid_settings_t*))
        FUNCTION_LOADER(fluid_player_add, int (*)(fluid_player_t*, const char*))
        FUNCTION_LOADER(fluid_player_add_mem, int (*)(fluid_player_t*, const void*, size_t))
        FUNCTION_LOADER(fluid_player_get_status, int (*)(fluid_player_t*))
        FUNCTION_LOADER(fluid_player_play, int (*)(fluid_player_t*))
        FUNCTION_LOADER(fluid_player_set_loop, int (*)(fluid_player_t*, int))
        FUNCTION_LOADER(fluid_player_stop, int (*)(fluid_player_t*))
        FUNCTION_LOADER(fluid_settings_setnum, int (*)(fluid_settings_t*, const char*, double))
        FUNCTION_LOADER(fluid_settings_getnum, int (*)(fluid_settings_t*, const char*, double*))
        FUNCTION_LOADER(fluid_synth_get_settings, fluid_settings_t* (*)(fluid_synth_t*))
        FUNCTION_LOADER(fluid_synth_set_gain, void (*)(fluid_synth_t*, float))
        FUNCTION_LOADER(fluid_synth_sfload, int(*)(fluid_synth_t*, const char*, int))
        FUNCTION_LOADER(fluid_synth_write_s16, int(*)(fluid_synth_t*, int, void*, int, int, void*, int, int))
        FUNCTION_LOADER(fluid_synth_write_float, int(*)(fluid_synth_t*, int, void*, int, int, void*, int, int))
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
    fluid_synth_t *synth;
    fluid_settings_t *settings;
    fluid_player_t *player;
    int (*synth_write)(fluid_synth_t*, int, void*, int, int, void*, int, int);
    SDL_AudioStream *stream;
    void *buffer;
    int buffer_size;
    float volume;
    bool is_paused;
} FLUIDSYNTH_Music;

static void FLUIDSYNTH_Delete(void *context);

static bool SDLCALL fluidsynth_check_soundfont(const char *path, void *data)
{
    SDL_IOStream *io = SDL_IOFromFile(path, "rb");

    (void)data;
    if (io) {
        SDL_CloseIO(io);
        return true;
    } else {
        SDL_SetError("Failed to access the SoundFont %s", path);
        return false;
    }
}

static bool SDLCALL fluidsynth_load_soundfont(const char *path, void *data)
{
    /* If this fails, it's too late to try Timidity so pray that at least one works. */
    fluidsynth.fluid_synth_sfload((fluid_synth_t*) data, path, 1);
    return true;
}

static int FLUIDSYNTH_Open(const SDL_AudioSpec *spec)
{
    (void)spec;
    if (!Mix_EachSoundFont(fluidsynth_check_soundfont, NULL)) {
        return -1;
    }
    return 0;
}

static FLUIDSYNTH_Music *FLUIDSYNTH_LoadMusic(void *data)
{
    SDL_AudioSpec srcspec;
    SDL_IOStream *src = (SDL_IOStream *)data;
    FLUIDSYNTH_Music *music;
    double samplerate; /* as set by the lib. */
    const Uint8 channels = 2;
    int src_format = SDL_AUDIO_S16;
    void *io_mem;
    size_t io_size;
    int ret;

    if (!(music = SDL_calloc(1, sizeof(FLUIDSYNTH_Music)))) {
        return NULL;
    }

    music->volume = 1.0f;
    music->buffer_size = 4096/*music_spec.samples*/ * sizeof(Sint16) * channels;
    music->synth_write = fluidsynth.fluid_synth_write_s16;
    if (music_spec.format & 0x0020) { /* 32 bit. */
        src_format = SDL_AUDIO_F32;
        music->buffer_size <<= 1;
        music->synth_write = fluidsynth.fluid_synth_write_float;
    }

    if (!(music->buffer = SDL_malloc((size_t)music->buffer_size))) {
        goto fail;
    }

    if (!(music->settings = fluidsynth.new_fluid_settings())) {
        SDL_SetError("Failed to create FluidSynth settings");
        goto fail;
    }

    fluidsynth.fluid_settings_setnum(music->settings, "synth.sample-rate", (double) music_spec.freq);
    fluidsynth.fluid_settings_getnum(music->settings, "synth.sample-rate", &samplerate);

    if (!(music->synth = fluidsynth.new_fluid_synth(music->settings))) {
        SDL_SetError("Failed to create FluidSynth synthesizer");
        goto fail;
    }

    if (!Mix_EachSoundFont(fluidsynth_load_soundfont, music->synth)) {
        goto fail;
    }

    if (!(music->player = fluidsynth.new_fluid_player(music->synth))) {
        SDL_SetError("Failed to create FluidSynth player");
        goto fail;
    }

    io_mem = SDL_LoadFile_IO(src, &io_size, false);
    if (!io_mem) {
        goto fail;
    }

    ret = fluidsynth.fluid_player_add_mem(music->player, io_mem, io_size);
    SDL_free(io_mem);
    if (ret != FLUID_OK) {
        SDL_SetError("FluidSynth failed to load in-memory song");
        goto fail;
    }

    SDL_zero(srcspec);
    srcspec.format = src_format;
    srcspec.channels = channels;
    srcspec.freq = (int) samplerate;
    if (!(music->stream = SDL_CreateAudioStream(&srcspec, &music_spec))) {
        goto fail;
    }

    return music;
fail:
    FLUIDSYNTH_Delete(music);
    return NULL;
}

static void *FLUIDSYNTH_CreateFromIO(SDL_IOStream *src, bool closeio)
{
    FLUIDSYNTH_Music *music;

    music = FLUIDSYNTH_LoadMusic(src);
    if (music && closeio) {
        SDL_CloseIO(src);
    }
    return music;
}

static void FLUIDSYNTH_SetVolume(void *context, float volume)
{
    FLUIDSYNTH_Music *music = (FLUIDSYNTH_Music *)context;
    /* FluidSynth's default gain is 0.2. Make 1.0 the maximum gain value to avoid sound overload. */
    music->volume = volume;
    fluidsynth.fluid_synth_set_gain(music->synth, volume);
}

static float FLUIDSYNTH_GetVolume(void *context)
{
    FLUIDSYNTH_Music *music = (FLUIDSYNTH_Music *)context;
    return music->volume;
}

static int FLUIDSYNTH_Play(void *context, int play_count)
{
    FLUIDSYNTH_Music *music = (FLUIDSYNTH_Music *)context;
    fluidsynth.fluid_player_set_loop(music->player, play_count);
#if (FLUIDSYNTH_VERSION_MAJOR >= 2)
    fluidsynth.fluid_player_seek(music->player, 0);
#endif
    fluidsynth.fluid_player_play(music->player);
    music->is_paused = false;
    return 0;
}

static void FLUIDSYNTH_Resume(void *context)
{
    FLUIDSYNTH_Music *music = (FLUIDSYNTH_Music *)context;
    fluidsynth.fluid_player_play(music->player);
    music->is_paused = false;
}

static bool FLUIDSYNTH_IsPlaying(void *context)
{
    FLUIDSYNTH_Music *music = (FLUIDSYNTH_Music *)context;
    return music->is_paused || fluidsynth.fluid_player_get_status(music->player) == FLUID_PLAYER_PLAYING ? true : false;
}

static int FLUIDSYNTH_GetSome(void *context, void *data, int bytes, bool *done)
{
    FLUIDSYNTH_Music *music = (FLUIDSYNTH_Music *)context;
    int filled;

    (void)done;

    filled = SDL_GetAudioStreamData(music->stream, data, bytes);
    if (filled != 0) {
        return filled;
    }

    if (music->synth_write(music->synth, 4096/*music_spec.samples*/, music->buffer, 0, 2, music->buffer, 1, 2) != FLUID_OK) {
        SDL_SetError("Error generating FluidSynth audio");
        return -1;
    }
    if (!SDL_PutAudioStreamData(music->stream, music->buffer, music->buffer_size)) {
        return -1;
    }
    return 0;
}
static int FLUIDSYNTH_GetAudio(void *context, void *data, int bytes)
{
    return music_pcm_getaudio(context, data, bytes, 1.0f, FLUIDSYNTH_GetSome);
}

static void FLUIDSYNTH_Stop(void *context)
{
    FLUIDSYNTH_Music *music = (FLUIDSYNTH_Music *)context;
    fluidsynth.fluid_player_stop(music->player);
#if (FLUIDSYNTH_VERSION_MAJOR >= 2)
    fluidsynth.fluid_player_seek(music->player, 0);
#endif
    music->is_paused = false;
}

static void FLUIDSYNTH_Pause(void *context)
{
    FLUIDSYNTH_Music *music = (FLUIDSYNTH_Music *)context;
    fluidsynth.fluid_player_stop(music->player);
    music->is_paused = true;
}

static void FLUIDSYNTH_Delete(void *context)
{
    FLUIDSYNTH_Music *music = (FLUIDSYNTH_Music *)context;

    if (music->player) {
        fluidsynth.delete_fluid_player(music->player);
    }
    if (music->synth) {
        fluidsynth.delete_fluid_synth(music->synth);
    }
    if (music->settings) {
        fluidsynth.delete_fluid_settings(music->settings);
    }
    if (music->stream) {
        SDL_DestroyAudioStream(music->stream);
    }
    if (music->buffer) {
        SDL_free(music->buffer);
    }
    SDL_free(music);
}

Mix_MusicInterface Mix_MusicInterface_FLUIDSYNTH =
{
    "FLUIDSYNTH",
    MIX_MUSIC_FLUIDSYNTH,
    MUS_MID,
    false,
    false,

    FLUIDSYNTH_Load,
    FLUIDSYNTH_Open,
    FLUIDSYNTH_CreateFromIO,
    NULL,   /* CreateFromFile */
    FLUIDSYNTH_SetVolume,
    FLUIDSYNTH_GetVolume,
    FLUIDSYNTH_Play,
    FLUIDSYNTH_IsPlaying,
    FLUIDSYNTH_GetAudio,
    NULL,   /* Jump */
    NULL,   /* Seek */
    NULL,   /* Tell */
    NULL,   /* Duration */
    NULL,   /* LoopStart */
    NULL,   /* LoopEnd */
    NULL,   /* LoopLength */
    NULL,   /* GetMetaTag */
    NULL,   /* GetNumTracks */
    NULL,   /* StartTrack */
    FLUIDSYNTH_Pause,
    FLUIDSYNTH_Resume,
    FLUIDSYNTH_Stop,
    FLUIDSYNTH_Delete,
    NULL,   /* Close */
    FLUIDSYNTH_Unload
};

#endif /* MUSIC_MID_FLUIDSYNTH */

/* vi: set ts=4 sw=4 expandtab: */
