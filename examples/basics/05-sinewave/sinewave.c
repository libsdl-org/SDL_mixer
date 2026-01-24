/*
 * This example code creates a mixer, and plays a sinewave forever.
 *
 * It's not super-useful to play a sinewave, but it is _something_
 * to play if you don't have anything else, and it's built-in to
 * SDL_mixer.
 *
 * This code is public domain. Feel free to use it for any purpose!
 */

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_mixer/SDL_mixer.h>

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    MIX_Mixer *mixer = NULL;
    MIX_Audio *audio = NULL;
    MIX_Track *track = NULL;

    SDL_SetAppMetadata("Example Load And Play", "1.0", "com.example.load-and-play");

    /* this doesn't have to run very much, so give up tons of CPU time between iterations. Optional! */
    SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "5");

    /* we don't need video, but we'll make a window for smooth operation. */
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("examples/basic/load-and-play", 640, 480, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!MIX_Init()) {
        SDL_Log("Couldn't init SDL_mixer library: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    /* Create a mixer on the default audio device. Don't care about the specific audio format. */
    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (!mixer) {
        SDL_Log("Couldn't create mixer on default device: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    audio = MIX_CreateSineWaveAudio(mixer, 300, 0.25f, -1);   // -1: play forever. You can specify milliseconds otherwise to have a limit.
    if (!audio) {
        SDL_Log("Couldn't generate sinewave: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    /* we need a track on the mixer to play the audio. Each track has audio assigned to it, and
       all playing tracks are mixed together for the final output. */

    track = MIX_CreateTrack(mixer);
    if (!track) {
        SDL_Log("Couldn't create a mixer track: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    MIX_SetTrackAudio(track, audio);

    /* start the audio playing! */
    MIX_PlayTrack(track, 0);  /* no extra options this time, so a zero for the second argument. */

    /* we don't save `mixer`, `audio`, or `track`; SDL_mixer will clean it up for us during MIX_Quit(). */

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    /* draw a blank video frame to keep the OS happy */
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    /* there's nothing for use to do here, the sinewave will play forever until the app is manually quit. */

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us, MIX_Quit() destroys any mixer objects we made. */
    MIX_Quit();
}

