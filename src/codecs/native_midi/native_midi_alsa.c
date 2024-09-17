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

#include <alsa/asoundlib.h>

#include "native_midi.h"
#include "native_midi_common.h"

struct _NativeMidiSong {
    Uint16 division;
    MIDIEvent *event_list;
};

static enum { STOPPED, PLAYING, SHUTDOWN } state = STOPPED;
static SDL_Thread *native_midi_thread;
static snd_seq_t *output;
static int output_queue;

int native_midi_detect(void)
{
    int err;

    if (output != NULL) {
        return 1;
    }

    // TODO: Allow output port to be specified explicitly
    err = snd_seq_open(&output, "default", SND_SEQ_OPEN_OUTPUT, 0);
    if (err < 0) {
        SDL_Log("native_midi_detect: Failed to open sequencer device: %s",
                snd_strerror(err));
        return 0;
    }
    snd_seq_set_client_name(output, "SDL_mixer");
    snd_seq_set_output_buffer_size(output, 512);

    output_queue = snd_seq_alloc_queue(output);
    if (output_queue < 0) {
        snd_seq_close(output);
        output = NULL;
        return 0;
    }

    return 1;
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

static int playback_thread(void *data)
{
    NativeMidiSong *song = data;

    snd_seq_start_queue(output, output_queue, NULL);

    while (state == PLAYING) {
        // TODO
        break;
    }

    state = STOPPED;
    return 0;
}

void native_midi_start(NativeMidiSong *song, int loops)
{
    native_midi_stop();
    state = PLAYING;
    native_midi_thread = SDL_CreateThread(
        playback_thread, "native midi playback", song);
}

void native_midi_pause(void)
{
    snd_seq_stop_queue(output, output_queue, NULL);
}

void native_midi_resume(void)
{
    snd_seq_continue_queue(output, output_queue, NULL);
}

void native_midi_stop(void)
{
    if (state != PLAYING) {
        return;
    }

    state = SHUTDOWN;
    SDL_WaitThread(native_midi_thread, NULL);

    snd_seq_drop_output(output);
    snd_seq_stop_queue(output, output_queue, NULL);
}

int native_midi_active(void)
{
    return state == PLAYING;
}

void native_midi_setvolume(int volume)
{
}

const char *native_midi_error(void)
{
    return "";
}

#endif /* #ifdef __LINUX__ */
