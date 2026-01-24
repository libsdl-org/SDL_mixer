/*
 * This example code creates a mixer, loads two sounds, and mixes them.
 *
 * SDL_mixer is, of course, a mixer, so here are two sounds mixing together!
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
static MIX_Track *track1 = NULL;

static MIX_Audio *load_audio(const char *fname)
{
    char *path = NULL;
    MIX_Audio *audio;
    SDL_asprintf(&path, "%s%s", SDL_GetBasePath(), fname);  /* allocate a string of the full file path */
    audio = MIX_LoadAudio(mixer, path, false);
    if (!audio) {
        SDL_Log("Couldn't load %s: %s", path, SDL_GetError());
    }

    SDL_free(path);  /* done with this. */
    return audio;
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    MIX_Audio *music = NULL;
    MIX_Audio *sound = NULL;
    MIX_Track *track2 = NULL;
    SDL_PropertiesID options = 0;

    SDL_SetAppMetadata("Example Play Multiple Sounds", "1.0", "com.example.play-multiple-sounds");

    /* this doesn't have to run very much, so give up tons of CPU time between iterations. Optional! */
    SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "5");

    /* we don't need video, but we'll make a window for smooth operation. */
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("examples/basic/play-multiple-sounds", 640, 480, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
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

    /* load our audio files. Note that you can use any supported file format! */
    music = load_audio("music.mp3");
    if (!music) {
        return SDL_APP_FAILURE;  /* we reported the error in load_audio */
    }
    sound = load_audio("sword.wav");
    if (!sound) {
        return SDL_APP_FAILURE;  /* we reported the error in load_audio */
    }

    /* we need a track on the mixer to play the audio. Each track has audio assigned to it, and
       all playing tracks are mixed together for the final output. */

    track1 = MIX_CreateTrack(mixer);
    if (!track1) {
        SDL_Log("Couldn't create a mixer track: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    MIX_SetTrackAudio(track1, music);

    track2 = MIX_CreateTrack(mixer);
    if (!track2) {
        SDL_Log("Couldn't create a mixer track: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    MIX_SetTrackAudio(track2, sound);
    options = SDL_CreateProperties();
    if (!options) {
        SDL_Log("Couldn't create play options: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);  /* loop forever. */

    /* start the audio playing! music plays through once, with the sound effect playing in a loop at the same time. */
    MIX_PlayTrack(track1, 0);  /* no extra options this time, so a zero for the second argument. */
    MIX_PlayTrack(track2, options);
    SDL_DestroyProperties(options);  /* MIX_PlayTrack makes a copy of the options, so this can go away. */

    /* we don't save `music`, `sound`, or `track2`; SDL_mixer will clean them up for us during MIX_Quit(). */

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

    /* when the music has finished playing, end the program. */
    if (!MIX_TrackPlaying(track1)) {
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

