/*
  PLAYMUS:  A test application for the SDL mixer library.
  Copyright (C) 1997-2022 Sam Lantinga <slouken@libsdl.org>

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

/* Quiet windows compiler warnings */
#define _CRT_SECURE_NO_WARNINGS

#include "SDL_stdinc.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef unix
#include <unistd.h>
#endif

#include "SDL.h"
#include "SDL_mixer.h"

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif


static int audio_open = 0;
static Mix_Music *music = NULL;
static int next_track = 0;

void CleanUp(int exitcode)
{
    if(Mix_PlayingMusic()) {
        Mix_FadeOutMusic(1500);
        SDL_Delay(1500);
    }
    if (music) {
        Mix_FreeMusic(music);
        music = NULL;
    }
    if (audio_open) {
        Mix_CloseAudio();
        audio_open = 0;
    }
    SDL_Quit();
    exit(exitcode);
}

void Usage(char *argv0)
{
    SDL_Log("Usage: %s [-i] [-l] [-8] [-f32] [-r rate] [-c channels] [-b buffers] [-v N] [-rwops] <musicfile>\n", argv0);
}

/*#define SEEK_TEST */
void Menu(void)
{
    char buf[10];

    printf("Available commands: (p)ause (r)esume (h)alt volume(v#) > ");
    fflush(stdin);
    if (scanf("%s",buf) == 1) {
        switch(buf[0]){
#if defined(SEEK_TEST)
        case '0': Mix_SetMusicPosition(0); break;
        case '1': Mix_SetMusicPosition(10);break;
        case '2': Mix_SetMusicPosition(20);break;
        case '3': Mix_SetMusicPosition(30);break;
        case '4': Mix_SetMusicPosition(40);break;
#endif /* SEEK_TEST */
        case 'p': case 'P':
            Mix_PauseMusic();
            break;
        case 'r': case 'R':
            Mix_ResumeMusic();
            break;
        case 'h': case 'H':
            Mix_HaltMusic();
            break;
        case 'v': case 'V':
            Mix_VolumeMusic(atoi(buf+1));
            break;
        }
    }
    printf("Music playing: %s Paused: %s\n", Mix_PlayingMusic() ? "yes" : "no",
           Mix_PausedMusic() ? "yes" : "no");
}

#ifdef HAVE_SIGNAL_H

void IntHandler(int sig)
{
    switch (sig) {
            case SIGINT:
            next_track++;
            break;
    }
}

#endif

int main(int argc, char *argv[])
{
    int audio_rate;
    Uint16 audio_format;
    int audio_channels;
    int audio_buffers;
    int audio_volume = MIX_MAX_VOLUME;
    int looping = 0;
    int interactive = 0;
    int rwops = 0;
    int i;
    const char *typ;
    const char *tag_title = NULL;
    const char *tag_artist = NULL;
    const char *tag_album = NULL;
    const char *tag_copyright = NULL;
    double loop_start, loop_end, loop_length, current_position;

    (void) argc;

    /* Initialize variables */
    audio_rate = MIX_DEFAULT_FREQUENCY;
    audio_format = MIX_DEFAULT_FORMAT;
    audio_channels = MIX_DEFAULT_CHANNELS;
    audio_buffers = 4096;

    /* Check command line usage */
    for (i=1; argv[i] && (*argv[i] == '-'); ++i) {
        if ((strcmp(argv[i], "-r") == 0) && argv[i+1]) {
            ++i;
            audio_rate = atoi(argv[i]);
        } else
        if (strcmp(argv[i], "-m") == 0) {
            audio_channels = 1;
        } else
        if ((strcmp(argv[i], "-c") == 0) && argv[i+1]) {
            ++i;
            audio_channels = atoi(argv[i]);
        } else
        if ((strcmp(argv[i], "-b") == 0) && argv[i+1]) {
            ++i;
            audio_buffers = atoi(argv[i]);
        } else
        if ((strcmp(argv[i], "-v") == 0) && argv[i+1]) {
            ++i;
            audio_volume = atoi(argv[i]);
        } else
        if (strcmp(argv[i], "-l") == 0) {
            looping = -1;
        } else
        if (strcmp(argv[i], "-i") == 0) {
            interactive = 1;
        } else
        if (strcmp(argv[i], "-8") == 0) {
            audio_format = AUDIO_U8;
        } else
        if (strcmp(argv[i], "-f32") == 0) {
            audio_format = AUDIO_F32;
        } else
        if (strcmp(argv[i], "-rwops") == 0) {
            rwops = 1;
        } else {
            Usage(argv[0]);
            return(1);
        }
    }
    if (! argv[i]) {
        Usage(argv[0]);
        return(1);
    }

    /* Initialize the SDL library */
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        SDL_Log("Couldn't initialize SDL: %s\n",SDL_GetError());
        return(255);
    }

#ifdef HAVE_SIGNAL_H
    signal(SIGINT, IntHandler);
    signal(SIGTERM, CleanUp);
#endif

    /* Open the audio device */
    if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0) {
        SDL_Log("Couldn't open audio: %s\n", SDL_GetError());
        return(2);
    } else {
        Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
        SDL_Log("Opened audio at %d Hz %d bit%s %s %d bytes audio buffer\n", audio_rate,
            (audio_format&0xFF),
            (SDL_AUDIO_ISFLOAT(audio_format) ? " (float)" : ""),
            (audio_channels > 2) ? "surround" : (audio_channels > 1) ? "stereo" : "mono",
            audio_buffers);
    }
    audio_open = 1;

    /* Set the music volume */
    Mix_VolumeMusic(audio_volume);

    /* Set the external music player, if any */
    Mix_SetMusicCMD(SDL_getenv("MUSIC_CMD"));

    while (argv[i]) {
        next_track = 0;

        /* Load the requested music file */
        if (rwops) {
            music = Mix_LoadMUS_RW(SDL_RWFromFile(argv[i], "rb"), SDL_TRUE);
        } else {
            music = Mix_LoadMUS(argv[i]);
        }
        if (music == NULL) {
            SDL_Log("Couldn't load %s: %s\n",
                argv[i], SDL_GetError());
            CleanUp(2);
        }

        switch (Mix_GetMusicType(music)) {
        case MUS_CMD:
            typ = "CMD";
            break;
        case MUS_WAV:
            typ = "WAV";
            break;
        case MUS_MOD:
        case MUS_MODPLUG_UNUSED:
            typ = "MOD";
            break;
        case MUS_FLAC:
            typ = "FLAC";
            break;
        case MUS_MID:
            typ = "MIDI";
            break;
        case MUS_OGG:
            typ = "OGG Vorbis";
            break;
        case MUS_MP3:
        case MUS_MP3_MAD_UNUSED:
            typ = "MP3";
            break;
        case MUS_OPUS:
            typ = "OPUS";
            break;
        case MUS_NONE:
        default:
            typ = "NONE";
            break;
        }
        SDL_Log("Detected music type: %s", typ);

        tag_title = Mix_GetMusicTitleTag(music);
        if (tag_title && SDL_strlen(tag_title) > 0) {
            SDL_Log("Title: %s", tag_title);
        }

        tag_artist = Mix_GetMusicArtistTag(music);
        if (tag_artist && SDL_strlen(tag_artist) > 0) {
            SDL_Log("Artist: %s", tag_artist);
        }

        tag_album = Mix_GetMusicAlbumTag(music);
        if (tag_album && SDL_strlen(tag_album) > 0) {
            SDL_Log("Album: %s", tag_album);
        }

        tag_copyright = Mix_GetMusicCopyrightTag(music);
        if (tag_copyright && SDL_strlen(tag_copyright) > 0) {
            SDL_Log("Copyright: %s", tag_copyright);
        }

        loop_start = Mix_GetMusicLoopStartTime(music);
        loop_end = Mix_GetMusicLoopEndTime(music);
        loop_length = Mix_GetMusicLoopLengthTime(music);

        /* Play and then exit */
        SDL_Log("Playing %s, duration %f\n", argv[i], Mix_MusicDuration(music));
        if (loop_start > 0.0 && loop_end > 0.0 && loop_length > 0.0) {
            SDL_Log("Loop points: start %g s, end %g s, length %g s\n", loop_start, loop_end, loop_length);
        }
        Mix_FadeInMusic(music,looping,2000);
        while (!next_track && (Mix_PlayingMusic() || Mix_PausedMusic())) {
            if(interactive)
                Menu();
            else {
                current_position = Mix_GetMusicPosition(music);
                if (current_position >= 0.0) {
                    printf("Position: %g seconds             \r", current_position);
                    fflush(stdout);
                }
                SDL_Delay(100);
            }
        }
        Mix_FreeMusic(music);
        music = NULL;

        /* If the user presses Ctrl-C more than once, exit. */
        SDL_Delay(500);
        if (next_track > 1) break;

        i++;
    }
    CleanUp(0);

    /* Not reached, but fixes compiler warnings */
    return 0;
}

/* vi: set ts=4 sw=4 expandtab: */
