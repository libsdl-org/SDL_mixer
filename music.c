/*
    SDL_mixer:  An audio mixer library based on the SDL library
    Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga

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
    slouken@libsdl.org
*/

/* $Id$ */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "SDL_endian.h"
#include "SDL_audio.h"
#include "SDL_timer.h"

#include "SDL_mixer.h"

/* The music command hack is UNIX specific */
#ifndef unix
#undef CMD_MUSIC
#endif

#ifdef CMD_MUSIC
#include "music_cmd.h"
#endif
#ifdef WAV_MUSIC
#include "wavestream.h"
#endif
#ifdef MOD_MUSIC
#  include "mikmod.h"
#  if defined(LIBMIKMOD_VERSION)                /* libmikmod 3.1.8 */
#    define UNIMOD			MODULE
#    define MikMod_Init()		MikMod_Init(NULL)
#    define MikMod_LoadSong(a,b)	Player_Load(a,b,0)
#    define MikMod_FreeSong		Player_Free
     extern int MikMod_errno;
#  else                                        /* old MikMod 3.0.3 */
#    define MikMod_strerror(x)		_mm_errmsg[x])
#    define MikMod_errno		_mm_errno
#  endif
#endif
#ifdef MID_MUSIC
#  ifdef USE_TIMIDITY_MIDI
#    include "timidity.h"
#  endif
#  ifdef USE_NATIVE_MIDI
#    include "native_midi.h"
#  endif
#  if defined(USE_TIMIDITY_MIDI) && defined(USE_NATIVE_MIDI)
#    define MIDI_ELSE	else
#  else
#    define MIDI_ELSE
#  endif
#endif
#ifdef OGG_MUSIC
#include "music_ogg.h"
#endif
#ifdef MP3_MUSIC
#include "smpeg.h"

static SDL_AudioSpec used_mixer;
#endif

int volatile music_active = 1;
static int volatile music_stopped = 0;
static int music_loops = 0;
static char *music_cmd = NULL;
static Mix_Music * volatile music_playing = NULL;
static int music_volume = MIX_MAX_VOLUME;
static int music_swap8;
static int music_swap16;

struct _Mix_Music {
	Mix_MusicType type;
	union {
#ifdef CMD_MUSIC
		MusicCMD *cmd;
#endif
#ifdef WAV_MUSIC
		WAVStream *wave;
#endif
#ifdef MOD_MUSIC
		UNIMOD *module;
#endif
#ifdef MID_MUSIC
#ifdef USE_TIMIDITY_MIDI
		MidiSong *midi;
#endif
#ifdef USE_NATIVE_MIDI
		NativeMidiSong *nativemidi;
#endif
#endif
#ifdef OGG_MUSIC
		OGG_music *ogg;
#endif
#ifdef MP3_MUSIC
		SMPEG *mp3;
#endif
	} data;
	Mix_Fading fading;
	int fade_step;
	int fade_steps;
	int error;
};
#ifdef MID_MUSIC
#ifdef USE_TIMIDITY_MIDI
static int timidity_ok;
static int samplesize;
#endif
#ifdef USE_NATIVE_MIDI
static int native_midi_ok;
#endif
#endif

/* Used to calculate fading steps */
static int ms_per_step;

/* Local low-level functions prototypes */
static void music_internal_volume(int volume);
static int  music_internal_play(Mix_Music *music, double position);
static int  music_internal_position(double position);
static int  music_internal_playing();
static void music_internal_halt(void);


/* Support for hooking when the music has finished */
static void (*music_finished_hook)(void) = NULL;

void Mix_HookMusicFinished(void (*music_finished)(void))
{
	SDL_LockAudio();
	music_finished_hook = music_finished;
	SDL_UnlockAudio();
}


/* Mixing function */
void music_mixer(void *udata, Uint8 *stream, int len)
{
	if ( music_playing && music_active ) {
		/* Handle fading */
		if ( music_playing->fading != MIX_NO_FADING ) {
			if ( music_playing->fade_step++ < music_playing->fade_steps ) {
				int volume;
				int fade_step = music_playing->fade_step;
				int fade_steps = music_playing->fade_steps;

				if ( music_playing->fading == MIX_FADING_OUT ) {
					volume = (music_volume * (fade_steps-fade_step)) / fade_steps;
				} else { /* Fading in */
					volume = (music_volume * fade_step) / fade_steps;
				}
				music_internal_volume(volume);
			} else {
				if ( music_playing->fading == MIX_FADING_OUT ) {
					music_internal_halt();
					if ( music_finished_hook ) {
						music_finished_hook();
					}
					return;
				}
				music_playing->fading = MIX_NO_FADING;
			}
		}
		/* Restart music if it has to loop */
		if ( !music_internal_playing() ) {
			/* Restart music if it has to loop at a high level */
			if ( music_loops && --music_loops ) {
				music_internal_play(music_playing, 0.0);
			} else {
				music_internal_halt();
				if ( music_finished_hook ) {
					music_finished_hook();
				}
				return;
			}
		}
		switch (music_playing->type) {
#ifdef CMD_MUSIC
			case MUS_CMD:
				/* The playing is done externally */
				break;
#endif
#ifdef WAV_MUSIC
			case MUS_WAV:
				WAVStream_PlaySome(stream, len);
				break;
#endif
#ifdef MOD_MUSIC
			case MUS_MOD:
				VC_WriteBytes((SBYTE *)stream, len);
				if ( music_swap8 ) {
					Uint8 *dst;
					int i;

					dst = stream;
					for ( i=len; i; --i ) {
						*dst++ ^= 0x80;
					}
				} else
				if ( music_swap16 ) {
					Uint8 *dst, tmp;
					int i;

					dst = stream;
					for ( i=(len/2); i; --i ) {
						tmp = dst[0];
						dst[0] = dst[1];
						dst[1] = tmp;
						dst += 2;
					}
				}
				break;
#endif
#ifdef MID_MUSIC
#ifdef USE_TIMIDITY_MIDI
			case MUS_MID:
				if ( timidity_ok ) {
					int samples = len / samplesize;
  					Timidity_PlaySome(stream, samples);
				}
				break;
#endif
#endif
#ifdef OGG_MUSIC
			case MUS_OGG:
				OGG_playAudio(music_playing->data.ogg, stream, len);
				break;
#endif
#ifdef MP3_MUSIC
			case MUS_MP3:
				SMPEG_playAudio(music_playing->data.mp3, stream, len);
				break;
#endif
			default:
				/* Unknown music type?? */
				break;
		}
	}
}

/* Initialize the music players with a certain desired audio format */
int open_music(SDL_AudioSpec *mixer)
{
	int music_error;

	music_error = 0;
#ifdef WAV_MUSIC
	if ( WAVStream_Init(mixer) < 0 ) {
		++music_error;
	}
#endif
#ifdef MOD_MUSIC
	/* Set the MikMod music format */
	music_swap8 = 0;
	music_swap16 = 0;
	switch (mixer->format) {

		case AUDIO_U8:
		case AUDIO_S8: {
			if ( mixer->format == AUDIO_S8 ) {
				music_swap8 = 1;
			}
			md_mode = 0;
		}
		break;

		case AUDIO_S16LSB:
		case AUDIO_S16MSB: {
			/* See if we need to correct MikMod mixing */
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
			if ( mixer->format == AUDIO_S16MSB ) {
#else
			if ( mixer->format == AUDIO_S16LSB ) {
#endif
				music_swap16 = 1;
			}
			md_mode = DMODE_16BITS;
		}
		break;

		default: {
			Mix_SetError("Unknown hardware audio format");
			++music_error;
		}
	}
	if ( mixer->channels > 1 ) {
		if ( mixer->channels > 2 ) {
			Mix_SetError("Hardware uses more channels than mixer");
			++music_error;
		}
		md_mode |= DMODE_STEREO;
	}
	md_mixfreq	 = mixer->freq;
	md_device	  = 0;
	md_volume	  = 96;
	md_musicvolume = 128;
	md_sndfxvolume = 128;
	md_pansep	  = 128;
	md_reverb	  = 0;
	MikMod_RegisterAllLoaders();
	MikMod_RegisterAllDrivers();
	if ( MikMod_Init() ) {
		Mix_SetError("%s", MikMod_strerror(MikMod_errno));
		++music_error;
	}
#endif
#ifdef MID_MUSIC
#ifdef USE_TIMIDITY_MIDI
	samplesize = mixer->size / mixer->samples;
	if ( Timidity_Init(mixer->freq, mixer->format,
	                    mixer->channels, mixer->samples) == 0 ) {
		timidity_ok = 1;
	} else {
		timidity_ok = 0;
	}
#endif
#ifdef USE_NATIVE_MIDI
#ifdef USE_TIMIDITY_MIDI
	native_midi_ok = !timidity_ok;
	if ( native_midi_ok )
#endif
		native_midi_ok = native_midi_detect();
#endif
#endif
#ifdef OGG_MUSIC
	if ( OGG_init(mixer) < 0 ) {
		++music_error;
	}
#endif
#ifdef MP3_MUSIC
	/* Keep a copy of the mixer */
	used_mixer = *mixer;
#endif
	music_playing = NULL;
	music_stopped = 0;
	if ( music_error ) {
		return(-1);
	}
	Mix_VolumeMusic(SDL_MIX_MAXVOLUME);

	/* Calculate the number of ms for each callback */
	ms_per_step = (int) (((float)mixer->samples * 1000.0) / mixer->freq);

	return(0);
}

/* Portable case-insensitive string compare function */
int MIX_string_equals(const char *str1, const char *str2)
{
	while ( *str1 && *str2 ) {
		if ( toupper((unsigned char)*str1) !=
		     toupper((unsigned char)*str2) )
			break;
		++str1;
		++str2;
	}
	return (!*str1 && !*str2);
}

/* Load a music file */
Mix_Music *Mix_LoadMUS(const char *file)
{
	FILE *fp;
	char *ext;
	Uint8 magic[5];
	Mix_Music *music;

	/* Figure out what kind of file this is */
	fp = fopen(file, "rb");
	if ( (fp == NULL) || !fread(magic, 4, 1, fp) ) {
		if ( fp != NULL ) {
			fclose(fp);
		}
		Mix_SetError("Couldn't read from '%s'", file);
		return(NULL);
	}
	magic[4] = '\0';
	fclose(fp);

	/* Figure out the file extension, so we can determine the type */
	ext = strrchr(file, '.');
	if ( ext ) ++ext; /* skip the dot in the extension */

	/* Allocate memory for the music structure */
	music = (Mix_Music *)malloc(sizeof(Mix_Music));
	if ( music == NULL ) {
		Mix_SetError("Out of memory");
		return(NULL);
	}
	music->error = 0;

#ifdef CMD_MUSIC
	if ( music_cmd ) {
		music->type = MUS_CMD;
		music->data.cmd = MusicCMD_LoadSong(music_cmd, file);
		if ( music->data.cmd == NULL ) {
			music->error = 1;
		}
	} else
#endif
#ifdef WAV_MUSIC
	/* WAVE files have the magic four bytes "RIFF"
	   AIFF files have the magic 12 bytes "FORM" XXXX "AIFF"
	 */
	if ( (ext && MIX_string_equals(ext, "WAV")) ||
	     (strcmp((char *)magic, "RIFF") == 0) ||
	     (strcmp((char *)magic, "FORM") == 0) ) {
		music->type = MUS_WAV;
		music->data.wave = WAVStream_LoadSong(file, (char *)magic);
		if ( music->data.wave == NULL ) {
		  	Mix_SetError("Unable to load WAV file");
			music->error = 1;
		}
	} else
#endif
#ifdef MID_MUSIC
	/* MIDI files have the magic four bytes "MThd" */
	if ( (ext && MIX_string_equals(ext, "MID")) ||
	     (ext && MIX_string_equals(ext, "MIDI")) ||
	     strcmp((char *)magic, "MThd") == 0 ) {
		music->type = MUS_MID;
#ifdef USE_NATIVE_MIDI
  		if ( native_midi_ok ) {
  			music->data.nativemidi = native_midi_loadsong((char *)file);
	  		if ( music->data.nativemidi == NULL ) {
		  		Mix_SetError("%s", native_midi_error());
			  	music->error = 1;
			}
	  	} MIDI_ELSE
#endif
#ifdef USE_TIMIDITY_MIDI
		if ( timidity_ok ) {
			music->data.midi = Timidity_LoadSong((char *)file);
			if ( music->data.midi == NULL ) {
				Mix_SetError("%s", Timidity_Error());
				music->error = 1;
			}
		} else {
			Mix_SetError("%s", Timidity_Error());
			music->error = 1;
		}
#endif
	} else
#endif
#ifdef OGG_MUSIC
	/* Ogg Vorbis files have the magic four bytes "OggS" */
	if ( (ext && MIX_string_equals(ext, "OGG")) ||
	     strcmp((char *)magic, "OggS") == 0 ) {
		music->type = MUS_OGG;
		music->data.ogg = OGG_new(file);
		if ( music->data.ogg == NULL ) {
			music->error = 1;
		}
	} else
#endif
#ifdef MP3_MUSIC
	if ( (ext && MIX_string_equals(ext, "MPG")) ||
	     (ext && MIX_string_equals(ext, "MPEG")) ||
	     magic[0]==0xFF && (magic[1]&0xF0)==0xF0) {
		SMPEG_Info info;
		music->type = MUS_MP3;
		music->data.mp3 = SMPEG_new(file, &info, 0);
		if(!info.has_audio){
			Mix_SetError("MPEG file does not have any audio stream.");
			music->error = 1;
		}else{
			SMPEG_actualSpec(music->data.mp3, &used_mixer);
		}
	} else
#endif
#ifdef MOD_MUSIC
	if ( 1 ) {
		music->type = MUS_MOD;
		music->data.module = MikMod_LoadSong((char *)file, 64);
		if ( music->data.module == NULL ) {
			Mix_SetError("%s", MikMod_strerror(MikMod_errno));
			music->error = 1;
		} else {
			/* Stop implicit looping, fade out and other flags. */
			music->data.module->extspd  = 1;
			music->data.module->panflag = 1;
			music->data.module->wrap    = 0;
			music->data.module->loop    = 0;
#if 0 /* Don't set fade out by default - unfortunately there's no real way
         to query the status of the song or set trigger actions.  Hum. */
			music->data.module->fadeout = 1;
#endif
		}
	} else
#endif
	{
		Mix_SetError("Unrecognized music format");
		music->error = 1;
	}
	if ( music->error ) {
		free(music);
		music = NULL;
	}
	return(music);
}

/* Free a music chunk previously loaded */
void Mix_FreeMusic(Mix_Music *music)
{
	if ( music ) {
		/* Stop the music if it's currently playing */
		SDL_LockAudio();
		if ( music == music_playing ) {
			/* Wait for any fade out to finish */
			while ( music->fading == MIX_FADING_OUT ) {
				SDL_UnlockAudio();
				SDL_Delay(100);
				SDL_LockAudio();
			}
			if ( music == music_playing ) {
				music_internal_halt();
			}
		}
		SDL_UnlockAudio();
		switch (music->type) {
#ifdef CMD_MUSIC
			case MUS_CMD:
				MusicCMD_FreeSong(music->data.cmd);
				break;
#endif
#ifdef WAV_MUSIC
			case MUS_WAV:
				WAVStream_FreeSong(music->data.wave);
				break;
#endif
#ifdef MOD_MUSIC
			case MUS_MOD:
				MikMod_FreeSong(music->data.module);
				break;
#endif
#ifdef MID_MUSIC
			case MUS_MID:
#ifdef USE_NATIVE_MIDI
  				if ( native_midi_ok ) {
					native_midi_freesong(music->data.nativemidi);
				} MIDI_ELSE
#endif
#ifdef USE_TIMIDITY_MIDI
				if ( timidity_ok ) {
					Timidity_FreeSong(music->data.midi);
				}
#endif
				break;
#endif
#ifdef OGG_MUSIC
			case MUS_OGG:
				OGG_delete(music->data.ogg);
				break;
#endif
#ifdef MP3_MUSIC
			case MUS_MP3:
				SMPEG_delete(music->data.mp3);
				break;
#endif
			default:
				/* Unknown music type?? */
				break;
		}
		free(music);
	}
}

/* Find out the music format of a mixer music, or the currently playing
   music, if 'music' is NULL.
*/
Mix_MusicType Mix_GetMusicType(const Mix_Music *music)
{
	Mix_MusicType type = MUS_NONE;

	if ( music ) {
		type = music->type;
	} else {
		SDL_LockAudio();
		if ( music_playing ) {
			type = music_playing->type;
		}
		SDL_UnlockAudio();
	}
	return(type);
}

/* Play a music chunk.  Returns 0, or -1 if there was an error.
 */
static int music_internal_play(Mix_Music *music, double position)
{
	int retval = 0;

	/* Note the music we're playing */
	if ( music_playing ) {
		music_internal_halt();
	}
	music_playing = music;

	/* Set the initial volume */
	if ( music->fading == MIX_FADING_IN ) {
		music_internal_volume(0);
	} else {
		music_internal_volume(music_volume);
	}

	/* Set up for playback */
	switch (music->type) {
#ifdef CMD_MUSIC
	    case MUS_CMD:
		MusicCMD_Start(music->data.cmd);
		break;
#endif
#ifdef WAV_MUSIC
	    case MUS_WAV:
		WAVStream_Start(music->data.wave);
		break;
#endif
#ifdef MOD_MUSIC
	    case MUS_MOD:
		Player_Start(music->data.module);
		break;
#endif
#ifdef MID_MUSIC
	    case MUS_MID:
#ifdef USE_NATIVE_MIDI
		if ( native_midi_ok ) {
			native_midi_start(music->data.nativemidi);
		} MIDI_ELSE
#endif
#ifdef USE_TIMIDITY_MIDI
		if ( timidity_ok ) {
			Timidity_Start(music->data.midi);
		}
#endif
		break;
#endif
#ifdef OGG_MUSIC
	    case MUS_OGG:
		OGG_play(music->data.ogg);
		break;
#endif
#ifdef MP3_MUSIC
	    case MUS_MP3:
		SMPEG_enableaudio(music->data.mp3,1);
		SMPEG_enablevideo(music->data.mp3,0);
		SMPEG_play(music_playing->data.mp3);
		break;
#endif
	    default:
		Mix_SetError("Can't play unknown music type");
		retval = -1;
		break;
	}

	/* Set the playback position, note any errors if an offset is used */
	if ( retval == 0 ) {
		if ( position > 0.0 ) {
			if ( music_internal_position(position) < 0 ) {
				Mix_SetError("Position not implemented for music type");
				retval = -1;
			}
		} else {
			music_internal_position(0.0);
		}
	}

	/* If the setup failed, we're not playing any music anymore */
	if ( retval < 0 ) {
		music_playing = NULL;
	}
	return(retval);
}
int Mix_FadeInMusicPos(Mix_Music *music, int loops, int ms, double position)
{
	int retval;

	/* Don't play null pointers :-) */
	if ( music == NULL ) {
		Mix_SetError("music parameter was NULL");
		return(-1);
	}

	/* Setup the data */
	if ( ms ) {
		music->fading = MIX_FADING_IN;
	} else {
		music->fading = MIX_NO_FADING;
	}
	music->fade_step = 0;
	music->fade_steps = ms/ms_per_step;

	/* Play the puppy */
	SDL_LockAudio();
	/* If the current music is fading out, wait for the fade to complete */
	while ( music_playing && (music_playing->fading == MIX_FADING_OUT) ) {
		SDL_UnlockAudio();
		SDL_Delay(100);
		SDL_LockAudio();
	}
	music_active = 1;
	music_loops = loops;
	retval = music_internal_play(music, position);
	SDL_UnlockAudio();

	return(retval);
}
int Mix_FadeInMusic(Mix_Music *music, int loops, int ms)
{
	return Mix_FadeInMusicPos(music, loops, ms, 0.0);
}
int Mix_PlayMusic(Mix_Music *music, int loops)
{
	return Mix_FadeInMusicPos(music, loops, 0, 0.0);
}

/* Set the playing music position */
int music_internal_position(double position)
{
	int retval = 0;

	switch (music_playing->type) {
#ifdef MOD_MUSIC
	    case MUS_MOD:
		Player_SetPosition((UWORD)position);
		break;
#endif
#ifdef OGG_MUSIC
	    case MUS_OGG:
		OGG_jump_to_time(music_playing->data.ogg, position);
		break;
#endif
#ifdef MP3_MUSIC
	    case MUS_MP3:
		if ( position > 0.0 ) {
			SMPEG_skip(music_playing->data.mp3, position);
		} else {
			SMPEG_rewind(music_playing->data.mp3);
			SMPEG_play(music_playing->data.mp3);
		}
		break;
#endif
	    default:
		/* TODO: Implement this for other music backends */
		retval = -1;
		break;
	}
	return(retval);
}
int Mix_SetMusicPosition(double position)
{
	int retval;

	SDL_LockAudio();
	if ( music_playing ) {
		retval = music_internal_position(position);
		if ( retval < 0 ) {
			Mix_SetError("Position not implemented for music type");
		}
	} else {
		Mix_SetError("Music isn't playing");
		retval = -1;
	}
	SDL_UnlockAudio();

	return(retval);
}

/* Set the music volume */
static void music_internal_volume(int volume)
{
	switch (music_playing->type) {
#ifdef CMD_MUSIC
	    case MUS_CMD:
		MusicCMD_SetVolume(volume);
		break;
#endif
#ifdef WAV_MUSIC
	    case MUS_WAV:
		WAVStream_SetVolume(volume);
		break;
#endif
#ifdef MOD_MUSIC
	    case MUS_MOD:
		Player_SetVolume((SWORD)volume);
		break;
#endif
#ifdef MID_MUSIC
	    case MUS_MID:
#ifdef USE_NATIVE_MIDI
		if ( native_midi_ok ) {
			native_midi_setvolume(volume);
		} MIDI_ELSE
#endif
#ifdef USE_TIMIDITY_MIDI
		if ( timidity_ok ) {
			Timidity_SetVolume(volume);
		}
#endif
		break;
#endif
#ifdef OGG_MUSIC
	    case MUS_OGG:
		OGG_setvolume(music_playing->data.ogg, volume);
		break;
#endif
#ifdef MP3_MUSIC
	    case MUS_MP3:
		SMPEG_setvolume(music_playing->data.mp3,(int)(((float)volume/(float)MIX_MAX_VOLUME)*100.0));
		break;
#endif
	    default:
		/* Unknown music type?? */
		break;
	}
}
int Mix_VolumeMusic(int volume)
{
	int prev_volume;

	prev_volume = music_volume;
	if ( volume < 0 ) {
		return prev_volume;
	}
	if ( volume > SDL_MIX_MAXVOLUME ) {
		volume = SDL_MIX_MAXVOLUME;
	}
	music_volume = volume;
	SDL_LockAudio();
	if ( music_playing ) {
		music_internal_volume(music_volume);
	}
	SDL_UnlockAudio();
	return(prev_volume);
}

/* Halt playing of music */
static void music_internal_halt(void)
{
	switch (music_playing->type) {
#ifdef CMD_MUSIC
	    case MUS_CMD:
		MusicCMD_Stop(music_playing->data.cmd);
		break;
#endif
#ifdef WAV_MUSIC
	    case MUS_WAV:
		WAVStream_Stop();
		break;
#endif
#ifdef MOD_MUSIC
	    case MUS_MOD:
		Player_Stop();
		break;
#endif
#ifdef MID_MUSIC
	    case MUS_MID:
#ifdef USE_NATIVE_MIDI
		if ( native_midi_ok ) {
			native_midi_stop();
		} MIDI_ELSE
#endif
#ifdef USE_TIMIDITY_MIDI
		if ( timidity_ok ) {
			Timidity_Stop();
		}
#endif
		break;
#endif
#ifdef OGG_MUSIC
	    case MUS_OGG:
		OGG_stop(music_playing->data.ogg);
		break;
#endif
#ifdef MP3_MUSIC
	    case MUS_MP3:
		SMPEG_stop(music_playing->data.mp3);
		break;
#endif
	    default:
		/* Unknown music type?? */
		return;
	}
	music_playing->fading = MIX_NO_FADING;
	music_playing = NULL;
}
int Mix_HaltMusic(void)
{
	SDL_LockAudio();
	if ( music_playing ) {
		music_internal_halt();
	}
	SDL_UnlockAudio();

	return(0);
}

/* Progressively stop the music */
int Mix_FadeOutMusic(int ms)
{
	int retval = 0;

	SDL_LockAudio();
	if ( music_playing && (music_playing->fading == MIX_NO_FADING) ) {
		music_playing->fading = MIX_FADING_OUT;
		music_playing->fade_step = 0;
		music_playing->fade_steps = ms/ms_per_step;
		retval = 1;
	}
	SDL_UnlockAudio();

	return(retval);
}

Mix_Fading Mix_FadingMusic(void)
{
	Mix_Fading fading = MIX_NO_FADING;

	SDL_LockAudio();
	if ( music_playing ) {
		fading = music_playing->fading;
	}
	SDL_UnlockAudio();

	return(fading);
}

/* Pause/Resume the music stream */
void Mix_PauseMusic(void)
{
	music_active = 0;
}

void Mix_ResumeMusic(void)
{
	music_active = 1;
}

void Mix_RewindMusic(void)
{
	Mix_SetMusicPosition(0.0);
}

int Mix_PausedMusic(void)
{
	return (music_active == 0);
}

/* Check the status of the music */
static int music_internal_playing()
{
	int playing = 1;
	switch (music_playing->type) {
#ifdef CMD_MUSIC
	    case MUS_CMD:
		if (!MusicCMD_Active(music_playing->data.cmd)) {
			playing = 0;
		}
		break;
#endif
#ifdef WAV_MUSIC
	    case MUS_WAV:
		if ( ! WAVStream_Active() ) {
			playing = 0;
		}
		break;
#endif
#ifdef MOD_MUSIC
	    case MUS_MOD:
		if ( ! Player_Active() ) {
			playing = 0;
		}
		break;
#endif
#ifdef MID_MUSIC
	    case MUS_MID:
#ifdef USE_NATIVE_MIDI
		if ( native_midi_ok ) {
			if ( ! native_midi_active() )
				playing = 0;
		} MIDI_ELSE
#endif
#ifdef USE_TIMIDITY_MIDI
		if ( timidity_ok ) {
			if ( ! Timidity_Active() )
				playing = 0;
		}
#endif
		break;
#endif
#ifdef OGG_MUSIC
	    case MUS_OGG:
		if ( ! OGG_playing(music_playing->data.ogg) ) {
			playing = 0;
		}
		break;
#endif
#ifdef MP3_MUSIC
	    case MUS_MP3:
		if ( SMPEG_status(music_playing->data.mp3) != SMPEG_PLAYING )
			playing = 0;
		break;
#endif
	    default:
		playing = 0;
		break;
	}
	return(playing);
}
int Mix_PlayingMusic(void)
{
	int playing = 0;

	SDL_LockAudio();
	if ( music_playing ) {
		playing = music_internal_playing();
	}
	SDL_UnlockAudio();

	return(playing);
}

/* Set the external music playback command */
int Mix_SetMusicCMD(const char *command)
{
	Mix_HaltMusic();
	if ( music_cmd ) {
		free(music_cmd);
		music_cmd = NULL;
	}
	if ( command ) {
		music_cmd = (char *)malloc(strlen(command)+1);
		if ( music_cmd == NULL ) {
			return(-1);
		}
		strcpy(music_cmd, command);
	}
	return(0);
}

int Mix_SetSynchroValue(int i)
{
	if ( music_playing && ! music_stopped ) {
		switch (music_playing->type) {
#ifdef MOD_MUSIC
		    case MUS_MOD:
			if ( ! Player_Active() ) {
				return(-1);
			}
			Player_SetSynchroValue(i);
			return 0;
			break;
#endif
		    default:
			return(-1);
			break;
		}
		return(-1);
	}
	return(-1);
}

int Mix_GetSynchroValue(void)
{
	if ( music_playing && ! music_stopped ) {
		switch (music_playing->type) {
#ifdef MOD_MUSIC
		    case MUS_MOD:
			if ( ! Player_Active() ) {
				return(-1);
			}
			return Player_GetSynchroValue();
			break;
#endif
		    default:
			return(-1);
			break;
		}
		return(-1);
	}
	return(-1);
}


/* Uninitialize the music players */
void close_music(void)
{
	Mix_HaltMusic();
#ifdef CMD_MUSIC
	Mix_SetMusicCMD(NULL);
#endif
#ifdef MOD_MUSIC
	MikMod_Exit();
	MikMod_UnregisterAllLoaders();
	MikMod_UnregisterAllDrivers();
#endif
}

