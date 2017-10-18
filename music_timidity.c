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


static int TIMIDITY_Open(const SDL_AudioSpec *spec)
{
    return Timidity_Init();
}

static void TIMIDITY_Close(void)
{
    Timidity_Exit();
}

void *TIMIDITY_CreateFromRW(SDL_RWops *src, int freesrc)
{
    MidiSong *music = Timidity_LoadSong(src, &music_spec);
    if (music && freesrc) {
        SDL_RWclose(src);
    }
    return music;
}

static void TIMIDITY_SetVolume(void *context, int volume)
{
    MidiSong *music = (MidiSong *)context;
    Timidity_SetVolume(music, volume);
}

static int TIMIDITY_Play(void *context)
{
    MidiSong *music = (MidiSong *)context;
    Timidity_Start(music);
    return 0;
}

static int TIMIDITY_GetAudio(void *context, void *data, int bytes)
{
    MidiSong *music = (MidiSong *)context;
    if (!Timidity_PlaySome(music, data, bytes)) {
        /* Nothing consumed, everything left */
        return bytes;
    }
    return 0;
}

static int TIMIDITY_Seek(void *context, double position)
{
    MidiSong *music = (MidiSong *)context;
    Timidity_Seek(music, (Uint32)(position * 1000));
    return 0;
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
    NULL,   /* IsPlaying */
    TIMIDITY_GetAudio,
    TIMIDITY_Seek,
    NULL,   /* Pause */
    NULL,   /* Resume */
    NULL,   /* Stop */
    TIMIDITY_Delete,
    TIMIDITY_Close,
    NULL,   /* Unload */
};

#endif /* MUSIC_MID_TIMIDITY */

/* vi: set ts=4 sw=4 expandtab: */
