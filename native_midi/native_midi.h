/*
    native_midi:  Hardware Midi support for the SDL_mixer library
    Copyright (C) 2000  Florian 'Proff' Schulze

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

    Florian 'Proff' Schulze
    florian.proff.schulze@gmx.net
*/

#ifndef _NATIVE_MIDI_H_
#define _NATIVE_MIDI_H_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>

#define MIDI_TRACKS 32

typedef unsigned char byte;
typedef struct MIDI                    /* a midi file */
{
   int divisions;                      /* number of ticks per quarter note */
   struct {
      unsigned char *data;             /* MIDI message stream */
      int len;                         /* length of the track data */
   } track[MIDI_TRACKS];
   int loaded;
} MIDI;

struct _NativeMidiSong {
  MIDI mididata;
  int MusicLoaded;
  int MusicPlaying;
  MIDIEVENT *MidiEvents[MIDI_TRACKS];
  MIDIHDR MidiStreamHdr;
  MIDIEVENT *NewEvents;
  int NewSize;
  int NewPos;
  int BytesRecorded[MIDI_TRACKS];
  int BufferSize[MIDI_TRACKS];
  int CurrentTrack;
  int CurrentPos;
};
#endif /* WINDOWS */

typedef struct _NativeMidiSong NativeMidiSong;

int native_midi_init();
NativeMidiSong *native_midi_loadsong(char *midifile);
void native_midi_freesong(NativeMidiSong *song);
void native_midi_start(NativeMidiSong *song);
void native_midi_stop();
int native_midi_active();
void native_midi_setvolume(int volume);
char *native_midi_error();

#endif /* _NATIVE_MIDI_H_ */
