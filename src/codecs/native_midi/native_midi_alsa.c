/*
  native_midi: Linux (ALSA) native MIDI for the SDL_mixer library
  Copyright (C) 2024 Simon Howard

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
#include "SDL_config.h"
#ifdef __LINUX__

#include "native_midi.h"
#include "native_midi_common.h"

struct _NativeMidiSong {
    Uint16 division;
    MIDIEvent *event_list;
};

static NativeMidiSong *currentsong;

int native_midi_detect(void)
{
    return 0;
}

NativeMidiSong *native_midi_loadsong_RW(SDL_RWops *src, int freesrc)
{
    NativeMidiSong *result = SDL_malloc(sizeof(NativeMidiSong));
    if (result == NULL) {
        return NULL;
    }

    result->event_list = CreateMIDIEventList(src, &result->division);
    if (result->event_list == NULL) {
        SDL_free(result);
        return NULL;
    }

    return result;
}

void native_midi_freesong(NativeMidiSong *song)
{
    FreeMIDIEventList(song->event_list);
    SDL_free(song);
}

void native_midi_start(NativeMidiSong *song, int loops)
{
}

void native_midi_pause(void)
{
}

void native_midi_resume(void)
{
}

void native_midi_stop(void)
{
}

int native_midi_active(void)
{
    return 0;
}

void native_midi_setvolume(int volume)
{
}

const char *native_midi_error(void)
{
    return "";
}

#endif /* #ifdef __LINUX__ */
