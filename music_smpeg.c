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

#ifdef MUSIC_MP3_SMPEG

#include "SDL_loadso.h"

#include "music_smpeg.h"

#if SDL_VERSION_ATLEAST(2, 0, 0)
/* Forward declaration for SDL 2.0  because struct is not available there but
   still used in a file included with smpeg.h. May not compile if missing. */
typedef struct SDL_Overlay SDL_Overlay;
#endif

#include "smpeg.h"

typedef struct {
    int loaded;
    void *handle;
    void (*SMPEG_actualSpec)(SMPEG *mpeg, SDL_AudioSpec *spec);
    void (*SMPEG_delete)(SMPEG* mpeg);
    void (*SMPEG_enableaudio)(SMPEG* mpeg, int enable);
    void (*SMPEG_enablevideo)(SMPEG* mpeg, int enable);
    SMPEG* (*SMPEG_new_rwops)(SDL_RWops *src, SMPEG_Info* info, int freesrc, int sdl_audio);
    void (*SMPEG_play)(SMPEG* mpeg);
    int (*SMPEG_playAudio)(SMPEG *mpeg, Uint8 *stream, int len);
    void (*SMPEG_rewind)(SMPEG* mpeg);
    void (*SMPEG_setvolume)(SMPEG* mpeg, int volume);
    void (*SMPEG_skip)(SMPEG* mpeg, float seconds);
    SMPEGstatus (*SMPEG_status)(SMPEG* mpeg);
    void (*SMPEG_stop)(SMPEG* mpeg);
} smpeg_loader;

static smpeg_loader smpeg = {
    0, NULL
};

#ifdef SMPEG_DYNAMIC

static int SMPEG_Load(void)
{
    if (smpeg.loaded == 0) {
        smpeg.handle = SDL_LoadObject(SMPEG_DYNAMIC);
        if (smpeg.handle == NULL) {
            return -1;
        }
        smpeg.SMPEG_actualSpec =
            (void (*)(SMPEG *, SDL_AudioSpec *))
            SDL_LoadFunction(smpeg.handle, "SMPEG_actualSpec");
        if (smpeg.SMPEG_actualSpec == NULL) {
            SDL_UnloadObject(smpeg.handle);
            return -1;
        }
        smpeg.SMPEG_delete =
            (void (*)(SMPEG*))
            SDL_LoadFunction(smpeg.handle, "SMPEG_delete");
        if (smpeg.SMPEG_delete == NULL) {
            SDL_UnloadObject(smpeg.handle);
            return -1;
        }
        smpeg.SMPEG_enableaudio =
            (void (*)(SMPEG*, int))
            SDL_LoadFunction(smpeg.handle, "SMPEG_enableaudio");
        if (smpeg.SMPEG_enableaudio == NULL) {
            SDL_UnloadObject(smpeg.handle);
            return -1;
        }
        smpeg.SMPEG_enablevideo =
            (void (*)(SMPEG*, int))
            SDL_LoadFunction(smpeg.handle, "SMPEG_enablevideo");
        if (smpeg.SMPEG_enablevideo == NULL) {
            SDL_UnloadObject(smpeg.handle);
            return -1;
        }
        smpeg.SMPEG_new_rwops =
            (SMPEG* (*)(SDL_RWops *, SMPEG_Info*, int, int))
            SDL_LoadFunction(smpeg.handle, "SMPEG_new_rwops");
        if (smpeg.SMPEG_new_rwops == NULL) {
            SDL_UnloadObject(smpeg.handle);
            return -1;
        }
        smpeg.SMPEG_play =
            (void (*)(SMPEG*))
            SDL_LoadFunction(smpeg.handle, "SMPEG_play");
        if (smpeg.SMPEG_play == NULL) {
            SDL_UnloadObject(smpeg.handle);
            return -1;
        }
        smpeg.SMPEG_playAudio =
            (int (*)(SMPEG *, Uint8 *, int))
            SDL_LoadFunction(smpeg.handle, "SMPEG_playAudio");
        if (smpeg.SMPEG_playAudio == NULL) {
            SDL_UnloadObject(smpeg.handle);
            return -1;
        }
        smpeg.SMPEG_rewind =
            (void (*)(SMPEG*))
            SDL_LoadFunction(smpeg.handle, "SMPEG_rewind");
        if (smpeg.SMPEG_rewind == NULL) {
            SDL_UnloadObject(smpeg.handle);
            return -1;
        }
        smpeg.SMPEG_setvolume =
            (void (*)(SMPEG*, int))
            SDL_LoadFunction(smpeg.handle, "SMPEG_setvolume");
        if (smpeg.SMPEG_setvolume == NULL) {
            SDL_UnloadObject(smpeg.handle);
            return -1;
        }
        smpeg.SMPEG_skip =
            (void (*)(SMPEG*, float))
            SDL_LoadFunction(smpeg.handle, "SMPEG_skip");
        if (smpeg.SMPEG_skip == NULL) {
            SDL_UnloadObject(smpeg.handle);
            return -1;
        }
        smpeg.SMPEG_status =
            (SMPEGstatus (*)(SMPEG*))
            SDL_LoadFunction(smpeg.handle, "SMPEG_status");
        if (smpeg.SMPEG_status == NULL) {
            SDL_UnloadObject(smpeg.handle);
            return -1;
        }
        smpeg.SMPEG_stop =
            (void (*)(SMPEG*))
            SDL_LoadFunction(smpeg.handle, "SMPEG_stop");
        if (smpeg.SMPEG_stop == NULL) {
            SDL_UnloadObject(smpeg.handle);
            return -1;
        }
    }
    ++smpeg.loaded;

    return 0;
}

static void SMPEG_Unload(void)
{
    if (smpeg.loaded == 0) {
        return;
    }
    if (smpeg.loaded == 1) {
        SDL_UnloadObject(smpeg.handle);
    }
    --smpeg.loaded;
}

#else /* !SMPEG_DYNAMIC */

static int SMPEG_Load(void)
{
    if (smpeg.loaded == 0) {
#ifdef __MACOSX__
        extern SMPEG* SMPEG_new_rwops(SDL_RWops*, SMPEG_Info*, int, int) __attribute__((weak_import));
        if (SMPEG_new_rwops == NULL)
        {
            /* Missing weakly linked framework */
            Mix_SetError("Missing smpeg2.framework");
            return -1;
        }
#endif // __MACOSX__

        smpeg.SMPEG_actualSpec = SMPEG_actualSpec;
        smpeg.SMPEG_delete = SMPEG_delete;
        smpeg.SMPEG_enableaudio = SMPEG_enableaudio;
        smpeg.SMPEG_enablevideo = SMPEG_enablevideo;
        smpeg.SMPEG_new_rwops = SMPEG_new_rwops;
        smpeg.SMPEG_play = SMPEG_play;
        smpeg.SMPEG_playAudio = SMPEG_playAudio;
        smpeg.SMPEG_rewind = SMPEG_rewind;
        smpeg.SMPEG_setvolume = SMPEG_setvolume;
        smpeg.SMPEG_skip = SMPEG_skip;
        smpeg.SMPEG_status = SMPEG_status;
        smpeg.SMPEG_stop = SMPEG_stop;
    }
    ++smpeg.loaded;

    return 0;
}

static void SMPEG_Unload(void)
{
    if (smpeg.loaded == 0) {
        return;
    }
    if (smpeg.loaded == 1) {
    }
    --smpeg.loaded;
}

#endif /* SMPEG_DYNAMIC */


static void *SMPEG_CreateFromRW(SDL_RWops *src, int freesrc)
{
    SMPEG_Info info;
    SMPEG *mp3 = smpeg.SMPEG_new_rwops(src, &info, freesrc, 0);
    if (!info.has_audio) {
        Mix_SetError("MPEG file does not have any audio stream.");
        smpeg.SMPEG_delete(mp3);
        return NULL;
    }
    smpeg.SMPEG_actualSpec(mp3, &music_spec);
    return mp3;
}

static void SMPEG_SetVolume(void *context, int volume)
{
    SMPEG *mp3 = (SMPEG *)context;
    smpeg.SMPEG_setvolume(mp3,(int)(((float)volume/(float)MIX_MAX_VOLUME)*100.0));
}

static int SMPEG_Play(void *context)
{
    SMPEG *mp3 = (SMPEG *)context;
    smpeg.SMPEG_enableaudio(mp3,1);
    smpeg.SMPEG_enablevideo(mp3,0);
    smpeg.SMPEG_play(mp3);
    return 0;
}

static SDL_bool SMPEG_IsPlaying(void *context)
{
    SMPEG *mp3 = (SMPEG *)context;
    return smpeg.SMPEG_status(mp3) == SMPEG_PLAYING ? SDL_TRUE : SDL_FALSE;
}

static int SMPEG_GetAudio(void *context, void *data, int bytes)
{
    SMPEG *mp3 = (SMPEG *)context;
    Uint8 *stream = (Uint8 *)data;
    int len = bytes;
    int left = (len - smpeg.SMPEG_playAudio(mp3, stream, len));
    return left;
}

static int SMPEG_Seek(void *context, double position)
{
    SMPEG *mp3 = (SMPEG *)context;
    smpeg.SMPEG_rewind(mp3);
    smpeg.SMPEG_play(mp3);
    if (position > 0.0) {
        smpeg.SMPEG_skip(mp3, (float)position);
    }
    return 0;
}

static void SMPEG_Stop(void *context)
{
    SMPEG *mp3 = (SMPEG *)context;
    smpeg.SMPEG_stop(mp3);
}

static void SMPEG_Delete(void *context)
{
    SMPEG *mp3 = (SMPEG *)context;
    smpeg.SMPEG_delete(mp3);
}

Mix_MusicInterface Mix_MusicInterface_SMPEG =
{
    "SMPEG",
    MIX_MUSIC_SMPEG,
    MUS_MP3,
    SDL_FALSE,
    SDL_FALSE,

    SMPEG_Load,
    NULL,   /* Open */
    SMPEG_CreateFromRW,
    NULL,   /* CreateFromFile */
    SMPEG_SetVolume,
    SMPEG_Play,
    SMPEG_IsPlaying,
    SMPEG_GetAudio,
    SMPEG_Seek,
    NULL,   /* Pause */
    NULL,   /* Resume */
    SMPEG_Stop,
    SMPEG_Delete,
    NULL,   /* Close */
    SMPEG_Unload,
};

#endif /* MUSIC_MP3_SMPEG */

/* vi: set ts=4 sw=4 expandtab: */
