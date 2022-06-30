/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2022 Sam Lantinga <slouken@libsdl.org>

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

/**
 *  \file SDL_mixer.h
 *
 *  Header file for SDL_mixer library
 *
 * A simple library to play and mix sounds and musics
 */
#ifndef SDL_MIXER_H_
#define SDL_MIXER_H_

#include "SDL_stdinc.h"
#include "SDL_rwops.h"
#include "SDL_audio.h"
#include "SDL_endian.h"
#include "SDL_version.h"
#include "begin_code.h"

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Printable format: "%d.%d.%d", MAJOR, MINOR, PATCHLEVEL
 */
#define SDL_MIXER_MAJOR_VERSION 2
#define SDL_MIXER_MINOR_VERSION 5
#define SDL_MIXER_PATCHLEVEL    2

/**
 * This macro can be used to fill a version structure with the compile-time
 * version of the SDL_mixer library.
 */
#define SDL_MIXER_VERSION(X)                        \
{                                                   \
    (X)->major = SDL_MIXER_MAJOR_VERSION;           \
    (X)->minor = SDL_MIXER_MINOR_VERSION;           \
    (X)->patch = SDL_MIXER_PATCHLEVEL;              \
}

/* Backwards compatibility */
#define MIX_MAJOR_VERSION   SDL_MIXER_MAJOR_VERSION
#define MIX_MINOR_VERSION   SDL_MIXER_MINOR_VERSION
#define MIX_PATCHLEVEL      SDL_MIXER_PATCHLEVEL
#define MIX_VERSION(X)      SDL_MIXER_VERSION(X)

#if SDL_MIXER_MAJOR_VERSION < 3 && SDL_MAJOR_VERSION < 3
/**
 *  This is the version number macro for the current SDL_mixer version.
 *
 *  In versions higher than 2.9.0, the minor version overflows into
 *  the thousands digit: for example, 2.23.0 is encoded as 4300.
 *  This macro will not be available in SDL 3.x or SDL_mixer 3.x.
 *
 *  Deprecated, use SDL_MIXER_VERSION_ATLEAST or SDL_MIXER_VERSION instead.
 */
#define SDL_MIXER_COMPILEDVERSION \
    SDL_VERSIONNUM(SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL)
#endif /* SDL_MIXER_MAJOR_VERSION < 3 && SDL_MAJOR_VERSION < 3 */

/**
 *  This macro will evaluate to true if compiled with SDL_mixer at least X.Y.Z.
 */
#define SDL_MIXER_VERSION_ATLEAST(X, Y, Z) \
    ((SDL_MIXER_MAJOR_VERSION >= X) && \
     (SDL_MIXER_MAJOR_VERSION > X || SDL_MIXER_MINOR_VERSION >= Y) && \
     (SDL_MIXER_MAJOR_VERSION > X || SDL_MIXER_MINOR_VERSION > Y || SDL_MIXER_PATCHLEVEL >= Z))

/**
 * Query the version of SDL_mixer that the program is linked against.
 *
 * This function gets the version of the dynamically linked SDL_mixer library.
 * This is separate from the SDL_MIXER_VERSION() macro, which tells you what
 * version of the SDL_mixer headers you compiled against.
 *
 * This returns static internal data; do not free or modify it!
 *
 * \returns a pointer to the version information.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC const SDL_version * SDLCALL Mix_Linked_Version(void);

/**
 * Initialization flags
 */
typedef enum
{
    MIX_INIT_FLAC   = 0x00000001,
    MIX_INIT_MOD    = 0x00000002,
    MIX_INIT_MP3    = 0x00000008,
    MIX_INIT_OGG    = 0x00000010,
    MIX_INIT_MID    = 0x00000020,
    MIX_INIT_OPUS   = 0x00000040
} MIX_InitFlags;

/**
 * Initialize SDL_mixer.
 *
 * This function loads dynamic libraries that SDL_mixer needs, and prepares
 * them for use. This must be the first function you call in SDL_mixer, and if
 * it fails you should not continue with the library.
 *
 * Flags should be one or more flags from MIX_InitFlags OR'd together. It
 * returns the flags successfully initialized, or 0 on failure.
 *
 * Currently, these flags are:
 *
 * - `MIX_INIT_FLAC`
 * - `MIX_INIT_MOD`
 * - `MIX_INIT_MP3`
 * - `MIX_INIT_OGG`
 * - `MIX_INIT_MID`
 * - `MIX_INIT_OPUS`
 *
 * More flags may be added in a future SDL_mixer release.
 *
 * This function may need to load external shared libraries to support various
 * codecs, which means this function can fail to initialize that support on an
 * otherwise-reasonable system if the library isn't available; this is not
 * just a question of exceptional circumstances like running out of memory at
 * startup!
 *
 * Note that you may call this function more than once to initialize with
 * additional flags. The return value will reflect both new flags that
 * successfully initialized, and also include flags that had previously been
 * initialized as well.
 *
 * As this will return previously-initialized flags, it's legal to call this
 * with zero (no flags set). This is a safe no-op that can be used to query
 * the current initialization state without changing it at all.
 *
 * Since this returns previously-initialized flags as well as new ones, and
 * you can call this with zero, you should not check for a zero return value
 * to determine an error condition. Instead, you should check to make sure all
 * the flags you require are set in the return value. If you have are a game
 * with data in a specific format, this might be a fatal error. If you're a
 * generic media player, perhaps you are fine with only having WAV and MP3
 * support and can live without Opus playback, even if you request support for
 * everything.
 *
 * Unlike other SDL satellite libraries, calls to Mix_Init do not stack; a
 * single call to Mix_Quit() will deinitialize everything and does not have to
 * be paired with a matching Mix_Init call. For that reason, it's considered
 * best practices to have a single Mix_Init and Mix_Quit call in your program.
 * While this isn't required, be aware of the risks of deviating from that
 * behavior.
 *
 * After initializing SDL_mixer, the next step is to open an audio device to
 * prepare to play sound (with Mix_OpenAudio() or Mix_OpenAudioDevice()), and
 * load audio data to play with that device.
 *
 * \param flags initialization flags, OR'd together.
 * \returns all currently initialized flags.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 *
 * \sa Mix_Quit
 */
extern DECLSPEC int SDLCALL Mix_Init(int flags);

/**
 * Deinitialize SDL_mixer.
 *
 * This should be the last function you call in SDL_mixer, after freeing all
 * other resources and closing all audio devices. This will unload any shared
 * libraries it is using for various codecs.
 *
 * After this call, a call to Mix_Init(0) will return 0 (no codecs loaded).
 *
 * You can safely call Mix_Init() to reload various codec support after this
 * call.
 *
 * Unlike other SDL satellite libraries, calls to Mix_Init do not stack; a
 * single call to Mix_Quit() will deinitialize everything and does not have to
 * be paired with a matching Mix_Init call. For that reason, it's considered
 * best practices to have a single Mix_Init and Mix_Quit call in your program.
 * While this isn't required, be aware of the risks of deviating from that
 * behavior.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 *
 * \sa Mix_Init
 */
extern DECLSPEC void SDLCALL Mix_Quit(void);


/**
 * The default mixer has 8 simultaneous mixing channels
 */
#ifndef MIX_CHANNELS
#define MIX_CHANNELS    8
#endif

/* Good default values for a PC soundcard */
#define MIX_DEFAULT_FREQUENCY   44100
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define MIX_DEFAULT_FORMAT  AUDIO_S16LSB
#else
#define MIX_DEFAULT_FORMAT  AUDIO_S16MSB
#endif
#define MIX_DEFAULT_CHANNELS    2
#define MIX_MAX_VOLUME          SDL_MIX_MAXVOLUME /* Volume of a chunk */

/**
 * The internal format for an audio chunk
 */
typedef struct Mix_Chunk {
    int allocated;
    Uint8 *abuf;
    Uint32 alen;
    Uint8 volume;       /* Per-sample volume, 0-128 */
} Mix_Chunk;

/**
 * The different fading types supported
 */
typedef enum {
    MIX_NO_FADING,
    MIX_FADING_OUT,
    MIX_FADING_IN
} Mix_Fading;

/**
 * These are types of music files (not libraries used to load them)
 */
typedef enum {
    MUS_NONE,
    MUS_CMD,
    MUS_WAV,
    MUS_MOD,
    MUS_MID,
    MUS_OGG,
    MUS_MP3,
    MUS_MP3_MAD_UNUSED,
    MUS_FLAC,
    MUS_MODPLUG_UNUSED,
    MUS_OPUS
} Mix_MusicType;

/**
 * The internal format for a music chunk interpreted via codecs
 */
typedef struct _Mix_Music Mix_Music;

/**
 * Open the default audio device for playback.
 *
 * An audio device is what generates sound, so the app must open one to make
 * noise.
 *
 * This function will check if SDL's audio system is initialized, and if not,
 * it will initialize it by calling `SDL_Init(SDL_INIT_AUDIO)` on your behalf.
 * You are free to (and encouraged to!) initialize it yourself before calling
 * this function, as this gives your program more control over the process.
 *
 * This function might cover all of an application's needs, but for those that
 * need more flexibility, the more powerful version of this function is
 * Mix_OpenAudioDevice(). This function is equivalent to calling:
 *
 * ```c
 * Mix_OpenAudioDevice(frequency, format, nchannels, chunksize, NULL,
 *                     SDL_AUDIO_ALLOW_FREQUENCY_CHANGE |
 *                     SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
 * ```
 *
 * If you aren't particularly concerned with the specifics of the audio
 * device, and your data isn't in a specific format, the values you use here
 * can just be reasonable defaults. SDL_mixer will convert audio data you feed
 * it to the correct format on demand.
 *
 * That being said, if you have control of your audio data and you know its
 * format ahead of time, you can save CPU time by opening the audio device in
 * that exact format so SDL_mixer does not have to spend time converting
 * anything behind the scenes, and can just pass the data straight through to
 * the hardware. On some platforms, where the hardware only supports specific
 * settings, you might have to be careful to make everything match, but your
 * own data is often easier to control, so aim to open the device for what you
 * need.
 *
 * The other reason to care about specific formats: if you plan to touch the
 * mix buffer directly (with Mix_SetPostMix, a registered effect, or
 * Mix_HookMusic), you might have code that expects it to be in a specific
 * format, and you should specify that here.
 *
 * The audio device frequency is specified in Hz; in modern times, 48000 is
 * often a reasonable default.
 *
 * The audio device format is one of SDL's AUDIO_* constants. AUDIO_S16SYS
 * (16-bit audio) is probably a safe default. More modern systems may prefer
 * AUDIO_F32SYS (32-bit floating point audio).
 *
 * The audio device channels are generally 1 for mono output, or 2 for stereo,
 * but the brave can try surround sound configs with 4 (quad), 6 (5.1), 7
 * (6.1) or 8 (7.1).
 *
 * The audio device's chunk size is the number of sample frames (one sample
 * per frame for mono output, two samples per frame in a stereo setup, etc)
 * that are fed to the device at once. The lower the number, the lower the
 * latency, but you risk dropouts if it gets too low. 2048 is often a
 * reasonable default, but your app might want to experiment with 1024 or
 * 4096.
 *
 * You may only have one audio device open at a time; if you want to change a
 * setting, you must close the device and reopen it, which is not something
 * you can do seamlessly during playback.
 *
 * This function does not allow you to select a specific audio device on the
 * system, it always chooses the best default it can on your behalf (which, in
 * many cases, is exactly what you want anyhow). If you must choose a specific
 * device, you can do so with Mix_OpenAudioDevice() instead.
 *
 * If this function reports success, you are ready to start making noise! Load
 * some audio data and start playing!
 *
 * The app can use Mix_QuerySpec() to determine the final device settings.
 *
 * When done with an audio device, probably at the end of the program, the app
 * should dispose of the device with Mix_CloseDevice().
 *
 * \param frequency the frequency to playback audio at (in Hz).
 * \param format audio format, one of SDL's AUDIO_* values.
 * \param channels number of channels (1 is mono, 2 is stereo, etc).
 * \param chunksize audio buffer size in sample FRAMES (total samples divided
 *                  by channel count).
 * \returns 0 if successful, -1 on error.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 *
 * \sa Mix_OpenAudioDevice
 * \sa Mix_CloseDevice
 */
extern DECLSPEC int SDLCALL Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize);


/**
 * Open a specific audio device for playback.
 *
 * (A slightly simpler version of this function is available in
 * Mix_OpenAudio(), which still might meet most applications' needs.)
 *
 * An audio device is what generates sound, so the app must open one to make
 * noise.
 *
 * This function will check if SDL's audio system is initialized, and if not,
 * it will initialize it by calling `SDL_Init(SDL_INIT_AUDIO)` on your behalf.
 * You are free to (and encouraged to!) initialize it yourself before calling
 * this function, as this gives your program more control over the process.
 *
 * If you aren't particularly concerned with the specifics of the audio
 * device, and your data isn't in a specific format, the values you use here
 * can just be reasonable defaults. SDL_mixer will convert audio data you feed
 * it to the correct format on demand.
 *
 * That being said, if you have control of your audio data and you know its
 * format ahead of time, you can save CPU time by opening the audio device in
 * that exact format so SDL_mixer does not have to spend time converting
 * anything behind the scenes, and can just pass the data straight through to
 * the hardware. On some platforms, where the hardware only supports specific
 * settings, you might have to be careful to make everything match, but your
 * own data is often easier to control, so aim to open the device for what you
 * need.
 *
 * The other reason to care about specific formats: if you plan to touch the
 * mix buffer directly (with Mix_SetPostMix, a registered effect, or
 * Mix_HookMusic), you might have code that expects it to be in a specific
 * format, and you should specify that here.
 *
 * The audio device frequency is specified in Hz; in modern times, 48000 is
 * often a reasonable default.
 *
 * The audio device format is one of SDL's AUDIO_* constants. AUDIO_S16SYS
 * (16-bit audio) is probably a safe default. More modern systems may prefer
 * AUDIO_F32SYS (32-bit floating point audio).
 *
 * The audio device channels are generally 1 for mono output, or 2 for stereo,
 * but the brave can try surround sound configs with 4 (quad), 6 (5.1), 7
 * (6.1) or 8 (7.1).
 *
 * The audio device's chunk size is the number of sample frames (one sample
 * per frame for mono output, two samples per frame in a stereo setup, etc)
 * that are fed to the device at once. The lower the number, the lower the
 * latency, but you risk dropouts if it gets too low. 2048 is often a
 * reasonable default, but your app might want to experiment with 1024 or
 * 4096.
 *
 * You may only have one audio device open at a time; if you want to change a
 * setting, you must close the device and reopen it, which is not something
 * you can do seamlessly during playback.
 *
 * This function allows you to select specific audio hardware on the system
 * with the `device` parameter. If you specify NULL, SDL_mixer will choose the
 * best default it can on your behalf (which, in many cases, is exactly what
 * you want anyhow). SDL_mixer does not offer a mechanism to determine device
 * names to open, but you can use SDL_GetNumAudioDevices() to get a count of
 * available devices and then SDL_GetAudioDeviceName() in a loop to obtain a
 * list. If you do this, be sure to call `SDL_Init(SDL_INIT_AUDIO)` first to
 * initialize SDL's audio system!
 *
 * The `allowed_changes` parameter specifies what settings are flexible. These
 * are the `SDL_AUDIO_ALLOW_*` flags from SDL. These tell SDL_mixer that the
 * app doesn't mind if a specific setting changes. For example, the app might
 * need stereo data in Sint16 format, but if the sample rate or chunk size
 * changes, the app can handle that. In that case, the app would specify
 * `SDL_AUDIO_ALLOW_FORMAT_CHANGE|SDL_AUDIO_ALLOW_SAMPLES_CHANGE`. In this
 * case, if the system's hardware requires something other than the requested
 * format, SDL_mixer can select what the hardware demands instead of the app.
 * If the `SDL_AUDIO_ALLOW_` flag is not specified, SDL_mixer must convert
 * data behind the scenes between what the app demands and what the hardware
 * requires. If your app needs precisely what is requested, specify zero for
 * `allowed_changes`.
 *
 * If changes were allowed, the app can use Mix_QuerySpec() to determine the
 * final device settings.
 *
 * If this function reports success, you are ready to start making noise! Load
 * some audio data and start playing!
 *
 * When done with an audio device, probably at the end of the program, the app
 * should dispose of the device with Mix_CloseDevice().
 *
 * \param frequency the frequency to playback audio at (in Hz).
 * \param format audio format, one of SDL's AUDIO_* values.
 * \param channels number of channels (1 is mono, 2 is stereo, etc).
 * \param chunksize audio buffer size in sample FRAMES (total samples divided
 *                  by channel count).
 * \param device the device name to open, or NULL to choose a reasonable
 *               default.
 * \param allowed_changes Allow change flags (see SDL_AUDIO_ALLOW_* flags)
 * \returns 0 if successful, -1 on error.
 *
 * \since This function is available since SDL_mixer 2.0.2.
 *
 * \sa Mix_OpenAudio
 * \sa Mix_CloseDevice
 * \sa Mix_QuerySpec
 */
extern DECLSPEC int SDLCALL Mix_OpenAudioDevice(int frequency, Uint16 format, int channels, int chunksize, const char* device, int allowed_changes);

/**
 * Find out what the actual audio device parameters are.
 *
 * If Mix_OpenAudioDevice() was called with `allowed_changes` set to anything
 * but zero, or Mix_OpenAudio() was used, some audio device settings may be
 * different from the application's request. This function will report what
 * the device is actually running at.
 *
 * Note this is only important if the app intends to touch the audio buffers
 * being sent to the hardware directly. If an app just wants to play audio
 * files and let SDL_mixer handle the low-level details, this function can
 * probably be ignored.
 *
 * If the audio device is not opened, this function will return 0.
 *
 * \param frequency On return, will be filled with the audio device's
 *                  frequency in Hz.
 * \param format On return, will be filled with the audio device's format.
 * \param channels On return, will be filled with the audio device's channel
 *                 count.
 * \returns 1 if the audio device has been opened, 0 otherwise.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 *
 * \sa Mix_OpenAudio
 * \sa Mix_OpenAudioDevice
 */
extern DECLSPEC int SDLCALL Mix_QuerySpec(int *frequency, Uint16 *format, int *channels);

/**
 * Dynamically change the number of channels managed by the mixer.
 *
 * SDL_mixer deals with "channels," which is not the same thing as the
 * mono/stereo channels; they might be better described as "tracks," as each
 * one corresponds to a separate source of audio data. Three different WAV
 * files playing at the same time would be three separate SDL_mixer channels,
 * for example.
 *
 * An app needs as many channels as it has audio data it wants to play
 * simultaneously, mixing them into a single stream to send to the audio
 * device.
 *
 * SDL_mixer allocates `MIX_CHANNELS` (currently 8) channels when you open an
 * audio device, which may be more than an app needs, but if the app needs
 * more or wants less, this function can change it.
 *
 * If decreasing the number of channels, any upper channels currently playing
 * are stopped. This will deregister all effects on those channels and call
 * any callback specified by Mix_ChannelFinished() for each removed channel.
 *
 * If `numchans` is less than zero, this will return the current number of
 * channels without changing anything.
 *
 * \param numchans the new number of channels, or < 0 to query current channel
 *                 count.
 * \returns the new number of allocated channels.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_AllocateChannels(int numchans);

/**
 * Load a supported audio format into a chunk.
 *
 * SDL_mixer has two separate data structures for audio data. One it calls a
 * "chunk," which is meant to be a file completely decoded into memory up
 * front, and the other it calls "music" which is a file intended to be
 * decoded on demand. Originally, simple formats like uncompressed WAV files
 * were meant to be chunks and compressed things, like MP3s, were meant to be
 * music, and you would stream one thing for a game's music and make repeating
 * sound effects with the chunks.
 *
 * In modern times, this isn't split by format anymore, and most are
 * interchangeable, so the question is what the app thinks is worth
 * predecoding or not. Chunks might take more memory, but once they are loaded
 * won't need to decode again, whereas music always needs to be decoded on the
 * fly. Also, crucially, there are as many channels for chunks as the app can
 * allocate, but SDL_mixer only offers a single "music" channel.
 *
 * If `freesrc` is non-zero, the RWops will be closed before returning,
 * whether this function succeeds or not. SDL_mixer reads everything it needs
 * from the RWops during this call in any case.
 *
 * As a convenience, there is a macro to read files from disk without having
 * to deal with SDL_RWops: `Mix_LoadWAV("filename.wav")` will call this
 * function and manage those details for you.
 *
 * When done with a chunk, the app should dispose of it with a call to
 * Mix_FreeChunk().
 *
 * \param src an SDL_RWops that data will be read from.
 * \param freesrc non-zero to close/free the SDL_RWops before returning, zero
 *                to leave it open.
 * \returns a new chunk, or NULL on error.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 *
 * \sa Mix_FreeChunk
 */
extern DECLSPEC Mix_Chunk * SDLCALL Mix_LoadWAV_RW(SDL_RWops *src, int freesrc);


/**
 * Load a wave file or a music (.mod .s3m .it .xm) file
 *
 * \param file file name
 *
 * \returns Mix Chunk, or NULL on error.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 *
 * \sa Mix_FreeChunk
 */
#define Mix_LoadWAV(file)   Mix_LoadWAV_RW(SDL_RWFromFile(file, "rb"), 1)

/**
 * Load a supported audio format into a music object.
 *
 * SDL_mixer has two separate data structures for audio data. One it calls a
 * "chunk," which is meant to be a file completely decoded into memory up
 * front, and the other it calls "music" which is a file intended to be
 * decoded on demand. Originally, simple formats like uncompressed WAV files
 * were meant to be chunks and compressed things, like MP3s, were meant to be
 * music, and you would stream one thing for a game's music and make repeating
 * sound effects with the chunks.
 *
 * In modern times, this isn't split by format anymore, and most are
 * interchangeable, so the question is what the app thinks is worth
 * predecoding or not. Chunks might take more memory, but once they are loaded
 * won't need to decode again, whereas music always needs to be decoded on the
 * fly. Also, crucially, there are as many channels for chunks as the app can
 * allocate, but SDL_mixer only offers a single "music" channel.
 *
 * When done with this music, the app should dispose of it with a call to
 * Mix_FreeMusic().
 *
 * \param file a file path from where to load music data.
 * \returns a new music object, or NULL on error.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 *
 * \sa Mix_FreeMusic
 */
extern DECLSPEC Mix_Music * SDLCALL Mix_LoadMUS(const char *file);

/**
 * Load a supported audio format into a music object.
 *
 * SDL_mixer has two separate data structures for audio data. One it calls a
 * "chunk," which is meant to be a file completely decoded into memory up
 * front, and the other it calls "music" which is a file intended to be
 * decoded on demand. Originally, simple formats like uncompressed WAV files
 * were meant to be chunks and compressed things, like MP3s, were meant to be
 * music, and you would stream one thing for a game's music and make repeating
 * sound effects with the chunks.
 *
 * In modern times, this isn't split by format anymore, and most are
 * interchangeable, so the question is what the app thinks is worth
 * predecoding or not. Chunks might take more memory, but once they are loaded
 * won't need to decode again, whereas music always needs to be decoded on the
 * fly. Also, crucially, there are as many channels for chunks as the app can
 * allocate, but SDL_mixer only offers a single "music" channel.
 *
 * If `freesrc` is non-zero, the RWops will be closed before returning,
 * whether this function succeeds or not. SDL_mixer reads everything it needs
 * from the RWops during this call in any case.
 *
 * As a convenience, there is a function to read files from disk without
 * having to deal with SDL_RWops: `Mix_LoadMUS("filename.mp3")` will manage
 * those details for you.
 *
 * This function attempts to guess the file format from incoming data. If the
 * caller knows the format, or wants to force it, it should use
 * Mix_LoadMUSType_RW() instead.
 *
 * When done with this music, the app should dispose of it with a call to
 * Mix_FreeMusic().
 *
 * \param src an SDL_RWops that data will be read from.
 * \param freesrc non-zero to close/free the SDL_RWops before returning, zero
 *                to leave it open.
 * \returns a new music object, or NULL on error.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 *
 * \sa Mix_FreeMusic
 */
extern DECLSPEC Mix_Music * SDLCALL Mix_LoadMUS_RW(SDL_RWops *src, int freesrc);

/**
 * Load an audio format into a music object, assuming a specific format.
 *
 * SDL_mixer has two separate data structures for audio data. One it calls a
 * "chunk," which is meant to be a file completely decoded into memory up
 * front, and the other it calls "music" which is a file intended to be
 * decoded on demand. Originally, simple formats like uncompressed WAV files
 * were meant to be chunks and compressed things, like MP3s, were meant to be
 * music, and you would stream one thing for a game's music and make repeating
 * sound effects with the chunks.
 *
 * In modern times, this isn't split by format anymore, and most are
 * interchangeable, so the question is what the app thinks is worth
 * predecoding or not. Chunks might take more memory, but once they are loaded
 * won't need to decode again, whereas music always needs to be decoded on the
 * fly. Also, crucially, there are as many channels for chunks as the app can
 * allocate, but SDL_mixer only offers a single "music" channel.
 *
 * This function loads music data, and lets the application specify the type
 * of music being loaded, which might be useful if SDL_mixer cannot figure it
 * out from the data stream itself.
 *
 * Currently, the following types are supported:
 *
 * - `MUS_NONE` (SDL_mixer should guess, based on the data)
 * - `MUS_WAV` (Microsoft WAV files)
 * - `MUS_MOD` (Various tracker formats)
 * - `MUS_MID` (MIDI files)
 * - `MUS_OGG` (Ogg Vorbis files)
 * - `MUS_MP3` (MP3 files)
 * - `MUS_FLAC` (FLAC files)
 * - `MUS_OPUS` (Opus files)
 *
 * If `freesrc` is non-zero, the RWops will be closed before returning,
 * whether this function succeeds or not. SDL_mixer reads everything it needs
 * from the RWops during this call in any case.
 *
 * As a convenience, there is a function to read files from disk without
 * having to deal with SDL_RWops: `Mix_LoadMUS("filename.mp3")` will manage
 * those details for you (but not let you specify the music type explicitly)..
 *
 * When done with this music, the app should dispose of it with a call to
 * Mix_FreeMusic().
 *
 * \param src an SDL_RWops that data will be read from.
 * \param type the type of audio data provided by `src`.
 * \param freesrc non-zero to close/free the SDL_RWops before returning, zero
 *                to leave it open.
 * \returns a new music object, or NULL on error.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 *
 * \sa Mix_FreeMusic
 */
extern DECLSPEC Mix_Music * SDLCALL Mix_LoadMUSType_RW(SDL_RWops *src, Mix_MusicType type, int freesrc);

/**
 * Load a wave file of the mixer format from a memory buffer
 *
 * \param mem memory buffer
 * \returns Mix Chunk, or NULL on error.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 *
 * \sa Mix_FreeChunk
 */
extern DECLSPEC Mix_Chunk * SDLCALL Mix_QuickLoad_WAV(Uint8 *mem);

/**
 * Load raw audio data of the mixer format from a memory buffer
 *
 * \param mem memory buffer
 * \param len length
 * \returns Mix Chunk, or NULL on error.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 *
 * \sa Mix_FreeChunk
 */
extern DECLSPEC Mix_Chunk * SDLCALL Mix_QuickLoad_RAW(Uint8 *mem, Uint32 len);

/**
 * Free an audio chunk previously loaded
 *
 * SDL_mixer will stop any channels this chunk is currently playing on. This
 * will deregister all effects on those channels and call any callback
 * specified by Mix_ChannelFinished() for each removed channel.
 *
 * \param chunk Mix Chunk
 *
 * \since This function is available since SDL_mixer 2.0.0.
 *
 * \sa Mix_LoadWAV
 * \sa Mix_LoadWAV_RW
 * \sa Mix_QuickLoad_WAV
 * \sa Mix_QuickLoad_RAW
 */
extern DECLSPEC void SDLCALL Mix_FreeChunk(Mix_Chunk *chunk);

/**
 * Free an audio music previously loaded
 *
 * \param music Mix Music
 *
 * \since This function is available since SDL_mixer 2.0.0.
 *
 * \sa Mix_LoadMUS
 * \sa Mix_LoadMUS_RW
 * \sa Mix_LoadMUSType_RW
 */
extern DECLSPEC void SDLCALL Mix_FreeMusic(Mix_Music *music);

/**
 * Get a list of chunk/music decoders that this build of SDL_mixer provides.
 *
 * This list can change between builds AND runs of the program, if external
 * libraries that add functionality become available. You must successfully
 * call Mix_OpenAudio() before calling these functions.
 *
 * Appearing in this list doesn't promise your specific audio file will
 * decode...but it's handy to know if you have, say, a functioning Timidity
 * install.
 *
 * These return values are static, read-only data; do not modify or free it.
 * The pointers remain valid until you call Mix_CloseAudio().
 *
 * \returns number of chunk decoders
 *
 * \since This function is available since SDL_mixer 2.0.0.
 *
 * \sa Mix_GetChunkDecoder
 * \sa Mix_HasChunkDecoder
 */
extern DECLSPEC int SDLCALL Mix_GetNumChunkDecoders(void);

/**
 * Get chunk decoder name
 *
 * \param index index of the chunk decoder
 * \returns chunk decoder name
 *
 * \since This function is available since SDL_mixer 2.0.0.
 *
 * \sa Mix_GetNumChunkDecoders
 */
extern DECLSPEC const char * SDLCALL Mix_GetChunkDecoder(int index);

/**
 * Check if `name` is a chunk decoder
 *
 * \param name name
 * \returns SDL_TRUE or SDL_FALSE
 *
 * \since This function is available since SDL_mixer 2.0.2.
 */
extern DECLSPEC SDL_bool SDLCALL Mix_HasChunkDecoder(const char *name);

/**
 * Get the number of music decoders
 *
 * \returns number of music decoders
 *
 * \since This function is available since SDL_mixer 2.0.0.
 *
 * \sa Mix_GetMusicDecoder
 * \sa Mix_HasMusicDecoder
 */
extern DECLSPEC int SDLCALL Mix_GetNumMusicDecoders(void);

/**
 * Get music decoder name
 *
 * \param index index of the music decoder
 * \returns music decoder name
 *
 * \since This function is available since SDL_mixer 2.0.0.
 *
 * \sa Mix_GetNumMusicDecoders
 */
extern DECLSPEC const char * SDLCALL Mix_GetMusicDecoder(int index);

/**
 * Check if `name` is a music decoder
 *
 * \param name name
 * \returns SDL_TRUE or SDL_FALSE
 *
 * \since This function is available since SDL_mixer 2.6.0.
 */
extern DECLSPEC SDL_bool SDLCALL Mix_HasMusicDecoder(const char *name);

/* Find out the music format of a mixer music, or the currently playing
   music, if 'music' is NULL.
*/
extern DECLSPEC Mix_MusicType SDLCALL Mix_GetMusicType(const Mix_Music *music);

/**
 * Get music title from meta-tag if possible.
 *
 * If title tag is empty, filename will be returned
 *
 * \param music Mix Music
 * \returns music title
 *
 * \since This function is available since SDL_mixer 2.6.0.
 */
extern DECLSPEC const char *SDLCALL Mix_GetMusicTitle(const Mix_Music *music);

/**
 * Get music title from meta-tag if possible
 *
 * \param music Mix Music
 * \returns music title tag
 *
 * \since This function is available since SDL_mixer 2.6.0.
 */
extern DECLSPEC const char *SDLCALL Mix_GetMusicTitleTag(const Mix_Music *music);

/**
 * Get music artist from meta-tag if possible
 *
 * \param music Mix Music
 * \returns music artist tag
 *
 * \since This function is available since SDL_mixer 2.6.0.
 */
extern DECLSPEC const char *SDLCALL Mix_GetMusicArtistTag(const Mix_Music *music);

/**
 * Get music album from meta-tag if possible
 *
 * \param music Mix Music
 * \returns music album tag
 *
 * \since This function is available since SDL_mixer 2.6.0.
 */
extern DECLSPEC const char *SDLCALL Mix_GetMusicAlbumTag(const Mix_Music *music);

/**
 * Get music copyright from meta-tag if possible
 *
 * \param music Mix Music
 * \returns music copyright tag
 *
 * \since This function is available since SDL_mixer 2.6.0.
 */
extern DECLSPEC const char *SDLCALL Mix_GetMusicCopyrightTag(const Mix_Music *music);

/**
 * Set a function that is called after all mixing is performed.
 *
 * This can be used to provide real-time visual display of the audio stream or
 * add a custom mixer filter for the stream data.
 *
 * \param mix_func callback function
 * \param arg callback argument to be passed
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC void SDLCALL Mix_SetPostMix(void (SDLCALL *mix_func)(void *udata, Uint8 *stream, int len), void *arg);

/**
 * Add your own music player or additional mixer function.
 *
 * If 'mix_func' is NULL, the default music player is re-enabled.
 *
 * \param mix_func mixer function callback
 * \param arg callback argument to be passed
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC void SDLCALL Mix_HookMusic(void (SDLCALL *mix_func)(void *udata, Uint8 *stream, int len), void *arg);

/**
 * Add your own callback for when the music has finished playing or when it is
 * stopped from a call to Mix_HaltMusic.
 *
 * \param music_finished callback function
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC void SDLCALL Mix_HookMusicFinished(void (SDLCALL *music_finished)(void));

/**
 * Get a pointer to the user data for the current music hook
 *
 * \returns pointer to the user data
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC void * SDLCALL Mix_GetMusicHookData(void);

/**
 * Add your own callback when a channel has finished playing.
 *
 * NULL to disable callback. The callback may be called from the mixer's audio
 * callback or it could be called as a result of Mix_HaltChannel(), etc. do
 * not call SDL_LockAudio() from this callback; you will either be inside the
 * audio callback, or SDL_mixer will explicitly lock the audio before calling
 * your callback.
 *
 * \param channel_finished callback function
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC void SDLCALL Mix_ChannelFinished(void (SDLCALL *channel_finished)(int channel));


/* Special Effects API by ryan c. gordon. (icculus@icculus.org) */

#define MIX_CHANNEL_POST  (-2)

/**
 * This is the format of a special effect callback:
 *
 *   myeffect(int chan, void *stream, int len, void *udata);
 *
 * (chan) is the channel number that your effect is affecting. (stream) is
 *  the buffer of data to work upon. (len) is the size of (stream), and
 *  (udata) is a user-defined bit of data, which you pass as the last arg of
 *  Mix_RegisterEffect(), and is passed back unmolested to your callback.
 *  Your effect changes the contents of (stream) based on whatever parameters
 *  are significant, or just leaves it be, if you prefer. You can do whatever
 *  you like to the buffer, though, and it will continue in its changed state
 *  down the mixing pipeline, through any other effect functions, then finally
 *  to be mixed with the rest of the channels and music for the final output
 *  stream.
 *
 * DO NOT EVER call SDL_LockAudio() from your callback function!
 */
typedef void (SDLCALL *Mix_EffectFunc_t)(int chan, void *stream, int len, void *udata);

/**
 * This is a callback that signifies that a channel has finished all its
 *  loops and has completed playback. This gets called if the buffer
 *  plays out normally, or if you call Mix_HaltChannel(), implicitly stop
 *  a channel via Mix_AllocateChannels(), or unregister a callback while
 *  it's still playing.
 *
 * DO NOT EVER call SDL_LockAudio() from your callback function!
 */
typedef void (SDLCALL *Mix_EffectDone_t)(int chan, void *udata);


/**
 * Register a special effect function.
 *
 * At mixing time, the channel data is copied into a buffer and passed through
 * each registered effect function. After it passes through all the functions,
 * it is mixed into the final output stream. The copy to buffer is performed
 * once, then each effect function performs on the output of the previous
 * effect. Understand that this extra copy to a buffer is not performed if
 * there are no effects registered for a given chunk, which saves CPU cycles,
 * and any given effect will be extra cycles, too, so it is crucial that your
 * code run fast. Also note that the data that your function is given is in
 * the format of the sound device, and not the format you gave to
 * Mix_OpenAudio(), although they may in reality be the same. This is an
 * unfortunate but necessary speed concern. Use Mix_QuerySpec() to determine
 * if you can handle the data before you register your effect, and take
 * appropriate actions.
 *
 * You may also specify a callback (Mix_EffectDone_t) that is called when the
 * channel finishes playing. This gives you a more fine-grained control than
 * Mix_ChannelFinished(), in case you need to free effect-specific resources,
 * etc. If you don't need this, you can specify NULL.
 *
 * You may set the callbacks before or after calling Mix_PlayChannel().
 *
 * Things like Mix_SetPanning() are just internal special effect functions, so
 * if you are using that, you've already incurred the overhead of a copy to a
 * separate buffer, and that these effects will be in the queue with any
 * functions you've registered. The list of registered effects for a channel
 * is reset when a chunk finishes playing, so you need to explicitly set them
 * with each call to Mix_PlayChannel*().
 *
 * You may also register a special effect function that is to be run after
 * final mixing occurs. The rules for these callbacks are identical to those
 * in Mix_RegisterEffect, but they are run after all the channels and the
 * music have been mixed into a single stream, whereas channel-specific
 * effects run on a given channel before any other mixing occurs. These global
 * effect callbacks are call "posteffects". Posteffects only have their
 * Mix_EffectDone_t function called when they are unregistered (since the main
 * output stream is never "done" in the same sense as a channel). You must
 * unregister them manually when you've had enough. Your callback will be told
 * that the channel being mixed is (MIX_CHANNEL_POST) if the processing is
 * considered a posteffect.
 *
 * After all these effects have finished processing, the callback registered
 * through Mix_SetPostMix() runs, and then the stream goes to the audio
 * device.
 *
 * DO NOT EVER call SDL_LockAudio() from your callback function!
 *
 * \param chan channel
 * \param f effect callback
 * \param d effect done callback
 * \param arg argument
 * \returns zero if error (no such channel), nonzero if added. Error messages
 *          can be retrieved from Mix_GetError().
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_RegisterEffect(int chan, Mix_EffectFunc_t f, Mix_EffectDone_t d, void *arg);


/**
 * You may not need to call this explicitly, unless you need to stop an effect
 * from processing in the middle of a chunk's playback.
 *
 * Posteffects are never implicitly unregistered as they are for channels, but
 * they may be explicitly unregistered through this function by specifying
 * MIX_CHANNEL_POST for a channel.
 *
 * \param channel channel
 * \param f effect callback
 * \returns zero if error (no such channel or effect), nonzero if removed.
 *          Error messages can be retrieved from Mix_GetError().
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_UnregisterEffect(int channel, Mix_EffectFunc_t f);

/**
 * You may not need to call this explicitly, unless you need to stop all
 * effects from processing in the middle of a chunk's playback.
 *
 * Note that this will also shut off some internal effect processing, since
 * Mix_SetPanning() and others may use this API under the hood. This is called
 * internally when a channel completes playback. Posteffects are never
 * implicitly unregistered as they are for channels, but they may be
 * explicitly unregistered through this function by specifying
 * MIX_CHANNEL_POST for a channel.
 *
 * \param channel channel
 * \returns zero if error (no such channel), nonzero if all effects removed.
 *          Error messages can be retrieved from Mix_GetError().
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_UnregisterAllEffects(int channel);


#define MIX_EFFECTSMAXSPEED  "MIX_EFFECTSMAXSPEED"

/*
 * These are the internally-defined mixing effects. They use the same API that
 *  effects defined in the application use, but are provided here as a
 *  convenience. Some effects can reduce their quality or use more memory in
 *  the name of speed; to enable this, make sure the environment variable
 *  MIX_EFFECTSMAXSPEED (see above) is defined before you call
 *  Mix_OpenAudio().
 */


/**
 * Set the panning of a channel.
 *
 * The left and right channels are specified as integers between 0 and 255,
 * quietest to loudest, respectively.
 *
 * Technically, this is just individual volume control for a sample with two
 * (stereo) channels, so it can be used for more than just panning. If you
 * want real panning, call it like this:
 *
 * ```c
 * Mix_SetPanning(channel, left, 255 - left);
 * ```
 *
 * ...which isn't so hard.
 *
 * Setting `channel` to MIX_CHANNEL_POST registers this as a posteffect, and
 * the panning will be done to the final mixed stream before passing it on to
 * the audio device.
 *
 * This uses the Mix_RegisterEffect() API internally, and returns without
 * registering the effect function if the audio device is not configured for
 * stereo output. Setting both `left` and `right` to 255 causes this effect to
 * be unregistered, since that is the data's normal state.
 *
 * Note that an audio device in mono mode is a no-op, but this call will
 * return successful in that case. Error messages can be retrieved from
 * Mix_GetError().
 *
 * \param channel The mixer channel to pan or MIX_CHANNEL_POST.
 * \param left Volume of stereo left channel, 0 is silence, 255 is full
 *             volume.
 * \param right Volume of stereo right channel, 0 is silence, 255 is full
 *              volume.
 * \returns zero if error (no such channel or Mix_RegisterEffect() fails),
 *          nonzero if panning effect enabled.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 *
 * \sa Mix_SetPosition
 * \sa Mix_SetDistance
 */
extern DECLSPEC int SDLCALL Mix_SetPanning(int channel, Uint8 left, Uint8 right);


/**
 * Set the position of a channel.
 *
 * (angle) is an integer from 0 to 360, that specifies the location of the
 * sound in relation to the listener. (angle) will be reduced as neccesary
 * (540 becomes 180 degrees, -100 becomes 260). Angle 0 is due north, and
 * rotates clockwise as the value increases. For efficiency, the precision of
 * this effect may be limited (angles 1 through 7 might all produce the same
 * effect, 8 through 15 are equal, etc). (distance) is an integer between 0
 * and 255 that specifies the space between the sound and the listener. The
 * larger the number, the further away the sound is. Using 255 does not
 * guarantee that the channel will be culled from the mixing process or be
 * completely silent. For efficiency, the precision of this effect may be
 * limited (distance 0 through 5 might all produce the same effect, 6 through
 * 10 are equal, etc). Setting (angle) and (distance) to 0 unregisters this
 * effect, since the data would be unchanged.
 *
 * If you need more precise positional audio, consider using OpenAL for
 * spatialized effects instead of SDL_mixer. This is only meant to be a basic
 * effect for simple "3D" games.
 *
 * If the audio device is configured for mono output, then you won't get any
 * effectiveness from the angle; however, distance attenuation on the channel
 * will still occur. While this effect will function with stereo voices, it
 * makes more sense to use voices with only one channel of sound, so when they
 * are mixed through this effect, the positioning will sound correct. You can
 * convert them to mono through SDL before giving them to the mixer in the
 * first place if you like.
 *
 * Setting (channel) to MIX_CHANNEL_POST registers this as a posteffect, and
 * the positioning will be done to the final mixed stream before passing it on
 * to the audio device.
 *
 * This is a convenience wrapper over Mix_SetDistance() and Mix_SetPanning().
 *
 * \param channel channel
 * \param angle angle
 * \param distance distance
 * \returns zero if error (no such channel or Mix_RegisterEffect() fails),
 *          nonzero if position effect is enabled. Error messages can be
 *          retrieved from Mix_GetError().
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_SetPosition(int channel, Sint16 angle, Uint8 distance);


/**
 * Set the "distance" of a channel.
 *
 * (distance) is an integer from 0 to 255 that specifies the location of the
 * sound in relation to the listener. Distance 0 is overlapping the listener,
 * and 255 is as far away as possible A distance of 255 does not guarantee
 * silence; in such a case, you might want to try changing the chunk's volume,
 * or just cull the sample from the mixing process with Mix_HaltChannel(). For
 * efficiency, the precision of this effect may be limited (distances 1
 * through 7 might all produce the same effect, 8 through 15 are equal, etc).
 * (distance) is an integer between 0 and 255 that specifies the space between
 * the sound and the listener. The larger the number, the further away the
 * sound is. Setting (distance) to 0 unregisters this effect, since the data
 * would be unchanged. If you need more precise positional audio, consider
 * using OpenAL for spatialized effects instead of SDL_mixer. This is only
 * meant to be a basic effect for simple "3D" games.
 *
 * Setting (channel) to MIX_CHANNEL_POST registers this as a posteffect, and
 * the distance attenuation will be done to the final mixed stream before
 * passing it on to the audio device.
 *
 * This uses the Mix_RegisterEffect() API internally.
 *
 * \param channel channel
 * \param distance distance
 * \returns zero if error (no such channel or Mix_RegisterEffect() fails),
 *          nonzero if position effect is enabled. Error messages can be
 *          retrieved from Mix_GetError().
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_SetDistance(int channel, Uint8 distance);


/**
 * Causes a channel to reverse its stereo.
 *
 * This is handy if the user has his speakers hooked up backwards, or you
 * would like to have a minor bit of psychedelia in your sound code. :)
 * Calling this function with (flip) set to non-zero reverses the chunks's
 * usual channels. If (flip) is zero, the effect is unregistered.
 *
 * This uses the Mix_RegisterEffect() API internally, and thus is probably
 * more CPU intensive than having the user just plug in his speakers
 * correctly. Mix_SetReverseStereo() returns without registering the effect
 * function if the audio device is not configured for stereo output.
 *
 * If you specify MIX_CHANNEL_POST for (channel), then this the effect is used
 * on the final mixed stream before sending it on to the audio device (a
 * posteffect).
 *
 * \param channel channel
 * \param flip flip
 * \returns zero if error (no such channel or Mix_RegisterEffect() fails),
 *          nonzero if reversing effect is enabled. Note that an audio device
 *          in mono mode is a no-op, but this call will return successful in
 *          that case. Error messages can be retrieved from Mix_GetError().
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_SetReverseStereo(int channel, int flip);

/* end of effects API. --ryan. */


/**
 * Reserve the first channels (0 -> n-1) for the application, i.e.
 *
 * don't allocate them dynamically to the next sample if requested with a -1
 * value below.
 *
 * \param num number of channels
 * \returns the number of reserved channels.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_ReserveChannels(int num);

/* Channel grouping functions */

/** Attach a tag to a channel. A tag can be assigned to several mixer
 * channels, to form groups of channels.
 * If 'tag' is -1, the tag is removed (actually -1 is the tag used to
 * represent the group of all the channels).
 *
 * \param which which
 * \param tag tag
 *
 * \returns true if everything was OK.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_GroupChannel(int which, int tag);

/**
 * Assign several consecutive channels to a group
 *
 * \param from from
 * \param to to
 * \param tag tag
 * \returns 0 if successful, negative on error
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_GroupChannels(int from, int to, int tag);

/**
 * Finds the first available channel in a group of channels,
 *
 * \param tag tag
 * \returns first available channel, or -1 if none are available.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_GroupAvailable(int tag);

/**
 * Returns the number of channels in a group.
 *
 * This is also a subtle way to get the total number of channels when 'tag' is
 * -1
 *
 * \param tag tag
 * \returns the number of channels in a group.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_GroupCount(int tag);

/**
 * Finds the "oldest" sample playing in a group of channels
 *
 * \param tag tag
 * \returns the "oldest" sample playing in a group of channels
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_GroupOldest(int tag);

/**
 * Finds the "most recent" (i.e.
 *
 * last) sample playing in a group of channels
 *
 * \param tag tag
 * \returns the "most recent" (i.e. last) sample playing in a group of
 *          channels
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_GroupNewer(int tag);

/**
 * Play an audio chunk on a specific channel.
 * If the specified channel is -1, play on the first free channel.
 * If 'loops' is greater than zero, loop the sound that many times.
 * If 'loops' is -1, loop inifinitely (~65000 times).
 *
 * \param channel channel
 * \param chunk chunk
 * \param loop loop
 *
 * \returns which channel was used to play the sound.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
#define Mix_PlayChannel(channel,chunk,loops) Mix_PlayChannelTimed(channel,chunk,loops,-1)

/**
 * The same as above, but the sound is played at most 'ticks' milliseconds
 *
 * \param channel channel
 * \param chunk chunk
 * \param loops loops
 * \param ticks ticks
 * \returns which channel was used to play the sound.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ticks);

/**
 * Play Music
 *
 * \param music music
 * \param loops loops
 * \returns which channel was used to play the music.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_PlayMusic(Mix_Music *music, int loops);

/**
 * Fade in music or a channel over "ms" milliseconds, same semantics as the
 * "Play" functions
 *
 * \param music music
 * \param loops loops
 * \param ms milliseconds
 * \returns 0 if successful, -1 on error
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_FadeInMusic(Mix_Music *music, int loops, int ms);

/**
 * Fade in music over "ms" milliseconds at position
 *
 * \param music music
 * \param loops loops
 * \param ms milliseconds
 * \param position position
 * \returns 0 if successful, -1 on error
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_FadeInMusicPos(Mix_Music *music, int loops, int ms, double position);

#define Mix_FadeInChannel(channel,chunk,loops,ms) Mix_FadeInChannelTimed(channel,chunk,loops,ms,-1)

/**
 * Fade in a channel over "ms" milliseconds at position
 *
 * \param channel channel
 * \param chunk chunk
 * \param loops loops
 * \param ms milliseconds
 * \param ticks ticks
 * \returns 0 if successful, -1 on error
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_FadeInChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ms, int ticks);

/**
 * Set the volume in the range of 0-128 of a specific channel.
 *
 * If the specified channel is -1, set volume for all channels.
 *
 * \param channel channel
 * \param volume volume
 * \returns the original volume. If the specified volume is -1, just return
 *          the current volume.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_Volume(int channel, int volume);

/**
 * Set the volume in the range of 0-128 of a specific chunk.
 *
 * \param chunk chunk
 * \param volume volume
 * \returns the original volume. If the specified volume is -1, just return
 *          the current volume.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_VolumeChunk(Mix_Chunk *chunk, int volume);

/**
 * Set the volume in the range of 0-128 of music
 *
 * \param volume volume
 * \returns the original volume. If the specified volume is -1, just return
 *          the current volume.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_VolumeMusic(int volume);

/**
 * Get the current volume value in the range of 0-128 of a music stream
 *
 * \param music music
 * \returns volume
 *
 * \since This function is available since SDL_mixer 2.6.0.
 */
extern DECLSPEC int SDLCALL Mix_GetMusicVolume(Mix_Music *music);

/**
 * Set the master volume for all channels.
 *
 * This did not affect the member variables of channel or chunk volume. If the
 * specified volume is -1, just return the current master volume.
 *
 * \param volume volume
 * \returns volume
 *
 * \since This function is available since SDL_mixer 2.6.0.
 */
extern DECLSPEC int SDLCALL Mix_MasterVolume(int volume);

/**
 * Halt playing of a particular channel
 *
 * \param channel channel
 * \returns 0 on success, or -1 on error.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_HaltChannel(int channel);

/**
 * Halt playing of a particular group
 *
 * \param tag tag
 * \returns 0 on success, or -1 on error.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_HaltGroup(int tag);

/**
 * Halt playing of music stream
 *
 * \returns 0 on success, or -1 on error.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_HaltMusic(void);

/**
 * Change the expiration delay for a particular channel.
 *
 * The sample will stop playing after the 'ticks' milliseconds have elapsed,
 * or remove the expiration if 'ticks' is -1
 *
 * \param channel channel
 * \param ticks ticks
 * \returns 0 on success, or -1 on error.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_ExpireChannel(int channel, int ticks);

/**
 * Halt a channel, fading it out progressively till it's silent The ms
 * parameter indicates the number of milliseconds the fading will take.
 *
 * \param which which channel
 * \param ms milliseconds
 * \returns 0 on success, or -1 on error.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_FadeOutChannel(int which, int ms);

/**
 * Halt a group, fading it out progressively till it's silent The ms parameter
 * indicates the number of milliseconds the fading will take.
 *
 * \param tag tag
 * \param ms milliseconds
 * \returns 0 on success, or -1 on error.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_FadeOutGroup(int tag, int ms);

/**
 * Halt music stream, fading it out progressively till it's silent The ms
 * parameter indicates the number of milliseconds the fading will take.
 *
 * \param ms milliseconds
 * \returns 0 on success, or -1 on error.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_FadeOutMusic(int ms);

/**
 * Query the fading status of music stream
 *
 * \returns fading type
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC Mix_Fading SDLCALL Mix_FadingMusic(void);

/**
 * Query the fading status of a channel
 *
 * \param which which channel
 * \returns fading type
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC Mix_Fading SDLCALL Mix_FadingChannel(int which);

/**
 * Pause a particular channel
 *
 * \param channel channel
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC void SDLCALL Mix_Pause(int channel);

/**
 * Resume a particular channel
 *
 * \param channel channel
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC void SDLCALL Mix_Resume(int channel);

/**
 * Tell whether a particular channel is paused
 *
 * \param channel channel
 * \return 1 if paused. 0 if not. or number of paused channel (if channel ==
 *         -1)
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_Paused(int channel);

/**
 * Pause the music stream
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC void SDLCALL Mix_PauseMusic(void);

/**
 * Resume the music stream
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC void SDLCALL Mix_ResumeMusic(void);

/**
 * Rewind the music stream
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC void SDLCALL Mix_RewindMusic(void);

/**
 * Tell whether the music stream is paused
 *
 * \returns 1 if paused, 0 if not.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_PausedMusic(void);

/**
 * Jump to a given order in mod music.
 *
 * Only for MOD music formats.
 *
 * \param order order
 * \returns 0 if successful, or -1 if failed or isn't implemented.
 *
 * \since This function is available since SDL_mixer 2.6.0.
 */
extern DECLSPEC int SDLCALL Mix_ModMusicJumpToOrder(int order);

/**
 * Set the current position in the music stream (in seconds).
 *
 * This function is only implemented for MOD music formats (set pattern order
 * number) and for WAV, OGG, FLAC, MP3, and MODPLUG music at the moment.
 *
 * \param position position
 * \returns 0 if successful, or -1 if it failed or not implemented.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_SetMusicPosition(double position);

/**
 * Get the time current position of music stream
 *
 * \param music music
 * \returns -1.0 if this feature is not supported for some codec
 *
 * \since This function is available since SDL_mixer 2.6.0.
 */
extern DECLSPEC double SDLCALL Mix_GetMusicPosition(Mix_Music *music);

/**
 * Get Music duration.
 *
 * If NULL is passed, returns duration of current playing music.
 *
 * \returns music duration in seconds. -1 on error
 *
 * \since This function is available since SDL_mixer 2.6.0.
 */
extern DECLSPEC double SDLCALL Mix_MusicDuration(Mix_Music *music);

/**
 * Get the loop start time position of music stream
 *
 * \param music music
 * \returns -1.0 if this feature is not used for this music or not supported
 *          for some codec
 *
 * \since This function is available since SDL_mixer 2.6.0.
 */
extern DECLSPEC double SDLCALL Mix_GetMusicLoopStartTime(Mix_Music *music);

/**
 * Get the loop end time position of music stream
 *
 * \param music music
 * \returns -1.0 if this feature is not used for this music or not supported
 *          for some codec
 *
 * \since This function is available since SDL_mixer 2.6.0.
 */
extern DECLSPEC double SDLCALL Mix_GetMusicLoopEndTime(Mix_Music *music);

/**
 * Get the loop time length of music stream
 *
 * \param music music
 * \returns -1.0 if this feature is not used for this music or not supported
 *          for some codec
 *
 * \since This function is available since SDL_mixer 2.6.0.
 */
extern DECLSPEC double SDLCALL Mix_GetMusicLoopLengthTime(Mix_Music *music);

/**
 * Check the status of a specific channel.
 *
 * If the specified channel is -1, check all channels.
 *
 * \param channel channel
 * \returns number of channel playings
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_Playing(int channel);

/**
 * Check the status of music stream
 *
 * \returns 1 if playing, 0 if not
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_PlayingMusic(void);

/**
 * Stop music and set external music playback command
 *
 * \param command command
 * \returns 0 if successful, -1 on error
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_SetMusicCMD(const char *command);

/**
 * Synchro value is set from modules while playing
 *
 * Not supported by any players at this time
 *
 * \param value value
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_SetSynchroValue(int value);

/**
 * Get Synchro value
 *
 * Not supported by any players at this time
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_GetSynchroValue(void);

/**
 * Set SoundFonts paths to use by supported MIDI backends
 *
 * \param paths paths
 * \returns 1 if successful, 0 on error
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_SetSoundFonts(const char *paths);

/**
 * Get SoundFonts paths to use by supported MIDI backends
 *
 * \returns sound fonts
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC const char* SDLCALL Mix_GetSoundFonts(void);

/**
 * Iterate SoundFonts paths to use by supported MIDI backends
 *
 * \param function callback
 * \returns position if successful, 0 on error
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC int SDLCALL Mix_EachSoundFont(int (SDLCALL *function)(const char*, void*), void *data);

/**
 * Set full path of Timidity config file.
 *
 * (e.g. /etc/timidity.cfg)
 *
 * \param path path
 * \returns 1 if successful, 0 on error
 *
 * \since This function is available since SDL_mixer 2.6.0.
 */
extern DECLSPEC int SDLCALL Mix_SetTimidityCfg(const char *path);

/**
 * Get full path of Timidity config file.
 *
 * (e.g. /etc/timidity.cfg)
 *
 * \returns path, NULL if not set
 *
 * \since This function is available since SDL_mixer 2.6.0.
 */
extern DECLSPEC const char* SDLCALL Mix_GetTimidityCfg(void);

/**
 * Get the Mix_Chunk currently associated with a mixer channel
 *
 * \param channel channel
 * \returns NULL if it's an invalid channel, or there's no chunk associated.
 *
 * \since This function is available since SDL_mixer 2.0.0.
 */
extern DECLSPEC Mix_Chunk * SDLCALL Mix_GetChunk(int channel);

/**
 * Close the mixer, halting all playing audio
 *
 * \since This function is available since SDL_mixer 2.0.0.
 *
 * \sa Mix_Init
 */
extern DECLSPEC void SDLCALL Mix_CloseAudio(void);

/* We'll use SDL for reporting errors */

/**
 * Report SDL_mixer errors
 *
 * \sa Mix_GetError
 */
#define Mix_SetError    SDL_SetError

/**
 * Get last SDL_mixer error
 *
 * \sa Mix_SetError
 */
#define Mix_GetError    SDL_GetError

/**
 * Clear last SDL_mixer error
 *
 * \sa Mix_SetError
 */
#define Mix_ClearError  SDL_ClearError

/**
 * Set OutOfMemory error
 */
#define Mix_OutOfMemory SDL_OutOfMemory

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* SDL_MIXER_H_ */

/* vi: set ts=4 sw=4 expandtab: */
