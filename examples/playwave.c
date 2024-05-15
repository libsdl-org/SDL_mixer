/*
  PLAYWAVE:  A test application for the SDL mixer library.
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

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_test.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef unix
#include <unistd.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

static int audio_open = 0;
static Mix_Chunk *g_wave = NULL;
static SDLTest_CommonState *state;
static SDL_bool verbose = SDL_FALSE;
static SDL_bool test_position = SDL_FALSE;
static SDL_bool test_distance = SDL_FALSE;
static SDL_bool test_panning = SDL_FALSE;

static void report_decoders(void)
{
    int i, total;

    SDL_Log("Supported decoders...\n");
    total = Mix_GetNumChunkDecoders();
    for (i = 0; i < total; i++) {
        SDL_Log(" - chunk decoder: %s\n", Mix_GetChunkDecoder(i));
    }

    total = Mix_GetNumMusicDecoders();
    for (i = 0; i < total; i++) {
        SDL_Log(" - music decoder: %s\n", Mix_GetMusicDecoder(i));
    }
}

static void output_versions(const char *libname, int compiled, int linked)
{
    SDL_Log("This program was compiled against %s %d.%d.%d,\n"
            " and is dynamically linked to %d.%d.%d.\n", libname,
        SDL_VERSIONNUM_MAJOR(compiled),
        SDL_VERSIONNUM_MINOR(compiled),
        SDL_VERSIONNUM_MICRO(compiled),
        SDL_VERSIONNUM_MAJOR(linked),
        SDL_VERSIONNUM_MINOR(linked),
        SDL_VERSIONNUM_MICRO(linked));
}

static void test_versions(void)
{
    output_versions("SDL", SDL_VERSION, SDL_GetVersion());
    output_versions("SDL_mixer", SDL_MIXER_VERSION, Mix_Version());
}

static int channel_is_done = 0;
static void SDLCALL channel_complete_callback (int chan)
{
    if (verbose) {
        Mix_Chunk *done_chunk = Mix_GetChunk(chan);
        SDL_Log("We were just alerted that Mixer channel #%d is done.\n", chan);
        SDL_Log("Channel's chunk pointer is (%p).\n", (void *) done_chunk);
        SDL_Log(" Which %s correct.\n", (g_wave == done_chunk) ? "is" : "is NOT");
    }
    channel_is_done = 1;
}

/* rcg06192001 abstract this out for testing purposes. */
static int still_playing(void)
{
    return Mix_Playing(0);
}

static void do_panning_update(void)
{
    static Uint8 leftvol = 128;
    static Uint8 rightvol = 128;
    static Sint8 leftincr = -1;
    static Sint8 rightincr = 1;
    static int panningok = 1;
    static Uint32 next_panning_update = 0;

    if (panningok && (SDL_GetTicks() >= next_panning_update)) {
        panningok = Mix_SetPanning(0, leftvol, rightvol);
        if (!panningok) {
            SDL_Log("Mix_SetPanning(0, %d, %d) failed!\n",
                    (int) leftvol, (int) rightvol);
            SDL_Log("Reason: [%s].\n", SDL_GetError());
        }

        if ((leftvol == 255) || (leftvol == 0)) {
            if (leftvol == 255) {
                SDL_Log("All the way in the left speaker.\n");
            }
            leftincr *= -1;
        }

        if ((rightvol == 255) || (rightvol == 0)) {
            if (rightvol == 255) {
                SDL_Log("All the way in the right speaker.\n");
            }
            rightincr *= -1;
        }

        leftvol += leftincr;
        rightvol += rightincr;
        next_panning_update = SDL_GetTicks() + 10;
    }
}

static void do_distance_update(void)
{
    static Uint8 distance = 1;
    static Sint8 distincr = 1;
    static int distanceok = 1;
    static Uint32 next_distance_update = 0;

    if ((distanceok) && (SDL_GetTicks() >= next_distance_update)) {
        distanceok = Mix_SetDistance(0, distance);
        if (!distanceok) {
            SDL_Log("Mix_SetDistance(0, %d) failed!\n", (int) distance);
            SDL_Log("Reason: [%s].\n", SDL_GetError());
        }

        if (distance == 0) {
            SDL_Log("Distance at nearest point.\n");
            distincr *= -1;
        }
        else if (distance == 255) {
            SDL_Log("Distance at furthest point.\n");
            distincr *= -1;
        }

        distance += distincr;
        next_distance_update = SDL_GetTicks() + 15;
    }
}

static void do_position_update(void)
{
    static Sint16 distance = 1;
    static Sint8 distincr = 1;
    static Sint16 angle = 0;
    static Sint8 angleincr = 1;
    static int positionok = 1;
    static Uint32 next_position_update = 0;

    if (positionok && (SDL_GetTicks() >= next_position_update)) {
        positionok = Mix_SetPosition(0, angle, (Uint8)distance);
        if (!positionok) {
            SDL_Log("Mix_SetPosition(0, %d, %d) failed!\n",
                    (int) angle, (int) distance);
            SDL_Log("Reason: [%s].\n", SDL_GetError());
        }

        if (angle == 0) {
            SDL_Log("Due north; now rotating clockwise...\n");
            angleincr = 1;
        }

        else if (angle == 360) {
            SDL_Log("Due north; now rotating counter-clockwise...\n");
            angleincr = -1;
        }

        distance += distincr;
        if (distance < 0) {
            distance = 0;
            distincr = 3;
            SDL_Log("Distance is very, very near. Stepping away by threes...\n");
        } else if (distance > 255) {
            distance = 255;
            distincr = -3;
            SDL_Log("Distance is very, very far. Stepping towards by threes...\n");
        }

        angle += angleincr;
        next_position_update = SDL_GetTicks() + 30;
    }
}

static void CleanUp(int exitcode)
{
    if (g_wave) {
        Mix_FreeChunk(g_wave);
        g_wave = NULL;
    }
    if (audio_open) {
        Mix_CloseAudio();
        audio_open = 0;
    }
    SDL_Quit();
    SDLTest_CommonDestroyState(state);

    exit(exitcode);
}

/*
 * rcg06182001 This is sick, but cool.
 *
 *  Actually, it's meant to be an example of how to manipulate a voice
 *  without having to use the mixer effects API. This is more processing
 *  up front, but no extra during the mixing process. Also, in a case like
 *  this, when you need to touch the whole sample at once, it's the only
 *  option you've got. And, with the effects API, you are altering a copy of
 *  the original sample for each playback, and thus, your changes aren't
 *  permanent; here, you've got a reversed sample, and that's that until
 *  you either reverse it again, or reload it.
 */
static void flip_sample(Mix_Chunk *wave)
{
    SDL_AudioFormat format;
    int channels, i, incr;
    Uint8 *start = wave->abuf;
    Uint8 *end = wave->abuf + wave->alen;

    Mix_QuerySpec(NULL, &format, &channels);
    incr = SDL_AUDIO_BITSIZE(format) * channels;

    end -= incr;

    switch (incr) {
        case 8:
            for (i = wave->alen / 2; i >= 0; i -= 1) {
                Uint8 tmp = *start;
                *start = *end;
                *end = tmp;
                start++;
                end--;
            }
            break;

        case 16:
            for (i = wave->alen / 2; i >= 0; i -= 2) {
                Uint16 tmp = *start;
                *((Uint16 *) start) = *((Uint16 *) end);
                *((Uint16 *) end) = tmp;
                start += 2;
                end -= 2;
            }
            break;

        case 32:
            for (i = wave->alen / 2; i >= 0; i -= 4) {
                Uint32 tmp = *start;
                *((Uint32 *) start) = *((Uint32 *) end);
                *((Uint32 *) end) = tmp;
                start += 4;
                end -= 4;
            }
            break;

        case 64:
            for (i = wave->alen / 2; i >= 0; i -= 8) {
                Uint64 tmp = *start;
                *((Uint64 *) start) = *((Uint64 *) end);
                *((Uint64 *) end) = tmp;
                start += 8;
                end -= 8;
            }
            break;

        default:
            SDL_Log("Unhandled format in sample flipping.\n");
            return;
    }
}


int main(int argc, char *argv[])
{
    SDL_AudioSpec spec;
    int loops = 0;
    int i;
    int reverse_stereo = 0;
    int reverse_sample = 0;
    const char *filename = NULL;

    /* Initialize test framework */
    state = SDLTest_CommonCreateState(argv, 0);
    if (!state) {
        return 1;
    }

    /* Enable standard application logging */
    SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

#ifdef HAVE_SETBUF
    setbuf(stdout, NULL);    /* rcg06132001 for debugging purposes. */
    setbuf(stderr, NULL);    /* rcg06192001 for debugging purposes, too. */
#endif

    /* Initialize variables */
    spec.freq = MIX_DEFAULT_FREQUENCY;
    spec.format = MIX_DEFAULT_FORMAT;
    spec.channels = MIX_DEFAULT_CHANNELS;

    /* Parse commandline */
    for (i = 1; i < argc;) {
        int consumed;

        consumed = SDLTest_CommonArg(state, i);
        if (!consumed) {
            if (SDL_strcmp("-r", argv[i]) == 0) {
                spec.freq = SDL_atoi(argv[i + 1]);
                consumed = 2;
            } else if (SDL_strcmp("-m", argv[i]) == 0) {
                spec.channels = 1;
                consumed = 1;
            } else if (SDL_strcmp("-c", argv[i]) == 0) {
                spec.channels = SDL_atoi(argv[i + 1]);
                consumed = 2;
            } else if (SDL_strcmp("-l", argv[i]) == 0) {
                loops = -1;
                consumed = 1;
            } else if (SDL_strcmp("-8", argv[i]) == 0) {
                spec.format = SDL_AUDIO_U8;
                consumed = 1;
            } else if (SDL_strcmp("-f32", argv[i]) == 0) {
                spec.format = SDL_AUDIO_F32;
                consumed = 1;
            } else if (SDL_strcmp("-f", argv[i]) == 0) {
                reverse_stereo = 1;
                consumed = 1;
            } else if (SDL_strcmp("-F", argv[i]) == 0) {
                reverse_sample = 1;
                consumed = 1;
            } else if (SDL_strcmp("--panning", argv[i]) == 0) {
                test_panning = SDL_TRUE;
                consumed = 1;
            } else if (SDL_strcmp("--distance", argv[i]) == 0) {
                test_distance = SDL_TRUE;
                consumed = 1;
            } else if (SDL_strcmp("--position", argv[i]) == 0) {
                test_position = SDL_TRUE;
                consumed = 1;
            } else if (SDL_strcmp("--version", argv[i]) == 0) {
                test_versions();
                CleanUp(0);
                consumed = 1;
            } else if (SDL_strcmp("--verbose", argv[i]) == 0) {
                verbose = SDL_TRUE;
                consumed = 1;
            } else if (argv[i][0] != '-' && !filename) {
                filename = argv[i];
                consumed = 1;
            }
        }
        if (consumed <= 0) {
            static const char *options[] = { "[-r rate]", "[-m]", "[-c channels]", "[-l]", "[-8]",
                                             "[-f32]", "[-f]", "[-F]", "[--distance]", "[--panning]",
                                             "[--position]", "[--version]", "<wavefile>", NULL };
            SDLTest_CommonLogUsage(state, argv[0], options);
            return 1;
        }

        i += consumed;
    }
    if (test_position && (test_distance || test_panning)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "position cannot be combined with distance or panning");
        CleanUp(1);
    }

    /* Initialize the SDL library */
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        SDL_Log("Couldn't initialize SDL: %s\n",SDL_GetError());
        return 255;
    }
#ifdef HAVE_SIGNAL_H
    signal(SIGINT, CleanUp);
    signal(SIGTERM, CleanUp);
#endif

    /* Open the audio device */
    if (!Mix_OpenAudio(0, &spec)) {
        SDL_Log("Couldn't open audio: %s\n", SDL_GetError());
        CleanUp(2);
    } else {
        Mix_QuerySpec(&spec.freq, &spec.format, &spec.channels);
        SDL_Log("Opened audio at %d Hz %d bit%s %s", spec.freq,
            (spec.format&0xFF),
            (SDL_AUDIO_ISFLOAT(spec.format) ? " (float)" : ""),
            (spec.channels > 2) ? "surround" :
            (spec.channels > 1) ? "stereo" : "mono");
        if (loops) {
          SDL_Log(" (looping)\n");
        } else {
          putchar('\n');
        }
    }
    audio_open = 1;

    if (verbose) {
        report_decoders();
    }

    /* Load the requested wave file */
    g_wave = Mix_LoadWAV(filename);
    if (g_wave == NULL) {
        SDL_Log("Couldn't load %s: %s\n", filename, SDL_GetError());
        CleanUp(2);
    }

    if (reverse_sample) {
        flip_sample(g_wave);
    }

    Mix_ChannelFinished(channel_complete_callback);

    if ((!Mix_SetReverseStereo(MIX_CHANNEL_POST, reverse_stereo)) &&
         (reverse_stereo))
    {
        SDL_Log("Failed to set up reverse stereo effect!\n");
        SDL_Log("Reason: [%s].\n", SDL_GetError());
    }

    /* Play and then exit */
    Mix_PlayChannel(0, g_wave, loops);

    while (still_playing()) {

        if (test_panning) {
            do_panning_update();
        }

        if (test_distance) {
            do_distance_update();
        }

        if (test_position) {
            do_position_update();
        }

        SDL_Delay(1);

    } /* while still_playing() loop... */

    CleanUp(0);

    /* Not reached, but fixes compiler warnings */
    return 0;
}

/* end of playwave.c ... */
