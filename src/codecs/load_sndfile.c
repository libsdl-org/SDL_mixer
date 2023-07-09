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

  This is the source needed to decode a file in any format supported by
  libsndfile. The only externally-callable function is Mix_LoadSndFile_RW(),
  which is meant to act as identically to SDL_LoadWAV_RW() as possible.

  This file by Fabian Greffrath (fabian@greffrath.com).
*/

#include <SDL3/SDL_audio.h>

#include "load_sndfile.h"

#ifdef LOAD_SNDFILE

#include <SDL3/SDL_loadso.h>

#include <sndfile.h>

static SNDFILE* (*SF_sf_open_virtual) (SF_VIRTUAL_IO *sfvirtual, int mode, SF_INFO *sfinfo, void *user_data);
static int (*SF_sf_close) (SNDFILE *sndfile);
static sf_count_t (*SF_sf_readf_short) (SNDFILE *sndfile, short *ptr, sf_count_t frames);
static const char* (*SF_sf_strerror) (SNDFILE *sndfile);

static int SNDFILE_loaded;
static void *SNDFILE_lib;

static int SNDFILE_init (void)
{
    if (SNDFILE_loaded == 0) {
#ifdef SNDFILE_DYNAMIC
        SNDFILE_lib = SDL_LoadObject(SNDFILE_DYNAMIC);
        if (SNDFILE_lib == NULL) {
            return -1;
        }

        /* *INDENT-OFF* */ /* clang-format off */
        SF_sf_open_virtual = (SNDFILE* (*)(SF_VIRTUAL_IO *sfvirtual, int mode, SF_INFO *sfinfo, void *user_data))SDL_LoadFunction(SNDFILE_lib, "sf_open_virtual");
        SF_sf_close = (int (*)(SNDFILE *sndfile))SDL_LoadFunction(SNDFILE_lib, "sf_close");
        SF_sf_readf_short = (sf_count_t(*)(SNDFILE *sndfile, short *ptr, sf_count_t frames))SDL_LoadFunction(SNDFILE_lib, "sf_readf_short");
        SF_sf_strerror = (const char* (*)(SNDFILE *sndfile))SDL_LoadFunction(SNDFILE_lib, "sf_strerror");
        /* *INDENT-ON* */ /* clang-format on */

        if (SF_sf_open_virtual == NULL || SF_sf_close == NULL ||
            SF_sf_readf_short == NULL || SF_sf_strerror == NULL) {
            SDL_UnloadObject(SNDFILE_lib);
            SNDFILE_lib = NULL;
            return -1;
        }
#else
        SF_sf_open_virtual = sf_open_virtual;
        SF_sf_close = sf_close;
        SF_sf_readf_short = sf_readf_short;
        SF_sf_strerror = sf_strerror;
#endif

        SNDFILE_loaded = 1;
    }

    return 0;
}

void SNDFILE_uninit (void)
{
    if (SNDFILE_lib != NULL) {
        SDL_UnloadObject(SNDFILE_lib);
        SNDFILE_lib = NULL;

        SF_sf_open_virtual = NULL;
        SF_sf_close = NULL;
        SF_sf_readf_short = NULL;
        SF_sf_strerror = NULL;

        SNDFILE_loaded = 0;
    }
}

static sf_count_t sfvio_size(void *user_data)
{
    SDL_RWops *RWops = user_data;
    return SDL_RWsize(RWops);
}

static sf_count_t sfvio_seek(sf_count_t offset, int whence, void *user_data)
{
    SDL_RWops *RWops = user_data;
    return SDL_RWseek(RWops, offset, whence);
}

static sf_count_t sfvio_read(void *ptr, sf_count_t count, void *user_data)
{
    SDL_RWops *RWops = user_data;
    return SDL_RWread(RWops, ptr, count);
}

static sf_count_t sfvio_tell(void *user_data)
{
    SDL_RWops *RWops = user_data;
    return SDL_RWtell(RWops);
}

SDL_AudioSpec *Mix_LoadSndFile_RW (SDL_RWops *src, SDL_bool freesrc,
        SDL_AudioSpec *spec, Uint8 **audio_buf, Uint32 *audio_len)
{
    SDL_bool was_error = SDL_TRUE;
    SNDFILE *sndfile = NULL;
    SF_INFO sfinfo;
    SF_VIRTUAL_IO sfvio = {
        sfvio_size,
        sfvio_seek,
        sfvio_read,
        NULL,
        sfvio_tell
    };
    Uint32 len;
    short *buf = NULL;

    /* Sanity checks */
    if (audio_buf) {
        *audio_buf = NULL;
    }
    if (!src) {
        SDL_InvalidParamError("src");
        goto done;
    }
    if (!spec) {
        SDL_InvalidParamError("spec");
        goto done;
    }
    if (!audio_buf) {
        SDL_InvalidParamError("audio_buf");
        goto done;
    }
    if (!audio_len) {
        SDL_InvalidParamError("audio_len");
        goto done;
    }

    if (SNDFILE_loaded == 0) {
        if (SNDFILE_init() != 0) {
            goto done;
        }
    }

    SDL_memset(&sfinfo, 0, sizeof(sfinfo));

    sndfile = SF_sf_open_virtual(&sfvio, SFM_READ, &sfinfo, src);

    if (sndfile == NULL) {
        Mix_SetError("sf_open_virtual: %s", SF_sf_strerror(sndfile));
        goto done;
    }

    if (sfinfo.frames <= 0) {
        Mix_SetError("Invalid number of frames: %ld", (long)sfinfo.frames);
        goto done;
    }

    if (sfinfo.channels <= 0) {
        Mix_SetError("Invalid number of channels: %d", sfinfo.channels);
        goto done;
    }

    len = sfinfo.frames * sfinfo.channels * sizeof(short);
    buf = SDL_malloc(len);

    if (buf == NULL) {
        Mix_OutOfMemory();
        goto done;
    }

    if (SF_sf_readf_short(sndfile, buf, sfinfo.frames) < sfinfo.frames) {
        SDL_free(buf);
        Mix_SetError("sf_readf_short: %s", SF_sf_strerror(sndfile));
        goto done;
    }

    SDL_zerop(spec);
    spec->channels = sfinfo.channels;
    spec->freq = sfinfo.samplerate;
    spec->format = SDL_AUDIO_S16;

    *audio_buf = (Uint8 *)buf;
    *audio_len = len;

    was_error = SDL_FALSE;

done:
    if (sndfile) {
        SF_sf_close(sndfile);
    }
    if (freesrc && src) {
        SDL_RWclose(src);
    }
    if (was_error) {
        if (audio_buf && *audio_buf) {
            SDL_free(*audio_buf);
            *audio_buf = NULL;
        }
        if (audio_len) {
            *audio_len = 0;
        }
        spec = NULL;
    }
    return spec;
}

#else

SDL_AudioSpec *Mix_LoadSndFile_RW (SDL_RWops *src, SDL_bool freesrc,
        SDL_AudioSpec *spec, Uint8 **audio_buf, Uint32 *audio_len)
{
    (void) src;
    (void) freesrc;
    (void) spec;
    (void) audio_buf;
    (void) audio_len;
    return NULL;
}

void SNDFILE_uninit (void)
{
    /* no-op */
}

#endif

/* vi: set ts=4 sw=4 expandtab: */
