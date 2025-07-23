#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "SDL3_mixer/SDL_mixer.h"

#define USE_MIX_GENERATE 0

//static SDL_Window *window = NULL;
//static SDL_Renderer *renderer = NULL;
static MIX_Mixer *mixer = NULL;
static MIX_Track *track1 = NULL;
static MIX_Track *track2 = NULL;

#if USE_MIX_GENERATE
static SDL_IOStream *io = NULL;
#endif

static SDL_IOStream *ioraw = NULL;
static SDL_IOStream *iocooked = NULL;
static SDL_IOStream *iopostmix = NULL;

static void SDLCALL WritePCMCallback(void *userdata, MIX_Track *track, const SDL_AudioSpec *spec, float *pcm, int samples)
{
    SDL_WriteIO((SDL_IOStream *) userdata, pcm, samples * sizeof (float));
}

static void SDLCALL WritePostmixPCMCallback(void *userdata, MIX_Mixer *mixer, const SDL_AudioSpec *spec, float *pcm, int samples)
{
    WritePCMCallback(userdata, NULL, spec, pcm, samples);
}

static void LogMetadata(SDL_PropertiesID props, const char *name)
{
    switch (SDL_GetPropertyType(props, name)) {
        case SDL_PROPERTY_TYPE_INVALID:
            SDL_Log(" - %s [invalid type]", name);
            break;

        case SDL_PROPERTY_TYPE_POINTER:
            SDL_Log(" - %s [pointer=%p]", name, SDL_GetPointerProperty(props, name, NULL));
            break;

        case SDL_PROPERTY_TYPE_STRING:
            SDL_Log(" - %s [string=\"%s\"]", name, SDL_GetStringProperty(props, name, ""));
            break;

        case SDL_PROPERTY_TYPE_NUMBER:
            SDL_Log(" - %s [number=%" SDL_PRIs64 "]", name, SDL_GetNumberProperty(props, name, 0));
            break;

        case SDL_PROPERTY_TYPE_FLOAT:
            SDL_Log(" - %s [float=%f]", name, SDL_GetFloatProperty(props, name, 0.0f));
            break;

        case SDL_PROPERTY_TYPE_BOOLEAN:
            SDL_Log(" - %s [boolean=%s]", name, SDL_GetBooleanProperty(props, name, false) ? "true" : "false");
            break;

        default:
            SDL_Log(" - %s [unknown type]", name);
            break;
    }
}

typedef struct MetadataKeys
{
    char **keys;
    size_t num_keys;
} MetadataKeys;

static void SDLCALL CollectMetadata(void *userdata, SDL_PropertiesID props, const char *name)
{
    MetadataKeys *mkeys = (MetadataKeys *) userdata;
    char *key = SDL_strdup(name);
    if (key) {
        void *ptr = SDL_realloc(mkeys->keys, (mkeys->num_keys + 1) * sizeof (*mkeys->keys));
        if (!ptr) {
            SDL_free(key);
        } else {
            mkeys->keys = (char **) ptr;
            mkeys->keys[mkeys->num_keys++] = key;
        }
    }
}

static int SDLCALL CompareMetadataKeys(const void *a, const void *b)
{
    return SDL_strcmp(*(const char **) a, *(const char **) b);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("Test SDL_mixer", "1.0", "org.libsdl.testmixer");
#if USE_MIX_GENERATE
    const SDL_AudioSpec spec = { SDL_AUDIO_S16, 2, 48000 };
#endif

    if ((argc != 2) && (argc != 3)) {
        SDL_Log("USAGE: %s <file_to_play1> [file_to_play2]", argv[0]);
        return SDL_APP_FAILURE;
    } else if (!SDL_Init(SDL_INIT_VIDEO)) {   // it's safe to SDL_INIT_AUDIO, but MIX_Init will do it for us.
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    } else if (!MIX_Init()) {
        SDL_Log("Couldn't initialize SDL_mixer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
//    } else if (!SDL_CreateWindowAndRenderer("testmixer", 640, 480, 0, &window, &renderer)) {
//        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
//        return SDL_APP_FAILURE;
#if USE_MIX_GENERATE
    } else if ((io = SDL_IOFromFile("dump.raw", "wb")) == NULL) {
        SDL_Log("Couldn't create dump.raw: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    } else if ((mixer = MIX_CreateMixer(&spec)) == NULL) {
#else
    } else if ((mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL)) == NULL) {
#endif
        SDL_Log("Couldn't create mixer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_AudioSpec mixerspec;
    MIX_GetMixerFormat(mixer, &mixerspec);
    SDL_Log("Mixer is format %s, %d channels, %d frequency", SDL_GetAudioFormatName(mixerspec.format), mixerspec.channels, mixerspec.freq);

    SDL_Log("Available decoders:");
    const int num_decoders = MIX_GetNumAudioDecoders();
    if (num_decoders < 0) {
        SDL_Log(" - [error (%s)]", SDL_GetError());
    } else if (num_decoders == 0) {
        SDL_Log(" - [none]");
    } else {
        for (int i = 0; i < num_decoders; i++) {
            SDL_Log(" - %s", MIX_GetAudioDecoder(i));
        }
    }
    SDL_Log("%s", "");

    const char *audiofname = argv[1];
    MIX_Audio *audio = MIX_LoadAudio(mixer, audiofname, false);
    if (!audio) {
        SDL_Log("Failed to load '%s': %s", audiofname, SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_AudioSpec audiospec;
    MIX_GetAudioFormat(audio, &audiospec);
    SDL_Log("%s: %s, %d channel%s, %d freq", audiofname, SDL_GetAudioFormatName(audiospec.format), audiospec.channels, (audiospec.channels == 1) ? "" : "s", audiospec.freq);

    SDL_Log("%s metadata:", audiofname);
    SDL_PropertiesID props = MIX_GetAudioProperties(audio);
    bool had_metadata = false;
    if (props) {
        MetadataKeys mkeys;
        SDL_zero(mkeys);
        SDL_EnumerateProperties(props, CollectMetadata, &mkeys);
        if (mkeys.num_keys > 0) {
            SDL_qsort(mkeys.keys, mkeys.num_keys, sizeof (*mkeys.keys), CompareMetadataKeys);
            for (size_t i = 0; i < mkeys.num_keys; i++) {
                LogMetadata(props, mkeys.keys[i]);
                SDL_free(mkeys.keys[i]);
                had_metadata = true;
            }
        }
        SDL_free(mkeys.keys);
    }

    if (!had_metadata) {
        SDL_Log(" - [none]");
    }
    SDL_Log("%s", "");

    track1 = MIX_CreateTrack(mixer);

    //ioraw = SDL_IOFromFile("trackraw.raw", "wb"); if (ioraw) { MIX_SetTrackRawCallback(track1, WritePCMCallback, ioraw); }
    //iocooked = SDL_IOFromFile("trackcooked.raw", "wb"); if (iocooked) { MIX_SetTrackCookedCallback(track1, WritePCMCallback, iocooked); }
    //iopostmix = SDL_IOFromFile("postmix.raw", "wb"); if (iopostmix) { MIX_SetPostMixCallback(mixer, WritePostmixPCMCallback, iopostmix); }
    (void) WritePostmixPCMCallback;  // stop a compiler warning when not using the callbacks.

    //const int chmap[] = { 1, 0 }; MIX_SetTrackOutputChannelMap(track1, chmap, SDL_arraysize(chmap));
    MIX_SetTrackAudio(track1, audio);
    if (argv[2]) {
        track2 = MIX_CreateTrack(mixer);
        MIX_SetTrackIOStream(track2, SDL_IOFromFile(argv[2], "rb"), true);
    }

    SDL_PropertiesID options;

    options = SDL_CreateProperties();
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_MAX_MILLISECONDS_NUMBER, 9440);
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, 3);
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOP_START_MILLISECOND_NUMBER, 6097);
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, 30000);
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_APPEND_SILENCE_MILLISECONDS_NUMBER, 30000);
    MIX_PlayTrack(track1, options);
    SDL_DestroyProperties(options);

    if (track2) {
        options = SDL_CreateProperties();
        SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
        MIX_PlayTrack(track2, options);
        SDL_DestroyProperties(options);
    }

    // we cheat here with PlayAudio, since the sinewave decoder produces infinite audio.
    //MIX_PlayAudio(mixer, MIX_CreateSineWaveAudio(mixer, 300, 0.25f));

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
    #if USE_MIX_GENERATE
    float buf[1024];
    if (!MIX_Generate(mixer, buf, sizeof (buf))) {
        SDL_Log("MIX_Generate failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_WriteIO(io, buf, sizeof (buf));
    #endif

//    SDL_RenderClear(renderer);
//    SDL_RenderPresent(renderer);

    #if 0
    float ratio = ((float) ((((Sint64) SDL_GetTicks()) - 1000) / 1000)) * 0.10f;
    ratio = SDL_clamp(ratio, 0.1f, 5.0f);
    static float prev_ratio = 1.0f;
    if (ratio != prev_ratio) {
        SDL_Log("new frequency ratio: %f", ratio);
        MIX_SetTrackFrequencyRatio(track1, ratio);
        prev_ratio = ratio;
    }
    #endif

    return MIX_TrackPlaying(track1) ? SDL_APP_CONTINUE : SDL_APP_SUCCESS;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    // SDL will clean up the window/renderer for us.
    // SDL_mixer will clean up the tracks and audio.
    MIX_Quit();

    if (ioraw) { SDL_CloseIO(ioraw); }
    if (iocooked) { SDL_CloseIO(iocooked); }
    if (iopostmix) { SDL_CloseIO(iopostmix); }

    #if USE_MIX_GENERATE
    SDL_CloseIO(io);
    #endif
}

