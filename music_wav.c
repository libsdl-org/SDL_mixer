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

#ifdef MUSIC_WAV

/* This file supports streaming WAV files */

#include "music_wav.h"


typedef struct {
    SDL_bool active;
    Uint32 start;
    Uint32 stop;
    Uint32 initial_play_count;
    Uint32 current_play_count;
} WAVLoopPoint;

typedef struct {
    SDL_RWops *src;
    SDL_bool freesrc;
    SDL_AudioSpec spec;
    int volume;
    Sint64 start;
    Sint64 stop;
    SDL_AudioCVT cvt;
    int numloops;
    WAVLoopPoint *loops;
} WAVStream;

/*
    Taken with permission from SDL_wave.h, part of the SDL library,
    available at: http://www.libsdl.org/
    and placed under the same license as this mixer library.
*/

/* WAVE files are little-endian */

/*******************************************/
/* Define values for Microsoft WAVE format */
/*******************************************/
#define RIFF        0x46464952      /* "RIFF" */
#define WAVE        0x45564157      /* "WAVE" */
#define FMT         0x20746D66      /* "fmt " */
#define DATA        0x61746164      /* "data" */
#define SMPL        0x6c706d73      /* "smpl" */
#define PCM_CODE    1
#define ADPCM_CODE  2
#define WAVE_MONO   1
#define WAVE_STEREO 2

typedef struct {
/* Not saved in the chunk we read:
    Uint32  chunkID;
    Uint32  chunkLen;
*/
    Uint16  encoding;
    Uint16  channels;       /* 1 = mono, 2 = stereo */
    Uint32  frequency;      /* One of 11025, 22050, or 44100 Hz */
    Uint32  byterate;       /* Average bytes per second */
    Uint16  blockalign;     /* Bytes per sample block */
    Uint16  bitspersample;      /* One of 8, 12, 16, or 4 for ADPCM */
} WaveFMT;

typedef struct {
    Uint32 identifier;
    Uint32 type;
    Uint32 start;
    Uint32 end;
    Uint32 fraction;
    Uint32 play_count;
} SampleLoop;

typedef struct {
/* Not saved in the chunk we read:
    Uint32  chunkID;
    Uint32  chunkLen;
*/
    Uint32  manufacturer;
    Uint32  product;
    Uint32  sample_period;
    Uint32  MIDI_unity_note;
    Uint32  MIDI_pitch_fraction;
    Uint32  SMTPE_format;
    Uint32  SMTPE_offset;
    Uint32  sample_loops;
    Uint32  sampler_data;
    SampleLoop loops[];
} SamplerChunk;

/*********************************************/
/* Define values for AIFF (IFF audio) format */
/*********************************************/
#define FORM        0x4d524f46      /* "FORM" */
#define AIFF        0x46464941      /* "AIFF" */
#define SSND        0x444e5353      /* "SSND" */
#define COMM        0x4d4d4f43      /* "COMM" */


/* Function to load the WAV/AIFF stream */
static SDL_bool LoadWAVStream(WAVStream *wave);
static SDL_bool LoadAIFFStream(WAVStream *wave);


/* Load a WAV stream from the given RWops object */
static void *WAVStream_CreateFromRW(SDL_RWops *src, int freesrc)
{
    WAVStream *wave;
    SDL_bool loaded = SDL_FALSE;

    wave = (WAVStream *)SDL_calloc(1, sizeof(*wave));
    if (wave) {
        Uint32 magic;

        wave->src = src;
        wave->freesrc = freesrc;

        magic = SDL_ReadLE32(src);
        if (magic == RIFF || magic == WAVE) {
            loaded = LoadWAVStream(wave);
        } else if (magic == FORM) {
            loaded = LoadAIFFStream(wave);
        } else {
            Mix_SetError("Unknown WAVE format");
        }
        if (!loaded) {
            SDL_free(wave);
            return(NULL);
        }
        SDL_BuildAudioCVT(&wave->cvt,
            wave->spec.format, wave->spec.channels, wave->spec.freq,
            music_spec.format, music_spec.channels, music_spec.freq);
        wave->volume = MIX_MAX_VOLUME;
    } else {
        SDL_OutOfMemory();
    }
    return wave;
}

static void WAVStream_SetVolume(void *context, int volume)
{
    WAVStream *wave = (WAVStream *)context;
    wave->volume = volume;
}

/* Start playback of a given WAV stream */
static int WAVStream_Play(void *context)
{
    WAVStream *wave = (WAVStream *)context;
    int i;
    for (i = 0; i < wave->numloops; ++i) {
        WAVLoopPoint *loop = &wave->loops[i];
        loop->active = SDL_TRUE;
        loop->current_play_count = loop->initial_play_count;
    }
    SDL_RWseek(wave->src, wave->start, RW_SEEK_SET);
    return 0;
}

/* Play some of a stream previously started with WAVStream_Play() */
static int PlaySome(WAVStream *wave, Uint8 *stream, int len)
{
    Sint64 pos, stop;
    WAVLoopPoint *loop;
    Sint64 loop_start;
    Sint64 loop_stop;
    int i;
    int consumed;

    pos = SDL_RWtell(wave->src);
    stop = wave->stop;
    loop = NULL;
    for (i = 0; i < wave->numloops; ++i) {
        loop = &wave->loops[i];
        if (loop->active) {
            const int bytes_per_sample = (SDL_AUDIO_BITSIZE(wave->spec.format) / 8) * wave->spec.channels;
            loop_start = wave->start + loop->start * bytes_per_sample;
            loop_stop = wave->start + (loop->stop + 1) * bytes_per_sample;
            if (pos >= loop_start && pos < loop_stop)
            {
                stop = loop_stop;
                break;
            }
        }
        loop = NULL;
    }

    if (wave->cvt.needed) {
        int original_len = (int)((double)len/wave->cvt.len_ratio); 
        /* Make sure the length is a multiple of the sample size */
        {
            const int bytes_per_sample = (SDL_AUDIO_BITSIZE(wave->spec.format) / 8) * wave->spec.channels;
            const int alignment_mask = (bytes_per_sample - 1);
            original_len &= ~alignment_mask;
        }
        if (wave->cvt.len != original_len) {
            int worksize;
            if (wave->cvt.buf != NULL) {
                SDL_free(wave->cvt.buf);
            }
            worksize = original_len*wave->cvt.len_mult;
            wave->cvt.buf=(Uint8 *)SDL_malloc(worksize);
            if (wave->cvt.buf == NULL) {
                return 0;
            }
            wave->cvt.len = original_len;
        }
        if ((stop - pos) < original_len) {
            original_len = (int)(stop - pos);
        }
        original_len = (int)SDL_RWread(wave->src, wave->cvt.buf, 1, original_len);
        wave->cvt.len = original_len;
        SDL_ConvertAudio(&wave->cvt);
        SDL_MixAudioFormat(stream, wave->cvt.buf, music_spec.format, wave->cvt.len_cvt, wave->volume);
        consumed = wave->cvt.len_cvt;
    } else {
        Uint8 *data;
        if ((stop - pos) < len) {
            len = (int)(stop - pos);
        }
        data = SDL_stack_alloc(Uint8, len);
        if (data) {
            len = (int)SDL_RWread(wave->src, data, 1, len);
            SDL_MixAudioFormat(stream, data, music_spec.format, len, wave->volume);
            SDL_stack_free(data);
        }
        consumed = len;
    }

    if (loop && SDL_RWtell(wave->src) >= stop) {
        if (loop->current_play_count == 1) {
            loop->active = SDL_FALSE;
        } else {
            if (loop->current_play_count > 0) {
                --loop->current_play_count;
            }
            SDL_RWseek(wave->src, loop_start, RW_SEEK_SET);
        }
    }
    return consumed;
}

static int WAVStream_GetAudio(void *context, void *data, int bytes)
{
    WAVStream *wave = (WAVStream *)context;
    Uint8 *stream = (Uint8 *)data;
    int len = bytes;

    while ((SDL_RWtell(wave->src) < wave->stop) && (len > 0)) {
        int consumed = PlaySome(wave, stream, len);
        if (!consumed)
            break;

        stream += consumed;
        len -= consumed;
    }
    return len;
}

/* Close the given WAV stream */
static void WAVStream_Delete(void *context)
{
    WAVStream *wave = (WAVStream *)context;

    /* Clean up associated data */
    if (wave->loops) {
        SDL_free(wave->loops);
    }
    if (wave->cvt.buf) {
        SDL_free(wave->cvt.buf);
    }
    if (wave->freesrc) {
        SDL_RWclose(wave->src);
    }
    SDL_free(wave);
}

static SDL_bool ParseFMT(WAVStream *wave, Uint32 chunk_length)
{
    SDL_AudioSpec *spec = &wave->spec;
    WaveFMT *format;
    Uint8 *data;
    SDL_bool loaded = SDL_FALSE;

    if (chunk_length < sizeof(*format)) {
        Mix_SetError("Wave format chunk too small");
        return SDL_FALSE;
    }

    data = (Uint8 *)SDL_malloc(chunk_length);
    if (!data) {
        Mix_SetError("Out of memory");
        return SDL_FALSE;
    }
    if (!SDL_RWread(wave->src, data, chunk_length, 1)) {
        Mix_SetError("Couldn't read %d bytes from WAV file", chunk_length);
        return SDL_FALSE;
    }
    format = (WaveFMT *)data;

    /* Decode the audio data format */
    switch (SDL_SwapLE16(format->encoding)) {
        case PCM_CODE:
            /* We can understand this */
            break;
        default:
            Mix_SetError("Unknown WAVE data format");
            goto done;
    }
    spec->freq = SDL_SwapLE32(format->frequency);
    switch (SDL_SwapLE16(format->bitspersample)) {
        case 8:
            spec->format = AUDIO_U8;
            break;
        case 16:
            spec->format = AUDIO_S16;
            break;
        default:
            Mix_SetError("Unknown PCM data format");
            goto done;
    }
    spec->channels = (Uint8) SDL_SwapLE16(format->channels);
    spec->samples = 4096;       /* Good default buffer size */

    loaded = SDL_TRUE;

done:
    SDL_free(data);
    return loaded;
}

static SDL_bool ParseDATA(WAVStream *wave, Uint32 chunk_length)
{
    wave->start = SDL_RWtell(wave->src);
    wave->stop = wave->start + chunk_length;
    SDL_RWseek(wave->src, chunk_length, RW_SEEK_CUR);
    return SDL_TRUE;
}

static SDL_bool AddLoopPoint(WAVStream *wave, Uint32 play_count, Uint32 start, Uint32 stop)
{
    WAVLoopPoint *loop;
    WAVLoopPoint *loops = SDL_realloc(wave->loops, (wave->numloops + 1)*sizeof(*wave->loops));
    if (!loops) {
        Mix_SetError("Out of memory");
        return SDL_FALSE;
    }

    loop = &loops[ wave->numloops ];
    loop->start = start;
    loop->stop = stop;
    loop->initial_play_count = play_count;
    loop->current_play_count = play_count;

    wave->loops = loops;
    ++wave->numloops;
    return SDL_TRUE;
}

static SDL_bool ParseSMPL(WAVStream *wave, Uint32 chunk_length)
{
    SamplerChunk *chunk;
    Uint8 *data;
    Uint32 i;
    SDL_bool loaded = SDL_FALSE;

    data = (Uint8 *)SDL_malloc(chunk_length);
    if (!data) {
        Mix_SetError("Out of memory");
        return SDL_FALSE;
    }
    if (!SDL_RWread(wave->src, data, chunk_length, 1)) {
        Mix_SetError("Couldn't read %d bytes from WAV file", chunk_length);
        return SDL_FALSE;
    }
    chunk = (SamplerChunk *)data;

    for (i = 0; i < SDL_SwapLE32(chunk->sample_loops); ++i) {
        const Uint32 LOOP_TYPE_FORWARD = 0;
        Uint32 loop_type = SDL_SwapLE32(chunk->loops[i].type);
        if (loop_type == LOOP_TYPE_FORWARD) {
            AddLoopPoint(wave, SDL_SwapLE32(chunk->loops[i].play_count), SDL_SwapLE32(chunk->loops[i].start), SDL_SwapLE32(chunk->loops[i].end));
        }
    }

    loaded = SDL_TRUE;
    SDL_free(data);
    return loaded;
}

static SDL_bool LoadWAVStream(WAVStream *wave)
{
    SDL_RWops *src = wave->src;
    Uint32 chunk_type;
    Uint32 chunk_length;
    SDL_bool found_FMT = SDL_FALSE;
    SDL_bool found_DATA = SDL_FALSE;

    /* WAV magic header */
    Uint32 wavelen;
    Uint32 WAVEmagic;

    /* Check the magic header */
    wavelen = SDL_ReadLE32(src);
    WAVEmagic = SDL_ReadLE32(src);

    /* Read the chunks */
    for (; ;) {
        chunk_type = SDL_ReadLE32(src);
        chunk_length = SDL_ReadLE32(src);

        if (chunk_length == 0)
            break;

        switch (chunk_type)
        {
        case FMT:
            found_FMT = SDL_TRUE;
            if (!ParseFMT(wave, chunk_length))
                return SDL_FALSE;
            break;
        case DATA:
            found_DATA = SDL_TRUE;
            if (!ParseDATA(wave, chunk_length))
                return SDL_FALSE;
            break;
        case SMPL:
            if (!ParseSMPL(wave, chunk_length))
                return SDL_FALSE;
            break;
        default:
            SDL_RWseek(src, chunk_length, RW_SEEK_CUR);
            break;
        }
    }

    if (!found_FMT) {
        Mix_SetError("Bad WAV file (no FMT chunk)");
        return SDL_FALSE;
    }

    if (!found_DATA) {
        Mix_SetError("Bad WAV file (no DATA chunk)");
        return SDL_FALSE;
    }

    return SDL_TRUE;
}

/* I couldn't get SANE_to_double() to work, so I stole this from libsndfile.
 * I don't pretend to fully understand it.
 */

static Uint32 SANE_to_Uint32 (Uint8 *sanebuf)
{
    /* Negative number? */
    if (sanebuf[0] & 0x80)
        return 0;

    /* Less than 1? */
    if (sanebuf[0] <= 0x3F)
        return 1;

    /* Way too big? */
    if (sanebuf[0] > 0x40)
        return 0x4000000;

    /* Still too big? */
    if (sanebuf[0] == 0x40 && sanebuf[1] > 0x1C)
        return 800000000;

    return ((sanebuf[2] << 23) | (sanebuf[3] << 15) | (sanebuf[4] << 7) |
            (sanebuf[5] >> 1)) >> (29 - sanebuf[1]);
}

static SDL_bool LoadAIFFStream(WAVStream *wave)
{
    SDL_RWops *src = wave->src;
    SDL_AudioSpec *spec = &wave->spec;
    SDL_bool found_SSND = SDL_FALSE;
    SDL_bool found_COMM = SDL_FALSE;

    Uint32 chunk_type;
    Uint32 chunk_length;
    Sint64 next_chunk;

    /* AIFF magic header */
    Uint32 AIFFmagic;
    /* SSND chunk        */
    Uint32 offset;
    Uint32 blocksize;
    /* COMM format chunk */
    Uint16 channels = 0;
    Uint32 numsamples = 0;
    Uint16 samplesize = 0;
    Uint8 sane_freq[10];
    Uint32 frequency = 0;

    /* Check the magic header */
    chunk_length = SDL_ReadBE32(src);
    AIFFmagic = SDL_ReadLE32(src);
    if (AIFFmagic != AIFF) {
        Mix_SetError("Unrecognized file type (not AIFF)");
        return SDL_FALSE;
    }

    /* From what I understand of the specification, chunks may appear in
     * any order, and we should just ignore unknown ones.
     *
     * TODO: Better sanity-checking. E.g. what happens if the AIFF file
     *       contains compressed sound data?
     */
    do {
        chunk_type      = SDL_ReadLE32(src);
        chunk_length    = SDL_ReadBE32(src);
        next_chunk      = SDL_RWtell(src) + chunk_length;

        /* Paranoia to avoid infinite loops */
        if (chunk_length == 0)
            break;

        switch (chunk_type) {
        case SSND:
            found_SSND = SDL_TRUE;
            offset = SDL_ReadBE32(src);
            blocksize = SDL_ReadBE32(src);
            wave->start = SDL_RWtell(src) + offset;
            break;

        case COMM:
            found_COMM = SDL_TRUE;

            /* Read the audio data format chunk */
            channels = SDL_ReadBE16(src);
            numsamples = SDL_ReadBE32(src);
            samplesize = SDL_ReadBE16(src);
            SDL_RWread(src, sane_freq, sizeof(sane_freq), 1);
            frequency = SANE_to_Uint32(sane_freq);
            break;

        default:
            break;
        }
    } while ((!found_SSND || !found_COMM)
         && SDL_RWseek(src, next_chunk, RW_SEEK_SET) != -1);

    if (!found_SSND) {
        Mix_SetError("Bad AIFF file (no SSND chunk)");
        return SDL_FALSE;
    }

    if (!found_COMM) {
        Mix_SetError("Bad AIFF file (no COMM chunk)");
        return SDL_FALSE;
    }

    wave->stop = wave->start + channels * numsamples * (samplesize / 8);

    /* Decode the audio data format */
    SDL_memset(spec, 0, (sizeof *spec));
    spec->freq = frequency;
    switch (samplesize) {
        case 8:
            spec->format = AUDIO_S8;
            break;
        case 16:
            spec->format = AUDIO_S16MSB;
            break;
        default:
            Mix_SetError("Unknown samplesize in data format");
            return SDL_FALSE;
    }
    spec->channels = (Uint8) channels;
    spec->samples = 4096;       /* Good default buffer size */

    return SDL_TRUE;
}

Mix_MusicInterface Mix_MusicInterface_WAV =
{
    "WAVE",
    MIX_MUSIC_WAVE,
    MUS_WAV,
    SDL_FALSE,
    SDL_FALSE,

    NULL,   /* Load */
    NULL,   /* Open */
    WAVStream_CreateFromRW,
    NULL,   /* CreateFromFile */
    WAVStream_SetVolume,
    WAVStream_Play,
    NULL,   /* IsPlaying */
    WAVStream_GetAudio,
    NULL,   /* Seek */
    NULL,   /* Pause */
    NULL,   /* Resume */
    NULL,   /* Stop */
    WAVStream_Delete,
    NULL,   /* Close */
    NULL,   /* Unload */
};

#endif /* MUSIC_WAV */

/* vi: set ts=4 sw=4 expandtab: */
