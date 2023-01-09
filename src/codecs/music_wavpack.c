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
*/

#if defined(MUSIC_WAVPACK)

#define WAVPACK_DBG 0

/* This file supports WavPack music streams */

#include "SDL_loadso.h"
#include "SDL_log.h"

#include "music_wavpack.h"

#if defined(WAVPACK_HEADER)
#include WAVPACK_HEADER
#elif defined(HAVE_WAVPACK_H)
#include <wavpack.h>
#else
#include <wavpack/wavpack.h>
#endif
#include <stdio.h>  /* SEEK_SET, ... */

#ifndef OPEN_DSD_NATIVE
#define OPEN_DSD_NATIVE 0x100
#define OPEN_DSD_AS_PCM 0x200
#define WAVPACK4_OR_OLDER
#endif

#ifdef WAVPACK4_OR_OLDER
typedef struct {
    int32_t (*read_bytes)(void *id, void *data, int32_t bcount);
    int32_t (*write_bytes)(void *id, void *data, int32_t bcount);
    int64_t (*get_pos)(void *id);
    int (*set_pos_abs)(void *id, int64_t pos);
    int (*set_pos_rel)(void *id, int64_t delta, int mode);
    int (*push_back_byte)(void *id, int c);
    int64_t (*get_length)(void *id);
    int (*can_seek)(void *id);
    int (*truncate_here)(void *id);
    int (*close)(void *id);
} WavpackStreamReader64;
#endif

typedef struct {
    int loaded;
    void *handle;
    uint32_t libversion;
    uint32_t (*WavpackGetLibraryVersion)(void);
    char *(*WavpackGetErrorMessage)(WavpackContext*);
    WavpackContext *(*WavpackOpenFileInput)(const char *infilename, char *error, int flags, int norm_offset);
    WavpackContext *(*WavpackOpenFileInputEx)(WavpackStreamReader *reader, void *wv_id, void *wvc_id, char *error, int flags, int norm_offset);
    WavpackContext *(*WavpackCloseFile)(WavpackContext*);
    int (*WavpackGetMode)(WavpackContext*);
    int (*WavpackGetBytesPerSample)(WavpackContext*);
    int (*WavpackGetNumChannels)(WavpackContext*);
    uint32_t (*WavpackGetNumSamples)(WavpackContext*);
    uint32_t (*WavpackGetSampleRate)(WavpackContext*);
    uint32_t (*WavpackUnpackSamples)(WavpackContext*, int32_t *buffer, uint32_t samples);
    int (*WavpackSeekSample)(WavpackContext*, uint32_t sample);
    uint32_t (*WavpackGetSampleIndex)(WavpackContext*);
    int (*WavpackGetTagItem)(WavpackContext*, const char *item, char *value, int size);
    /* WavPack 5.x functions with 64 bit support: */
    WavpackContext *(*WavpackOpenFileInputEx64)(WavpackStreamReader64 *reader, void *wv_id, void *wvc_id, char *error, int flags, int norm_offset);
    int64_t (*WavpackGetNumSamples64)(WavpackContext*);
    int64_t (*WavpackGetSampleIndex64)(WavpackContext*);
    int (*WavpackSeekSample64)(WavpackContext*, int64_t sample);
} wavpack_loader;

static wavpack_loader wvpk;

#ifdef WAVPACK_DYNAMIC
#define FUNCTION_LOADER(FUNC, SIG) \
    wvpk.FUNC = (SIG) SDL_LoadFunction(wvpk.handle, #FUNC); \
    if (wvpk.FUNC == NULL) { SDL_UnloadObject(wvpk.handle); return -1; }
#else
#define FUNCTION_LOADER(FUNC, SIG) \
    wvpk.FUNC = FUNC; \
    if (wvpk.FUNC == NULL) { Mix_SetError("Missing WavPack.framework"); return -1; }
#endif

static int WAVPACK_Load(void)
{
    if (wvpk.loaded == 0) {
#ifdef WAVPACK_DYNAMIC
        wvpk.handle = SDL_LoadObject(WAVPACK_DYNAMIC);
        if (wvpk.handle == NULL) {
            return -1;
        }
#endif
        FUNCTION_LOADER(WavpackGetLibraryVersion, uint32_t (*)(void));
        FUNCTION_LOADER(WavpackGetErrorMessage, char *(*)(WavpackContext*));
        FUNCTION_LOADER(WavpackOpenFileInput, WavpackContext *(*)(const char*, char*, int, int));
        FUNCTION_LOADER(WavpackOpenFileInputEx, WavpackContext *(*)(WavpackStreamReader*, void*, void*, char*, int, int));
        FUNCTION_LOADER(WavpackCloseFile, WavpackContext *(*)(WavpackContext*));
        FUNCTION_LOADER(WavpackGetMode, int (*)(WavpackContext*));
        FUNCTION_LOADER(WavpackGetBytesPerSample, int (*)(WavpackContext*));
        FUNCTION_LOADER(WavpackGetNumChannels, int (*)(WavpackContext*));
        FUNCTION_LOADER(WavpackGetNumSamples, uint32_t (*)(WavpackContext*));
        FUNCTION_LOADER(WavpackGetSampleRate, uint32_t (*)(WavpackContext*));
        FUNCTION_LOADER(WavpackUnpackSamples, uint32_t (*)(WavpackContext*, int32_t*, uint32_t));
        FUNCTION_LOADER(WavpackSeekSample, int (*)(WavpackContext*, uint32_t));
        FUNCTION_LOADER(WavpackGetSampleIndex, uint32_t (*)(WavpackContext*));
        FUNCTION_LOADER(WavpackGetTagItem, int (*)(WavpackContext*, const char*, char*, int));

        /* WavPack 5.x functions with 64 bit support: */
#ifdef WAVPACK_DYNAMIC
        wvpk.WavpackOpenFileInputEx64 = (WavpackContext *(*)(WavpackStreamReader64*, void*, void*, char*, int, int)) SDL_LoadFunction(wvpk.handle, "WavpackOpenFileInputEx64");
        wvpk.WavpackGetNumSamples64 = (int64_t (*)(WavpackContext*)) SDL_LoadFunction(wvpk.handle, "WavpackGetNumSamples64");
        wvpk.WavpackGetSampleIndex64 = (int64_t (*)(WavpackContext*)) SDL_LoadFunction(wvpk.handle, "WavpackGetSampleIndex64");
        wvpk.WavpackSeekSample64 = (int (*)(WavpackContext*, int64_t)) SDL_LoadFunction(wvpk.handle, "WavpackSeekSample64");
        if (!wvpk.WavpackOpenFileInputEx64 || !wvpk.WavpackGetNumSamples64 || !wvpk.WavpackGetSampleIndex64 || !wvpk.WavpackSeekSample64) {
            wvpk.WavpackOpenFileInputEx64 = NULL;
            wvpk.WavpackGetNumSamples64 = NULL;
            wvpk.WavpackGetSampleIndex64 = NULL;
            wvpk.WavpackSeekSample64 = NULL;
            SDL_ClearError();   /* WavPack 5.x functions are optional. */
        }
        #if WAVPACK_DBG
        else {
            SDL_Log("WavPack 5.x functions available");
        }
        #endif
#elif !defined(WAVPACK4_OR_OLDER)
        wvpk.WavpackOpenFileInputEx64 = WavpackOpenFileInputEx64;
        wvpk.WavpackGetNumSamples64 = WavpackGetNumSamples64;
        wvpk.WavpackGetSampleIndex64 = WavpackGetSampleIndex64;
        wvpk.WavpackSeekSample64 = WavpackSeekSample64;
#else
        wvpk.WavpackOpenFileInputEx64 = NULL;
        wvpk.WavpackGetNumSamples64 = NULL;
        wvpk.WavpackGetSampleIndex64 = NULL;
        wvpk.WavpackSeekSample64 = NULL;
#endif

        wvpk.libversion = wvpk.WavpackGetLibraryVersion();
        #if WAVPACK_DBG
        SDL_Log("WavPack library version: 0x%x", wvpk.libversion);
        #endif
    }
    ++wvpk.loaded;

    return 0;
}

static void WAVPACK_Unload(void)
{
    if (wvpk.loaded == 0) {
        return;
    }
    if (wvpk.loaded == 1) {
#ifdef WAVPACK_DYNAMIC
        SDL_UnloadObject(wvpk.handle);
#endif
    }
    --wvpk.loaded;
}


typedef struct {
    SDL_RWops *src1; /* wavpack file    */
    SDL_RWops *src2; /* correction file */
    int freesrc;
    int play_count;
    int volume;

    WavpackContext *ctx;
    int64_t numsamples;
    uint32_t samplerate;
    int bps, channels, mode, decimation;
    void *decimation_ctx;

    SDL_AudioStream *stream;
    void *buffer;
    int32_t frames;

    Mix_MusicMetaTags tags;
} WAVPACK_music;


static int32_t sdl_read_bytes(void *id, void *data, int32_t bcount)
{
    return (int32_t)SDL_RWread((SDL_RWops*)id, data, 1, bcount);
}

static uint32_t sdl_get_pos32(void *id)
{
    return (uint32_t)SDL_RWtell((SDL_RWops*)id);
}

static int64_t sdl_get_pos64(void *id)
{
    return SDL_RWtell((SDL_RWops*)id);
}

static int sdl_setpos_rel64(void *id, int64_t delta, int mode)
{
    switch (mode) { /* just in case SDL_RW doesn't match stdio.. */
    case SEEK_SET: mode = RW_SEEK_SET; break;
    case SEEK_CUR: mode = RW_SEEK_CUR; break;
    case SEEK_END: mode = RW_SEEK_END; break;
    default: return -1;
    }
    return (SDL_RWseek((SDL_RWops*)id, delta, mode) < 0)? -1 : 0;
}

static int sdl_setpos_rel32(void *id, int32_t delta, int mode)
{
    return sdl_setpos_rel64(id, delta, mode);
}

static int sdl_setpos_abs64(void *id, int64_t pos)
{
    return (SDL_RWseek((SDL_RWops*)id, pos, RW_SEEK_SET) < 0)? -1 : 0;
}

static int sdl_setpos_abs32(void *id, uint32_t pos)
{
    return (SDL_RWseek((SDL_RWops*)id, pos, RW_SEEK_SET) < 0)? -1 : 0;
}

static int sdl_pushback_byte(void *id, int c)
{
    (void)c;
    /* libwavpack calls ungetc(), but doesn't really modify buffer. */
    return (SDL_RWseek((SDL_RWops*)id, -1, RW_SEEK_CUR) < 0)? -1 : 0;
}

static uint32_t sdl_get_length32(void *id)
{
    return (uint32_t)SDL_RWsize((SDL_RWops*)id);
}

static int64_t sdl_get_length64(void *id)
{
    return SDL_RWsize((SDL_RWops*)id);
}

static int sdl_can_seek(void *id)
{
    return (SDL_RWseek((SDL_RWops*)id, 0, RW_SEEK_CUR) < 0)? 0 : 1;
}

static WavpackStreamReader sdl_reader32 = {
    sdl_read_bytes,
    sdl_get_pos32,
    sdl_setpos_abs32,
    sdl_setpos_rel32,
    sdl_pushback_byte,
    sdl_get_length32,
    sdl_can_seek,
    NULL  /* write_bytes */
};

static WavpackStreamReader64 sdl_reader64 = {
    sdl_read_bytes,
    NULL, /* write_bytes */
    sdl_get_pos64,
    sdl_setpos_abs64,
    sdl_setpos_rel64,
    sdl_pushback_byte,
    sdl_get_length64,
    sdl_can_seek,
    NULL, /* truncate_here */
    NULL  /* close */
};

static int WAVPACK_Seek(void *context, double time);
static void WAVPACK_Delete(void *context);
static void *WAVPACK_CreateFromRW_internal(SDL_RWops *src1, SDL_RWops *src2, int freesrc, int *freesrc2);

static void *decimation_init(int num_channels, int ratio);
static int decimation_run(void *context, int32_t *samples, int num_samples);
static void decimation_reset(void *context);

static void *WAVPACK_CreateFromRW(SDL_RWops *src, int freesrc)
{
    return WAVPACK_CreateFromRW_internal(src, NULL, freesrc, NULL);
}

static void *WAVPACK_CreateFromFile(const char *file)
{
    SDL_RWops *src1, *src2;
    WAVPACK_music *music;
    int freesrc2 = 1;
    size_t len;
    char *file2;

    src1 = SDL_RWFromFile(file, "rb");
    if (!src1) {
        Mix_SetError("Couldn't open '%s'", file);
        return NULL;
    }

    len = SDL_strlen(file);
    file2 = SDL_stack_alloc(char, len + 2);
    if (!file2) src2 = NULL;
    else {
        SDL_memcpy(file2, file, len);
        file2[len] =  'c';
        file2[len + 1] = '\0';
        src2 = SDL_RWFromFile(file2, "rb");
        #if WAVPACK_DBG
        if (src2) {
            SDL_Log("Loaded WavPack correction file %s", file2);
        }
        #endif
        SDL_stack_free(file2);
    }

    music = WAVPACK_CreateFromRW_internal(src1, src2, 1, &freesrc2);
    if (!music) {
        SDL_RWclose(src1);
        if (freesrc2 && src2) {
            SDL_RWclose(src2);
        }
    }
    return music;
}

/* Load a WavPack stream from an SDL_RWops object */
static void *WAVPACK_CreateFromRW_internal(SDL_RWops *src1, SDL_RWops *src2, int freesrc, int *freesrc2)
{
    WAVPACK_music *music;
    SDL_AudioFormat format;
    char *tag;
    char err[80];
    int n;

    music = (WAVPACK_music *)SDL_calloc(1, sizeof *music);
    if (!music) {
        SDL_OutOfMemory();
        return NULL;
    }
    music->src1 = src1;
    music->src2 = src2;
    music->volume = MIX_MAX_VOLUME;

    music->ctx = (wvpk.WavpackOpenFileInputEx64 != NULL) ?
                  wvpk.WavpackOpenFileInputEx64(&sdl_reader64, src1, src2, err, OPEN_NORMALIZE|OPEN_TAGS|OPEN_DSD_AS_PCM, 0) :
                  wvpk.WavpackOpenFileInputEx(&sdl_reader32, src1, src2, err, OPEN_NORMALIZE|OPEN_TAGS, 0);
    if (!music->ctx) {
        Mix_SetError("%s", err);
        SDL_free(music);
        if (src2) {
            SDL_RWclose(src2);
        }
        return NULL;
    }

    music->numsamples = (wvpk.WavpackGetNumSamples64 != NULL) ?
                         wvpk.WavpackGetNumSamples64(music->ctx) :
                         wvpk.WavpackGetNumSamples(music->ctx);
    music->samplerate = wvpk.WavpackGetSampleRate(music->ctx);
    music->bps = wvpk.WavpackGetBytesPerSample(music->ctx) << 3;
    music->channels = wvpk.WavpackGetNumChannels(music->ctx);
    music->mode = wvpk.WavpackGetMode(music->ctx);
    music->decimation = 1;

    if (freesrc2) {
       *freesrc2 = 0; /* WAVPACK_Delete() will free it. */
    }

    /* for very high sample rates (including DSD, which will normally be 352,800 Hz)
     * decimate 4x here before sending on */
    if (music->samplerate >= 256000) {
        music->decimation = 4;
        music->decimation_ctx = decimation_init(music->channels, music->decimation);
        if (!music->decimation_ctx) {
            SDL_OutOfMemory();
            WAVPACK_Delete(music);
            return NULL;
        }
    }

    #if WAVPACK_DBG
    SDL_Log("WavPack loader:\n numsamples: %" SDL_PRIs64 "\n samplerate: %d\n bitspersample: %d\n channels: %d\n mode: 0x%x\n lossy: %d\n duration: %f\n",
            (Sint64)music->numsamples, music->samplerate, music->bps, music->channels, music->mode, !(music->mode & MODE_LOSSLESS), music->numsamples/(double)music->samplerate);
    #endif

    /* library returns the samples in 8, 16, 24, or 32 bit depth, but
     * always in an int32_t[] buffer, in signed host-endian format. */
    switch (music->bps) {
    case 8:
        format = AUDIO_U8;
        break;
    case 16:
        format = AUDIO_S16SYS;
        break;
    default:
        format = (music->mode & MODE_FLOAT) ? AUDIO_F32SYS : AUDIO_S32SYS;
        break;
    }
    music->stream = SDL_NewAudioStream(format, (Uint8)music->channels, (int)music->samplerate / music->decimation,
                                       music_spec.format, music_spec.channels, music_spec.freq);
    if (!music->stream) {
        WAVPACK_Delete(music);
        return NULL;
    }

    music->frames = music_spec.samples;
    music->buffer = SDL_malloc(music->frames * music->channels * sizeof(int32_t) * music->decimation);
    if (!music->buffer) {
        SDL_OutOfMemory();
        WAVPACK_Delete(music);
        return NULL;
    }

    tag = NULL;
    n = wvpk.WavpackGetTagItem(music->ctx, "TITLE", NULL, 0);
    if (n > 0) {
        tag = SDL_realloc(tag, (size_t)(++n));
        wvpk.WavpackGetTagItem(music->ctx, "TITLE", tag, n);
        meta_tags_set(&music->tags, MIX_META_TITLE, tag);
    }
    n = wvpk.WavpackGetTagItem(music->ctx, "ARTIST", NULL, 0);
    if (n > 0) {
        tag = SDL_realloc(tag, (size_t)(++n));
        wvpk.WavpackGetTagItem(music->ctx, "ARTIST", tag, n);
        meta_tags_set(&music->tags, MIX_META_ARTIST, tag);
    }
    n = wvpk.WavpackGetTagItem(music->ctx, "ALBUM", NULL, 0);
    if (n > 0) {
        tag = SDL_realloc(tag, (size_t)(++n));
        wvpk.WavpackGetTagItem(music->ctx, "ALBUM", tag, n);
        meta_tags_set(&music->tags, MIX_META_ALBUM, tag);
    }
    n = wvpk.WavpackGetTagItem(music->ctx, "COPYRIGHT", NULL, 0);
    if (n > 0) {
        tag = SDL_realloc(tag, (size_t)(++n));
        wvpk.WavpackGetTagItem(music->ctx, "COPYRIGHT", tag, n);
        meta_tags_set(&music->tags, MIX_META_COPYRIGHT, tag);
    }
    SDL_free(tag);

    music->freesrc = freesrc;
    return music;
}

static const char* WAVPACK_GetMetaTag(void *context, Mix_MusicMetaTag tag_type)
{
    WAVPACK_music *music = (WAVPACK_music *)context;
    return meta_tags_get(&music->tags, tag_type);
}

static void WAVPACK_SetVolume(void *context, int volume)
{
    WAVPACK_music *music = (WAVPACK_music *)context;
    music->volume = volume;
}

static int WAVPACK_GetVolume(void *context)
{
    WAVPACK_music *music = (WAVPACK_music *)context;
    return music->volume;
}

/* Start playback of a given WavPack stream */
static int WAVPACK_Play(void *context, int play_count)
{
    WAVPACK_music *music = (WAVPACK_music *)context;
    music->play_count = play_count;
    return WAVPACK_Seek(music, 0.0);
}

static void WAVPACK_Stop(void *context)
{
    WAVPACK_music *music = (WAVPACK_music *)context;
    SDL_AudioStreamClear(music->stream);
}

/* Play some of a stream previously started with WAVPACK_play() */
static int WAVPACK_GetSome(void *context, void *data, int bytes, SDL_bool *done)
{
    WAVPACK_music *music = (WAVPACK_music *)context;
    int amount;

    amount = SDL_AudioStreamGet(music->stream, data, bytes);
    if (amount != 0) {
        return amount;
    }

    if (!music->play_count) {
        /* All done */
        *done = SDL_TRUE;
        return 0;
    }

    amount = (int) wvpk.WavpackUnpackSamples(music->ctx, music->buffer, music->frames * music->decimation);

    if (amount && music->decimation_ctx) {
        amount = decimation_run(music->decimation_ctx, music->buffer, amount);
    }

    if (amount) {
        int32_t *src = (int32_t *)music->buffer;
        int c = 0;
        amount *= music->channels;
        switch (music->bps) {
        case 8: {
            Uint8 *dst = (Uint8 *)music->buffer;
            for (; c < amount; ++c) {
                *dst++ = 0x80 ^ (Uint8)*src++;
            } }
            break;
        case 16: {
            Sint16 *dst = (Sint16 *)music->buffer;
            for (; c < amount; ++c) {
                *dst++ = *src++;
            } }
            amount *= sizeof(Sint16);
            break;
        case 24:
            for (; c < amount; ++c) {
                src[c] <<= 8;
            }
            /* FALLTHRU */
        default:
            amount *= sizeof(Sint32);
            break;
        }
        if (SDL_AudioStreamPut(music->stream, music->buffer, amount) < 0) {
            return -1;
        }
    } else {
        if (music->play_count == 1) {
            music->play_count = 0;
            SDL_AudioStreamFlush(music->stream);
        } else {
            int play_count = -1;
            if (music->play_count > 0) {
                play_count = (music->play_count - 1);
            }
            if (WAVPACK_Play(music, play_count) < 0) {
                return -1;
            }
        }
    }
    return 0;
}
static int WAVPACK_GetAudio(void *context, void *data, int bytes)
{
    WAVPACK_music *music = (WAVPACK_music *)context;
    return music_pcm_getaudio(music, data, bytes, music->volume, WAVPACK_GetSome);
}

/* Jump (seek) to a given position (time is in seconds) */
static int WAVPACK_Seek(void *context, double time)
{
    WAVPACK_music *music = (WAVPACK_music *)context;
    int64_t sample = (int64_t)(time * music->samplerate);
    int success = (wvpk.WavpackSeekSample64 != NULL) ?
                   wvpk.WavpackSeekSample64(music->ctx, sample) :
                   wvpk.WavpackSeekSample(music->ctx, sample);
    if (!success) {
        return Mix_SetError("%s", wvpk.WavpackGetErrorMessage(music->ctx));
    }
    if (music->decimation_ctx) {
        decimation_reset(music->decimation_ctx);
    }
    return 0;
}

static double WAVPACK_Tell(void *context)
{
    WAVPACK_music *music = (WAVPACK_music *)context;
    if (wvpk.WavpackGetSampleIndex64 != NULL) {
        return wvpk.WavpackGetSampleIndex64(music->ctx) / (double)music->samplerate;
    }
    return wvpk.WavpackGetSampleIndex(music->ctx) / (double)music->samplerate;
}

/* Return music duration in seconds */
static double WAVPACK_Duration(void *context)
{
    WAVPACK_music *music = (WAVPACK_music *)context;
    return music->numsamples / (double)music->samplerate;
}

/* Close the given WavPack stream */
static void WAVPACK_Delete(void *context)
{
    WAVPACK_music *music = (WAVPACK_music *)context;
    meta_tags_clear(&music->tags);
    wvpk.WavpackCloseFile(music->ctx);
    if (music->stream) {
        SDL_FreeAudioStream(music->stream);
    }
    SDL_free(music->buffer);
    SDL_free(music->decimation_ctx);
    if (music->src2) {
        SDL_RWclose(music->src2);
    }
    if (music->freesrc) {
        SDL_RWclose(music->src1);
    }
    SDL_free(music);
}

/* Decimation code for playing DSD (which comes from the library already decimated 8x) */
/* Code provided by David Bryant. */
/* sinc low-pass filter, cutoff = fs/12, 80 terms */
#define NUM_TERMS 80
static const int32_t filter[NUM_TERMS] = {
         50,     464,     968,     711,   -1203,   -5028,   -9818,  -13376,
     -12870,   -6021,    7526,   25238,   41688,   49778,   43050,   18447,
     -21428,  -67553, -105876, -120890, -100640,  -41752,   47201,  145510,
     224022,  252377,  208224,   86014,  -97312, -301919, -470919, -541796,
    -461126, -199113,  239795,  813326, 1446343, 2043793, 2509064, 2763659,
    2763659, 2509064, 2043793, 1446343,  813326,  239795, -199113, -461126,
    -541796, -470919, -301919,  -97312,   86014,  208224,  252377,  224022,
     145510,   47201,  -41752, -100640, -120890, -105876,  -67553,  -21428,
      18447,   43050,   49778,   41688,   25238,    7526,   -6021,  -12870,
     -13376,   -9818,   -5028,   -1203,     711,     968,     464,      50
};

typedef struct chan_state {
    int32_t delay[NUM_TERMS];
    int index, num_channels, ratio;
} ChanState;

static void *decimation_init(int num_channels, int ratio)
{
    ChanState *sp = (ChanState *)SDL_calloc(num_channels, sizeof(ChanState));

    if (sp) {
        int i = 0;
        for (; i < num_channels; ++i) {
            sp[i].num_channels = num_channels;
            sp[i].index = NUM_TERMS - ratio;
            sp[i].ratio = ratio;
        }
    }

    return sp;
}

/** FIXME: This isn't particularly easy on the CPU ! **/
static int decimation_run(void *context, int32_t *samples, int num_samples)
{
    ChanState *sp = (ChanState *)context;
    int32_t *in_samples = samples;
    int32_t *out_samples = samples;
    const int num_channels = sp->num_channels;
    const int ratio = sp->ratio;
    int chan = 0;

    while (num_samples) {
        sp = (ChanState *)context + chan;

        sp->delay[sp->index++] = *in_samples++;

        if (sp->index == NUM_TERMS) {
            int64_t sum = 0;
            int i = 0;
            for (; i < NUM_TERMS; ++i) {
                sum += (int64_t)filter[i] * sp->delay[i];
            }
            *out_samples++ = (int32_t)(sum >> 24);
            SDL_memmove(sp->delay, sp->delay + ratio, sizeof(sp->delay[0]) * (NUM_TERMS - ratio));
            sp->index = NUM_TERMS - ratio;
        }

        if (++chan == num_channels) {
            num_samples--;
            chan = 0;
        }
    }

    return (int)(out_samples - samples) / num_channels;
}

static void decimation_reset(void *context)
{
    ChanState *sp = (ChanState *)context;
    const int num_channels = sp->num_channels;
    const int ratio = sp->ratio;
    int i = 0;

    SDL_memset(sp, 0, sizeof(ChanState) * num_channels);
    for (; i < num_channels; ++i) {
        sp[i].num_channels = num_channels;
        sp[i].index = NUM_TERMS - ratio;
        sp[i].ratio = ratio;
    }
}

Mix_MusicInterface Mix_MusicInterface_WAVPACK =
{
    "WAVPACK",
    MIX_MUSIC_WAVPACK,
    MUS_WAVPACK,
    SDL_FALSE,
    SDL_FALSE,

    WAVPACK_Load,
    NULL,   /* Open */
    WAVPACK_CreateFromRW,
    WAVPACK_CreateFromFile,
    WAVPACK_SetVolume,
    WAVPACK_GetVolume,
    WAVPACK_Play,
    NULL,   /* IsPlaying */
    WAVPACK_GetAudio,
    NULL,   /* Jump */
    WAVPACK_Seek,
    WAVPACK_Tell,
    WAVPACK_Duration,
    NULL, /* LoopStart */
    NULL, /* LoopEnd */
    NULL, /* LoopLength */
    WAVPACK_GetMetaTag,
    NULL,   /* GetNumTracks */
    NULL,   /* StartTrack */
    NULL,   /* Pause */
    NULL,   /* Resume */
    WAVPACK_Stop,
    WAVPACK_Delete,
    NULL,   /* Close */
    WAVPACK_Unload
};

#endif /* MUSIC_WAVPACK */

/* vi: set ts=4 sw=4 expandtab: */
