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
*/

#ifdef MUSIC_OGG

/* This file supports Ogg Vorbis music streams */

#include "SDL_loadso.h"

#include "music_ogg.h"

#if defined(OGG_HEADER)
#include OGG_HEADER
#elif defined(OGG_USE_TREMOR)
#include <tremor/ivorbisfile.h>
#else
#include <vorbis/vorbisfile.h>
#endif

typedef struct {
    int loaded;
    void *handle;
    int (*ov_clear)(OggVorbis_File *vf);
    vorbis_info *(*ov_info)(OggVorbis_File *vf,int link);
    int (*ov_open_callbacks)(void *datasource, OggVorbis_File *vf, const char *initial, long ibytes, ov_callbacks callbacks);
    ogg_int64_t (*ov_pcm_total)(OggVorbis_File *vf,int i);
#ifdef OGG_USE_TREMOR
    long (*ov_read)(OggVorbis_File *vf,char *buffer,int length, int *bitstream);
#else
    long (*ov_read)(OggVorbis_File *vf,char *buffer,int length, int bigendianp,int word,int sgned,int *bitstream);
#endif
#ifdef OGG_USE_TREMOR
    int (*ov_time_seek)(OggVorbis_File *vf,ogg_int64_t pos);
#else
    int (*ov_time_seek)(OggVorbis_File *vf,double pos);
#endif
} vorbis_loader;

static vorbis_loader vorbis = {
    0, NULL
};

#ifdef OGG_DYNAMIC

static int OGG_Load(void)
{
    if (vorbis.loaded == 0) {
        vorbis.handle = SDL_LoadObject(OGG_DYNAMIC);
        if (vorbis.handle == NULL) {
            return -1;
        }
        vorbis.ov_clear =
            (int (*)(OggVorbis_File *))
            SDL_LoadFunction(vorbis.handle, "ov_clear");
        if (vorbis.ov_clear == NULL) {
            SDL_UnloadObject(vorbis.handle);
            return -1;
        }
        vorbis.ov_info =
            (vorbis_info *(*)(OggVorbis_File *,int))
            SDL_LoadFunction(vorbis.handle, "ov_info");
        if (vorbis.ov_info == NULL) {
            SDL_UnloadObject(vorbis.handle);
            return -1;
        }
        vorbis.ov_open_callbacks =
            (int (*)(void *, OggVorbis_File *, const char *, long, ov_callbacks))
            SDL_LoadFunction(vorbis.handle, "ov_open_callbacks");
        if (vorbis.ov_open_callbacks == NULL) {
            SDL_UnloadObject(vorbis.handle);
            return -1;
        }
        vorbis.ov_pcm_total =
            (ogg_int64_t (*)(OggVorbis_File *,int))
            SDL_LoadFunction(vorbis.handle, "ov_pcm_total");
        if (vorbis.ov_pcm_total == NULL) {
            SDL_UnloadObject(vorbis.handle);
            return -1;
        }
        vorbis.ov_read =
#ifdef OGG_USE_TREMOR
            (long (*)(OggVorbis_File *,char *,int,int *))
#else
            (long (*)(OggVorbis_File *,char *,int,int,int,int,int *))
#endif
            SDL_LoadFunction(vorbis.handle, "ov_read");
        if (vorbis.ov_read == NULL) {
            SDL_UnloadObject(vorbis.handle);
            return -1;
        }
        vorbis.ov_time_seek =
#ifdef OGG_USE_TREMOR
            (long (*)(OggVorbis_File *,ogg_int64_t))
#else
            (int (*)(OggVorbis_File *,double))
#endif
            SDL_LoadFunction(vorbis.handle, "ov_time_seek");
        if (vorbis.ov_time_seek == NULL) {
            SDL_UnloadObject(vorbis.handle);
            return -1;
        }
    }
    ++vorbis.loaded;

    return 0;
}

static void OGG_Unload(void)
{
    if (vorbis.loaded == 0) {
        return;
    }
    if (vorbis.loaded == 1) {
        SDL_UnloadObject(vorbis.handle);
    }
    --vorbis.loaded;
}

#else /* !OGG_DYNAMIC */

static int OGG_Load(void)
{
    if (vorbis.loaded == 0) {
#ifdef __MACOSX__
        extern int ov_open_callbacks(void*, OggVorbis_File*, const char*, long, ov_callbacks) __attribute__((weak_import));
        if (ov_open_callbacks == NULL)
        {
            /* Missing weakly linked framework */
            Mix_SetError("Missing Vorbis.framework");
            return -1;
        }
#endif // __MACOSX__

        vorbis.ov_clear = ov_clear;
        vorbis.ov_info = ov_info;
        vorbis.ov_open_callbacks = ov_open_callbacks;
        vorbis.ov_pcm_total = ov_pcm_total;
        vorbis.ov_read = ov_read;
        vorbis.ov_time_seek = ov_time_seek;
    }
    ++vorbis.loaded;

    return 0;
}

static void OGG_Unload(void)
{
    if (vorbis.loaded == 0) {
        return;
    }
    if (vorbis.loaded == 1) {
    }
    --vorbis.loaded;
}

#endif /* OGG_DYNAMIC */


typedef struct {
    SDL_RWops *src;
    int freesrc;
    SDL_bool playing;
    int volume;
    OggVorbis_File vf;
    int section;
    SDL_AudioCVT cvt;
    int len_available;
    Uint8 *snd_available;
} OGG_music;

static size_t sdl_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
    return SDL_RWread((SDL_RWops*)datasource, ptr, size, nmemb);
}

static int sdl_seek_func(void *datasource, ogg_int64_t offset, int whence)
{
    return (int)SDL_RWseek((SDL_RWops*)datasource, offset, whence);
}

static long sdl_tell_func(void *datasource)
{
    return (long)SDL_RWtell((SDL_RWops*)datasource);
}

/* Load an OGG stream from an SDL_RWops object */
static void *OGG_CreateFromRW(SDL_RWops *src, int freesrc)
{
    OGG_music *music;
    ov_callbacks callbacks;

    SDL_memset(&callbacks, 0, sizeof(callbacks));
    callbacks.read_func = sdl_read_func;
    callbacks.seek_func = sdl_seek_func;
    callbacks.tell_func = sdl_tell_func;

    music = (OGG_music *)SDL_calloc(1, sizeof *music);
    if (music) {
        /* Initialize the music structure */
        music->src = src;
        music->freesrc = freesrc;
        music->volume = MIX_MAX_VOLUME;
        music->section = -1;

        if (vorbis.ov_open_callbacks(src, &music->vf, NULL, 0, callbacks) < 0) {
            SDL_SetError("Not an Ogg Vorbis audio stream");
            SDL_free(music);
            return NULL;
        }
    } else {
        SDL_OutOfMemory();
    }
    return music;
}

/* Set the volume for an OGG stream */
static void OGG_SetVolume(void *context, int volume)
{
    OGG_music *music = (OGG_music *)context;
    music->volume = volume;
}

/* Start playback of a given OGG stream */
static int OGG_Play(void *context)
{
    OGG_music *music = (OGG_music *)context;
    music->playing = SDL_TRUE;
    return 0;
}

/* Return non-zero if a stream is currently playing */
static SDL_bool OGG_IsPlaying(void *context)
{
    OGG_music *music = (OGG_music *)context;
    return music->playing;
}

/* Read some Ogg stream data and convert it for output */
static void OGG_getsome(OGG_music *music)
{
    int section;
    int len;
    char data[4096];
    SDL_AudioCVT *cvt;

#ifdef OGG_USE_TREMOR
    len = vorbis.ov_read(&music->vf, data, sizeof(data), &section);
#else
    len = vorbis.ov_read(&music->vf, data, sizeof(data), 0, 2, 1, &section);
#endif
    if (len <= 0) {
        if (len == 0) {
            music->playing = SDL_FALSE;
        }
        return;
    }
    cvt = &music->cvt;
    if (section != music->section) {
        vorbis_info *vi;

        vi = vorbis.ov_info(&music->vf, -1);
        SDL_BuildAudioCVT(cvt, AUDIO_S16, vi->channels, vi->rate,
                               music_spec.format, music_spec.channels, music_spec.freq);
        if (cvt->buf) {
            SDL_free(cvt->buf);
        }
        cvt->buf = (Uint8 *)SDL_malloc(sizeof(data)*cvt->len_mult);
        music->section = section;
    }
    if (cvt->buf) {
        SDL_memcpy(cvt->buf, data, len);
        if (cvt->needed) {
            cvt->len = len;
            SDL_ConvertAudio(cvt);
        } else {
            cvt->len_cvt = len;
        }
        music->len_available = music->cvt.len_cvt;
        music->snd_available = music->cvt.buf;
    } else {
        SDL_SetError("Out of memory");
        music->playing = SDL_FALSE;
    }
}

/* Play some of a stream previously started with OGG_play() */
static int OGG_GetAudio(void *context, void *data, int bytes)
{
    OGG_music *music = (OGG_music *)context;
    Uint8 *snd = (Uint8 *)data;
    int len = bytes;
    int mixable;

    while ((len > 0) && music->playing) {
        if (!music->len_available) {
            OGG_getsome(music);
        }
        mixable = len;
        if (mixable > music->len_available) {
            mixable = music->len_available;
        }
        if (music->volume == MIX_MAX_VOLUME) {
            SDL_memcpy(snd, music->snd_available, mixable);
        } else {
            SDL_MixAudioFormat(snd, music->snd_available, music_spec.format, mixable, music->volume);
        }
        music->len_available -= mixable;
        music->snd_available += mixable;
        len -= mixable;
        snd += mixable;
    }

    return len;
}

/* Jump (seek) to a given position (time is in seconds) */
static int OGG_Seek(void *context, double time)
{
    OGG_music *music = (OGG_music *)context;
#ifdef OGG_USE_TREMOR
    vorbis.ov_time_seek(&music->vf, (ogg_int64_t)(time * 1000.0));
#else
    vorbis.ov_time_seek(&music->vf, time);
#endif
    return 0;
}

/* Stop playback of a stream previously started with OGG_play() */
static void OGG_Stop(void *context)
{
    OGG_music *music = (OGG_music *)context;
    music->playing = SDL_FALSE;
}

/* Close the given OGG stream */
static void OGG_Delete(void *context)
{
    OGG_music *music = (OGG_music *)context;
    if (music) {
        if (music->cvt.buf) {
            SDL_free(music->cvt.buf);
        }
        if (music->freesrc) {
            SDL_RWclose(music->src);
        }
        vorbis.ov_clear(&music->vf);
        SDL_free(music);
    }
}

Mix_MusicInterface Mix_MusicInterface_OGG =
{
    "OGG",
    MIX_MUSIC_OGG,
    MUS_OGG,
    SDL_FALSE,
    SDL_FALSE,

    OGG_Load,
    NULL,   /* Open */
    OGG_CreateFromRW,
    NULL,   /* CreateFromFile */
    OGG_SetVolume,
    OGG_Play,
    OGG_IsPlaying,
    OGG_GetAudio,
    OGG_Seek,
    NULL,   /* Pause */
    NULL,   /* Resume */
    NULL,   /* Stop */
    OGG_Delete,
    NULL,   /* Close */
    OGG_Unload,
};

#endif /* MUSIC_OGG */

/* vi: set ts=4 sw=4 expandtab: */
