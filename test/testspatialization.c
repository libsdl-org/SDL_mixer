#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "SDL3_mixer/SDL_mixer.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static MIX_Mixer *mixer = NULL;
static MIX_Track *track = NULL;
static bool autopilot = true;
static bool mouse_down = false;
static float mouse_x, mouse_y;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("Test SDL_mixer spatialization", "1.0", "org.libsdl.testmixerspatialization");

    if (argc != 2) {
        SDL_Log("USAGE: %s <file_to_play>", argv[0]);
        return SDL_APP_FAILURE;
    } else if (!SDL_Init(SDL_INIT_VIDEO)) {   // it's safe to SDL_INIT_AUDIO, but MIX_Init will do it for us.
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    } else if (!MIX_Init()) {
        SDL_Log("Couldn't initialize SDL_mixer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    } else if (!SDL_CreateWindowAndRenderer("testmixer", 640, 480, 0, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    } else if ((mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL)) == NULL) {
        SDL_Log("Couldn't create mixer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    const char *audiofname = argv[1];
    MIX_Audio *audio = MIX_LoadAudio(mixer, audiofname, false);
    if (!audio) {
        SDL_Log("Failed to load '%s': %s", audiofname, SDL_GetError());
        return SDL_APP_FAILURE;
    }

    track = MIX_CreateTrack(mixer);
    MIX_SetTrackAudio(track, audio);

    SDL_PropertiesID options = SDL_CreateProperties();
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
    MIX_PlayTrack(track, options);
    SDL_DestroyProperties(options);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    } else if ((event->type == SDL_EVENT_KEY_DOWN) && (event->key.key == SDLK_ESCAPE)) {
        return SDL_APP_SUCCESS;
    } else if ((event->type == SDL_EVENT_KEY_DOWN) && (event->key.key == SDLK_A)) {
        autopilot = true;
    } else if ((event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) && (event->button.button == 1)) {
        mouse_down = true;
    } else if ((event->type == SDL_EVENT_MOUSE_BUTTON_UP) && (event->button.button == 1)) {
        autopilot = false;
        mouse_down = false;
    } else if (event->type == SDL_EVENT_MOUSE_MOTION) {
        if (mouse_down) {
            autopilot = false;
            mouse_x = event->motion.x;
            mouse_y = event->motion.y;
        }
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    const Uint64 period = 5000;
    const float moment = ((float) (SDL_GetTicks() % period)) / ((float) period);  // moment in the time period, from 0.0f to 1.0f
    const float angle = moment * 2.0f * SDL_PI_F;  // angle on the circle for this moment, in radians.
    static const SDL_FPoint center = { 640.0f / 2.0f, 480.0f / 2.0f };
    const float radius = 200.0f;  // size of half the circle (radius, not diameter).
    const float boxsize = 30.0f;
    SDL_FPoint sourcept;
    MIX_Point3D position;

    position.y = 0.0f;  // always horizontal.

    if (autopilot) {  // run in a horizontal circle around the listener (circling on X and Z coordinates).
        position.x = SDL_cosf(angle);
        position.z = SDL_sinf(angle);
        sourcept.x = center.x + (position.x * radius);
        sourcept.y = center.y + (position.z * radius);
    } else {
        position.x = (((mouse_x / 640.0f) * 2.0f) - 1.0f);  // scale to -1.0f to 1.0f
        position.z = (((mouse_y / 480.0f) * 2.0f) - 1.0f);
        sourcept.x = mouse_x - (boxsize / 2.0f);
        sourcept.y = mouse_y - (boxsize / 2.0f);
    }

    #if 0
    const float scale = 3.0f;
    position.x *= scale;  // make distance attenuation noticable.
    position.z *= scale;
    MIX_SetTrack3DPosition(track, &position);
    #else
    const float right = (position.x + 1.0f) / 2.0f;  // move to 0.0f - 1.0f.
    const MIX_StereoGains gains = { 1.0f - right, right };
    MIX_SetTrackStereo(track, &gains);
    #endif

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    const SDL_FRect source_rect = { sourcept.x, sourcept.y, boxsize, boxsize };
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(renderer, &source_rect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(renderer, source_rect.x + ((boxsize - 48.0f) / 2.0f), source_rect.y + boxsize, "SOURCE");

    const SDL_FRect listener_rect = { center.x, center.y, boxsize, boxsize };
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &listener_rect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(renderer, listener_rect.x + ((boxsize - 64.0f) / 2.0f), listener_rect.y + boxsize, "LISTENER");

    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    // SDL will clean up the window/renderer for us.
    // SDL_mixer will clean up the tracks and audio.
    MIX_Quit();
}

