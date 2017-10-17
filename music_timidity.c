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

/* This file supports playing MIDI files with timidity */

#ifdef MUSIC_MID_TIMIDITY

#include "music_timidity.h"

#include "timidity/timidity.h"


static int samplesize;

static int TIMIDITY_Open(const SDL_AudioSpec *spec)
{
    samplesize = spec->size / spec->samples;
    if (Timidity_Init(spec->freq, spec->format, spec->channels, spec->samples) != 0) {
        Mix_SetError("%s", Timidity_Error());
        return -1;
    }
    return 0;
}

static void TIMIDITY_Close(void)
{
    Timidity_Close();
}

void *TIMIDITY_CreateFromRW(SDL_RWops *src, int freesrc)
{
    MidiSong *music = Timidity_LoadSong_RW(src, freesrc);
    if (!music) {
        Mix_SetError("%s", Timidity_Error());
    }
    return music;
}

static void TIMIDITY_SetVolume(void *context, int volume)
{
    Timidity_SetVolume(volume);
}

static int TIMIDITY_Play(void *context)
{
    MidiSong *music = (MidiSong *)context;
    Timidity_Start(music);
    return 0;
}

static SDL_bool TIMIDITY_IsPlaying(void *context)
{
    return Timidity_Active() ? SDL_TRUE : SDL_FALSE;
}

static int TIMIDITY_GetAudio(void *context, void *data, int bytes)
{
    int samples = (bytes / samplesize);
    Timidity_PlaySome(data, samples);
    return 0;
}

static void TIMIDITY_Stop(void *context)
{
    Timidity_Stop();
}

static void TIMIDITY_Delete(void *context)
{
    MidiSong *music = (MidiSong *)context;
    Timidity_FreeSong(music);
}

Mix_MusicInterface Mix_MusicInterface_TIMIDITY =
{
    "TIMIDITY",
    MIX_MUSIC_TIMIDITY,
    MUS_MID,
    SDL_FALSE,
    SDL_FALSE,

    NULL,   /* Load */
    TIMIDITY_Open,
    TIMIDITY_CreateFromRW,
    NULL,   /* CreateFromFile */
    TIMIDITY_SetVolume,
    TIMIDITY_Play,
    TIMIDITY_IsPlaying,
    TIMIDITY_GetAudio,
    NULL,   /* Seek */
    NULL,   /* Pause */
    NULL,   /* Resume */
    NULL,   /* Stop */
    TIMIDITY_Delete,
    TIMIDITY_Close,
    NULL,   /* Unload */
};

#endif /* MUSIC_MID_TIMIDITY */

/* vi: set ts=4 sw=4 expandtab: */
