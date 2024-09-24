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

static const snd_seq_addr_t default_ports[] = {
    {65, 0},
    {17, 0},
    {128, 0},   // Usual port for timidity
};

struct _NativeMidiSong {
    Uint16 division;
    MIDIEvent *event_list;
};

static enum { STOPPED, PLAYING, SHUTDOWN } state = STOPPED;
static SDL_Thread *native_midi_thread;
static snd_seq_addr_t connected_addr;
static snd_seq_t *output;
static int local_port;
static int output_queue;
static int plays_remaining;   // -1 means "loop forever"
static int poll_abort_pipe[2];

static SDL_bool try_connect(void)
{
    int i;

    local_port = snd_seq_create_simple_port(output, "SDL_mixer",
        SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
        SND_SEQ_PORT_TYPE_MIDI_GENERIC);
    if (local_port < 0) {
        return SDL_FALSE;
    }

    for (i = 0; i < sizeof(default_ports) / sizeof(*default_ports); ++i) {
        if (snd_seq_connect_to(output, local_port, default_ports[i].client,
                               default_ports[i].port) == 0) {
            connected_addr = default_ports[i];
            return SDL_TRUE;
        }
    }

    SDL_Log("native_midi_detect: Failed to find an output sequencer device.");

    return SDL_FALSE;
}

int native_midi_detect(void)
{
    int err;

    if (output != NULL) {
        return 1;
    }

    // TODO: Allow output port to be specified explicitly
    err = snd_seq_open(&output, "default", SND_SEQ_OPEN_OUTPUT,
                       SND_SEQ_NONBLOCK);
    if (err < 0) {
        SDL_Log("native_midi_detect: Failed to open sequencer device: %s",
                snd_strerror(err));
        return 0;
    }
    snd_seq_set_client_name(output, "SDL_mixer");

    if (!try_connect()) {
        snd_seq_close(output);
        output = NULL;
        return 0;
    }

    output_queue = snd_seq_alloc_queue(output);
    if (output_queue < 0) {
        snd_seq_close(output);
        output = NULL;
        return 0;
    }

    SDL_Log("native_midi_detect: Opened ALSA sequencer port %d:%d",
            connected_addr.client, connected_addr.port);

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

static int map_event_type(int ev_type)
{
    switch (ev_type) {
    case MIDI_STATUS_NOTE_OFF:
        return SND_SEQ_EVENT_NOTEOFF;
    case MIDI_STATUS_NOTE_ON:
        return SND_SEQ_EVENT_NOTEON;
    case MIDI_STATUS_AFTERTOUCH:
        return SND_SEQ_EVENT_KEYPRESS;
    case MIDI_STATUS_CONTROLLER:
        return SND_SEQ_EVENT_CONTROLLER;
    case MIDI_STATUS_PROG_CHANGE:
        return SND_SEQ_EVENT_PGMCHANGE;
    case MIDI_STATUS_PRESSURE:
        return SND_SEQ_EVENT_CHANPRESS;
    case MIDI_STATUS_PITCH_WHEEL:
        return SND_SEQ_EVENT_PITCHBEND;
    case MIDI_STATUS_SYSEX:
        return SND_SEQ_EVENT_SYSEX;
    default:
        return SND_SEQ_EVENT_NONE;
    }
}

static void convert_event(snd_seq_event_t *alsa_ev, MIDIEvent *ev)
{
    switch ((ev->status & 0xf0) >> 4) {
    case MIDI_STATUS_NOTE_OFF:
    case MIDI_STATUS_NOTE_ON:
    case MIDI_STATUS_AFTERTOUCH:
        snd_seq_ev_set_fixed(alsa_ev);
        alsa_ev->data.note.channel = ev->status & 0x0f;
        alsa_ev->data.note.note = ev->data[0];
        alsa_ev->data.note.velocity = ev->data[1];
        break;

    case MIDI_STATUS_CONTROLLER:
        snd_seq_ev_set_fixed(alsa_ev);
        alsa_ev->data.control.channel = ev->status & 0x0f;
        alsa_ev->data.control.param = ev->data[0];
        alsa_ev->data.control.value = ev->data[1];
        break;

    case MIDI_STATUS_PROG_CHANGE:
    case MIDI_STATUS_PRESSURE:
        snd_seq_ev_set_fixed(alsa_ev);
        alsa_ev->data.control.channel = ev->status & 0x0f;
        alsa_ev->data.control.value = ev->data[0];
        break;

    case MIDI_STATUS_PITCH_WHEEL:
        snd_seq_ev_set_fixed(alsa_ev);
        alsa_ev->data.control.channel = ev->status & 0x0f;
        alsa_ev->data.control.value =
            ((ev->data[0]) | ((ev->data[1]) << 7)) - 0x2000;
        break;

    case MIDI_STATUS_SYSEX:
        snd_seq_ev_set_variable(alsa_ev, ev->extraLen, ev->extraData);
        break;

    default:
        break;
    }
}

static void set_queue_tempo(Uint16 division)
{
    int err;

    // TODO: SMPTE
    snd_seq_queue_tempo_t *queue_tempo;
    snd_seq_queue_tempo_alloca(&queue_tempo);
    snd_seq_queue_tempo_set_tempo(queue_tempo, 500000);
    snd_seq_queue_tempo_set_ppq(queue_tempo, division);
    err = snd_seq_set_queue_tempo(output, output_queue, queue_tempo);
    if (err < 0) {
        SDL_Log("Failed to set tempo: err=%d", err);
    }
}

static void send_reset(void)
{
    static snd_seq_event_t alsa_ev;
    int i;

    snd_seq_ev_clear(&alsa_ev);
    snd_seq_ev_set_source(&alsa_ev, local_port);
    snd_seq_ev_set_subs(&alsa_ev);
    snd_seq_ev_schedule_tick(&alsa_ev, output_queue, 0, 0);

    // We send an ALSA reset event, but first send the standard MIDI control
    // events to stop all notes on all channels, just in case.
    for (i = 0; i < 16; i++) {
        alsa_ev.type = SND_SEQ_EVENT_CONTROLLER;
        snd_seq_ev_set_fixed(&alsa_ev);
        alsa_ev.data.control.channel = i;
        alsa_ev.data.control.param = MIDI_CTL_ALL_NOTES_OFF;
        alsa_ev.data.control.value = 0;
        snd_seq_event_output(output, &alsa_ev);

        alsa_ev.data.control.param = MIDI_CTL_RESET_CONTROLLERS;
        snd_seq_event_output(output, &alsa_ev);
    }

    alsa_ev.type = SND_SEQ_EVENT_RESET;
    snd_seq_event_output(output, &alsa_ev);
}

static void poll_output(void)
{
    struct pollfd fds[2];

    // Block until more events can (potentially) be written to the
    // ALSA output stream.
    snd_seq_poll_descriptors(output, &fds[0], 1, POLLOUT);

    // We also block on one of the file descriptors from the abort pipe;
    // this allows native_midi_stop() below to trigger poll() to return
    // and the playback thread to terminate.
    fds[1].fd = poll_abort_pipe[0];
    fds[1].events = POLLHUP|POLLERR;

    poll(fds, 2, -1);
}

static int playback_thread(void *data)
{
    NativeMidiSong *song = data;
    MIDIEvent *ev = NULL;
    snd_seq_event_t alsa_ev;
    int last_event_time = 0, time_offset = 0;

    snd_seq_drop_output(output);
    set_queue_tempo(song->division);
    snd_seq_start_queue(output, output_queue, NULL);
    send_reset();

    while (state == PLAYING) {
        if (ev == NULL) {
            // Loop until plays_remaining is zero, then we stop.
            if (plays_remaining == 0) {
                break;
            } else if (plays_remaining > 0) {
                --plays_remaining;
            }
            time_offset = last_event_time + 100;
            ev = song->event_list;
            if (ev == NULL) {
                break;
            }
        }

        snd_seq_ev_clear(&alsa_ev);
        alsa_ev.type = map_event_type((ev->status & 0xf0) >> 4);
        snd_seq_ev_set_source(&alsa_ev, local_port);
        snd_seq_ev_set_subs(&alsa_ev);

        snd_seq_ev_schedule_tick(&alsa_ev, output_queue, 0,
                                 time_offset + ev->time);
        last_event_time = time_offset + ev->time;

        convert_event(&alsa_ev, ev);
        ev = ev->next;

        // We use nonblocking mode, so we may not be able to write the
        // event to the buffer yet. If so, we poll until we can.
        while (state == PLAYING) {
            snd_seq_drain_output(output);
            if (snd_seq_event_output_buffer(output, &alsa_ev) != -EAGAIN) {
                break;
            }
            poll_output();
        }
    }

    state = STOPPED;
    snd_seq_drain_output(output);
    close(poll_abort_pipe[0]);
    close(poll_abort_pipe[1]);

    return 0;
}

void native_midi_start(NativeMidiSong *song, int loops)
{
    native_midi_stop();
    if (pipe(poll_abort_pipe) != 0) {
        SDL_Log("Failed to create poll abort pipe: %s", strerror(errno));
        return;
    }
    state = PLAYING;
    plays_remaining = loops < 0 ? -1 : loops + 1;
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

    // We trigger shutdown of the native MIDI thread by closing the file
    // descriptors for the abort pipe. This causes the poll_output()
    // function above to return instead of blocking on output, and the
    // playback thread to terminate.
    state = SHUTDOWN;
    close(poll_abort_pipe[0]);
    close(poll_abort_pipe[1]);
    SDL_WaitThread(native_midi_thread, NULL);

    snd_seq_drop_output(output);
    send_reset();
    snd_seq_drain_output(output);
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
