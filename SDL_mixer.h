/*
    MIXERLIB:  An audio mixer library based on the SDL library
    Copyright (C) 1997-1999  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    5635-34 Springhouse Dr.
    Pleasanton, CA 94588 (USA)
    slouken@devolution.com
*/

#ifndef _MIXER_H_
#define _MIXER_H_

#include "SDL_types.h"
#include "SDL_rwops.h"
#include "SDL_audio.h"
#include "SDL_byteorder.h"
#include "begin_code.h"

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/* The default mixer has 8 simultaneous mixing channels */
#ifndef MIX_CHANNELS
#define MIX_CHANNELS	8
#endif

/* Good default values for a PC soundcard */
#define MIX_DEFAULT_FREQUENCY	22050
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define MIX_DEFAULT_FORMAT	AUDIO_S16LSB
#else
#define MIX_DEFAULT_FORMAT	AUDIO_S16MSB
#endif
#define MIX_DEFAULT_CHANNELS	2
#define MIX_MAX_VOLUME		128	/* Volume of a chunk */

/* The internal format for an audio chunk */
typedef struct {
	int allocated;
	Uint8 *abuf;
	Uint32 alen;
	Uint8 volume;		/* Per-sample volume, 0-128 */
} Mix_Chunk;

/* The different fading types supported */
typedef enum {
	MIX_NO_FADING,
	MIX_FADING_OUT,
	MIX_FADING_IN
} Mix_Fading;

/* The internal format for a music chunk interpreted via mikmod */
typedef struct _Mix_Music Mix_Music;

/* Open the mixer with a certain audio format */
extern DECLSPEC int Mix_OpenAudio(int frequency, Uint16 format, int channels,
							int chunksize);

/* Dynamically change the number of channels managed by the mixer.
   If decreasing the number of channels, the upper channels are
   stopped.
   This function returns the new number of allocated channels.
 */
extern DECLSPEC int Mix_AllocateChannels(int numchans);

/* Find out what the actual audio device parameters are.
   This function returns 1 if the audio has been opened, 0 otherwise.
 */
extern DECLSPEC int Mix_QuerySpec(int *frequency,Uint16 *format,int *channels);

/* Load a wave file or a music (.mod .s3m .it .xm) file */
extern DECLSPEC Mix_Chunk *Mix_LoadWAV_RW(SDL_RWops *src, int freesrc);
#define Mix_LoadWAV(file)	Mix_LoadWAV_RW(SDL_RWFromFile(file, "rb"), 1)
extern DECLSPEC Mix_Music *Mix_LoadMUS(const char *file);

#if 0 /* This hasn't been hooked into music.c yet */
/* Load a music file from an SDL_RWop object (MikMod-specific currently)
   Matt Campbell (matt@campbellhome.dhs.org) April 2000 */
extern no_parse_DECLSPEC Mix_Music *Mix_LoadMUS_RW(SDL_RWops *rw);
#endif

/* Load a wave file of the mixer format from a memory buffer */
extern DECLSPEC Mix_Chunk *Mix_QuickLoad_WAV(Uint8 *mem);

/* Free an audio chunk previously loaded */
extern DECLSPEC void Mix_FreeChunk(Mix_Chunk *chunk);
extern DECLSPEC void Mix_FreeMusic(Mix_Music *music);

/* Set a function that is called after all mixing is performed.
   This can be used to provide real-time visual display of the audio stream
   or add a custom mixer filter for the stream data.
*/
extern DECLSPEC void Mix_SetPostMix(void (*mix_func)
                             (void *udata, Uint8 *stream, int len), void *arg);

/* Add your own music player or additional mixer function.
   If 'mix_func' is NULL, the default music player is re-enabled.
 */
extern DECLSPEC void Mix_HookMusic(void (*mix_func)
                          (void *udata, Uint8 *stream, int len), void *arg);

/* Add your own callback when the music has finished playing.
 */
extern DECLSPEC void Mix_HookMusicFinished(void (*music_finished)(void));

/* Get a pointer to the user data for the current music hook */
extern DECLSPEC void *Mix_GetMusicHookData(void);

/* Add your own callback when a channel has finished playing. NULL
 * to disable callback.
 */
extern DECLSPEC void Mix_ChannelFinished(void (*channel_finished)(int channel));

/* Reserve the first channels (0 -> n-1) for the application, i.e. don't allocate
   them dynamically to the next sample if requested with a -1 value below.
   Returns the number of reserved channels.
 */
extern DECLSPEC int Mix_ReserveChannels(int num);

/* Channel grouping functions */

/* Attach a tag to a channel. A tag can be assigned to several mixer
   channels, to form groups of channels.
   If 'tag' is -1, the tag is removed (actually -1 is the tag used to
   represent the group of all the channels).
   Returns true if everything was OK.
 */
extern DECLSPEC int Mix_GroupChannel(int which, int tag);
/* Assign several consecutive channels to a group */
extern DECLSPEC int Mix_GroupChannels(int from, int to, int tag);
/* Finds the first available channel in a group of channels */
extern DECLSPEC int Mix_GroupAvailable(int tag);
/* Returns the number of channels in a group. This is also a subtle
   way to get the total number of channels when 'tag' is -1
 */
extern DECLSPEC int Mix_GroupCount(int tag);
/* Finds the "oldest" sample playing in a group of channels */
extern DECLSPEC int Mix_GroupOldest(int tag);
/* Finds the "most recent" (i.e. last) sample playing in a group of channels */
extern DECLSPEC int Mix_GroupNewer(int tag);

/* Play an audio chunk on a specific channel.
   If the specified channel is -1, play on the first free channel.
   If 'loops' is greater than zero, loop the sound that many times.
   If 'loops' is -1, loop inifinitely (~65000 times).
   Returns which channel was used to play the sound.
*/
#define Mix_PlayChannel(channel,chunk,loops) Mix_PlayChannelTimed(channel,chunk,loops,-1)
/* The same as above, but the sound is played at most 'ticks' milliseconds */
extern DECLSPEC int Mix_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ticks);
extern DECLSPEC int Mix_PlayMusic(Mix_Music *music, int loops);

/* Fade in music or a channel over "ms" milliseconds, same semantics as the "Play" functions */
extern DECLSPEC int Mix_FadeInMusic(Mix_Music *music, int loops, int ms);
#define Mix_FadeInChannel(channel,chunk,loops,ms) Mix_FadeInChannelTimed(channel,chunk,loops,ms,-1)
extern DECLSPEC int Mix_FadeInChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ms, int ticks);

/* Set the volume in the range of 0-128 of a specific channel or chunk.
   If the specified channel is -1, set volume for all channels.
   Returns the original volume.
   If the specified volume is -1, just return the current volume.
*/
extern DECLSPEC int Mix_Volume(int channel, int volume);
extern DECLSPEC int Mix_VolumeChunk(Mix_Chunk *chunk, int volume);
extern DECLSPEC int Mix_VolumeMusic(int volume);

/* Halt playing of a particular channel */
extern DECLSPEC int Mix_HaltChannel(int channel);
extern DECLSPEC int Mix_HaltGroup(int tag);
extern DECLSPEC int Mix_HaltMusic(void);

/* Change the expiration delay for a particular channel.
   The sample will stop playing after the 'ticks' milliseconds have elapsed,
   or remove the expiration if 'ticks' is -1
*/
extern DECLSPEC int Mix_ExpireChannel(int channel, int ticks);

/* Halt a channel, fading it out progressively till it's silent
   The ms parameter indicates the number of milliseconds the fading
   will take.
 */
extern DECLSPEC int Mix_FadeOutChannel(int which, int ms);
extern DECLSPEC int Mix_FadeOutGroup(int tag, int ms);
extern DECLSPEC int Mix_FadeOutMusic(int ms);

/* Query the fading status of a channel */
extern DECLSPEC Mix_Fading Mix_FadingMusic(void);
extern DECLSPEC Mix_Fading Mix_FadingChannel(int which);

/* Pause/Resume a particular channel */
extern DECLSPEC void Mix_Pause(int channel);
extern DECLSPEC void Mix_Resume(int channel);
extern DECLSPEC int  Mix_Paused(int channel);

/* Pause/Resume the music stream */
extern DECLSPEC void Mix_PauseMusic(void);
extern DECLSPEC void Mix_ResumeMusic(void);
extern DECLSPEC void Mix_RewindMusic(void);
extern DECLSPEC int  Mix_PausedMusic(void);

/* Check the status of a specific channel.
   If the specified channel is -1, check all channels.
*/
extern DECLSPEC int Mix_Playing(int channel);
extern DECLSPEC int Mix_PlayingMusic(void);

/* Stop music and set external music playback command */
extern DECLSPEC int Mix_SetMusicCMD(const char *command);

/* Get the Mix_Chunk currently associated with a mixer channel
    Returns NULL if it's an invalid channel, or there's no chunk associated.
*/
extern DECLSPEC Mix_Chunk *Mix_GetChunk(int channel);

/* Close the mixer, halting all playing audio */
extern DECLSPEC void Mix_CloseAudio(void);

/* We'll use SDL for reporting errors */
#define Mix_SetError	SDL_SetError
#define Mix_GetError	SDL_GetError

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* _MIXER_H_ */
