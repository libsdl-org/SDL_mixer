/*
  SDL_mixer:    An audio mixer library based on the SDL library
  Copyright (C) 1997-2017 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.    In no event will the authors be held liable for any damages
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

/* This file supports playing MP3 files with mpg123 */

#ifdef MUSIC_MP3_MPG123

#include "SDL_loadso.h"

#include "music_mpg123.h"

#include <mpg123.h>


typedef struct {
    int loaded;
    void *handle;

    int (*mpg123_close)(mpg123_handle *mh);
    void (*mpg123_delete)(mpg123_handle *mh);
    void (*mpg123_exit)(void);
    int (*mpg123_format)( mpg123_handle *mh, long rate, int channels, int encodings );
    int (*mpg123_format_none)(mpg123_handle *mh);
    int (*mpg123_getformat)( mpg123_handle *mh, long *rate, int *channels, int *encoding );
    int (*mpg123_init)(void);
    mpg123_handle *(*mpg123_new)(const char* decoder, int *error);
    int (*mpg123_open_handle)(mpg123_handle *mh, void *iohandle);
    const char* (*mpg123_plain_strerror)(int errcode);
    int (*mpg123_read)(mpg123_handle *mh, unsigned char *outmemory, size_t outmemsize, size_t *done );
    int (*mpg123_replace_reader_handle)( mpg123_handle *mh, ssize_t (*r_read) (void *, void *, size_t), off_t (*r_lseek)(void *, off_t, int), void (*cleanup)(void*) );
    off_t (*mpg123_seek)( mpg123_handle *mh, off_t sampleoff, int whence );
    const char* (*mpg123_strerror)(mpg123_handle *mh);
} mpg123_loader;

static mpg123_loader mpg123 = {
    0, NULL
};

#ifdef MPG123_DYNAMIC
#define FUNCTION_LOADER(FUNC, SIG) \
    mpg123.FUNC = (SIG) SDL_LoadFunction(mpg123.handle, #FUNC); \
    if (mpg123.FUNC == NULL) { SDL_UnloadObject(mpg123.handle); return -1; }
#else
#define FUNCTION_LOADER(FUNC, SIG) \
    mpg123.FUNC = FUNC;
#endif

static int MPG123_Load()
{
    if (mpg123.loaded == 0) {
#ifdef MPG123_DYNAMIC
        mpg123.handle = SDL_LoadObject(MPG123_DYNAMIC);
        if (mpg123.handle == NULL) {
            return -1;
        }
#elif defined(__MACOSX__)
        extern int mpg123_init(void) __attribute__((weak_import));
        if (mpg123_init == NULL)
        {
            /* Missing weakly linked framework */
            Mix_SetError("Missing mpg123.framework");
            return -1;
        }
#endif

        FUNCTION_LOADER(mpg123_close, int (*)(mpg123_handle *mh))
        FUNCTION_LOADER(mpg123_delete, void (*)(mpg123_handle *mh))
        FUNCTION_LOADER(mpg123_exit, void (*)(void))
        FUNCTION_LOADER(mpg123_format, int (*)( mpg123_handle *mh, long rate, int channels, int encodings ))
        FUNCTION_LOADER(mpg123_format_none, int (*)(mpg123_handle *mh))
        FUNCTION_LOADER(mpg123_getformat, int (*)( mpg123_handle *mh, long *rate, int *channels, int *encoding ))
        FUNCTION_LOADER(mpg123_init, int (*)(void))
        FUNCTION_LOADER(mpg123_new, mpg123_handle *(*)(const char* decoder, int *error))
        FUNCTION_LOADER(mpg123_open_handle, int (*)(mpg123_handle *mh, void *iohandle))
        FUNCTION_LOADER(mpg123_plain_strerror, const char* (*)(int errcode))
        FUNCTION_LOADER(mpg123_read, int (*)(mpg123_handle *mh, unsigned char *outmemory, size_t outmemsize, size_t *done ))
        FUNCTION_LOADER(mpg123_replace_reader_handle, int (*)( mpg123_handle *mh, ssize_t (*r_read) (void *, void *, size_t), off_t (*r_lseek)(void *, off_t, int), void (*cleanup)(void*) ))
        FUNCTION_LOADER(mpg123_seek, off_t (*)( mpg123_handle *mh, off_t sampleoff, int whence ))
        FUNCTION_LOADER(mpg123_strerror, const char* (*)(mpg123_handle *mh))
    }
    ++mpg123.loaded;

    return 0;
}

static void MPG123_Unload()
{
    if (mpg123.loaded == 0) {
        return;
    }
    if (mpg123.loaded == 1) {
#ifdef MPG123_DYNAMIC
        SDL_UnloadObject(mpg123.handle);
#endif
    }
    --mpg123.loaded;
}


typedef struct
{
    SDL_RWops* src;
    int freesrc;

    SDL_AudioSpec mixer;

    SDL_bool playing;
    int volume;

    mpg123_handle* handle;

    int gotformat;
    SDL_AudioCVT cvt;
    Uint8 buf[8192];
    size_t len_available;
    Uint8* snd_available;
} mpg_data;

static
int
snd_format_to_mpg123(uint16_t sdl_fmt)
{
    switch (sdl_fmt)
    {
        case AUDIO_U8:     return MPG123_ENC_UNSIGNED_8;
        case AUDIO_U16SYS: return MPG123_ENC_UNSIGNED_16;
        case AUDIO_S8:     return MPG123_ENC_SIGNED_8;
        case AUDIO_S16SYS: return MPG123_ENC_SIGNED_16;
        case AUDIO_S32SYS: return MPG123_ENC_SIGNED_32;
    }

    return -1;
}

static
Uint16
mpg123_format_to_sdl(int fmt)
{
    switch (fmt)
    {
        case MPG123_ENC_UNSIGNED_8:  return AUDIO_U8;
        case MPG123_ENC_UNSIGNED_16: return AUDIO_U16SYS;
        case MPG123_ENC_SIGNED_8:    return AUDIO_S8;
        case MPG123_ENC_SIGNED_16:   return AUDIO_S16SYS;
        case MPG123_ENC_SIGNED_32:   return AUDIO_S32SYS;
    }

    return -1;
}

static
char const*
mpg123_format_str(int fmt)
{
    switch (fmt)
    {
#define f(x) case x: return #x;
        f(MPG123_ENC_UNSIGNED_8)
        f(MPG123_ENC_UNSIGNED_16)
        f(MPG123_ENC_SIGNED_8)
        f(MPG123_ENC_SIGNED_16)
        f(MPG123_ENC_SIGNED_32)
#undef f
    }

    return "unknown";
}

static
char const*
mpg_err(mpg123_handle* mpg, int code)
{
    char const* err = "unknown error";

    if (mpg && code == MPG123_ERR) {
        err = mpg123.mpg123_strerror(mpg);
    } else {
        err = mpg123.mpg123_plain_strerror(code);
    }

    return err;
}

/* we're gonna override mpg123's I/O with these wrappers for RWops */
static
ssize_t rwops_read(void* p, void* dst, size_t n) {
    return (ssize_t)SDL_RWread((SDL_RWops*)p, dst, 1, n);
}

static
off_t rwops_seek(void* p, off_t offset, int whence) {
    return (off_t)SDL_RWseek((SDL_RWops*)p, (Sint64)offset, whence);
}

static
void rwops_cleanup(void* p) {
    (void)p;
    /* do nothing, we will free the file later */
}

static int getsome(mpg_data* m);


static int MPG123_Open(const SDL_AudioSpec *spec)
{
    if (mpg123.mpg123_init() != MPG123_OK) {
        Mix_SetError("mpg123_init() failed");
        return -1;
    }
    return 0;
}

static void *MPG123_CreateFromRW(SDL_RWops *src, int freesrc)
{
    mpg_data* m;
    int result;
    int fmt;

    m = (mpg_data*)SDL_calloc(1, sizeof(mpg_data));
    if (!m) {
        return 0;
    }

    m->src = src;
    m->freesrc = freesrc;

    m->handle = mpg123.mpg123_new(0, &result);
    if (result != MPG123_OK) {
        return 0;
    }

    result = mpg123.mpg123_replace_reader_handle(
        m->handle,
        rwops_read, rwops_seek, rwops_cleanup
   );
    if (result != MPG123_OK) {
        return 0;
    }

    result = mpg123.mpg123_format_none(m->handle);
    if (result != MPG123_OK) {
        return 0;
    }

    fmt = snd_format_to_mpg123(music_spec.format);
    if (fmt == -1) {
        return 0;
    }

    result =  mpg123.mpg123_format(m->handle, music_spec.freq, music_spec.channels, fmt);
    if (result != MPG123_OK) {
        return 0;
    }

    result = mpg123.mpg123_open_handle(m->handle, m->src);
    if (result != MPG123_OK) {
        return 0;
    }

    m->volume = MIX_MAX_VOLUME;
    m->mixer = music_spec;

    /* hacky: read until we can figure out the format then rewind */
    while (!m->gotformat)
    {
        if (!getsome(m)) {
            return 0;
        }
    }

    /* rewind */
    mpg123.mpg123_seek(m->handle, 0, SEEK_SET);

    m->len_available = 0;
    m->snd_available = m->cvt.buf;

    return m;
}

static void MPG123_SetVolume(void *context, int volume) {
    mpg_data* m = (mpg_data *)context;
    m->volume = volume;
}

static int MPG123_Play(void *context)
{
    mpg_data* m = (mpg_data *)context;
    m->playing = SDL_TRUE;
    return 0;
}

static SDL_bool MPG123_IsPlaying(void *context)
{
    mpg_data* m = (mpg_data *)context;
    return m->playing;
}

/*
    updates the convert struct and buffer to match the format queried from
    mpg123.
*/
static int update_format(mpg_data* m)
{
    int code;
    long rate;
    int channels, encoding;
    Uint16 sdlfmt;
    size_t bufsize;

    m->gotformat = 1;

    code =
        mpg123.mpg123_getformat(
            m->handle,
            &rate, &channels, &encoding
       );

    if (code != MPG123_OK) {
        SDL_SetError("mpg123_getformat: %s", mpg_err(m->handle, code));
        return 0;
    }

    sdlfmt = mpg123_format_to_sdl(encoding);
    if (sdlfmt == (Uint16)-1)
    {
        SDL_SetError(
            "Format %s is not supported by SDL",
            mpg123_format_str(encoding)
       );
        return 0;
    }

    SDL_BuildAudioCVT(
        &m->cvt,
        sdlfmt, channels, (int)rate,
        m->mixer.format,
        m->mixer.channels,
        m->mixer.freq
   );

    if (m->cvt.buf) {
        SDL_free(m->cvt.buf);
    }

    bufsize = sizeof(m->buf) * m->cvt.len_mult;
    m->cvt.buf = SDL_malloc(bufsize);

    if (!m->cvt.buf)
    {
        SDL_SetError("Out of memory");
        m->playing = SDL_FALSE;
        return 0;
    }

    return 1;
}

/* read some mp3 stream data and convert it for output */
static int getsome(mpg_data* m)
{
    int code;
    size_t len;
    Uint8* data = m->buf;
    size_t cbdata = sizeof(m->buf);
    SDL_AudioCVT* cvt = &m->cvt;

    do
    {
        code = mpg123.mpg123_read(m->handle, data, sizeof(data), &len);
        switch (code)
        {
            case MPG123_NEW_FORMAT:
                if (!update_format(m)) {
                    return 0;
                }
                break;

            case MPG123_DONE:
                m->playing = SDL_FALSE;
                break;
            case MPG123_OK:
                break;

            default:
                SDL_SetError("mpg123_read: %s", mpg_err(m->handle, code));
                return 0;
        }
    }
    while (len && code != MPG123_OK);

    SDL_memcpy(cvt->buf, data, cbdata);

    if (cvt->needed) {
        /* we need to convert the audio to SDL's format */
        cvt->len = (int)len;
        SDL_ConvertAudio(cvt);
    }

    else {
        /* no conversion needed, just copy */
        cvt->len_cvt = (int)len;
    }

    m->len_available = cvt->len_cvt;
    m->snd_available = cvt->buf;

    return 1;
}

static int MPG123_GetAudio(void *context, void *data, int bytes)
{
    mpg_data* m = (mpg_data *)context;
    Uint8 *stream = (Uint8 *)data;
    int len = bytes;
    int mixable;

    while (len > 0 && m->playing)
    {
        if (!m->len_available)
        {
            if (!getsome(m)) {
                m->playing = SDL_FALSE;
                return len;
            }
        }

        mixable = len;

        if (mixable > m->len_available) {
            mixable = (int)m->len_available;
        }

        if (m->volume == MIX_MAX_VOLUME) {
            SDL_memcpy(stream, m->snd_available, mixable);
        }

        else
        {
            SDL_MixAudioFormat(
                stream,
                m->snd_available,
                m->mixer.format,
                mixable,
                m->volume
           );
        }

        m->len_available -= mixable;
        m->snd_available += mixable;
        len -= mixable;
        stream += mixable;
    }

    return len;
}

static int MPG123_Seek(void *context, double secs)
{
    mpg_data* m = (mpg_data *)context;
    off_t offset = m->mixer.freq * secs;

    if ((offset = mpg123.mpg123_seek(m->handle, offset, SEEK_SET)) < 0) {
        return Mix_SetError("mpg123_seek: %s", mpg_err(m->handle, (int)-offset));
    }
    return 0;
}

static void MPG123_Stop(void *context)
{
    mpg_data* m = (mpg_data *)context;
    m->playing = SDL_FALSE;
}

static void MPG123_Delete(void *context)
{
    mpg_data* m = (mpg_data *)context;

    if (m->freesrc) {
        SDL_RWclose(m->src);
    }

    if (m->cvt.buf) {
        SDL_free(m->cvt.buf);
    }

    mpg123.mpg123_close(m->handle);
    mpg123.mpg123_delete(m->handle);
    SDL_free(m);
}

static void MPG123_Close(void)
{
    mpg123.mpg123_exit();
}

Mix_MusicInterface Mix_MusicInterface_MPG123 =
{
    "MPG123",
    MIX_MUSIC_MPG123,
    MUS_MP3,
    SDL_FALSE,
    SDL_FALSE,

    MPG123_Load,
    MPG123_Open,
    MPG123_CreateFromRW,
    NULL,   /* CreateFromFile */
    MPG123_SetVolume,
    MPG123_Play,
    MPG123_IsPlaying,
    MPG123_GetAudio,
    MPG123_Seek,
    NULL,   /* Pause */
    NULL,   /* Resume */
    MPG123_Stop,
    MPG123_Delete,
    MPG123_Close,
    MPG123_Unload
};

#endif /* MUSIC_MP3_MPG123 */

/* vi: set ts=4 sw=4 expandtab: */
