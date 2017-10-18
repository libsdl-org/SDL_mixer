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

#ifdef MUSIC_MOD_MIKMOD

/* This file supports MOD tracker music streams */

#include "SDL_loadso.h"

#include "music_mikmod.h"

#include "mikmod.h"


#define MAX_OUTPUT_CHANNELS 6

/* libmikmod >= 3.3.2 constified several funcs */
#if (LIBMIKMOD_VERSION < 0x030302)
#define MIKMOD3_CONST
#else
#define MIKMOD3_CONST const
#endif

typedef struct {
    int loaded;
    void *handle;

    void (*MikMod_Exit)(void);
    CHAR* (*MikMod_InfoDriver)(void);
    CHAR* (*MikMod_InfoLoader)(void);
    int (*MikMod_Init)(MIKMOD3_CONST CHAR*);
    void (*MikMod_RegisterAllLoaders)(void);
    void (*MikMod_RegisterDriver)(struct MDRIVER*);
    int* MikMod_errno;
    MIKMOD3_CONST char* (*MikMod_strerror)(int);
    void (*MikMod_free)(void*);
    BOOL (*Player_Active)(void);
    void (*Player_Free)(MODULE*);
    MODULE* (*Player_LoadGeneric)(MREADER*,int,BOOL);
    void (*Player_SetPosition)(UWORD);
    void (*Player_SetVolume)(SWORD);
    void (*Player_Start)(MODULE*);
    void (*Player_Stop)(void);
    ULONG (*VC_WriteBytes)(SBYTE*,ULONG);
    struct MDRIVER* drv_nos;
    UWORD* md_device;
    UWORD* md_mixfreq;
    UWORD* md_mode;
    UBYTE* md_musicvolume;
    UBYTE* md_pansep;
    UBYTE* md_reverb;
    UBYTE* md_sndfxvolume;
    UBYTE* md_volume;
} mikmod_loader;

static mikmod_loader mikmod = {
    0, NULL
};

#ifdef MOD_DYNAMIC

static int MIKMOD_Load()
{
    if (mikmod.loaded == 0) {
        mikmod.handle = SDL_LoadObject(MOD_DYNAMIC);
        if (mikmod.handle == NULL) {
            return -1;
        }
        mikmod.MikMod_Exit =
            (void (*)(void))
            SDL_LoadFunction(mikmod.handle, "MikMod_Exit");
        if (mikmod.MikMod_Exit == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.MikMod_InfoDriver =
            (CHAR* (*)(void))
            SDL_LoadFunction(mikmod.handle, "MikMod_InfoDriver");
        if (mikmod.MikMod_InfoDriver == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.MikMod_InfoLoader =
            (CHAR* (*)(void))
            SDL_LoadFunction(mikmod.handle, "MikMod_InfoLoader");
        if (mikmod.MikMod_InfoLoader == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.MikMod_Init =
            (int (*)(MIKMOD3_CONST CHAR*))
            SDL_LoadFunction(mikmod.handle, "MikMod_Init");
        if (mikmod.MikMod_Init == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.MikMod_RegisterAllLoaders =
            (void (*)(void))
            SDL_LoadFunction(mikmod.handle, "MikMod_RegisterAllLoaders");
        if (mikmod.MikMod_RegisterAllLoaders == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.MikMod_RegisterDriver =
            (void (*)(struct MDRIVER*))
            SDL_LoadFunction(mikmod.handle, "MikMod_RegisterDriver");
        if (mikmod.MikMod_RegisterDriver == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.MikMod_errno =
            (int*)
            SDL_LoadFunction(mikmod.handle, "MikMod_errno");
        if (mikmod.MikMod_errno == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.MikMod_strerror =
            (MIKMOD3_CONST char* (*)(int))
            SDL_LoadFunction(mikmod.handle, "MikMod_strerror");
        if (mikmod.MikMod_strerror == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.MikMod_free =
            (void (*)(void*))
            SDL_LoadFunction(mikmod.handle, "MikMod_free");
        if (mikmod.MikMod_free == NULL) {
            /* libmikmod 3.1 and earlier doesn't have it */
            mikmod.MikMod_free = free;
        }
        mikmod.Player_Active =
            (BOOL (*)(void))
            SDL_LoadFunction(mikmod.handle, "Player_Active");
        if (mikmod.Player_Active == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.Player_Free =
            (void (*)(MODULE*))
            SDL_LoadFunction(mikmod.handle, "Player_Free");
        if (mikmod.Player_Free == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.Player_LoadGeneric =
            (MODULE* (*)(MREADER*,int,BOOL))
            SDL_LoadFunction(mikmod.handle, "Player_LoadGeneric");
        if (mikmod.Player_LoadGeneric == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.Player_SetPosition =
            (void (*)(UWORD))
            SDL_LoadFunction(mikmod.handle, "Player_SetPosition");
        if (mikmod.Player_SetPosition == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.Player_SetVolume =
            (void (*)(SWORD))
            SDL_LoadFunction(mikmod.handle, "Player_SetVolume");
        if (mikmod.Player_SetVolume == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.Player_Start =
            (void (*)(MODULE*))
            SDL_LoadFunction(mikmod.handle, "Player_Start");
        if (mikmod.Player_Start == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.Player_Stop =
            (void (*)(void))
            SDL_LoadFunction(mikmod.handle, "Player_Stop");
        if (mikmod.Player_Stop == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.VC_WriteBytes =
            (ULONG (*)(SBYTE*,ULONG))
            SDL_LoadFunction(mikmod.handle, "VC_WriteBytes");
        if (mikmod.VC_WriteBytes == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.drv_nos =
            (MDRIVER*)
            SDL_LoadFunction(mikmod.handle, "drv_nos");
        if (mikmod.drv_nos == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.md_device =
            (UWORD*)
            SDL_LoadFunction(mikmod.handle, "md_device");
        if (mikmod.md_device == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.md_mixfreq =
            (UWORD*)
            SDL_LoadFunction(mikmod.handle, "md_mixfreq");
        if (mikmod.md_mixfreq == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.md_mode =
            (UWORD*)
            SDL_LoadFunction(mikmod.handle, "md_mode");
        if (mikmod.md_mode == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.md_musicvolume =
            (UBYTE*)
            SDL_LoadFunction(mikmod.handle, "md_musicvolume");
        if (mikmod.md_musicvolume == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.md_pansep =
            (UBYTE*)
            SDL_LoadFunction(mikmod.handle, "md_pansep");
        if (mikmod.md_pansep == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.md_reverb =
            (UBYTE*)
            SDL_LoadFunction(mikmod.handle, "md_reverb");
        if (mikmod.md_reverb == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.md_sndfxvolume =
            (UBYTE*)
            SDL_LoadFunction(mikmod.handle, "md_sndfxvolume");
        if (mikmod.md_sndfxvolume == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
        mikmod.md_volume =
            (UBYTE*)
            SDL_LoadFunction(mikmod.handle, "md_volume");
        if (mikmod.md_volume == NULL) {
            SDL_UnloadObject(mikmod.handle);
            return -1;
        }
    }
    ++mikmod.loaded;

    return 0;
}

static void MIKMOD_Unload()
{
    if (mikmod.loaded == 0) {
        return;
    }
    if (mikmod.loaded == 1) {
        SDL_UnloadObject(mikmod.handle);
    }
    --mikmod.loaded;
}

#else /* !MOD_DYNAMIC */

static int MIKMOD_Load()
{
    if (mikmod.loaded == 0) {
#ifdef __MACOSX__
        extern void Player_Start(MODULE*) __attribute__((weak_import));
        if (Player_Start == NULL)
        {
            /* Missing weakly linked framework */
            Mix_SetError("Missing mikmod.framework");
            return -1;
        }
#endif // __MACOSX__

        mikmod.MikMod_Exit = MikMod_Exit;
        mikmod.MikMod_InfoDriver = MikMod_InfoDriver;
        mikmod.MikMod_InfoLoader = MikMod_InfoLoader;
        mikmod.MikMod_Init = MikMod_Init;
        mikmod.MikMod_RegisterAllLoaders = MikMod_RegisterAllLoaders;
        mikmod.MikMod_RegisterDriver = MikMod_RegisterDriver;
        mikmod.MikMod_errno = &MikMod_errno;
        mikmod.MikMod_strerror = MikMod_strerror;
#if LIBMIKMOD_VERSION < ((3<<16)|(2<<8))
        mikmod.MikMod_free = free;
#else
        mikmod.MikMod_free = MikMod_free;
#endif
        mikmod.Player_Active = Player_Active;
        mikmod.Player_Free = Player_Free;
        mikmod.Player_LoadGeneric = Player_LoadGeneric;
        mikmod.Player_SetPosition = Player_SetPosition;
        mikmod.Player_SetVolume = Player_SetVolume;
        mikmod.Player_Start = Player_Start;
        mikmod.Player_Stop = Player_Stop;
        mikmod.VC_WriteBytes = VC_WriteBytes;
        mikmod.drv_nos = &drv_nos;
        mikmod.md_device = &md_device;
        mikmod.md_mixfreq = &md_mixfreq;
        mikmod.md_mode = &md_mode;
        mikmod.md_musicvolume = &md_musicvolume;
        mikmod.md_pansep = &md_pansep;
        mikmod.md_reverb = &md_reverb;
        mikmod.md_sndfxvolume = &md_sndfxvolume;
        mikmod.md_volume = &md_volume;
    }
    ++mikmod.loaded;

    return 0;
}

static void MIKMOD_Unload()
{
    if (mikmod.loaded == 0) {
        return;
    }
    if (mikmod.loaded == 1) {
    }
    --mikmod.loaded;
}

#endif /* MOD_DYNAMIC */


/* Reference for converting mikmod output to 4/6 channels */
static int current_output_channels;
static Uint16 current_output_format;

static int music_swap8;
static int music_swap16;

/* Initialize the MOD player, with the given mixer settings
   This function returns 0, or -1 if there was an error.
 */
static int MIKMOD_Open(const SDL_AudioSpec *spec)
{
    CHAR *list;

    /* Set the MikMod music format */
    music_swap8 = 0;
    music_swap16 = 0;
    switch (spec->format) {

        case AUDIO_U8:
        case AUDIO_S8: {
            if (spec->format == AUDIO_S8) {
                music_swap8 = 1;
            }
            *mikmod.md_mode = 0;
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
            *mikmod.md_mode = DMODE_16BITS;
        }
        break;

        default: {
            Mix_SetError("Unknown hardware audio format");
            return -1;
        }
    }
    current_output_channels = spec->channels;
    current_output_format = spec->format;
    if (spec->channels > 1) {
        if (spec->channels > MAX_OUTPUT_CHANNELS) {
            Mix_SetError("Hardware uses more channels than supported");
            return -1;
        }
        *mikmod.md_mode |= DMODE_STEREO;
    }
    *mikmod.md_mixfreq = spec->freq;
    *mikmod.md_device  = 0;
    *mikmod.md_volume  = 96;
    *mikmod.md_musicvolume = 128;
    *mikmod.md_sndfxvolume = 128;
    *mikmod.md_pansep  = 128;
    *mikmod.md_reverb  = 0;
    *mikmod.md_mode    |= DMODE_HQMIXER|DMODE_SOFT_MUSIC|DMODE_SURROUND;

    list = mikmod.MikMod_InfoDriver();
    if (list)
      mikmod.MikMod_free(list);
    else
      mikmod.MikMod_RegisterDriver(mikmod.drv_nos);

    list = mikmod.MikMod_InfoLoader();
    if (list)
      mikmod.MikMod_free(list);
    else
      mikmod.MikMod_RegisterAllLoaders();

    if (mikmod.MikMod_Init(NULL)) {
        Mix_SetError("%s", mikmod.MikMod_strerror(*mikmod.MikMod_errno));
        return -1;
    }

    return 0;
}

/* Uninitialize the music players */
static void MIKMOD_Close(void)
{
    if (mikmod.MikMod_Exit) {
        mikmod.MikMod_Exit();
    }
}

typedef struct
{
    MREADER mr;
    /* struct MREADER in libmikmod <= 3.2.0-beta2
     * doesn't have iobase members. adding them here
     * so that if we compile against 3.2.0-beta2, we
     * can still run OK against 3.2.0b3 and newer. */
    long iobase, prev_iobase;
    Sint64 offset;
    Sint64 eof;
    SDL_RWops *src;
} LMM_MREADER;

int LMM_Seek(struct MREADER *mr,long to,int dir)
{
	Sint64 offset = to;
    LMM_MREADER* lmmmr = (LMM_MREADER*)mr;
    if (dir == SEEK_SET) {
        offset += lmmmr->offset;
        if (offset < lmmmr->offset)
            return -1;
    }
    return (int)(SDL_RWseek(lmmmr->src, offset, dir));
}
long LMM_Tell(struct MREADER *mr)
{
    LMM_MREADER* lmmmr = (LMM_MREADER*)mr;
    return (long)(SDL_RWtell(lmmmr->src) - lmmmr->offset);
}
BOOL LMM_Read(struct MREADER *mr,void *buf,size_t sz)
{
    LMM_MREADER* lmmmr = (LMM_MREADER*)mr;
    return SDL_RWread(lmmmr->src, buf, sz, 1);
}
int LMM_Get(struct MREADER *mr)
{
    unsigned char c;
    LMM_MREADER* lmmmr = (LMM_MREADER*)mr;
    if (SDL_RWread(lmmmr->src, &c, 1, 1)) {
        return c;
    }
    return EOF;
}
BOOL LMM_Eof(struct MREADER *mr)
{
    Sint64 offset;
    LMM_MREADER* lmmmr = (LMM_MREADER*)mr;
    offset = LMM_Tell(mr);
    return offset >= lmmmr->eof;
}
MODULE *MikMod_LoadSongRW(SDL_RWops *src, int maxchan)
{
    LMM_MREADER lmmmr = {
        { LMM_Seek, LMM_Tell, LMM_Read, LMM_Get, LMM_Eof },
        0,
        0,
        0
    };
    lmmmr.offset = SDL_RWtell(src);
    SDL_RWseek(src, 0, RW_SEEK_END);
    lmmmr.eof = SDL_RWtell(src);
    SDL_RWseek(src, lmmmr.offset, RW_SEEK_SET);
    lmmmr.src = src;
    return mikmod.Player_LoadGeneric((MREADER*)&lmmmr, maxchan, 0);
}

/* Load a MOD stream from an SDL_RWops object */
void *MIKMOD_CreateFromRW(SDL_RWops *src, int freesrc)
{
    MODULE *module;

    module = MikMod_LoadSongRW(src, 64);
    if (!module) {
        Mix_SetError("%s", mikmod.MikMod_strerror(*mikmod.MikMod_errno));
        return NULL;
    }

    /* Stop implicit looping, fade out and other flags. */
    module->extspd  = 1;
    module->panflag = 1;
    module->wrap    = 0;
    module->loop    = 0;
#if 0 /* Don't set fade out by default - unfortunately there's no real way
to query the status of the song or set trigger actions.  Hum. */
    module->fadeout = 1;
#endif

    if (freesrc) {
        SDL_RWclose(src);
    }
    return module;
}

/* Set the volume for a MOD stream */
static void MIKMOD_SetVolume(void *context, int volume)
{
    mikmod.Player_SetVolume((SWORD)volume);
}

/* Start playback of a given MOD stream */
static int MIKMOD_Play(void *context)
{
    MODULE *music = (MODULE *)context;
    mikmod.Player_Start(music);
    mikmod.Player_SetVolume((SWORD)music_volume);
    return 0;
}

/* Return non-zero if a stream is currently playing */
static SDL_bool MIKMOD_IsPlaying(void *context)
{
    return mikmod.Player_Active() ? SDL_TRUE : SDL_FALSE;
}

/* Play some of a stream previously started with MOD_play() */
static int MIKMOD_GetAudio(void *context, void *data, int bytes)
{
    MODULE *music = (MODULE *)context;
    Uint8 *stream = (Uint8 *)data;
    int len = bytes;

    if (current_output_channels > 2) {
        int small_len = 2 * len / current_output_channels;
        int i;
        Uint8 *src, *dst;

        mikmod.VC_WriteBytes((SBYTE *)stream, small_len);
        /* and extend to len by copying channels */
        src = stream + small_len;
        dst = stream + len;

        switch (current_output_format & 0xFF) {
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
        mikmod.VC_WriteBytes((SBYTE *)stream, len);
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
    return 0;
}

/* Jump (seek) to a given position (time is in seconds) */
static int MIKMOD_Seek(void *context, double position)
{
    mikmod.Player_SetPosition((UWORD)position);
    return 0;
}

/* Stop playback of a stream previously started with MOD_play() */
static void MIKMOD_Stop(void *context)
{
    mikmod.Player_Stop();
}

/* Close the given MOD stream */
static void MIKMOD_Delete(void *context)
{
    MODULE *music = (MODULE *)context;
    mikmod.Player_Free(music);
}

Mix_MusicInterface Mix_MusicInterface_MIKMOD =
{
    "MIKMOD",
    MIX_MUSIC_MIKMOD,
    MUS_MOD,
    SDL_FALSE,
    SDL_FALSE,

    MIKMOD_Load,
    MIKMOD_Open,
    MIKMOD_CreateFromRW,
    NULL,   /* CreateFromFile */
    MIKMOD_SetVolume,
    MIKMOD_Play,
    MIKMOD_IsPlaying,
    MIKMOD_GetAudio,
    MIKMOD_Seek,
    NULL,   /* Pause */
    NULL,   /* Resume */
    NULL,   /* Stop */
    MIKMOD_Delete,
    MIKMOD_Close,
    MIKMOD_Unload,
};

#endif /* MUSIC_MOD_MIKMOD */

/* vi: set ts=4 sw=4 expandtab: */
