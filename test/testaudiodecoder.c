#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "SDL3_mixer/SDL_mixer.h"

static bool done = false;
static SDL_AudioSpec spec;

static void SDLCALL AudioDeviceCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount)
{
    MIX_AudioDecoder *audiodecoder = (MIX_AudioDecoder *) userdata;
    if (!additional_amount) {
        return;
    }

    Uint8 buffer[1024];
    while (additional_amount > 0) {
        const int needed = SDL_min(sizeof (buffer), additional_amount);
        const int br = MIX_DecodeAudio(audiodecoder, buffer, needed, &spec);
        if (br <= 0) {
            done = true;
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

    SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL, AudioDeviceCallback, audiodecoder);
    if (!stream) {
        SDL_Log("Failed to open audio device: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GetAudioStreamFormat(stream, &spec, NULL);

    SDL_ResumeAudioStreamDevice(stream);

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
    return done ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    // SDL will clean up the audio device for us.
    MIX_Quit();    // SDL_mixer will clean up the audiodecoder.
}

