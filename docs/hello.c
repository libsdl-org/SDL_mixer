/*
  Copyright (C) 1997-2026 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_mixer/SDL_mixer.h>

static MIX_Mixer *mixer;
static MIX_Track *track;
static MIX_Audio *audio;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    const char *file;

    /* this doesn't have to run very much, so give up tons of CPU time between iterations. */
    SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "5");

    if (argc != 2) {
        SDL_Log("Usage: %s <file_to_play>", argv[0]);
        return SDL_APP_FAILURE;
    }
	file = argv[1];

    if (!MIX_Init()) {
        SDL_Log("Couldn't initialize SDL_mixer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (!mixer) {
        SDL_Log("Couldn't create mixer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    audio = MIX_LoadAudio(mixer, file, true);
    if (!audio) {
        SDL_Log("Couldn't load audio from %s: %s", file, SDL_GetError());
        return SDL_APP_FAILURE;
    }

    track = MIX_CreateTrack(mixer);
    if (!track) {
        SDL_Log("Couldn't create track: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    MIX_SetTrackAudio(track, audio);
    MIX_PlayTrack(track, 0);

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
    if (MIX_TrackPlaying(track)) {
        return SDL_APP_CONTINUE;
    }
    return SDL_APP_SUCCESS;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	/* This will close the audio device and free all mixers and audio data */
    MIX_Quit();
}

