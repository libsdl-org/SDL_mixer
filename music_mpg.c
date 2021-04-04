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

#ifdef MP3_MUSIC

#include "SDL_mixer.h"
#include "music_mpg.h"
#include "dynamic_mp3.h"

static Uint16
mpg123_format_to_sdl(int fmt)
{
    switch (fmt) {
        case MPG123_ENC_UNSIGNED_8:     return AUDIO_U8;
        case MPG123_ENC_UNSIGNED_16:    return AUDIO_U16SYS;
        case MPG123_ENC_SIGNED_8:       return AUDIO_S8;
        case MPG123_ENC_SIGNED_16:      return AUDIO_S16SYS;
    }
    return -1;
}

static char const*
mpg123_format_str(int fmt)
{
    switch (fmt) {
#define f(x) case x: return #x;
        f(MPG123_ENC_UNSIGNED_8)
        f(MPG123_ENC_UNSIGNED_16)
        f(MPG123_ENC_SIGNED_8)
        f(MPG123_ENC_SIGNED_16)
#undef f
    }
    return "unknown";
}

static char const*
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
    return (ssize_t)MP3_RWread((struct mp3file_t *)p, dst, 1, n);
}

static
off_t rwops_seek(void* p, off_t offset, int whence) {
    return (off_t)MP3_RWseek((struct mp3file_t *)p, (int)offset, whence);
}

static
void rwops_cleanup(void* p) {
    (void)p;
    /* do nothing, we will free the file later */
}

static int getsome(mpg_data* m);

mpg_data*
mpg_new_rw(SDL_RWops *src, SDL_AudioSpec* mixer, int freesrc)
{
    int result;
    mpg_data* m = NULL;
    const long *rates;
    size_t i, num_rates;

    if (!Mix_Init(MIX_INIT_MP3)) {
        goto fail;
    }

    m = (mpg_data*)SDL_calloc(1, sizeof(mpg_data));
    if (!m) {
        SDL_OutOfMemory();
        goto fail;
    }

    m->freesrc = freesrc;

    if (MP3_RWinit(&m->mp3file, src) < 0) {
        goto fail;
    }
    if (mp3_skiptags(&m->mp3file) < 0) {
        Mix_SetError("music_mpg: corrupt mp3 file (bad tags.)");
        goto fail;
    }

    m->handle = mpg123.mpg123_new(0, &result);
    if (result != MPG123_OK) {
        goto fail;
    }

    result = mpg123.mpg123_replace_reader_handle(
        m->handle,
        rwops_read, rwops_seek, rwops_cleanup
    );
    if (result != MPG123_OK) {
        goto fail;
    }

    result = mpg123.mpg123_format_none(m->handle);
    if (result != MPG123_OK) {
        goto fail;
    }

    mpg123.mpg123_rates(&rates, &num_rates);
    for (i = 0; i < num_rates; ++i) {
        const int channels = (MPG123_MONO|MPG123_STEREO);
        const int formats = (MPG123_ENC_SIGNED_8 |
                             MPG123_ENC_UNSIGNED_8 |
                             MPG123_ENC_SIGNED_16 |
                             MPG123_ENC_UNSIGNED_16);

        mpg123.mpg123_format(m->handle, rates[i], channels, formats);
    }

    result = mpg123.mpg123_open_handle(m->handle, &m->mp3file);
    if (result != MPG123_OK) {
        goto fail;
    }

    m->volume = MIX_MAX_VOLUME;
    m->mixer = *mixer;

    /* hacky: read until we can figure out the format then rewind */
    while (!m->gotformat) {
        if (!getsome(m)) {
            goto fail;
        }
    }

    /* rewind */
    mpg123.mpg123_seek(m->handle, 0, SEEK_SET);

    m->len_available = 0;
    m->snd_available = m->cvt.buf;

    return m;

    fail:
    if (!m && freesrc) {
        SDL_RWclose(src);
    }
    mpg_delete(m);
    return NULL;
}

void
mpg_delete(mpg_data* m)
{
    if (!m) return;

    if (m->freesrc) {
        SDL_RWclose(m->mp3file.rw);
    }
    if (m->cvt.buf) {
        SDL_free(m->cvt.buf);
    }
    mpg123.mpg123_close(m->handle);
    mpg123.mpg123_delete(m->handle);
    SDL_free(m);
}

void
mpg_start(mpg_data* m) {
    m->playing = 1;
}

void
mpg_stop(mpg_data* m) {
    m->playing = 0;
}

int
mpg_playing(mpg_data* m) {
    return m->playing;
}

/*
    updates the convert struct and buffer to match the format queried from
    mpg123.
*/
static int
update_format(mpg_data* m)
{
    int result, channels, encoding;
    long rate;
    Uint16 sdlfmt;
    size_t bufsize;

    m->gotformat = 1;

    result = mpg123.mpg123_getformat(m->handle, &rate, &channels, &encoding);
    if (result != MPG123_OK) {
        Mix_SetError("mpg123_getformat: %s", mpg_err(m->handle, result));
        return 0;
    }

    sdlfmt = mpg123_format_to_sdl(encoding);
    if (sdlfmt == (Uint16)-1) {
        Mix_SetError("Format %s is not supported by SDL", mpg123_format_str(encoding));
        return 0;
    }

    SDL_BuildAudioCVT(&m->cvt, sdlfmt, channels, rate,
                               m->mixer.format, m->mixer.channels, m->mixer.freq);
    if (m->cvt.buf) {
        SDL_free(m->cvt.buf);
    }
    bufsize = sizeof(m->buf) * m->cvt.len_mult;
    m->cvt.buf = SDL_malloc(bufsize);
    if (!m->cvt.buf) {
        Mix_SetError("Out of memory");
        mpg_stop(m);
        return 0;
    }

    return 1;
}

/* read some mp3 stream data and convert it for output */
static int
getsome(mpg_data* m)
{
    int code;
    size_t len = 0;
    Uint8* data = m->buf;
    const size_t cbdata = sizeof(m->buf);
    SDL_AudioCVT* cvt = &m->cvt;

    code = mpg123.mpg123_read(m->handle, data, cbdata, &len);
    switch (code) {
    case MPG123_NEW_FORMAT:
        if (!update_format(m)) {
            return 0;
        }
        break;

    case MPG123_DONE:
        mpg_stop(m);
    case MPG123_OK:
        break;

    default:
        Mix_SetError("mpg123_read: %s", mpg_err(m->handle, code));
        return 0;
    }

    if (!cvt->buf) {
        /* not received MPG123_NEW_FORMAT... */
        Mix_SetError("music_mpg: No format.");
        return 0;
    }

    SDL_memcpy(cvt->buf, data, cbdata);

    if (cvt->needed) {
        /* we need to convert the audio to SDL's format */
        cvt->len = len;
        SDL_ConvertAudio(cvt);
    }
    else {
        /* no conversion needed, just copy */
        cvt->len_cvt = len;
    }

    m->len_available = cvt->len_cvt;
    m->snd_available = cvt->buf;

    return 1;
}

int
mpg_get_samples(mpg_data* m, Uint8 *stream, int len)
{
    int mixable;

    while (len > 0 && m->playing) {

        if (!m->len_available) {
            if (!getsome(m)) {
                m->playing = 0;
                return len;
            }
        }

        mixable = len;

        if (mixable > m->len_available) {
            mixable = m->len_available;
        }

        if (m->volume == MIX_MAX_VOLUME) {
            SDL_memcpy(stream, m->snd_available, mixable);
        }
        else {
            SDL_MixAudio(stream, m->snd_available, mixable, m->volume);
        }

        m->len_available -= mixable;
        m->snd_available += mixable;
        len -= mixable;
        stream += mixable;
    }

    return len;
}

void
mpg_seek(mpg_data* m, double secs)
{
    off_t offset = m->mixer.freq * secs;

    if ((offset = mpg123.mpg123_seek(m->handle, offset, SEEK_SET)) < 0) {
        Mix_SetError("mpg123_seek: %s", mpg_err(m->handle, -offset));
    }
}

void
mpg_volume(mpg_data* m, int volume)
{
    m->volume = volume;
}

#endif    /* MP3_MUSIC */
