/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2026 Sam Lantinga <slouken@libsdl.org>

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

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "SDL3_mixer/SDL_mixer.h"

static SDL_AudioStream *stream = NULL;
static SDL_AtomicInt done;  /* the device callback happens in another thread, so use an atomic int for this flag. */
static SDL_AudioSpec spec;

static void SDLCALL AudioDeviceCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount)
{
    MIX_AudioDecoder *audiodecoder = (MIX_AudioDecoder *) userdata;
    Uint8 buffer[1024];

    while (additional_amount > 0) {
        const int needed = SDL_min((int)sizeof(buffer), additional_amount);
        const int br = MIX_DecodeAudio(audiodecoder, buffer, needed, &spec);
        if (br <= 0) {
            SDL_SetAtomicInt(&done, 1);
            break;
        }

        SDL_PutAudioStreamData(stream, buffer, br);
        additional_amount -= br;
    }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("Test SDL_mixer MIX_AudioDecoder", "1.0", "org.libsdl.testmixeraudiodecoder");

    /* this doesn't have to run very much, so give up tons of CPU time between iterations. */
    SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "5");

    if (argc != 2) {
        SDL_Log("USAGE: %s <file_to_play>", argv[0]);
        return SDL_APP_FAILURE;
    } else if (!SDL_Init(SDL_INIT_AUDIO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    } else if (!MIX_Init()) {
        SDL_Log("Couldn't initialize SDL_mixer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    const char *audiofname = argv[1];

    MIX_AudioDecoder *audiodecoder = MIX_CreateAudioDecoder(audiofname, 0);
    if (!audiodecoder) {
        SDL_Log("Failed to create audiodecoder for '%s': %s", audiofname, SDL_GetError());
        return SDL_APP_FAILURE;
    }

    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL, AudioDeviceCallback, audiodecoder);
    if (!stream) {
        SDL_Log("Failed to open audio device: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GetAudioStreamFormat(stream, &spec, NULL);

    SDL_ResumeAudioStreamDevice(stream);

    SDL_SetAtomicInt(&done, 0);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    return SDL_GetAtomicInt(&done) ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    // SDL and MIX_Quit will normally clean up everything we used here, but
    //  we need to make sure the callback doesn't fire while MIX_Quit is
    //  destroying things, so we destroy the audio stream manually.
    SDL_DestroyAudioStream(stream);
    MIX_Quit();    // SDL_mixer will clean up the audiodecoder.
}

