/*
  native_midi:  Hardware Midi support on Atari for the SDL_mixer library
  Copyright (C) 2026  Miro Kropacek <miro.kropacek@gmail.com>

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

#ifdef __MINT__

#include <stdlib.h>

#include <umidi.h>

#include "../SDL_mixer.h"

#include "native_midi.h"
#include "native_midi_common.h"

#define DEFAULT_TEMPO   500000UL    /* us per quarter note (120 bpm) */

struct _NativeMidiSong
{
    UMidiSong song;
};

static UMidiContext s_ctx;
static int s_volume = MIX_MAX_VOLUME;

static int umidi_volume(int volume)
{
    return volume * UMIDI_MAX_VOLUME / MIX_MAX_VOLUME;
}

/* Builds the blocks of a song from the event list: channel messages only,
 * their tick times turned into microseconds with the tempo changes applied.
 * An empty block at the time of the last event (end of track) keeps the
 * length of the song. */
static int convert_events(UMidiSong *song, const MIDIEvent *events, Uint16 division)
{
    const MIDIEvent *ev;
    UMidiBlock *blocks;
    Uint8 *data;
    Uint32 numEvents = 0, numBlocks = 0, size = 0, i;
    Uint32 tick = 0, time = 0, rem = 0;
    Uint32 usPerTick, usRem;

    if (!division)
        return 0;

    for (ev = events; ev; ev = ev->next)
        numEvents++;

    /* at most one block per event plus the end of track, at most three
     * bytes per event */
    blocks = (UMidiBlock *)malloc((numEvents + 1) * sizeof(*blocks));
    data = (Uint8 *)malloc(numEvents * 3 + 1);
    if (!blocks || !data)
    {
        free(blocks);
        free(data);
        return 0;
    }

    usPerTick = DEFAULT_TEMPO / division;
    usRem = DEFAULT_TEMPO % division;

    for (ev = events; ev; ev = ev->next)
    {
        const Uint8 status = ev->status;
        const Uint32 delta = ev->time - tick;
        Uint32 length;

        /* ticks to microseconds; the remainder carries over, so no
         * rounding error accumulates, and stays below division squared */
        tick = ev->time;
        time += delta * usPerTick + delta / division * usRem;
        rem += delta % division * usRem;
        if (rem >= division)
        {
            time += rem / division;
            rem %= division;
        }

        if (status == 0xff)
        {
            /* meta event: only a tempo change matters */
            if (ev->data[0] == 0x51 && ev->extraLen == 3)
            {
                Uint32 tempo = ((Uint32)ev->extraData[0] << 16)
                             | ((Uint32)ev->extraData[1] << 8)
                             |  (Uint32)ev->extraData[2];
                if (tempo)
                {
                    usPerTick = tempo / division;
                    usRem = tempo % division;
                }
            }
            continue;
        }
        if (status >= 0xf0)
            continue;   /* system exclusive: skipped */

        if (numBlocks == 0 || time != blocks[numBlocks - 1].time)
        {
            blocks[numBlocks].time = time;
            numBlocks++;
        }

        length = ((status & 0xf0) == 0xc0 || (status & 0xf0) == 0xd0) ? 2 : 3;
        data[size] = status;
        data[size + 1] = ev->data[0];
        if (length == 3)
            data[size + 2] = ev->data[1];
        size += length;
        blocks[numBlocks - 1].end = size;
    }

    if (numBlocks == 0 || time != blocks[numBlocks - 1].time)
    {
        blocks[numBlocks].time = time;
        blocks[numBlocks].end = size;
        numBlocks++;
    }

    for (i = 0; i < numBlocks; i++)
        blocks[i].time = UMidiPeriods(blocks[i].time);

    song->blocks = blocks;
    song->data = data;
    song->numBlocks = numBlocks;
    song->loopBlock = 0;

    return 1;
}

int native_midi_detect()
{
    /* no MIDI port on this machine, or another program uses uMIDI */
    if (!UMidiOpen(&s_ctx))
        return 0;
    UMidiClose(&s_ctx);
    return 1;
}

NativeMidiSong *native_midi_loadsong_RW(SDL_RWops *rw, int freerw)
{
    NativeMidiSong *song;
    MIDIEvent *events;
    Uint16 division;

    printf("%s\n", __FUNCTION__);

    song = (NativeMidiSong *)malloc(sizeof(*song));
    if (song)
    {
        events = CreateMIDIEventList(rw, &division);
        if (!events || !convert_events(&song->song, events, division))
        {
            free(song);
            song = NULL;
        }
        if (events)
            FreeMIDIEventList(events);
    }

    if (freerw)
        SDL_RWclose(rw);

    return song;
}

void native_midi_freesong(NativeMidiSong *song)
{
    printf("%s\n", __FUNCTION__);

    if (!song)
        return;

    if (&song->song == s_ctx.song)
        native_midi_stop();

    free((void *)song->song.blocks);
    free((void *)song->song.data);
    free(song);
}

/* SDL_mixer has no call to close native MIDI, so uMIDI is opened for each
 * song and closed when it stops */
void native_midi_start(NativeMidiSong *song, int loops)
{
    printf("%s: %d\n", __FUNCTION__, loops);

    UMidiClose(&s_ctx);
    if (UMidiOpen(&s_ctx))
    {
        UMidiSetVolume(&s_ctx, umidi_volume(s_volume));
        UMidiPlay(&s_ctx, &song->song, loops);
    }
}

void native_midi_stop()
{
    printf("%s\n", __FUNCTION__);

    UMidiClose(&s_ctx);
}

int native_midi_active()
{
    return UMidiIsPlaying(&s_ctx);
}

void native_midi_setvolume(int volume)
{
    printf("%s: %d\n", __FUNCTION__, volume);

    s_volume = volume;
    UMidiSetVolume(&s_ctx, umidi_volume(volume));
}

const char *native_midi_error(void)
{
    return "";
}

#endif  /* __MINT__ */
