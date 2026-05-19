/*
 * This example code creates a mixer, plays sounds, and tags them into groups.
 *
 * We'll play a background music track, and tag the other sounds as "in-game".
 * So when you click the "pause" button, (what would be) your game sounds can
 * all pause at once, but the background music can continue on.
 *
 * This code is public domain. Feel free to use it for any purpose!
 */

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_mixer/SDL_mixer.h>

#define TAG_INGAME "in-game"

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static MIX_Mixer *mixer = NULL;
static MIX_Track *tracks[32];

static Uint64 next_play_ticks = 0;  /* next time we will start a sound effect. */
static size_t next_play_track = 1;  /* next track we'll use when we start a sound effect. */
static bool paused = false;  /* whether our fake game is paused at the moment. */

static struct {
    const char *filename;
    MIX_Audio *audio;
} loaded_audio[] = {
    { "music.mp3", NULL },
    { "sword.wav", NULL },
    { "splash.wav", NULL },
    { "spring.wav", NULL }
};

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
    SDL_PropertiesID options = 0;
    int i;

    SDL_SetAppMetadata("Example Tagging Tracks", "1.0", "com.example.tagging-tracks");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("examples/advanced/tagging-tracks", 640, 480, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
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
    for (i = 0; i < (int) SDL_arraysize(loaded_audio); i++) {
        loaded_audio[i].audio = load_audio(loaded_audio[i].filename);
        if (!loaded_audio[i].audio) {
            return SDL_APP_FAILURE;  /* we reported the error in load_audio */
        }
    }

    /* we need tracks on the mixer to play the audio. Each track has audio
       assigned to it, and all playing tracks are mixed together for the final
       output. */
    for (i = 0; i < (int) SDL_arraysize(tracks); i++) {
        tracks[i] = MIX_CreateTrack(mixer);
        if (!tracks[i]) {
            SDL_Log("Couldn't create a mixer track: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }
        if (i > 0) {  /* everything but the background music is tagged as in-game for this example. */
            if (!MIX_TagTrack(tracks[i], TAG_INGAME)) {
                SDL_Log("Couldn't tag mixer track #%d: %s", i, SDL_GetError());
                return SDL_APP_FAILURE;
            }
        }
    }

    /* Put the music (first thing we loaded) on track[0], for simplicity here. */
    MIX_SetTrackAudio(tracks[0], loaded_audio[0].audio);

    options = SDL_CreateProperties();
    if (!options) {
        SDL_Log("Couldn't create play options: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);  /* loop forever. */

    /* start the music playing! it loops forever. */
    MIX_PlayTrack(tracks[0], options);
    SDL_DestroyProperties(options);  /* MIX_PlayTrack makes a copy of the options, so this can go away. */

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    } else if ((event->type == SDL_EVENT_KEY_DOWN) && (event->key.key == SDLK_SPACE)) {
        paused = !paused;
        if (paused) {
            MIX_PauseTag(mixer, TAG_INGAME);
        } else {
            MIX_ResumeTag(mixer, TAG_INGAME);
        }
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

static void draw_centered_text(SDL_Renderer *renderer, const int rw, int *y, const char *str)
{
    const int x = (rw - (((int) SDL_strlen(str)) * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE)) / 2;
    SDL_RenderDebugText(renderer, (float) x, (float) *y, str);
    *y += SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * 2;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    const Uint64 now = SDL_GetTicks();
    int rw, rh, y;

    if (!paused && (now >= next_play_ticks)) {   /* simulate video game sounds by starting a new one every now and then, with some randomness. */
        MIX_Track *track = tracks[next_play_track];
        /* these sounds are short enough that the tracks will finish playing before we reuse them */
        MIX_SetTrackAudio(track, loaded_audio[SDL_rand(SDL_arraysize(loaded_audio) - 1) + 1].audio);  /* pick a random not-music audio sound. */
        MIX_PlayTrack(track, 0);
        next_play_track++;
        if (next_play_track >= SDL_arraysize(tracks)) {
            next_play_track = 1;  /* tracks[0] is the music, skip it. */
        }
        next_play_ticks = now + SDL_rand(1000);
    }

    SDL_GetCurrentRenderOutputSize(renderer, &rw, &rh);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  /* clear to black */
    SDL_RenderClear(renderer);

    y = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * 8;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  /* white text */
    draw_centered_text(renderer, rw, &y, "PRETEND THIS IS A VIDEO GAME.");
    draw_centered_text(renderer, rw, &y, "THERE ARE SOUND EFFECTS AND BACKGROUND MUSIC.");
    draw_centered_text(renderer, rw, &y, "THE EFFECTS ARE ON TRACKS TAGGED AS \"in-game\".");
    draw_centered_text(renderer, rw, &y, "PRESS SPACE TO PAUSE/UNPAUSE THE GAME.");
    draw_centered_text(renderer, rw, &y, "THE IN-GAME SOUNDS WILL PAUSE.");
    draw_centered_text(renderer, rw, &y, "THE MUSIC TRACK, NOT TAGGED, WILL NOT.");

    y += SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * 8;
    if (paused) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  /* red text */
        draw_centered_text(renderer, rw, &y, "[ CURRENTLY PAUSED ]");
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  /* green text */
        draw_centered_text(renderer, rw, &y, "[ CURRENTLY UNPAUSED ]");
    }

    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us, MIX_Quit() destroys any mixer objects we made. */
    MIX_Quit();
}

