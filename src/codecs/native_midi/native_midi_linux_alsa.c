/*
  native_midi_linux_alsa:  Native Midi support on Linux for the SDL_mixer library
  Copyright (C) 2024 Tasos Sahanidis <code@tasossah.com>

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

#include <SDL3/SDL_platform.h>
#ifdef SDL_PLATFORM_LINUX

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_atomic.h>

#include "native_midi.h"
#include "native_midi_common.h"

#include <alsa/asoundef.h>
#include <alsa/asoundlib.h>

#include <errno.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <assert.h>

//#define SDL_NATIVE_MIDI_ALSA_DYNAMIC "libasound.so.2"

static int load_alsa_syms(void);

#ifdef SDL_NATIVE_MIDI_ALSA_DYNAMIC
#define snd_seq_client_info_sizeof ALSA_snd_seq_client_info_sizeof
#define snd_seq_port_info_sizeof   ALSA_snd_seq_port_info_sizeof
#define snd_seq_queue_tempo_sizeof ALSA_snd_seq_queue_tempo_sizeof
#define snd_seq_control_queue      ALSA_snd_seq_control_queue

static void *alsa_handle = NULL;

static int load_alsa_sym(const char *fn, void **addr)
{
    *addr = SDL_LoadFunction(alsa_handle, fn);
    if (!*addr) {
        // Don't call SDL_SetError(): SDL_LoadFunction already did.
        return 0;
    }

    return 1;
}

// cast funcs to char* first, to please GCC's strict aliasing rules.
#define SDL_ALSA_SYM(x)                                 \
    if (!load_alsa_sym(#x, (void **)(char *)&ALSA_##x)) \
    return -1

static void unload_alsa_library(void)
{
    if (alsa_handle) {
        SDL_UnloadObject(alsa_handle);
        alsa_handle = NULL;
    }
}

static int load_alsa_library(void)
{
    int retval = 0;
    if (!alsa_handle) {
        alsa_handle = SDL_LoadObject(SDL_NATIVE_MIDI_ALSA_DYNAMIC);
        if (!alsa_handle) {
            retval = -1;
            // Don't call SDL_SetError(): SDL_LoadObject already did.
        } else {
            retval = load_alsa_syms();
            if (retval < 0) {
                unload_alsa_library();
            }
        }
    }
    return retval;
}

#else

#define SDL_ALSA_SYM(x) ALSA_##x = x
static void unload_alsa_library(void)
{
}

static int load_alsa_library(void)
{
    return load_alsa_syms();
}
#endif // SDL_NATIVE_MIDI_ALSA_DYNAMIC

static int (*ALSA_snd_seq_alloc_named_queue)(snd_seq_t *seq, const char *name);
static int (*ALSA_snd_seq_client_id)(snd_seq_t *handle);
static int (*ALSA_snd_seq_client_info_get_client)(const snd_seq_client_info_t *info);
static size_t (*ALSA_snd_seq_client_info_sizeof)(void);
static int (*ALSA_snd_seq_close)(snd_seq_t *handle);
static int (*ALSA_snd_seq_connect_to)(snd_seq_t *seq, int my_port, int dest_client, int dest_port);
static int (*ALSA_snd_seq_control_queue)(snd_seq_t *seq, int q, int type, int value, snd_seq_event_t *ev);
static int (*ALSA_snd_seq_create_simple_port)(snd_seq_t *seq, const char *name, unsigned int caps, unsigned int type);
static int (*ALSA_snd_seq_delete_simple_port)(snd_seq_t *seq, int port);
static int (*ALSA_snd_seq_drain_output)(snd_seq_t *handle);
static int (*ALSA_snd_seq_drop_output)(snd_seq_t *handle);
static int (*ALSA_snd_seq_event_input)(snd_seq_t *handle, snd_seq_event_t **ev);
static int (*ALSA_snd_seq_event_output)(snd_seq_t *handle, snd_seq_event_t *ev);
static int (*ALSA_snd_seq_event_output_direct)(snd_seq_t *handle, snd_seq_event_t *ev);
static int (*ALSA_snd_seq_free_queue)(snd_seq_t *handle, int q);
static int (*ALSA_snd_seq_get_any_client_info)(snd_seq_t *handle, int client, snd_seq_client_info_t *info);
static int (*ALSA_snd_seq_get_any_port_info)(snd_seq_t *handle, int client, int port, snd_seq_port_info_t *info);
static int (*ALSA_snd_seq_nonblock)(snd_seq_t *handle, int nonblock);
static int (*ALSA_snd_seq_open)(snd_seq_t **handle, const char *name, int streams, int mode);
static int (*ALSA_snd_seq_parse_address)(snd_seq_t *seq, snd_seq_addr_t *addr, const char *str);
static int (*ALSA_snd_seq_poll_descriptors)(snd_seq_t *handle, struct pollfd *pfds, unsigned int space, short events);
static unsigned int (*ALSA_snd_seq_port_info_get_capability)(const snd_seq_port_info_t *info);
static int (*ALSA_snd_seq_port_info_get_port)(const snd_seq_port_info_t *info);
static unsigned int (*ALSA_snd_seq_port_info_get_type)(const snd_seq_port_info_t *info);
static size_t (*ALSA_snd_seq_port_info_sizeof)(void);
static int (*ALSA_snd_seq_query_next_client)(snd_seq_t *handle, snd_seq_client_info_t *info);
static int (*ALSA_snd_seq_query_next_port)(snd_seq_t *handle, snd_seq_port_info_t *info);
static void (*ALSA_snd_seq_queue_tempo_set_ppq)(snd_seq_queue_tempo_t *info, int ppq);
static void (*ALSA_snd_seq_queue_tempo_set_tempo)(snd_seq_queue_tempo_t *info, unsigned int tempo);
static size_t (*ALSA_snd_seq_queue_tempo_sizeof)(void);
static int (*ALSA_snd_seq_set_client_event_filter)(snd_seq_t *seq, int event_type);
static int (*ALSA_snd_seq_set_client_name)(snd_seq_t *seq, const char *name);
static int (*ALSA_snd_seq_set_queue_tempo)(snd_seq_t *handle, int q, snd_seq_queue_tempo_t *tempo);

static int load_alsa_syms(void)
{
    SDL_ALSA_SYM(snd_seq_alloc_named_queue);
    SDL_ALSA_SYM(snd_seq_client_id);
    SDL_ALSA_SYM(snd_seq_client_info_get_client);
    SDL_ALSA_SYM(snd_seq_client_info_sizeof);
    SDL_ALSA_SYM(snd_seq_close);
    SDL_ALSA_SYM(snd_seq_connect_to);
    SDL_ALSA_SYM(snd_seq_control_queue);
    SDL_ALSA_SYM(snd_seq_create_simple_port);
    SDL_ALSA_SYM(snd_seq_delete_simple_port);
    SDL_ALSA_SYM(snd_seq_drain_output);
    SDL_ALSA_SYM(snd_seq_drop_output);
    SDL_ALSA_SYM(snd_seq_event_input);
    SDL_ALSA_SYM(snd_seq_event_output);
    SDL_ALSA_SYM(snd_seq_event_output_direct);
    SDL_ALSA_SYM(snd_seq_free_queue);
    SDL_ALSA_SYM(snd_seq_get_any_client_info);
    SDL_ALSA_SYM(snd_seq_get_any_port_info);
    SDL_ALSA_SYM(snd_seq_nonblock);
    SDL_ALSA_SYM(snd_seq_open);
    SDL_ALSA_SYM(snd_seq_parse_address);
    SDL_ALSA_SYM(snd_seq_poll_descriptors);
    SDL_ALSA_SYM(snd_seq_port_info_get_capability);
    SDL_ALSA_SYM(snd_seq_port_info_get_port);
    SDL_ALSA_SYM(snd_seq_port_info_get_type);
    SDL_ALSA_SYM(snd_seq_port_info_sizeof);
    SDL_ALSA_SYM(snd_seq_query_next_client);
    SDL_ALSA_SYM(snd_seq_query_next_port);
    SDL_ALSA_SYM(snd_seq_queue_tempo_set_ppq);
    SDL_ALSA_SYM(snd_seq_queue_tempo_set_tempo);
    SDL_ALSA_SYM(snd_seq_queue_tempo_sizeof);
    SDL_ALSA_SYM(snd_seq_set_client_event_filter);
    SDL_ALSA_SYM(snd_seq_set_client_name);
    SDL_ALSA_SYM(snd_seq_set_queue_tempo);
    return 0;
}

#ifndef NDEBUG
#define MIDIDbgLog(...) SDL_LogDebug(SDL_LOG_CATEGORY_AUDIO, __VA_ARGS__)
#else
#define MIDIDbgLog(...) \
    do {                \
    } while (0)
#endif

#define MIDI_Mix_OutOfMemory() errmsg_final = oom

static const char *const oom = "Out of memory";
static char errmsg[256] = "";
static const char *errmsg_final = errmsg;
#define MIDI_SET_ERROR(...) SDL_snprintf(errmsg, sizeof(errmsg), __VA_ARGS__)

#define MIDI_SMF_META_EVENT 0xFF
#define MIDI_SMF_META_TEMPO 0x51

typedef enum
{
    NATIVE_MIDI_STOPPED,
    NATIVE_MIDI_STARTING,
    NATIVE_MIDI_PLAYING,
    NATIVE_MIDI_PAUSED
} native_midi_state;

typedef enum
{
    THREAD_CMD_QUIT = 1,
    THREAD_CMD_PAUSE,
    THREAD_CMD_RESUME,
    THREAD_CMD_SETVOL,
} native_midi_thread_cmd;

struct _NativeMidiSong
{
    SDL_Thread *playerthread;
    int mainsock, threadsock;
    Uint16 ppqn;
    MIDIEvent *evtlist;
    snd_seq_t *seq;
    int srcport;
    snd_seq_addr_t dstaddr;
    int loopcount;
    Uint32 endtime;
    SDL_AtomicInt playerstate; /* Stores a native_midi_state */
    bool allow_pause;
};

/* Fixed length command packets */
#define CMD_PKT_LEN 2
static const unsigned char pkt_thread_cmd_quit[CMD_PKT_LEN] = { THREAD_CMD_QUIT };
static const unsigned char pkt_thread_cmd_pause[CMD_PKT_LEN] = { THREAD_CMD_PAUSE };
static const unsigned char pkt_thread_cmd_resume[CMD_PKT_LEN] = { THREAD_CMD_RESUME };

static SDL_INLINE const char *get_app_name_hint(void)
{
    const char *ret = SDL_GetHint(SDL_HINT_AUDIO_DEVICE_APP_ICON_NAME);
    if (ret && *ret)
        return ret;

    ret = SDL_GetHint(SDL_HINT_APP_NAME);
    if (ret && *ret)
        return ret;

    return NULL;
}

/* Make sure to SDL_free this */
static char *get_app_name(void)
{
    /* Try the SDL hints first */
    const char *hint = get_app_name_hint();
    if (hint) {
        char *ret = SDL_strdup(hint);
        if (!ret)
            MIDI_Mix_OutOfMemory();
        return ret;
    }

    /* Build the path to access the application's cmdline */
    char procfs_path[64];
    SDL_snprintf(procfs_path, sizeof(procfs_path), "/proc/%ld/cmdline", (long)getpid());

    long pathmax = pathconf("/", _PC_PATH_MAX);
    if (pathmax == -1)
        pathmax = 4096;

    char *cmdline = SDL_calloc(1, pathmax + 1);
    if (!cmdline) {
        MIDI_Mix_OutOfMemory();
        return NULL;
    }

    size_t len = 0;

    int fd = open(procfs_path, O_RDONLY);
    if (fd >= 0) {
        if (read(fd, cmdline, pathmax) > 0) {
            char *base = SDL_strrchr(cmdline, '/') + 1; /* Absolute worst case we get to a '\0' */
            if (base) {
                len = strlen(base);
                if (len)
                    SDL_memmove(cmdline, base, len + 1);
            }
        }
        close(fd);
    }

    /* len is used both for checking if we read any data and if a path separator was found */
    if (!len)
        SDL_snprintf(cmdline, pathmax + 1, "SDL_Mixer Application");

    return cmdline;
}

static snd_seq_t *open_seq(int *srcport_out)
{
    snd_seq_t *seq;
    int ret;

    if (load_alsa_library()) {
        MIDI_SET_ERROR("Failed to load libasound");
        return NULL;
    }

    if ((ret = ALSA_snd_seq_open(&seq, "default", SND_SEQ_OPEN_DUPLEX, 0)) < 0) {
        MIDI_SET_ERROR("snd_seq_open returned %d", ret);
        return NULL;
    }

    char *seq_name = get_app_name();
    if (!seq_name) {
        ALSA_snd_seq_close(seq);
        return NULL;
    }

    ALSA_snd_seq_set_client_name(seq, seq_name);

    if ((ret = ALSA_snd_seq_create_simple_port(seq, seq_name,
                                               SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_READ | SND_SEQ_PORT_CAP_SYNC_READ,
                                               SND_SEQ_PORT_TYPE_APPLICATION | SND_SEQ_PORT_TYPE_MIDI_GENERIC)) < 0) {
        MIDI_SET_ERROR("snd_seq_create_simple_port failed with %d", ret);
        ALSA_snd_seq_close(seq);
        SDL_free(seq_name);
        return NULL;
    }

    SDL_free(seq_name);

    *srcport_out = ret;

    return seq;
}

static void close_seq(snd_seq_t *seq, const int port)
{
    ALSA_snd_seq_delete_simple_port(seq, port);
    ALSA_snd_seq_close(seq);
    unload_alsa_library();
}

bool native_midi_detect(void)
{
    int port;
    snd_seq_t *seq_temp = open_seq(&port);
    if (!seq_temp)
        return 0;
    close_seq(seq_temp, port);
    return 1;
}

static void close_sockpair(NativeMidiSong *song)
{
    shutdown(song->mainsock, SHUT_RDWR);
    shutdown(song->threadsock, SHUT_RDWR);
    close(song->mainsock);
    close(song->threadsock);
}

static SDL_INLINE int subscribe_to_first_available_port(snd_seq_t *seq, const int srcport, const unsigned int required_type)
{
    snd_seq_client_info_t *clientinfo;
    snd_seq_client_info_alloca(&clientinfo);

    /* Query System to fill the struct initially */
    if (ALSA_snd_seq_get_any_client_info(seq, 0, clientinfo))
        return -1;

    while (ALSA_snd_seq_query_next_client(seq, clientinfo) == 0) {
        int client = ALSA_snd_seq_client_info_get_client(clientinfo);

        /* Not necessary, as we don't allow subscription to our ports, but let's ignore ourselves anyway */
        if (client == ALSA_snd_seq_client_id(seq))
            continue;

        snd_seq_port_info_t *portinfo;
        snd_seq_port_info_alloca(&portinfo);

        /* Start with port 0 */
        if (ALSA_snd_seq_get_any_port_info(seq, client, 0, portinfo))
            continue;

        do {
            int port = ALSA_snd_seq_port_info_get_port(portinfo);
            unsigned int cap = ALSA_snd_seq_port_info_get_capability(portinfo);
            unsigned int type = ALSA_snd_seq_port_info_get_type(portinfo);

            if ((type & required_type) == required_type &&
                cap & SND_SEQ_PORT_CAP_WRITE &&
                cap & SND_SEQ_PORT_CAP_SUBS_WRITE &&
                !(cap & SND_SEQ_PORT_CAP_NO_EXPORT)) {

                MIDIDbgLog("Client %d Cap %x Type %x", client, cap, type);

                /* Could we connect to it? */
                if (ALSA_snd_seq_connect_to(seq, srcport, client, port) == 0)
                    return 0;
            }

        } while (ALSA_snd_seq_query_next_port(seq, portinfo) == 0);
    }
    return 1;
}

static SDL_INLINE void pick_seq_dest_addr(NativeMidiSong *song)
{
    /* Send events to all subscribers */
    song->dstaddr.client = SND_SEQ_ADDRESS_SUBSCRIBERS;
    song->dstaddr.port = SND_SEQ_ADDRESS_UNKNOWN;

    /* Connect us somewhere, unless it's not desired */
    if (SDL_GetHintBoolean("SDL_NATIVE_MUSIC_NO_CONNECT_PORTS", false))
        return;

    /* If ALSA_OUTPUT_PORTS is specified, try to parse it and connect to it */
    snd_seq_addr_t conn_addr;
    const char *ports_env = SDL_getenv("ALSA_OUTPUT_PORTS");
    if (ports_env && ALSA_snd_seq_parse_address(song->seq, &conn_addr, ports_env) == 0)
        if (ALSA_snd_seq_connect_to(song->seq, song->srcport, conn_addr.client, conn_addr.port) == 0)
            return;

    /* If we're not connecting to a specific client, pick the first one available after System (0) */
    /* Prefer connecting to synthesizers, as that is the primary use case */
    if (!subscribe_to_first_available_port(song->seq, song->srcport, SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_SYNTHESIZER))
        return;

    /* If we can't find a synth, then pick the first available port */
    if (!subscribe_to_first_available_port(song->seq, song->srcport, SND_SEQ_PORT_TYPE_MIDI_GENERIC))
        return;
}

static NativeMidiSong *currentsong = NULL;

NativeMidiSong *native_midi_loadsong_IO(SDL_IOStream *src, bool closeio)
{
    NativeMidiSong *song;
    MIDIEvent *event;
    int sv[2];

    if (!(song = SDL_calloc(1, sizeof(NativeMidiSong)))) {
        MIDI_Mix_OutOfMemory();
        return NULL;
    }

    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sv) == -1) {
        MIDI_SET_ERROR("Failed to create socketpair with errno %d", errno);
        SDL_free(song);
        return NULL;
    }

    song->mainsock = sv[0];
    song->threadsock = sv[1];

    event = song->evtlist = CreateMIDIEventList(src, &song->ppqn);

    if (!song->evtlist) {
        close_sockpair(song);
        SDL_free(song);
        MIDI_SET_ERROR("Failed to create MIDIEventList");
        return NULL;
    }

    /* Since ALSA requires the starting F0 for SysEx, but MIDIEvent.extraData doesn't contain it, we must preprocess the list */
    /* In addition, since we're going through the list, store the last event's time for looping purposes */
    do {
        /* Is this a SysEx? */
        if (event->status == MIDI_CMD_COMMON_SYSEX && event->extraLen) {
            /* Sanity check in case something changes in the future */
            /* This is safe to do since we can't have an F0 manufacturer ID */
            assert(event->extraData[0] != MIDI_CMD_COMMON_SYSEX);

            /* Resize by + 1 */
            Uint8 *newData = SDL_realloc(event->extraData, event->extraLen + 1);
            if (newData == NULL) {
                close_sockpair(song);
                /* Original allocation is still valid on failure */
                FreeMIDIEventList(song->evtlist);
                SDL_free(song);
                MIDI_SET_ERROR("Failed to preprocess MIDIEventList SysEx");
                return NULL;
            }

            /* Prepend the F0 */
            event->extraData = newData;
            SDL_memmove(event->extraData + 1, event->extraData, event->extraLen);
            event->extraData[0] = MIDI_CMD_COMMON_SYSEX;
            event->extraLen++;
        }

        /* Store the end time */
        song->endtime = event->time;
    } while ((event = event->next));

    if (!(song->seq = open_seq(&song->srcport))) {
        FreeMIDIEventList(song->evtlist);
        close_sockpair(song);
        SDL_free(song);
        return NULL;
    }

    /* Only allow echo events to be sent */
    ALSA_snd_seq_set_client_event_filter(song->seq, SND_SEQ_EVENT_ECHO);

    pick_seq_dest_addr(song);

    SDL_SetAtomicInt(&song->playerstate, NATIVE_MIDI_STOPPED);


    /* Since there's no reliable volume control solution it's better to leave the music playing instead of having hanging notes */
    song->allow_pause = SDL_GetHintBoolean("SDL_NATIVE_MUSIC_ALLOW_PAUSE", false);

    if (closeio)
        SDL_CloseIO(src);

    currentsong = song;
    return song;
}

void native_midi_freesong(NativeMidiSong *song)
{
    if (!song)
        return;

    close_seq(song->seq, song->srcport);
    FreeMIDIEventList(song->evtlist);
    close_sockpair(song);
    SDL_free(song);
}

/* Schedule an echo event right after the last event to know when playback is finished */
static SDL_INLINE void enqueue_echo_event(const NativeMidiSong *song, const int queue)
{
    snd_seq_event_t evt;
    snd_seq_ev_clear(&evt);
    evt.type = SND_SEQ_EVENT_ECHO;
    snd_seq_ev_set_source(&evt, song->srcport);
    snd_seq_ev_set_dest(&evt, ALSA_snd_seq_client_id(song->seq), song->srcport);
    snd_seq_ev_schedule_tick(&evt, queue, 0, song->endtime + 1);
    while (ALSA_snd_seq_event_output(song->seq, &evt) == -EAGAIN)
        ;
}

/* Reset the queue position to 0 */
static SDL_INLINE void enqueue_queue_reset_event(const NativeMidiSong *song, const int queue)
{
    snd_seq_event_t evt;
    snd_seq_ev_clear(&evt);
    snd_seq_ev_set_source(&evt, song->srcport);
    snd_seq_ev_set_queue_pos_tick(&evt, queue, 0);
    /* Schedule it to some point in the past, so that it is guaranteed */
    /* to run immediately and before the echo */
    snd_seq_ev_schedule_tick(&evt, queue, 0, 0);
    while (ALSA_snd_seq_event_output(song->seq, &evt) == -EAGAIN)
        ;
}

/* Sysex to set the volume */
static SDL_INLINE void send_volume_sysex(const NativeMidiSong *song, const unsigned char vol)
{
    unsigned char vol_sysex[] = { MIDI_CMD_COMMON_SYSEX, 0x7F, 0x7F, 0x04, 0x01, 0x00, vol, MIDI_CMD_COMMON_SYSEX_END };
    /* Event used to set the volume */
    snd_seq_event_t evt;
    snd_seq_ev_clear(&evt);
    snd_seq_ev_set_source(&evt, song->srcport);
    snd_seq_ev_set_dest(&evt, song->dstaddr.client, song->dstaddr.port);
    snd_seq_ev_set_direct(&evt);
    snd_seq_ev_set_sysex(&evt, sizeof(vol_sysex), vol_sysex);
    ALSA_snd_seq_event_output_direct(song->seq, &evt);
}

/* Sequencer queue control */
static SDL_INLINE void stop_queue(const NativeMidiSong *song, const int queue)
{
    snd_seq_event_t evt;
    snd_seq_ev_clear(&evt);
    snd_seq_ev_set_queue_control(&evt, SND_SEQ_EVENT_STOP, queue, 0);
    snd_seq_ev_set_direct(&evt);
    ALSA_snd_seq_event_output_direct(song->seq, &evt);
}

static SDL_INLINE void continue_queue(const NativeMidiSong *song, const int queue)
{
    snd_seq_event_t evt;
    snd_seq_ev_clear(&evt);
    snd_seq_ev_set_queue_control(&evt, SND_SEQ_EVENT_CONTINUE, queue, 0);
    snd_seq_ev_set_direct(&evt);
    ALSA_snd_seq_event_output_direct(song->seq, &evt);
}

/* Playback thread */
static int native_midi_player_thread(void *d)
{
    unsigned char current_volume = 0x7F;
    bool playback_finished = false;
    NativeMidiSong *song = d;
    MIDIEvent *event = song->evtlist;
    int i;
    int queue = ALSA_snd_seq_alloc_named_queue(song->seq, "SDL_Mixer Playback");
    snd_seq_start_queue(song->seq, queue, NULL);

    /* Prepare main sequencer event */
    snd_seq_event_t evt;
    snd_seq_ev_clear(&evt);
    snd_seq_ev_set_source(&evt, song->srcport);
    snd_seq_ev_set_dest(&evt, song->dstaddr.client, song->dstaddr.port);

    /* Set up nonblock functionality */
    struct pollfd pfds[2] = { {
        .fd = song->threadsock,
        .events = POLLIN,
    } };
    ALSA_snd_seq_poll_descriptors(song->seq, pfds + 1, 1, POLLIN | POLLOUT);
    ALSA_snd_seq_nonblock(song->seq, 1);

    /* Set initial queue tempo and ppqn */
    snd_seq_queue_tempo_t *tempo;
    snd_seq_queue_tempo_alloca(&tempo);
    ALSA_snd_seq_queue_tempo_set_tempo(tempo, 500000);
    ALSA_snd_seq_queue_tempo_set_ppq(tempo, song->ppqn);
    ALSA_snd_seq_set_queue_tempo(song->seq, queue, tempo);

    /* We use this to know when the track has finished playing */
    enqueue_echo_event(song, queue);

    SDL_SetAtomicInt(&song->playerstate, NATIVE_MIDI_PLAYING);

    while (1) {
        unsigned char readbuf[CMD_PKT_LEN];
        MIDIDbgLog("Poll...");
        if (poll(pfds, 2, -1) <= 0)
            break;
        MIDIDbgLog("revents: cmdsock %hd, ALSA %hd", pfds[0].revents, pfds[1].revents);

        /* Do we have a command from the main thread? */
        if (pfds[0].revents & POLLIN) {
            /* This will process exactly one command by design because all packets are fixed size (CMD_PKT_LEN) */
            if (read(song->threadsock, readbuf, sizeof(readbuf)) == sizeof(readbuf)) {
                MIDIDbgLog("Got control %hhx", readbuf[0]);
                switch ((native_midi_thread_cmd)readbuf[0]) {

                case THREAD_CMD_QUIT:
                    event = NULL;
                    song->loopcount = 0;
                    playback_finished = true;
                    break;

                case THREAD_CMD_SETVOL:
                    current_volume = readbuf[1];
                    send_volume_sysex(song, current_volume);
                    break;

                case THREAD_CMD_PAUSE:
                    send_volume_sysex(song, 0);
                    stop_queue(song, queue);
                    SDL_SetAtomicInt(&song->playerstate, NATIVE_MIDI_PAUSED);
                    break;

                case THREAD_CMD_RESUME:
                    continue_queue(song, queue);
                    send_volume_sysex(song, current_volume);
                    SDL_SetAtomicInt(&song->playerstate, NATIVE_MIDI_PLAYING);
                    break;
                }
            }
        }

        /* Can we read from the sequencer? */
        if (pfds[1].revents & POLLIN) {
            snd_seq_event_t *revt;
            /* Make sure we read an echo event, and that it came from us */
            if (ALSA_snd_seq_event_input(song->seq, &revt) >= 0 && revt->type == SND_SEQ_EVENT_ECHO && revt->source.client == ALSA_snd_seq_client_id(song->seq) && revt->source.port == song->srcport)
                playback_finished = true;
        }

        /* Have we reached the end of the event list? */
        if (!event) {
            /* If we have, are we done playing? */
            if (playback_finished) {
                if (song->loopcount == 0)
                    break;

                MIDIDbgLog("Playback is looping");

                /* If we need to loop, roll back the list head and keep going */
                event = song->evtlist;

                /* We need to reset the queue, otherwise the ticks will be wrong */
                enqueue_queue_reset_event(song, queue);
                enqueue_echo_event(song, queue);

                if (song->loopcount > 0)
                    song->loopcount--;

                playback_finished = false;

                /* Allow ready to write events again */
                pfds[1].events |= POLLOUT;
            } else {
                /* If not, keep draining, otherwise we'll never reach the echo event */
                /* When we finish though, prevent any "ready to write to alsa" polls */
                MIDIDbgLog("Draining output!");
                if (ALSA_snd_seq_drain_output(song->seq) == 0)
                    pfds[1].events &= ~POLLOUT;
                continue;
            }
        }

        /* Don't proceed if we can't write to the sequencer */
        if (!(pfds[1].revents & POLLOUT))
            continue;

        /* Finally, if we get here, we process MIDI events and send them to the sequencer */
        const unsigned char cmd = event->status & 0xF0;
        const unsigned char channel = event->status & 0x0F;

        snd_seq_ev_set_dest(&evt, song->dstaddr.client, song->dstaddr.port);
        snd_seq_ev_set_fixed(&evt);
        snd_seq_ev_schedule_tick(&evt, queue, 0, event->time);

        bool unhandled = false;

        switch (cmd) {

        case MIDI_CMD_NOTE_ON:
            snd_seq_ev_set_noteon(&evt, channel, event->data[0], event->data[1]);
            break;

        case MIDI_CMD_NOTE_OFF:
            snd_seq_ev_set_noteoff(&evt, channel, event->data[0], event->data[1]);
            break;

        case MIDI_CMD_CONTROL:
            snd_seq_ev_set_controller(&evt, channel, event->data[0], event->data[1]);
            break;

        case MIDI_CMD_NOTE_PRESSURE:
            snd_seq_ev_set_keypress(&evt, channel, event->data[0], event->data[1]);
            break;

        case MIDI_CMD_PGM_CHANGE:
            snd_seq_ev_set_pgmchange(&evt, channel, event->data[0]);
            break;

        case MIDI_CMD_BENDER:
            snd_seq_ev_set_pitchbend(&evt, channel, ((((int)event->data[1]) << 7) | (event->data[0] & 0x7F)) - 8192);
            break;

        default:
            if (event->status == MIDI_SMF_META_EVENT) {
                if (event->data[0] == MIDI_SMF_META_TEMPO && event->extraLen == 3) {
                    unsigned int t = ((unsigned)event->extraData[0] << 16) |
                                     ((unsigned)event->extraData[1] << 8) |
                                     event->extraData[2];

                    /* This changes the event destination, so we have to restore it in the next iteration */
                    snd_seq_ev_set_queue_tempo(&evt, queue, t);
                    break;
                }
            } else if (event->status == MIDI_CMD_COMMON_SYSEX) {
                snd_seq_ev_set_sysex(&evt, event->extraLen, event->extraData);
                break;
            }

            unhandled = true;
        }

        if (unhandled || ALSA_snd_seq_event_output(song->seq, &evt) != -EAGAIN) {
            MIDIDbgLog("%s %" SDL_PRIu32 ": %hhx %hhx %hhx (extraLen %" SDL_PRIu32 ")", (unhandled ? "Unhandled" : "Event"), event->time, event->status, event->data[0], event->data[1], event->extraLen);
            event = event->next;
        }
    }

    SDL_SetAtomicInt(&song->playerstate, NATIVE_MIDI_STOPPED);

    /* Switch back to blocking mode and drop everything */
    ALSA_snd_seq_nonblock(song->seq, 0);
    ALSA_snd_seq_drop_output(song->seq);
    snd_seq_stop_queue(song->seq, queue, NULL);
    ALSA_snd_seq_drain_output(song->seq);
    ALSA_snd_seq_free_queue(song->seq, queue);

    /* Stop all audio */
    /* Some of these are bound to work */
    snd_seq_ev_set_direct(&evt);
    for (i = 0; i < MIDI_CHANNELS; i++) {
        snd_seq_ev_set_controller(&evt, i, MIDI_CTL_SUSTAIN, 0);
        ALSA_snd_seq_event_output_direct(song->seq, &evt);
        snd_seq_ev_set_controller(&evt, i, MIDI_CTL_ALL_NOTES_OFF, 0);
        ALSA_snd_seq_event_output_direct(song->seq, &evt);
        snd_seq_ev_set_controller(&evt, i, MIDI_CTL_RESET_CONTROLLERS, 0);
        ALSA_snd_seq_event_output_direct(song->seq, &evt);
        snd_seq_ev_set_controller(&evt, i, MIDI_CTL_ALL_SOUNDS_OFF, 0);
        ALSA_snd_seq_event_output_direct(song->seq, &evt);
    }

    MIDIDbgLog("Playback thread returns");
    return 0;
}

void native_midi_start(NativeMidiSong *song, int loops)
{
    if (!song)
        return;

    if (song->playerthread) {
        if (SDL_GetAtomicInt(&currentsong->playerstate) > NATIVE_MIDI_STOPPED)
            if (write(song->mainsock, pkt_thread_cmd_quit, CMD_PKT_LEN) != sizeof(pkt_thread_cmd_quit))
                return;
        SDL_WaitThread(song->playerthread, NULL);
    }

    song->loopcount = loops;

    /* If this isn't set here, then the application might think we finished before playback even started */
    SDL_SetAtomicInt(&song->playerstate, NATIVE_MIDI_STARTING);

    song->playerthread = SDL_CreateThread(native_midi_player_thread, "SDL_Mixer Midi", song);
}

/* The following functions require song to be global (thus currentsong is used) */
void native_midi_pause(void)
{
    NativeMidiSong *song = currentsong;

    if (!song || SDL_GetAtomicInt(&song->playerstate) == NATIVE_MIDI_STOPPED || !song->allow_pause)
        return;

    (void)!write(song->mainsock, pkt_thread_cmd_pause, CMD_PKT_LEN);
}

void native_midi_resume(void)
{
    NativeMidiSong *song = currentsong;

    if (!song || SDL_GetAtomicInt(&song->playerstate) != NATIVE_MIDI_PAUSED || !song->allow_pause)
        return;

    (void)!write(song->mainsock, pkt_thread_cmd_resume, CMD_PKT_LEN);
}

void native_midi_stop(void)
{
    NativeMidiSong *song = currentsong;

    if (!song || !song->playerthread)
        return;

    /* Don't send any messages to the main thread if it's out of the main loop */
    if (SDL_GetAtomicInt(&song->playerstate) > NATIVE_MIDI_STOPPED)
        if (write(song->mainsock, pkt_thread_cmd_quit, CMD_PKT_LEN) != sizeof(pkt_thread_cmd_quit))
            return;

    SDL_WaitThread(song->playerthread, NULL);
    song->playerthread = NULL;
}

bool native_midi_active(void)
{
    NativeMidiSong *song = currentsong;

    if (!song)
        return 0;

    return SDL_GetAtomicInt(&song->playerstate) > NATIVE_MIDI_STOPPED;
}

void native_midi_setvolume(int volume)
{
    NativeMidiSong *song = currentsong;

    if (!song || SDL_GetAtomicInt(&song->playerstate) != NATIVE_MIDI_PLAYING)
        return;

    if (volume < 0)
        volume = 0;
    else if (volume > 0x7F)
        volume = 0x7F;

    unsigned char pkt_thread_cmd_setvol[CMD_PKT_LEN] = { THREAD_CMD_SETVOL, volume };
    (void)!write(song->mainsock, pkt_thread_cmd_setvol, CMD_PKT_LEN);
}

const char *native_midi_error(void)
{
    return errmsg_final;
}

#endif
