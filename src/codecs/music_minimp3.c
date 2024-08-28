/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2024 Sam Lantinga <slouken@libsdl.org>

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

#ifdef MUSIC_MP3_MINIMP3

#include "music_minimp3.h"
#include "mp3utils.h"

#define MINIMP3_IMPLEMENTATION
#define MINIMP3_NO_STDIO
#include "minimp3/minimp3_ex.h"


typedef struct {
    struct mp3file_t file;
    int play_count;
    int closeio;
    mp3dec_ex_t dec;
    mp3dec_io_t io;
    int volume;
    int status;
    SDL_AudioStream *stream;
    mp3d_sample_t *buffer;
    int buffer_size;
    uint64_t second_length;
    int channels;

    Mix_MusicMetaTags tags;
} MiniMP3_Music;

static size_t MiniMP3_ReadCB(void *buf, size_t size, void *context)
{
    MiniMP3_Music *music = (MiniMP3_Music *)context;
    return MP3_IOread(&music->file, buf, 1, size);
}

static int MiniMP3_SeekCB(uint64_t position, void *context)
{
    MiniMP3_Music *music = (MiniMP3_Music *)context;
    if (MP3_IOseek(&music->file, position, SDL_IO_SEEK_SET) < 0) {
        return -1;
    }
    return 0;
}

static int MINIMP3_Seek(void *context, double position);

static void *MINIMP3_CreateFromIO(SDL_IOStream *src, SDL_bool closeio)
{
    MiniMP3_Music *music;
    SDL_AudioSpec file_spec;

    music = (MiniMP3_Music *)SDL_calloc(1, sizeof(MiniMP3_Music));
    if (!music) {
        return NULL;
    }
    music->volume = MIX_MAX_VOLUME;

    if (MP3_IOinit(&music->file, src) < 0) {
        SDL_free(music);
        return NULL;
    }

    meta_tags_init(&music->tags);
    if (mp3_read_tags(&music->tags, &music->file, SDL_FALSE) < 0) {
        SDL_free(music);
        SDL_SetError("music_minimp3: corrupt mp3 file (bad tags).");
        return NULL;
    }

    music->io.read = MiniMP3_ReadCB;
    music->io.read_data = music;
    music->io.seek = MiniMP3_SeekCB;
    music->io.seek_data = music;

    MP3_IOseek(&music->file, 0, SDL_IO_SEEK_SET);

    if (mp3dec_ex_open_cb(&music->dec, &music->io, MP3D_SEEK_TO_SAMPLE) != 0) {
        mp3dec_ex_close(&music->dec);
        SDL_free(music);
        SDL_SetError("music_minimp3: corrupt mp3 file (bad stream).");
        return NULL;
    }

    SDL_zero(file_spec);
    file_spec.format = SDL_AUDIO_S16;
    file_spec.channels = (Uint8)music->dec.info.channels;
    file_spec.freq = (int)music->dec.info.hz;
    music->stream = SDL_CreateAudioStream(&file_spec, &music_spec);
    if (!music->stream) {
        mp3dec_ex_close(&music->dec);
        SDL_free(music);
        return NULL;
    }

    music->channels = music->dec.info.channels;
    music->second_length = music->channels * music->dec.info.hz;
    music->buffer_size = 4096/*music_spec.samples*/ * sizeof(mp3d_sample_t) * music->channels;
    music->buffer = (mp3d_sample_t*)SDL_calloc(1, music->buffer_size);
    if (!music->buffer) {
        mp3dec_ex_close(&music->dec);
        SDL_free(music);
        return NULL;
    }

    music->closeio = closeio;
    return music;
}

static void MINIMP3_SetVolume(void *context, int volume)
{
    MiniMP3_Music *music = (MiniMP3_Music *)context;
    music->volume = volume;
}

static int MINIMP3_GetVolume(void *context)
{
    MiniMP3_Music *music = (MiniMP3_Music *)context;
    return music->volume;
}

/* Starts the playback. */
static int MINIMP3_Play(void *context, int play_count)
{
    MiniMP3_Music *music = (MiniMP3_Music *)context;
    music->play_count = play_count;
    return MINIMP3_Seek(music, 0.0);
}

static void MINIMP3_Stop(void *context)
{
    MiniMP3_Music *music = (MiniMP3_Music *)context;
    SDL_ClearAudioStream(music->stream);
}

static int MINIMP3_GetSome(void *context, void *data, int bytes, SDL_bool *done)
{
    MiniMP3_Music *music = (MiniMP3_Music *)context;
    int filled, amount;

    if (music->stream) {
        filled = SDL_GetAudioStreamData(music->stream, data, bytes);
        if (filled != 0) {
            return filled;
        }
    }

    if (!music->play_count) {
        /* All done */
        *done = SDL_TRUE;
        return 0;
    }

    amount = (int)mp3dec_ex_read(&music->dec, music->buffer, 4096/*music_spec.samples*/ * music->channels);
    if (amount > 0) {
        if (!SDL_PutAudioStreamData(music->stream, music->buffer, (int)amount * sizeof(mp3d_sample_t))) {
            return -1;
        }
    } else {
        if (music->play_count == 1) {
            music->play_count = 0;
            SDL_FlushAudioStream(music->stream);
        } else {
            int play_count = -1;
            if (music->play_count > 0) {
                play_count = (music->play_count - 1);
            }
            if (MINIMP3_Play(music, play_count) < 0) {
                return -1;
            }
        }
    }

    return 0;
}

static int MINIMP3_GetAudio(void *context, void *data, int bytes)
{
    MiniMP3_Music *music = (MiniMP3_Music *)context;
    return music_pcm_getaudio(context, data, bytes, music->volume, MINIMP3_GetSome);
}

static int MINIMP3_Seek(void *context, double position)
{
    MiniMP3_Music *music = (MiniMP3_Music *)context;
    uint64_t destpos = (uint64_t)(position * music->second_length);
    if (destpos % music->channels != 0) {
        destpos -= destpos % music->channels;
    }
    mp3dec_ex_seek(&music->dec, destpos);
    return 0;
}

static double MINIMP3_Tell(void *context)
{
    MiniMP3_Music *music = (MiniMP3_Music *)context;
    return (double)music->dec.cur_sample / music->second_length;
}

static double MINIMP3_Duration(void *context)
{
    MiniMP3_Music *music = (MiniMP3_Music *)context;
    return (double)music->dec.samples / music->second_length;
}

static const char* MINIMP3_GetMetaTag(void *context, Mix_MusicMetaTag tag_type)
{
    MiniMP3_Music *music = (MiniMP3_Music *)context;
    return meta_tags_get(&music->tags, tag_type);
}

static void MINIMP3_Delete(void *context)
{
    MiniMP3_Music *music = (MiniMP3_Music *)context;

    mp3dec_ex_close(&music->dec);
    meta_tags_clear(&music->tags);

    if (music->stream) {
        SDL_DestroyAudioStream(music->stream);
    }
    if (music->buffer) {
        SDL_free(music->buffer);
    }
    if (music->closeio) {
        SDL_CloseIO(music->file.src);
    }
    SDL_free(music);
}

Mix_MusicInterface Mix_MusicInterface_MINIMP3 =
{
    "MINIMP3",
    MIX_MUSIC_MINIMP3,
    MUS_MP3,
    SDL_FALSE,
    SDL_FALSE,

    NULL,   /* Load */
    NULL,   /* Open */
    MINIMP3_CreateFromIO,
    NULL,   /* CreateFromFile */
    MINIMP3_SetVolume,
    MINIMP3_GetVolume,
    MINIMP3_Play,
    NULL,   /* IsPlaying */
    MINIMP3_GetAudio,
    NULL,   /* Jump */
    MINIMP3_Seek,
    MINIMP3_Tell,
    MINIMP3_Duration,
    NULL,   /* LoopStart */
    NULL,   /* LoopEnd */
    NULL,   /* LoopLength */
    MINIMP3_GetMetaTag,
    NULL,   /* GetNumTracks */
    NULL,   /* StartTrack */
    NULL,   /* Pause */
    NULL,   /* Resume */
    MINIMP3_Stop,
    MINIMP3_Delete,
    NULL,   /* Close */
    NULL    /* Unload */
};

#endif /* MUSIC_MP3_MINIMP3 */

/* vi: set ts=4 sw=4 expandtab: */
