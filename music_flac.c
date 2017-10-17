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

  This file is used to support SDL_LoadMUS playback of FLAC files.
    ~ Austen Dicken (admin@cvpcs.org)
*/

#ifdef MUSIC_FLAC

#include "SDL_loadso.h"

#include "music_flac.h"

#include <FLAC/stream_decoder.h>


typedef struct {
    int loaded;
    void *handle;
    FLAC__StreamDecoder *(*FLAC__stream_decoder_new)();
    void (*FLAC__stream_decoder_delete)(FLAC__StreamDecoder *decoder);
    FLAC__StreamDecoderInitStatus (*FLAC__stream_decoder_init_stream)(
                        FLAC__StreamDecoder *decoder,
                        FLAC__StreamDecoderReadCallback read_callback,
                        FLAC__StreamDecoderSeekCallback seek_callback,
                        FLAC__StreamDecoderTellCallback tell_callback,
                        FLAC__StreamDecoderLengthCallback length_callback,
                        FLAC__StreamDecoderEofCallback eof_callback,
                        FLAC__StreamDecoderWriteCallback write_callback,
                        FLAC__StreamDecoderMetadataCallback metadata_callback,
                        FLAC__StreamDecoderErrorCallback error_callback,
                        void *client_data);
    FLAC__bool (*FLAC__stream_decoder_finish)(FLAC__StreamDecoder *decoder);
    FLAC__bool (*FLAC__stream_decoder_flush)(FLAC__StreamDecoder *decoder);
    FLAC__bool (*FLAC__stream_decoder_process_single)(
                        FLAC__StreamDecoder *decoder);
    FLAC__bool (*FLAC__stream_decoder_process_until_end_of_metadata)(
                        FLAC__StreamDecoder *decoder);
    FLAC__bool (*FLAC__stream_decoder_process_until_end_of_stream)(
                        FLAC__StreamDecoder *decoder);
    FLAC__bool (*FLAC__stream_decoder_seek_absolute)(
                        FLAC__StreamDecoder *decoder,
                        FLAC__uint64 sample);
    FLAC__StreamDecoderState (*FLAC__stream_decoder_get_state)(
                        const FLAC__StreamDecoder *decoder);
} flac_loader;

static flac_loader flac = {
    0, NULL
};

#ifdef FLAC_DYNAMIC

static FLAC_Load(void)
{
    if (flac.loaded == 0) {
        flac.handle = SDL_LoadObject(FLAC_DYNAMIC);
        if (flac.handle == NULL) {
            return -1;
        }
        flac.FLAC__stream_decoder_new =
            (FLAC__StreamDecoder *(*)())
            SDL_LoadFunction(flac.handle, "FLAC__stream_decoder_new");
        if (flac.FLAC__stream_decoder_new == NULL) {
            SDL_UnloadObject(flac.handle);
            return -1;
        }
        flac.FLAC__stream_decoder_delete =
            (void (*)(FLAC__StreamDecoder *))
            SDL_LoadFunction(flac.handle, "FLAC__stream_decoder_delete");
        if (flac.FLAC__stream_decoder_delete == NULL) {
            SDL_UnloadObject(flac.handle);
            return -1;
        }
        flac.FLAC__stream_decoder_init_stream =
            (FLAC__StreamDecoderInitStatus (*)(
                        FLAC__StreamDecoder *,
                        FLAC__StreamDecoderReadCallback,
                        FLAC__StreamDecoderSeekCallback,
                        FLAC__StreamDecoderTellCallback,
                        FLAC__StreamDecoderLengthCallback,
                        FLAC__StreamDecoderEofCallback,
                        FLAC__StreamDecoderWriteCallback,
                        FLAC__StreamDecoderMetadataCallback,
                        FLAC__StreamDecoderErrorCallback,
                        void *))
            SDL_LoadFunction(flac.handle, "FLAC__stream_decoder_init_stream");
        if (flac.FLAC__stream_decoder_init_stream == NULL) {
            SDL_UnloadObject(flac.handle);
            return -1;
        }
        flac.FLAC__stream_decoder_finish =
            (FLAC__bool (*)(FLAC__StreamDecoder *))
            SDL_LoadFunction(flac.handle, "FLAC__stream_decoder_finish");
        if (flac.FLAC__stream_decoder_finish == NULL) {
            SDL_UnloadObject(flac.handle);
            return -1;
        }
        flac.FLAC__stream_decoder_flush =
            (FLAC__bool (*)(FLAC__StreamDecoder *))
            SDL_LoadFunction(flac.handle, "FLAC__stream_decoder_flush");
        if (flac.FLAC__stream_decoder_flush == NULL) {
            SDL_UnloadObject(flac.handle);
            return -1;
        }
        flac.FLAC__stream_decoder_process_single =
            (FLAC__bool (*)(FLAC__StreamDecoder *))
            SDL_LoadFunction(flac.handle,
                        "FLAC__stream_decoder_process_single");
        if (flac.FLAC__stream_decoder_process_single == NULL) {
            SDL_UnloadObject(flac.handle);
            return -1;
        }
        flac.FLAC__stream_decoder_process_until_end_of_metadata =
            (FLAC__bool (*)(FLAC__StreamDecoder *))
            SDL_LoadFunction(flac.handle,
                        "FLAC__stream_decoder_process_until_end_of_metadata");
        if (flac.FLAC__stream_decoder_process_until_end_of_metadata == NULL) {
            SDL_UnloadObject(flac.handle);
            return -1;
        }
        flac.FLAC__stream_decoder_process_until_end_of_stream =
            (FLAC__bool (*)(FLAC__StreamDecoder *))
            SDL_LoadFunction(flac.handle,
                        "FLAC__stream_decoder_process_until_end_of_stream");
        if (flac.FLAC__stream_decoder_process_until_end_of_stream == NULL) {
            SDL_UnloadObject(flac.handle);
            return -1;
        }
        flac.FLAC__stream_decoder_seek_absolute =
            (FLAC__bool (*)(FLAC__StreamDecoder *, FLAC__uint64))
            SDL_LoadFunction(flac.handle, "FLAC__stream_decoder_seek_absolute");
        if (flac.FLAC__stream_decoder_seek_absolute == NULL) {
            SDL_UnloadObject(flac.handle);
            return -1;
        }
        flac.FLAC__stream_decoder_get_state =
            (FLAC__StreamDecoderState (*)(const FLAC__StreamDecoder *decoder))
            SDL_LoadFunction(flac.handle, "FLAC__stream_decoder_get_state");
        if (flac.FLAC__stream_decoder_get_state == NULL) {
            SDL_UnloadObject(flac.handle);
            return -1;
        }
    }
    ++flac.loaded;

    return 0;
}

static void FLAC_Unload(void)
{
    if (flac.loaded == 0) {
        return;
    }
    if (flac.loaded == 1) {
        SDL_UnloadObject(flac.handle);
    }
    --flac.loaded;
}

#else /* !FLAC_DYNAMIC */

static int FLAC_Load(void)
{
    if (flac.loaded == 0) {
#ifdef __MACOSX__
        extern FLAC__StreamDecoder *FLAC__stream_decoder_new(void) __attribute__((weak_import));
        if (FLAC__stream_decoder_new == NULL)
        {
            /* Missing weakly linked framework */
            Mix_SetError("Missing FLAC.framework");
            return -1;
        }
#endif // __MACOSX__

        flac.FLAC__stream_decoder_new = FLAC__stream_decoder_new;
        flac.FLAC__stream_decoder_delete = FLAC__stream_decoder_delete;
        flac.FLAC__stream_decoder_init_stream =
                            FLAC__stream_decoder_init_stream;
        flac.FLAC__stream_decoder_finish = FLAC__stream_decoder_finish;
        flac.FLAC__stream_decoder_flush = FLAC__stream_decoder_flush;
        flac.FLAC__stream_decoder_process_single =
                            FLAC__stream_decoder_process_single;
        flac.FLAC__stream_decoder_process_until_end_of_metadata =
                            FLAC__stream_decoder_process_until_end_of_metadata;
        flac.FLAC__stream_decoder_process_until_end_of_stream =
                            FLAC__stream_decoder_process_until_end_of_stream;
        flac.FLAC__stream_decoder_seek_absolute =
                            FLAC__stream_decoder_seek_absolute;
        flac.FLAC__stream_decoder_get_state =
                            FLAC__stream_decoder_get_state;
    }
    ++flac.loaded;

    return 0;
}

static void FLAC_Unload(void)
{
    if (flac.loaded == 0) {
        return;
    }
    if (flac.loaded == 1) {
    }
    --flac.loaded;
}

#endif /* FLAC_DYNAMIC */


typedef struct {
    FLAC__uint64 sample_size;
    unsigned sample_rate;
    unsigned channels;
    unsigned bits_per_sample;
    FLAC__uint64 total_samples;

    // the following are used to handle the callback nature of the writer
    int max_to_read;
    char *data;             // pointer to beginning of data array
    int data_len;           // size of data array
    int data_read;          // amount of data array used
    char *overflow;         // pointer to beginning of overflow array
    int overflow_len;       // size of overflow array
    int overflow_read;      // amount of overflow array used
} FLAC_Data;

typedef struct {
    SDL_bool playing;
    int volume;
    int section;
    FLAC__StreamDecoder *flac_decoder;
    FLAC_Data flac_data;
    SDL_RWops *src;
    int freesrc;
    SDL_AudioCVT cvt;
    int len_available;
    Uint8 *snd_available;
} FLAC_music;


static FLAC__StreamDecoderReadStatus flac_read_music_cb(
                                    const FLAC__StreamDecoder *decoder,
                                    FLAC__byte buffer[],
                                    size_t *bytes,
                                    void *client_data)
{
    FLAC_music *data = (FLAC_music*)client_data;

    // make sure there is something to be reading
    if (*bytes > 0) {
        *bytes = SDL_RWread (data->src, buffer, sizeof (FLAC__byte), *bytes);

        if (*bytes == 0) { // error or no data was read (EOF)
            return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
        } else { // data was read, continue
            return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
        }
    } else {
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }
}

static FLAC__StreamDecoderSeekStatus flac_seek_music_cb(
                                    const FLAC__StreamDecoder *decoder,
                                    FLAC__uint64 absolute_byte_offset,
                                    void *client_data)
{
    FLAC_music *data = (FLAC_music*)client_data;

    if (SDL_RWseek (data->src, absolute_byte_offset, RW_SEEK_SET) < 0) {
        return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
    } else {
        return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
    }
}

static FLAC__StreamDecoderTellStatus flac_tell_music_cb(
                                    const FLAC__StreamDecoder *decoder,
                                    FLAC__uint64 *absolute_byte_offset,
                                    void *client_data)
{
    FLAC_music *data = (FLAC_music*)client_data;

    Sint64 pos = SDL_RWtell (data->src);

    if (pos < 0) {
        return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
    } else {
        *absolute_byte_offset = (FLAC__uint64)pos;
        return FLAC__STREAM_DECODER_TELL_STATUS_OK;
    }
}

static FLAC__StreamDecoderLengthStatus flac_length_music_cb (
                                    const FLAC__StreamDecoder *decoder,
                                    FLAC__uint64 *stream_length,
                                    void *client_data)
{
    FLAC_music *data = (FLAC_music*)client_data;

    Sint64 pos = SDL_RWtell (data->src);
    Sint64 length = SDL_RWseek (data->src, 0, RW_SEEK_END);

    if (SDL_RWseek (data->src, pos, RW_SEEK_SET) != pos || length < 0) {
        /* there was an error attempting to return the stream to the original
         * position, or the length was invalid. */
        return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
    } else {
        *stream_length = (FLAC__uint64)length;
        return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
    }
}

static FLAC__bool flac_eof_music_cb(
                                const FLAC__StreamDecoder *decoder,
                                void *client_data)
{
    FLAC_music *data = (FLAC_music*)client_data;

    Sint64 pos = SDL_RWtell (data->src);
    Sint64 end = SDL_RWseek (data->src, 0, RW_SEEK_END);

    // was the original position equal to the end (a.k.a. the seek didn't move)?
    if (pos == end) {
        // must be EOF
        return true;
    } else {
        // not EOF, return to the original position
        SDL_RWseek (data->src, pos, RW_SEEK_SET);
        return false;
    }
}

static FLAC__StreamDecoderWriteStatus flac_write_music_cb(
                                    const FLAC__StreamDecoder *decoder,
                                    const FLAC__Frame *frame,
                                    const FLAC__int32 *const buffer[],
                                    void *client_data)
{
    FLAC_music *data = (FLAC_music *)client_data;
    size_t i;

    if (data->flac_data.total_samples == 0) {
        SDL_SetError ("Given FLAC file does not specify its sample count.");
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    if (data->flac_data.channels != 2 ||
        data->flac_data.bits_per_sample != 16) {
        SDL_SetError("Current FLAC support is only for 16 bit Stereo files.");
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    for (i = 0; i < frame->header.blocksize; i++) {
        FLAC__int16 i16;
        FLAC__uint16 ui16;

        // make sure we still have at least two bytes that can be read (one for
        // each channel)
        if (data->flac_data.max_to_read >= 4) {
            // does the data block exist?
            if (!data->flac_data.data) {
                data->flac_data.data_len = data->flac_data.max_to_read;
                data->flac_data.data_read = 0;

                // create it
                data->flac_data.data =
                    (char *)SDL_malloc (data->flac_data.data_len);

                if (!data->flac_data.data) {
                    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
                }
            }

            i16 = (FLAC__int16)buffer[0][i];
            ui16 = (FLAC__uint16)i16;

            *((data->flac_data.data) + (data->flac_data.data_read++)) =
                                                            (char)(ui16);
            *((data->flac_data.data) + (data->flac_data.data_read++)) =
                                                            (char)(ui16 >> 8);

            i16 = (FLAC__int16)buffer[1][i];
            ui16 = (FLAC__uint16)i16;

            *((data->flac_data.data) + (data->flac_data.data_read++)) =
                                                            (char)(ui16);
            *((data->flac_data.data) + (data->flac_data.data_read++)) =
                                                            (char)(ui16 >> 8);

            data->flac_data.max_to_read -= 4;

            if (data->flac_data.max_to_read < 4) {
                // we need to set this so that the read halts from the
                // FLAC_getsome function.
                data->flac_data.max_to_read = 0;
            }
        } else {
            // we need to write to the overflow
            if (!data->flac_data.overflow) {
                data->flac_data.overflow_len = (int)(4 * (frame->header.blocksize - i));
                data->flac_data.overflow_read = 0;

                // make it big enough for the rest of the block
                data->flac_data.overflow =
                    (char *)SDL_malloc (data->flac_data.overflow_len);

                if (!data->flac_data.overflow) {
                    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
                }
            }

            i16 = (FLAC__int16)buffer[0][i];
            ui16 = (FLAC__uint16)i16;

            *((data->flac_data.overflow) + (data->flac_data.overflow_read++)) =
                                                            (char)(ui16);
            *((data->flac_data.overflow) + (data->flac_data.overflow_read++)) =
                                                            (char)(ui16 >> 8);

            i16 = (FLAC__int16)buffer[1][i];
            ui16 = (FLAC__uint16)i16;

            *((data->flac_data.overflow) + (data->flac_data.overflow_read++)) =
                                                            (char)(ui16);
            *((data->flac_data.overflow) + (data->flac_data.overflow_read++)) =
                                                            (char)(ui16 >> 8);
        }
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void flac_metadata_music_cb(
                    const FLAC__StreamDecoder *decoder,
                    const FLAC__StreamMetadata *metadata,
                    void *client_data)
{
    FLAC_music *data = (FLAC_music *)client_data;

    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        data->flac_data.sample_rate = metadata->data.stream_info.sample_rate;
        data->flac_data.channels = metadata->data.stream_info.channels;
        data->flac_data.total_samples =
                            metadata->data.stream_info.total_samples;
        data->flac_data.bits_per_sample =
                            metadata->data.stream_info.bits_per_sample;
        data->flac_data.sample_size = data->flac_data.channels *
                                        ((data->flac_data.bits_per_sample) / 8);
    }
}

static void flac_error_music_cb(
                const FLAC__StreamDecoder *decoder,
                FLAC__StreamDecoderErrorStatus status,
                void *client_data)
{
    // print an SDL error based on the error status
    switch (status) {
        case FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC:
            SDL_SetError ("Error processing the FLAC file [LOST_SYNC].");
        break;
        case FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER:
            SDL_SetError ("Error processing the FLAC file [BAD_HEADER].");
        break;
        case FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH:
            SDL_SetError ("Error processing the FLAC file [CRC_MISMATCH].");
        break;
        case FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM:
            SDL_SetError ("Error processing the FLAC file [UNPARSEABLE].");
        break;
        default:
            SDL_SetError ("Error processing the FLAC file [UNKNOWN].");
        break;
    }
}

/* Load an FLAC stream from an SDL_RWops object */
static void *FLAC_CreateFromRW(SDL_RWops *src, int freesrc)
{
    FLAC_music *music;
    int init_stage = 0;
    int was_error = 1;

    if (!Mix_Init(MIX_INIT_FLAC)) {
        return NULL;
    }

    music = (FLAC_music *)SDL_calloc(1, sizeof (*music));
    if (music) {
        /* Initialize the music structure */
        music->volume = MIX_MAX_VOLUME;
        music->section = -1;
        music->src = src;
        music->freesrc = freesrc;
        music->flac_data.max_to_read = 0;
        music->flac_data.overflow = NULL;
        music->flac_data.overflow_len = 0;
        music->flac_data.overflow_read = 0;
        music->flac_data.data = NULL;
        music->flac_data.data_len = 0;
        music->flac_data.data_read = 0;

        init_stage++; // stage 1!

        music->flac_decoder = flac.FLAC__stream_decoder_new ();

        if (music->flac_decoder != NULL) {
            init_stage++; // stage 2!

            if (flac.FLAC__stream_decoder_init_stream(
                        music->flac_decoder,
                        flac_read_music_cb, flac_seek_music_cb,
                        flac_tell_music_cb, flac_length_music_cb,
                        flac_eof_music_cb, flac_write_music_cb,
                        flac_metadata_music_cb, flac_error_music_cb,
                        music) == FLAC__STREAM_DECODER_INIT_STATUS_OK) {
                init_stage++; // stage 3!

                if (flac.FLAC__stream_decoder_process_until_end_of_metadata
                                        (music->flac_decoder)) {
                    was_error = 0;
                } else {
                    SDL_SetError("FLAC__stream_decoder_process_until_end_of_metadata() failed");
                }
            } else {
                SDL_SetError("FLAC__stream_decoder_init_stream() failed");
            }
        } else {
            SDL_SetError("FLAC__stream_decoder_new() failed");
        }

        if (was_error) {
            switch (init_stage) {
                case 3:
                    flac.FLAC__stream_decoder_finish(music->flac_decoder);
                case 2:
                    flac.FLAC__stream_decoder_delete(music->flac_decoder);
                case 1:
                case 0:
                    SDL_free(music);
                    break;
            }
            return NULL;
        }
    } else {
        SDL_OutOfMemory();
    }
    return music;
}

/* Set the volume for an FLAC stream */
static void FLAC_SetVolume(void *context, int volume)
{
    FLAC_music *music = (FLAC_music *)context;
    music->volume = volume;
}

/* Start playback of a given FLAC stream */
static int FLAC_Play(void *context)
{
    FLAC_music *music = (FLAC_music *)context;
    music->playing = SDL_TRUE;
    return 0;
}

/* Return non-zero if a stream is currently playing */
static SDL_bool FLAC_IsPlaying(void *context)
{
    FLAC_music *music = (FLAC_music *)context;
    return music->playing;
}

/* Read some FLAC stream data and convert it for output */
static void FLAC_getsome(FLAC_music *music)
{
    SDL_AudioCVT *cvt;

    /* GET AUDIO WAVE DATA */
    // set the max number of characters to read
    music->flac_data.max_to_read = 8192;
    music->flac_data.data_len = music->flac_data.max_to_read;
    music->flac_data.data_read = 0;
    if (!music->flac_data.data) {
        music->flac_data.data = (char *)SDL_malloc (music->flac_data.data_len);
    }

    // we have data to read
    while(music->flac_data.max_to_read > 0) {
        // first check if there is data in the overflow from before
        if (music->flac_data.overflow) {
            size_t overflow_len = music->flac_data.overflow_read;

            if (overflow_len > (size_t)music->flac_data.max_to_read) {
                size_t overflow_extra_len = overflow_len -
                                                music->flac_data.max_to_read;

                SDL_memcpy (music->flac_data.data+music->flac_data.data_read,
                    music->flac_data.overflow, music->flac_data.max_to_read);
                music->flac_data.data_read += music->flac_data.max_to_read;
                SDL_memcpy (music->flac_data.overflow,
                    music->flac_data.overflow + music->flac_data.max_to_read,
                    overflow_extra_len);
                music->flac_data.overflow_len = (int)overflow_extra_len;
                music->flac_data.overflow_read = (int)overflow_extra_len;
                music->flac_data.max_to_read = 0;
            } else {
                SDL_memcpy (music->flac_data.data+music->flac_data.data_read,
                    music->flac_data.overflow, overflow_len);
                music->flac_data.data_read += (int)overflow_len;
                SDL_free (music->flac_data.overflow);
                music->flac_data.overflow = NULL;
                music->flac_data.overflow_len = 0;
                music->flac_data.overflow_read = 0;
                music->flac_data.max_to_read -= (int)overflow_len;
            }
        } else {
            if (!flac.FLAC__stream_decoder_process_single (
                                                        music->flac_decoder)) {
                music->flac_data.max_to_read = 0;
            }

            if (flac.FLAC__stream_decoder_get_state (music->flac_decoder)
                                    == FLAC__STREAM_DECODER_END_OF_STREAM) {
                music->flac_data.max_to_read = 0;
            }
        }
    }

    if (music->flac_data.data_read <= 0) {
        if (music->flac_data.data_read == 0) {
            music->playing = SDL_FALSE;
        }
        return;
    }
    cvt = &music->cvt;
    if (music->section < 0) {
        SDL_BuildAudioCVT (cvt, AUDIO_S16, (Uint8)music->flac_data.channels,
                        (int)music->flac_data.sample_rate,
                        music_spec.format,
                        music_spec.channels,
                        music_spec.freq);
        if (cvt->buf) {
            SDL_free (cvt->buf);
        }
        cvt->buf = (Uint8 *)SDL_malloc (music->flac_data.data_len * cvt->len_mult);
        music->section = 0;
    }
    if (cvt->buf) {
        SDL_memcpy (cvt->buf, music->flac_data.data, music->flac_data.data_read);
        if (cvt->needed) {
            cvt->len = music->flac_data.data_read;
            SDL_ConvertAudio (cvt);
        } else {
            cvt->len_cvt = music->flac_data.data_read;
        }
        music->len_available = music->cvt.len_cvt;
        music->snd_available = music->cvt.buf;
    } else {
        SDL_SetError ("Out of memory");
        music->playing = SDL_FALSE;
    }
}

/* Play some of a stream previously started with FLAC_play() */
static int FLAC_GetAudio(void *context, void *data, int bytes)
{
    FLAC_music *music = (FLAC_music *)context;
    Uint8 *snd = (Uint8 *)data;
    int len = bytes;
    int mixable;

    while ((len > 0) && music->playing) {
        if (!music->len_available) {
            FLAC_getsome (music);
        }
        mixable = len;
        if (mixable > music->len_available) {
            mixable = music->len_available;
        }
        if (music->volume == MIX_MAX_VOLUME) {
            SDL_memcpy (snd, music->snd_available, mixable);
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

/* Jump (seek) to a given position (position is in seconds) */
static int FLAC_Seek(void *context, double position)
{
    FLAC_music *music = (FLAC_music *)context;
    double seek_sample = music->flac_data.sample_rate * position;

    // clear data if it has data
    if (music->flac_data.data) {
        SDL_free (music->flac_data.data);
        music->flac_data.data = NULL;
    }

    // clear overflow if it has data
    if (music->flac_data.overflow) {
        SDL_free (music->flac_data.overflow);
        music->flac_data.overflow = NULL;
    }

    if (!flac.FLAC__stream_decoder_seek_absolute (music->flac_decoder,
                                        (FLAC__uint64)seek_sample)) {
        if (flac.FLAC__stream_decoder_get_state (music->flac_decoder)
                                == FLAC__STREAM_DECODER_SEEK_ERROR) {
            flac.FLAC__stream_decoder_flush (music->flac_decoder);
        }

        SDL_SetError("Seeking of FLAC stream failed: libFLAC seek failed.");
        return -1;
    }
    return 0;
}

/* Stop playback of a stream previously started with FLAC_play() */
static void FLAC_Stop(void *context)
{
    FLAC_music *music = (FLAC_music *)context;
    music->playing = SDL_FALSE;
}

/* Close the given FLAC_music object */
static void FLAC_Delete(void *context)
{
    FLAC_music *music = (FLAC_music *)context;
    if (music) {
        if (music->flac_decoder) {
            flac.FLAC__stream_decoder_finish (music->flac_decoder);
            flac.FLAC__stream_decoder_delete (music->flac_decoder);
        }

        if (music->flac_data.data) {
            SDL_free (music->flac_data.data);
        }

        if (music->flac_data.overflow) {
            SDL_free (music->flac_data.overflow);
        }

        if (music->cvt.buf) {
            SDL_free (music->cvt.buf);
        }

        if (music->freesrc) {
            SDL_RWclose(music->src);
        }
        SDL_free (music);
    }
}

Mix_MusicInterface Mix_MusicInterface_FLAC =
{
    "FLAC",
    MIX_MUSIC_FLAC,
    MUS_FLAC,
    SDL_FALSE,
    SDL_FALSE,

    FLAC_Load,
    NULL,   /* Open */
    FLAC_CreateFromRW,
    NULL,   /* CreateFromFile */
    FLAC_SetVolume,
    FLAC_Play,
    FLAC_IsPlaying,
    FLAC_GetAudio,
    FLAC_Seek,
    NULL,   /* Pause */
    NULL,   /* Resume */
    FLAC_Stop,
    FLAC_Delete,
    NULL,   /* Close */
    FLAC_Unload,
};

#endif /* MUSIC_FLAC */

/* vi: set ts=4 sw=4 expandtab: */
