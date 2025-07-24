# Migrating to SDL_mixer 3.0

## Intro

SDL_mixer 3.0 (aka "SDL3_mixer") is a dramatically different library than
previous versions. The API has been completely redesigned. There is no
compatibility layer. If you want to use it, you have to migrate to it.

SDL3_mixer requires SDL3. It relies on many features that are new to SDL3,
both internally and in the public API, so if your project is on SDL 1.2 or
SDL2, you'll have to move your project to SDL3 at the same time.

That being said, we think SDL3_mixer and SDL3 are both great pieces of
software--significant improvements over their previous versions--and we
think that once you move to them, you'll be quite happy you did.

There are a lot of things that don't have simple replacements that can be
changed mechanically to migrate to SDL3_mixer. The new API is in many ways
more powerful, but also much simpler. For example, there's no equivalent of
Mix_ModMusicJumpToOrder(), for example, because messing with the specifics
of MOD files in the public API is both uncommon and generally pretty messy.

This migration guide will attempt to walk through the important details but
it's possible that some things can't be done the same way. Feel free to open
bug reports if you're totally stuck.

The end of this document provides a "tl;dr" section that lists each SDL2_mixer
function name and a brief explanation about what to do with it.


## Things that are totally gone

- Native MIDI has been removed. We have ripped this piece out of SDL2_mixer
  and packaged it as a separate library that can be used alongside SDL3_mixer,
  or without SDL_mixer at all: https://github.com/libsdl-org/SDL_native_midi

- Mix_GetNumTracks(), Mix_StartTrack(), and Mix_ModMusicJumpToOrder() have
  been removed; these were decoder-specific APIs.

- Mix_SetSoundFonts(), Mix_GetSoundFonts(), Mix_EachSoundFont(),
  Mix_SetTimidityCfg(), Mix_GetTimidityCfg(): these have been removed, but
  decoder-specific settings can be passed on in a generic way in SDL3_mixer,
  using SDL properties.

- Mix_QuickLoad_WAV(): this was just too unsafe to offer.


## Including SDL3_mixer

The proper way to include SDL3_mixer's header is:

```c
#include <SDL3_mixer/SDL_mixer.h>
```

Like SDL3, the new convention is to use `<>` brackets and a subdirectory.


## Symbol names

In SDL2_mixer, all functions started with `Mix_` and macros started with
`MIX_`. In SDL3_mixer, everything starts with `MIX_`.


## Versioning

Versions are now bits packed into a single int instead of a struct, using
SDL3's usual magic for unpacking these.


## Initialization

Previously you had to tell SDL_mixer what file formats you expected to use
during Mix_Init(). Now you don't:

```c
if (!MIX_Init()) {
    SDL_Log("MIX_Init failed: %s", SDL_GetError());
} else {
    SDL_Log("SDL_mixer is ready!");
}
```

MIX_Init() is reference-counted; it's safe to call it more than once, and
only the first call will do actual initialization tasks. Likewise, actual
deinitialization with MIX_Quit() will only happen when it has been called
as many times as MIX_Init() has. This allows different parts of the program
to initialize and use SDL3_mixer without knowing about other parts using it
too.

Previously the (singleton) mixer was opened with Mix_OpenAudio(), now one
calls MIX_CreateMixerDevice(). You can open multiple mixers now, in case
different parts of a program want to operate independently, or you need to
use different audio devices at the same time (or, with Mix_CreateMixer(),
write audio to a memory buffer instead of a device).

MIX_CreateMixerDevice() accepts an SDL_AudioSpec as a hint, but makes no
promises the audio device will be at any given format. The hint is just
to indicate that most of the inputs will be in a certain format, and maybe
this will reduce conversion work if the device can handle it. But SDL3_mixer
is well-prepared to manage any input or output formats it ends up with.

Mix_QuerySpec() has been replaced by MIX_GetMixerFormat().

## Music vs chunks

SDL2_mixer made a distinction between "chunks" (uncompressed audio sitting in
memory, to be played on a "channel") and "music" (streaming, probably
compressed, audio). It offered multiple chunks to be mixed together, along
with a single music input.

In SDL3_mixer, there is no distinction between types of audio; all inputs can
be compressed or uncompressed, preloaded into memory or streamed, in any
supported file format. As many as you like of any type can mix at once.

Preloaded (but not necessarily predecoded) audio is managed in MIX_Audio
objects. These are audio files precached in RAM. They can be decompressed
upfront, or on the fly while mixing.

In SDL2_mixer, data would be loaded with Mix_LoadWAV() for chunks, or
Mix_LoadMUS() for music, or some variant thereof. In SDL3_mixer, you will call
MIX_LoadAudio(), MIX_LoadAudio_IO(), or if you have some unconventional need,
MIX_LoadAudioWithProperties().

A special function in SDL2_mixer, Mix_QuickLoad_RAW(), has been replaced with
MIX_LoadRawAudioNoCopy(). It's a little more generic (it does not require your
data be in the same format as the hardware), but it will still let you get
data all the way through the mixing pipeline without any extra allocations or
copies.

Many functions in SDL2_mixer had two versions: one for chunks and one for
music, like Mix_FreeChunk() vs Mix_FreeMusic(). In SDL3_mixer, equivalent
APIs have a single function that works with everything.

Mix_GetNumChunkDecoders() and Mix_GetChunkDecoder() (and the Music
equivalents) have been replaced with MIX_GetNumAudioDecoders() and
MIX_GetAudioDecoder(). The returned strings might be different than in
SDL2_mixer for the same decoding backends.


## Channels

SDL2_mixer had "channels," each of which contained a single chunk to be mixed
during playback. Channels were referenced by index, and you would preallocate
the number of channels you intended to use with Mix_AllocateChannels() (unless
you needed 8 channels, the default, in which case you just used them).

SDL3_mixer has "tracks," each containing a single input (a loaded audio file,
an SDL_AudioStream for dynamically generated sound, or an SDL_IOStream to
stream a file on-demand). They are allocated one at a time, and referenced by
the pointer returned from MIX_CreateTrack(). To keep an indexable set of
tracks to match SDL2_mixer's channels, just call MIX_CreateTrack() in a loop,
storing the pointers in an array, and use indices into that array. There are
no tracks created by default, so to match SDL2_mixer's default, create 8 of
them at startup.


## Metadata

Functions that return metadata about an audio file, like Mix_GetMusicTitle(),
have been removed. Instead, all audio files have a set of SDL properties.

These properties offer both low-level tag information and a standard property
for well-known metadata types. For example, if your MP3 has an ID3v2 tag, you
might end up with a property named "SDL_mixer.metadata.id3v2.TIT2" and the
same info in a standard "SDL_mixer.metadata.title" property (which there's a
standard symbol for: `MIX_PROP_METADATA_TITLE_STRING`. This allows SDL3_mixer
to expose non-standard information, but also offer easy lookup of the most
common, and most important, metadata.

After loading an audio file, MIX_GetAudioProperties() will provide the
metadata (and also provide a convenient place for the app to hang their own
app-specific data, as well).


## Hooks

SDL3_mixer offers several callbacks at different points in the mixing
pipeline. They can be used to recreate SDL2_mixer's Mix_SetPostMix(),
Mix_HookMusic(), Mix_HookMusicFinished(), Mix_ChannelFinished(), and
Mix_RegisterEffect().

Note that in SDL3_mixer, all callbacks provide audio data in float32 format,
so you won't need to have different paths for different formats.


## Effects

SDL2_mixer had several "effects" that were internal code that used the
Mix_RegisterEffect() interface to modify chunks before they were mixed.

Mix_SetPanning() can be replaced with MIX_SetTrackStereo(), but this will
force the track to play on the Front Left and Front Right speakers in a
surround-sound setup. If you actually want the track to move left to
right, respecting any surround-sound layout, use MIX_SetTrack3DPosition()
and move the sound across the X axis.

Mix_SetPosition() and Mix_SetDistance() can also use MIX_SetTrack3DPosition()
to better effect, but you'll have to convert to 3D coordinates instead of
angle/distance. Here's some (untested) math to do that:

```c
const float radians = angle * (SDL_PI_F / 180.0f);  // convert degrees to radians.
const float fdistance = ((float) distance);
const float x = SDL_cosf(radians) * fdistance;  // left to right
const float y = 0.0f;   // leave everything vertically centered on the listener.
const float z = SDL_sinf(radians) * fdistance;  // front to back
```

Mix_SetReverseStereo() can be replaced with MIX_SetTrackOutputChannelMap():

```c
const int chmap = { 1, 0 };  // left becomes right, right becomes left.
MIX_SetTrackOutputChannelMap(track, chmap, 2);
```


## Channel groups

In SDL2_mixer, you could "tag" channels with an int value, and then reference
all channels that match that tag.

In SDL3_mixer, you can still tag a track, but the tag is an arbitrary string,
like "ui" or "ambient" or whatever. A track can have multiple tags.

Tag tracks with MIX_TagTrack() and remove a tag with MIX_UntagTrack().


## Playing

There are several different functions in SDL2_mixer to start a channel playing
(Mix_Play*, Mix_FadeIn*, etc). All of these have collapsed into MIX_PlayTrack().
MIX_PlayTrack() takes an SDL_PropertiesID for options (if the defaults don't cover
it). Before playing, a track must have an input assigned, either through
MIX_SetTrackAudio() to play from preloaded audio, MIX_SetTrackIOStream() to play
from a file loaded on-demand, or MIX_SetTrackAudioStream() to play procedural
audio, either buffered upfront or generated on-demand.


## Volume

Mix_VolumeChunk() and Mix_VolumeMusic() are replaced by MIX_SetTrackGain(). They
take a float instead of an int from 0 to 128, and can be used to not only quiet
the existing data, but also make it louder.

Mix_MasterVolume() is replaced by MIX_SetMasterGain(). Same idea.


## Halting

Mix_HaltChannel(), MIX_HaltMusic(), Mix_FadeOutChannel() and Mix_FadeOutMusic()
are replaced by MIX_StopTrack(). Be careful here: fade-outs work in sample
frames of the track's input, not milliseconds, in SDL3_mixer. Use
MIX_TrackMSToFrames() to convert. However, If stopping multiple tracks at once
with MIX_StopAllTracks() or MIX_StopTag(), the fade out is specified in
_milliseconds_, since we can't guarantee that all tracks have the same sample
rate.


## Pausing

Mix_Pause(), Mix_PauseMusic(), Mix_Resume() and Mix_ResumeMusic() became
MIX_PauseTrack() and MIX_ResumeTrack().


## Music state

Mix_MusicDuration() became MIX_GetAudioDuration() and Mix_GetMusicPosition() became
MIX_GetTrackPlaybackPosition(). MIX_SetMusicPosition() is now
MIX_SetTrackPlaybackPosition().


## Shutdown

Mix_CloseAudio() is now MIX_DestroyMixer(). Destroying a mixer will destroy
everything it owns (tracks, etc), but not MIX_Audio objects, since they can be
shared between mixers. MIX_Quit() can be used to destroy _everything_ that
SDL3_mixer created, including the mixers and MIX_Audio objects, which might be
more convenient.


## tl;dr

A very brief comment on what to do with each symbol in SDL2_mixer.

Some of these are listed as "no equivalent in SDL3_mixer" but could possibly
be added if there is a need. If you're stuck, please file an issue and we
can discuss it!

- MIX_MAJOR_VERSION => SDL_MIXER_MAJOR_VERSION
- MIX_MINOR_VERSION => SDL_MIXER_MAJOR_VERSION
- MIX_PATCHLEVEL => SDL_MIXER_MICRO_VERSION
- MIX_VERSION => SDL_MIXER_VERSION
- MIX_Linked_Version => MIX_Version
- Mix_Version => MIX_Version
- SDL_MIXER_COMPILEDVERSION => SDL_MIXER_VERSION
- MIX_InitFlags => not needed in SDL3_mixer.
- Mix_Init => MIX_Init (no initflags needed)
- Mix_Quit => MIX_Quit
- MIX_CHANNELS => not needed in SDL3_mixer.
- MIX_DEFAULT_FREQUENCY => 44100
- MIX_DEFAULT_FORMAT => SDL_AUDIO_S16
- MIX_DEFAULT_CHANNELS => 2
- MIX_MAX_VOLUME => 1.0f
- Mix_Chunk => MIX_Audio
- Mix_Fading => not needed in SDL3_mixer.
- Mix_MusicType => MIX_PROP_AUDIO_DECODER_STRING property with MIX_GetAudioProperties().
- Mix_Music => MIX_Audio
- Mix_OpenAudio => MIX_CreateMixerDevice
- Mix_PauseAudio => MIX_PauseAllTracks
- Mix_QuerySpec => MIX_GetMixerFormat
- Mix_AllocateChannels => MIX_CreateTrack
- Mix_LoadWAV_IO => MIX_LoadAudio_IO
- Mix_LoadWAV => MIX_LoadAudio
- Mix_LoadMUS_IO => MIX_LoadAudio_IO
- Mix_LoadMUS => MIX_LoadAudio
- Mix_LoadMUSType_IO => MIX_LoadAudioWithProperties with MIX_PROP_AUDIO_DECODER_STRING property set.
- Mix_QuickLoad_WAV => MIX_LoadAudio_IO with an SDL_IOFromConstMem() and closeio=true.
- Mix_QuickLoad_RAW => MIX_LoadRawAudioNoCopy
- Mix_FreeChunk => MIX_DestroyAudio
- Mix_FreeMusic => MIX_DestroyAudio
- Mix_GetNumChunkDecoders => MIX_GetNumAudioDecoders
- Mix_GetChunkDecoder => MIX_GetAudioDecoder
- Mix_HasChunkDecoder => call MIX_GetAudioDecoder, up to MIX_GetNumAudioDecoders times
- Mix_GetNumMusicDecoders => MIX_GetNumAudioDecoders
- Mix_GetMusicDecoder => MIX_GetAudioDecoder
- Mix_HasMusicDecoder => call MIX_GetAudioDecoder, up to MIX_GetNumAudioDecoders times
- Mix_MusicType => MIX_PROP_AUDIO_DECODER_STRING property with MIX_GetAudioProperties().
- Mix_GetMusicTitle => MIX_PROP_METADATA_TITLE_STRING property with Mix_GetAudioProperties()
- Mix_GetMusicTitleTag => MIX_PROP_METADATA_TITLE_STRING property with Mix_GetAudioProperties()
- Mix_GetMusicArtistTag => MIX_PROP_METADATA_ARTIST_STRING property with Mix_GetAudioProperties()
- Mix_GetMusicAlbumTag => MIX_PROP_METADATA_ALBUM_STRING property with Mix_GetAudioProperties()
- Mix_GetMusicCopyrightTag => MIX_PROP_METADATA_COPYRIGHT_STRING property with Mix_GetAudioProperties()
- Mix_SetPostMix => MIX_SetPostMixCallback
- Mix_HookMusic => MIX_SetTrackAudioStream and set a get-callback on that stream, start track playing.
- Mix_HookMusicFinished => MIX_SetTrackStoppedCallback
- Mix_GetMusicHookData => no equivalent in SDL3_mixer.
- Mix_ChannelFinished => MIX_SetTrackStoppedCallback
- MIX_CHANNEL_POST => MIX_SetPostMixCallback
- Mix_RegisterEffect => MIX_SetTrackCookedCallback, plus optional MIX_SetTrackStoppedCallback
- Mix_UnregisterEffect => MIX_SetTrackCookedCallback, plus optional MIX_SetTrackStoppedCallback
- Mix_UnregisterAllEffects => MIX_SetTrackCookedCallback, plus optional MIX_SetTrackStoppedCallback
- MIX_EFFECTSMAXSPEED => no equivalent in SDL3_mixer.
- Mix_SetPanning => MIX_SetTrackStereo or MIX_SetTrack3DPosition
- Mix_SetPosition => MIX_SetTrack3DPosition
- Mix_SetDistance => MIX_SetTrack3DPosition
- Mix_SetReverseStereo => MIX_SetTrackOutputChannelMap
- Mix_ReserveChannels => no equivalent in SDL3_mixer.
- Mix_GroupChannel => MIX_TagTrack
- Mix_GroupChannels => MIX_TagTrack
- Mix_GroupAvailable => no equivalent in SDL3_mixer.
- Mix_GroupCount => no equivalent in SDL3_mixer.
- Mix_GroupOldest => no equivalent in SDL3_mixer.
- Mix_GroupNewer => no equivalent in SDL3_mixer.
- Mix_PlayChannel => MIX_SetTrackAudio then MIX_PlayTrack
- Mix_PlayChannelTimed => MIX_SetTrackAudio then MIX_PlayTrack
- Mix_PlayMusic => MIX_SetTrackAudio then MIX_PlayTrack
- Mix_FadeInMusic => MIX_SetTrackAudio then MIX_PlayTrack
- Mix_FadeInMusicPos => MIX_SetTrackAudio then MIX_PlayTrack
- Mix_FadeInChannel => MIX_SetTrackAudio then MIX_PlayTrack
- Mix_FadeInChannelTimed => MIX_SetTrackAudio then MIX_PlayTrack
- Mix_Volume => MIX_SetTrackGain
- Mix_VolumeChunk => MIX_SetTrackGain; gain is not set on MIX_Audio, only on MIX_Track
- Mix_VolumeMusic => MIX_SetTrackGain
- Mix_GetMusicVolume => MIX_GetTrackGain
- Mix_MasterVolume => MIX_SetMasterGain
- Mix_HaltChannel => MIX_StopTrack
- Mix_HaltGroup => MIX_StopTag
- Mix_HaltMusic => MIX_StopTrack
- Mix_ExpireChannel => no equivalent in SDL3_mixer.
- Mix_FadeOutChannel => MIX_StopTrack
- Mix_FadeOutGroup => MIX_StopTag
- Mix_FadeOutMusic => MIX_StopTrack
- Mix_FadingMusic =>  no equivalent in SDL3_mixer.
- Mix_FadingChannel => no equivalent in SDL3_mixer.
- Mix_Pause => MIX_PauseTrack
- Mix_PauseGroup => MIX_PauseTag
- Mix_Resume => MIX_ResumeTrack
- Mix_PauseGroup => MIX_ResumeTag
- Mix_Paused => MIX_TrackPaused
- Mix_PauseMusic => MIX_PauseTrack
- Mix_ResumeMusic => MIX_ResumeTrack
- Mix_RewindMusic => MIX_SetTrackPlaybackPosition(track, 0)
- Mix_PausedMusic => MIX_TrackPaused
- Mix_ModMusicJumpToOrder => no equivalent in SDL3_mixer.
- Mix_StartTrack => no equivalent in SDL3_mixer.
- Mix_GetNumTracks => no equivalent in SDL3_mixer.
- Mix_SetMusicPosition => MIX_SetTrackPlaybackPosition
- Mix_GetMusicPosition => MIX_GetTrackPlaybackPosition
- Mix_MusicDuration => MIX_GetAudioDuration
- Mix_GetMusicLoopStartTime => no equivalent in SDL3_mixer.
- Mix_GetMusicLoopEndTime => no equivalent in SDL3_mixer.
- Mix_GetMusicLoopLengthTime => no equivalent in SDL3_mixer.
- Mix_Playing => MIX_TrackPlaying
- Mix_PlayingMusic => MIX_TrackPlaying
- Mix_SetSoundFonts => MIX_LoadAudioWithProperties with "SDL_mixer.decoder.fluidsynth.soundfont_iostream" or  "SDL_mixer.decoder.fluidsynth.soundfont_path" property set
- Mix_GetSoundFonts => no equivalent in SDL3_mixer.
- Mix_EachSoundFont => no equivalent in SDL3_mixer.
- Mix_SetTimidityCfg => FIXME currently SDL_setenv("TIMIDITY_CFG")
- Mix_GetTimidityCfg => no equivalent in SDL3_mixer.
- Mix_GetChunk => MIX_GetTrackAudio or MIX_GetTrackAudioStream
- Mix_CloseAudio => MIX_DestroyMixer

