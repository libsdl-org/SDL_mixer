/*
 * This example code creates a mixer, loads a single sound, and plays it, with
 * several playback options (fade-in, loop, etc).
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
static MIX_Mixer *mixer = NULL;
static MIX_Track *track = NULL;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    char *path = NULL;
    MIX_Audio *audio = NULL;
    SDL_PropertiesID options = 0;

    SDL_SetAppMetadata("Example Play With Options", "1.0", "com.example.play-with-options");

    /* this doesn't have to run very much, so give up tons of CPU time between iterations. Optional! */
    SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "5");

    /* we don't need video, but we'll make a window for smooth operation. */
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("examples/basic/play-with-options", 640, 480, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
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

    /* load a sound file */
    SDL_asprintf(&path, "%smusic.mp3", SDL_GetBasePath());  /* allocate a string of the full file path */
    audio = MIX_LoadAudio(mixer, path, false);
    if (!audio) {
        SDL_Log("Couldn't load %s: %s", path, SDL_GetError());
        SDL_free(path);
        return SDL_APP_FAILURE;
    }

    SDL_free(path);  /* done with this, the file is loaded. */

    /* we need a track on the mixer to play the audio. Each track has audio assigned to it, and
       all playing tracks are mixed together for the final output. */

    track = MIX_CreateTrack(mixer);
    if (!track) {
        SDL_Log("Couldn't create a mixer track: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    MIX_SetTrackAudio(track, audio);

    /* start the audio playing! */
    options = SDL_CreateProperties();
    if (!options) {
        SDL_Log("Couldn't create play options: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    /* note these all use MILLISECONDS, since the track is generic, but you can use equivalent FRAMES properties, for frame-perfect mixing. */
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_START_MILLISECOND_NUMBER, 1000);  /* start the first loop 1 second into the audio. */
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_MAX_MILLISECONDS_NUMBER, 10000);  /* play at most 10 seconds of audio before ending/looping (since we started 1000 in, this will be the eleventh second). */
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, 2);  /* loop 2 times, 3rd time through doesn't loop. */
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOP_START_MILLISECOND_NUMBER, 2000); /* when looping, start again here instead of the start of the track. */
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, 5000);  /* fade-in the first loop over five seconds. No fade on the loops, unless the initial fade is still in progress! */
    MIX_PlayTrack(track, options);
    SDL_DestroyProperties(options);  /* MIX_PlayTrack makes a copy of the options, so this can go away. */

    /* we don't save `audio`; SDL_mixer will clean it up for us during MIX_Quit(). */

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

    /* when the track has finished playing, end the program. */
    if (!MIX_TrackPlaying(track)) {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us, MIX_Quit() destroys any mixer objects we made. */
    MIX_Quit();
}

