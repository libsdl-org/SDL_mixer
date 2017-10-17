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

#ifdef MUSIC_MOD_MODPLUG

#include "SDL_loadso.h"

#include "music_modplug.h"

#ifdef MODPLUG_HEADER
#include MODPLUG_HEADER
#else
#include <libmodplug/modplug.h>
#endif

typedef struct {
    int loaded;
    void *handle;

    ModPlugFile* (*ModPlug_Load)(const void* data, int size);
    void (*ModPlug_Unload)(ModPlugFile* file);
    int  (*ModPlug_Read)(ModPlugFile* file, void* buffer, int size);
    void (*ModPlug_Seek)(ModPlugFile* file, int millisecond);
    void (*ModPlug_GetSettings)(ModPlug_Settings* settings);
    void (*ModPlug_SetSettings)(const ModPlug_Settings* settings);
    void (*ModPlug_SetMasterVolume)(ModPlugFile* file,unsigned int cvol) ;
} modplug_loader;

static modplug_loader modplug = {
    0, NULL
};


static int current_output_channels = 0;
static int music_swap8 = 0;
static int music_swap16 = 0;
static ModPlug_Settings settings;


#ifdef MODPLUG_DYNAMIC

static int MODPLUG_Load(void)
{
    if (modplug.loaded == 0) {
        modplug.handle = SDL_LoadObject(MODPLUG_DYNAMIC);
        if (modplug.handle == NULL) {
            return -1;
        }

        modplug.ModPlug_Load =
            (ModPlugFile* (*)(const void* data, int size))
            SDL_LoadFunction(modplug.handle, "ModPlug_Load");

        modplug.ModPlug_Unload =
            (void (*)(ModPlugFile* file))
            SDL_LoadFunction(modplug.handle, "ModPlug_Unload");

        modplug.ModPlug_Read =
            (int  (*)(ModPlugFile* file, void* buffer, int size))
            SDL_LoadFunction(modplug.handle, "ModPlug_Read");

        modplug.ModPlug_Seek =
            (void (*)(ModPlugFile* file, int millisecond))
            SDL_LoadFunction(modplug.handle, "ModPlug_Seek");

        modplug.ModPlug_GetSettings =
            (void (*)(ModPlug_Settings* settings))
            SDL_LoadFunction(modplug.handle, "ModPlug_GetSettings");

        modplug.ModPlug_SetSettings =
            (void (*)(const ModPlug_Settings* settings))
            SDL_LoadFunction(modplug.handle, "ModPlug_SetSettings");

        modplug.ModPlug_SetMasterVolume =
            (void (*)(ModPlugFile* file,unsigned int cvol))
            SDL_LoadFunction(modplug.handle, "ModPlug_SetMasterVolume");
    }
    ++modplug.loaded;

    return 0;
}

static void MODPLUG_Unload(void)
{
    if (modplug.loaded == 0) {
        return;
    }
    if (modplug.loaded == 1) {
        SDL_UnloadObject(modplug.handle);
    }
    --modplug.loaded;
}

#else /* !MODPLUG_DYNAMIC */

int MODPLUG_Load(void)
{
    if (modplug.loaded == 0) {
#ifdef __MACOSX__
        extern ModPlugFile* ModPlug_Load(const void* data, int size) __attribute__((weak_import));
        if (ModPlug_Load == NULL)
        {
            /* Missing weakly linked framework */
            Mix_SetError("Missing modplug.framework");
            return -1;
        }
#endif // __MACOSX__

        modplug.ModPlug_Load = ModPlug_Load;
        modplug.ModPlug_Unload = ModPlug_Unload;
        modplug.ModPlug_Read = ModPlug_Read;
        modplug.ModPlug_Seek = ModPlug_Seek;
        modplug.ModPlug_GetSettings = ModPlug_GetSettings;
        modplug.ModPlug_SetSettings = ModPlug_SetSettings;
        modplug.ModPlug_SetMasterVolume = ModPlug_SetMasterVolume;
    }
    ++modplug.loaded;

    return 0;
}

static void MODPLUG_Unload(void)
{
    if (modplug.loaded == 0) {
        return;
    }
    if (modplug.loaded == 1) {
    }
    --modplug.loaded;
}

#endif /* MODPLUG_DYNAMIC */

static int MODPLUG_Open(const SDL_AudioSpec *spec)
{
    modplug.ModPlug_GetSettings(&settings);
    settings.mFlags=MODPLUG_ENABLE_OVERSAMPLING;
    current_output_channels=spec->channels;
    settings.mChannels=spec->channels>1?2:1;
    settings.mBits=spec->format&0xFF;

    music_swap8 = 0;
    music_swap16 = 0;

    switch(spec->format)
    {
        case AUDIO_U8:
        case AUDIO_S8: {
            if (spec->format == AUDIO_S8) {
                music_swap8 = 1;
            }
            settings.mBits=8;
        }
        break;

        case AUDIO_S16LSB:
        case AUDIO_S16MSB: {
            /* See if we need to correct MikMod mixing */
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
            if (spec->format == AUDIO_S16MSB) {
#else
            if (spec->format == AUDIO_S16LSB) {
#endif
                music_swap16 = 1;
            }
            settings.mBits=16;
        }
        break;

        default: {
            Mix_SetError("Unknown hardware audio format");
            return -1;
        }

    }

    settings.mFrequency=spec->freq; /*TODO: limit to 11025, 22050, or 44100 ? */
    settings.mResamplingMode=MODPLUG_RESAMPLE_FIR;
    settings.mReverbDepth=0;
    settings.mReverbDelay=100;
    settings.mBassAmount=0;
    settings.mBassRange=50;
    settings.mSurroundDepth=0;
    settings.mSurroundDelay=10;
    settings.mLoopCount=0;
    modplug.ModPlug_SetSettings(&settings);
    return 0;
}

/* Load a modplug stream from an SDL_RWops object */
void *MODPLUG_CreateFromRW(SDL_RWops *src, int freesrc)
{
    ModPlugFile *music = NULL;
    Sint64 offset;
    size_t sz;
    char *buf;

    offset = SDL_RWtell(src);
    SDL_RWseek(src, 0, RW_SEEK_END);
    sz = (size_t)(SDL_RWtell(src) - offset);
    SDL_RWseek(src, offset, RW_SEEK_SET);
    buf = (char*)SDL_malloc(sz);
    if (buf) {
        if (SDL_RWread(src, buf, sz, 1) == 1) {
            music = modplug.ModPlug_Load(buf, (int)sz);
        }
        SDL_free(buf);
    } else {
        SDL_OutOfMemory();
    }
    if (music && freesrc) {
        SDL_RWclose(src);
    }
    return music;
}

/* Set the volume for a modplug stream */
static void MODPLUG_SetVolume(void *context, int volume)
{
    ModPlugFile *music = (ModPlugFile *)context;
    modplug.ModPlug_SetMasterVolume(music, volume*4);
}

/* Start playback of a given modplug stream */
static int MODPLUG_Play(void *context)
{
    ModPlugFile *music = (ModPlugFile *)context;
    modplug.ModPlug_Seek(music,0);
    return 0;
}

/* Play some of a stream previously started with modplug_play() */
static int MODPLUG_GetAudio(void *context, void *data, int bytes)
{
    ModPlugFile *music = (ModPlugFile *)context;
    Uint8 *stream = (Uint8 *)data;
    int len = bytes;
    int consumed;

    if (current_output_channels > 2) {
        int small_len = 2 * len / current_output_channels;
        int i;
        Uint8 *src, *dst;

        i=modplug.ModPlug_Read(music, stream, small_len);
        consumed = (i / 2) * current_output_channels;
        if(i<small_len)
        {
            SDL_memset(stream+i,0,small_len-i);
        }
        /* and extend to len by copying channels */
        src = stream + small_len;
        dst = stream + len;

        switch (settings.mBits) {
            case 8:
                for (i=small_len/2; i; --i) {
                    src -= 2;
                    dst -= current_output_channels;
                    dst[0] = src[0];
                    dst[1] = src[1];
                    dst[2] = src[0];
                    dst[3] = src[1];
                    if (current_output_channels == 6) {
                        dst[4] = src[0];
                        dst[5] = src[1];
                    }
                }
                break;
            case 16:
                for (i=small_len/4; i; --i) {
                    src -= 4;
                    dst -= 2 * current_output_channels;
                    dst[0] = src[0];
                    dst[1] = src[1];
                    dst[2] = src[2];
                    dst[3] = src[3];
                    dst[4] = src[0];
                    dst[5] = src[1];
                    dst[6] = src[2];
                    dst[7] = src[3];
                    if (current_output_channels == 6) {
                        dst[8] = src[0];
                        dst[9] = src[1];
                        dst[10] = src[2];
                        dst[11] = src[3];
                    }
                }
                break;
        }
    } else {
        consumed=modplug.ModPlug_Read(music, stream, len);
    }
    if (music_swap8) {
        Uint8 *dst;
        int i;

        dst = stream;
        for (i=len; i; --i) {
            *dst++ ^= 0x80;
        }
    } else
    if (music_swap16) {
        Uint8 *dst, tmp;
        int i;

        dst = stream;
        for (i=(len/2); i; --i) {
            tmp = dst[0];
            dst[0] = dst[1];
            dst[1] = tmp;
            dst += 2;
        }
    }
    return (len-consumed);
}

/* Jump (seek) to a given position */
static int MODPLUG_Seek(void *context, double position)
{
    ModPlugFile *music = (ModPlugFile *)context;
    modplug.ModPlug_Seek(music,(int)(position*1000));
    return 0;
}

/* Close the given modplug stream */
static void MODPLUG_Delete(void *context)
{
    ModPlugFile *music = (ModPlugFile *)context;
    modplug.ModPlug_Unload(music);
}

Mix_MusicInterface Mix_MusicInterface_MODPLUG =
{
    "MODPLUG",
    MIX_MUSIC_MODPLUG,
    MUS_MOD,
    SDL_FALSE,
    SDL_FALSE,

    MODPLUG_Load,
    MODPLUG_Open,
    MODPLUG_CreateFromRW,
    NULL,   /* CreateFromFile */
    MODPLUG_SetVolume,
    MODPLUG_Play,
    NULL,   /* IsPlaying */
    MODPLUG_GetAudio,
    MODPLUG_Seek,
    NULL,   /* Pause */
    NULL,   /* Resume */
    NULL,   /* Stop */
    MODPLUG_Delete,
    NULL,   /* Close */
    MODPLUG_Unload,
};

#endif /* MUSIC_MOD_MODPLUG */

/* vi: set ts=4 sw=4 expandtab: */
