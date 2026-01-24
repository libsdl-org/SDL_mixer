/*
 * This example code loads a single sound, and displays metadata.
 *
 * This code is public domain. Feel free to use it for any purpose!
 */

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_mixer/SDL_mixer.h>

static MIX_Audio *audio = NULL;
static char *path = NULL;

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;


/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("Example Metadata", "1.0", "com.example.metadata");

    /* this doesn't have to run very much, so give up tons of CPU time between iterations. Optional! */
    SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "5");

    /* we don't need video, but we'll make a window for smooth operation. */
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("examples/basic/metadata", 640, 480, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!MIX_Init()) {
        SDL_Log("Couldn't init SDL_mixer library: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_asprintf(&path, "%smusic.mp3", SDL_GetBasePath());  /* allocate a string of the full file path */
    audio = MIX_LoadAudio(NULL, path, false);  /* MIX_Audios are shared between mixers, so you can pass a NULL mixer here; non-NULL just lets it optimize for a specific output. */

    /* if MIX_LoadAudio failed, it's okay, the user can drop a new file on the window. */
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    } else if (event->type == SDL_EVENT_DROP_FILE) {
        MIX_DestroyAudio(audio);
        SDL_free(path);
        path = SDL_strdup(event->drop.data);
        audio = MIX_LoadAudio(NULL, path, false);
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

static void SDLCALL RenderMetadata(void *userdata, SDL_PropertiesID props, const char *name)
{
    float *y = (float *) userdata;

    switch (SDL_GetPropertyType(props, name)) {
        case SDL_PROPERTY_TYPE_INVALID:
            SDL_RenderDebugTextFormat(renderer, 0.0f, *y, " - %s [invalid type]", name);
            break;

        case SDL_PROPERTY_TYPE_POINTER:
            SDL_RenderDebugTextFormat(renderer, 0.0f, *y, " - %s [pointer=%p]", name, SDL_GetPointerProperty(props, name, NULL));
            break;

        case SDL_PROPERTY_TYPE_STRING:
            SDL_RenderDebugTextFormat(renderer, 0.0f, *y, " - %s [string=\"%s\"]", name, SDL_GetStringProperty(props, name, ""));
            break;

        case SDL_PROPERTY_TYPE_NUMBER:
            SDL_RenderDebugTextFormat(renderer, 0.0f, *y, " - %s [number=%" SDL_PRIs64 "]", name, SDL_GetNumberProperty(props, name, 0));
            break;

        case SDL_PROPERTY_TYPE_FLOAT:
            SDL_RenderDebugTextFormat(renderer, 0.0f, *y, " - %s [float=%f]", name, SDL_GetFloatProperty(props, name, 0.0f));
            break;

        case SDL_PROPERTY_TYPE_BOOLEAN:
            SDL_RenderDebugTextFormat(renderer, 0.0f, *y, " - %s [boolean=%s]", name, SDL_GetBooleanProperty(props, name, false) ? "true" : "false");
            break;

        default:
            SDL_RenderDebugTextFormat(renderer, 0.0f, *y, " - %s [unknown type]", name);
            break;
    }

    *y += SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE + 2;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    /* audio file metadata is stored in an SDL Properties group; you can also hang any app-specific metadata on here, too! */
    SDL_PropertiesID props = MIX_GetAudioProperties(audio);
    SDL_AudioSpec audiospec;
    float y = 0.0f;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  /* black */
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  /* white */

    MIX_GetAudioFormat(audio, &audiospec);
    SDL_RenderDebugText(renderer, 0.0f, y, "== Drop a file on this window to get its metadata. ==");
    y += (float) SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * 2.0f;

    if (audio) {
        SDL_RenderDebugText(renderer, 0.0f, y, path);
        y += (float) SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE + 2.0f;

        SDL_RenderDebugTextFormat(renderer, 0.0f, y, "%s, %d channel%s, %d freq",
                                    SDL_GetAudioFormatName(audiospec.format),
                                    audiospec.channels,
                                    (audiospec.channels == 1) ? "" : "s",
                                    audiospec.freq);
        y += (float) SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * 2.0f;

        /* now draw all the metadata. It happens in the RenderMetadata function, once per metadata item. */
        SDL_EnumerateProperties(props, RenderMetadata, &y);
    }

    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us, MIX_Quit() destroys any mixer objects we made. */
    MIX_Quit();
    SDL_free(path);
}

