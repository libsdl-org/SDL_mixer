/*
    native_midi_haiku:  Native Midi support on HaikuOS for the SDL_mixer library
    Copyright (C) 2010  Egor Suvorov

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

    Egor Suvorov
    egor_suvorov@mail.ru
*/
#include "SDL_config.h"

#ifdef __HAIKU__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MidiStore.h>
#include <MidiDefs.h>
#include <MidiSynthFile.h>
#include <algorithm>
#include <assert.h>
extern "C" {
#include "native_midi.h"
#include "native_midi_common.h"
}

bool compareMIDIEvent(const MIDIEvent &a, const MIDIEvent &b)
{
  return a.time < b.time;
}

class MidiEventsStore : public BMidi
{
  public:
  MidiEventsStore()
  {
    fPlaying = false;
  }
  virtual status_t Import(SDL_RWops *rw)
  {
    fEvs = CreateMIDIEventList(rw, &fDivision);
    if (!fEvs) {
      return B_BAD_MIDI_DATA;
    }
    fTotal = 0;
    for (MIDIEvent *x = fEvs; x; x = x->next) fTotal++;
    fPos = fTotal;

    sort_events();
    return B_OK;
  }
  virtual void Run()
  {
    fPlaying = true;
    fPos = 0;
    MIDIEvent *ev = fEvs;

    uint32 startTime = B_NOW;
    while (KeepRunning() && ev)
    {
      SprayEvent(ev, ev->time + startTime);
      ev = ev->next;
      fPos++;
    }
    fPos = fTotal;
    fPlaying = false;
  }
  virtual ~MidiEventsStore()
  {
    if (!fEvs) return;
    FreeMIDIEventList(fEvs);
    fEvs = 0;
  }

  int CurrentEvent()
  {
    return fPos;
  }
  int CountEvents()
  {
    return fTotal;
  }

  bool IsPlaying()
  {
    return fPlaying;
  }

  protected:
  MIDIEvent *fEvs;
  Uint16 fDivision;

  int fPos, fTotal;
  bool fPlaying;

  void SprayEvent(MIDIEvent *ev, uint32 time)
  {
    switch (ev->status & 0xF0)
    {
    case B_NOTE_OFF:
      SprayNoteOff((ev->status & 0x0F) + 1, ev->data[0], ev->data[1], time);
      break;
    case B_NOTE_ON:
      SprayNoteOn((ev->status & 0x0F) + 1, ev->data[0], ev->data[1], time);
      break;
    case B_KEY_PRESSURE:
      SprayKeyPressure((ev->status & 0x0F) + 1, ev->data[0], ev->data[1], time);
      break;
    case B_CONTROL_CHANGE:
      SprayControlChange((ev->status & 0x0F) + 1, ev->data[0], ev->data[1], time);
      break;
    case B_PROGRAM_CHANGE:
      SprayProgramChange((ev->status & 0x0F) + 1, ev->data[0], time);
      break;
    case B_CHANNEL_PRESSURE:
      SprayChannelPressure((ev->status & 0x0F) + 1, ev->data[0], time);
      break;
    case B_PITCH_BEND:
      SprayPitchBend((ev->status & 0x0F) + 1, ev->data[0], ev->data[1], time);
      break;
    case 0xF:
      switch (ev->status)
      {
      case B_SYS_EX_START:
        SpraySystemExclusive(ev->extraData, ev->extraLen, time);
	break;
      case B_MIDI_TIME_CODE:
      case B_SONG_POSITION:
      case B_SONG_SELECT:
      case B_CABLE_MESSAGE:
      case B_TUNE_REQUEST:
      case B_SYS_EX_END:
        SpraySystemCommon(ev->status, ev->data[0], ev->data[1], time);
	break;
      case B_TIMING_CLOCK:
      case B_START:
      case B_STOP:
      case B_CONTINUE:
      case B_ACTIVE_SENSING:
        SpraySystemRealTime(ev->status, time);
	break;
      case B_SYSTEM_RESET:
        if (ev->data[0] == 0x51 && ev->data[1] == 0x03)
	{
	  assert(ev->extraLen == 3);
	  int val = (ev->extraData[0] << 16) | (ev->extraData[1] << 8) | ev->extraData[2];
	  int tempo = 60000000 / val;
	  SprayTempoChange(tempo, time);
	}
	else
	{
	  SpraySystemRealTime(ev->status, time);
	}
      }
      break;
    }
  }

  void sort_events()
  {
    MIDIEvent *items = new MIDIEvent[fTotal];
    MIDIEvent *x = fEvs;
    for (int i = 0; i < fTotal; i++)
    {
      memcpy(items + i, x, sizeof(MIDIEvent));
      x = x->next;
    }
    std::sort(items, items + fTotal, compareMIDIEvent);

    x = fEvs;
    for (int i = 0; i < fTotal; i++)
    {
      MIDIEvent *ne = x->next;
      memcpy(x, items + i, sizeof(MIDIEvent));
      x->next = ne;
      x = ne;
    }

    for (x = fEvs; x && x->next; x = x->next)
      assert(x->time <= x->next->time);

    delete[] items;
  }
};

BMidiSynth synth;
struct _NativeMidiSong {
  MidiEventsStore *store;
} *currentSong = NULL;

char lasterr[1024];

int native_midi_detect()
{
  status_t res = synth.EnableInput(true, false);
  return res == B_OK;
}

void native_midi_setvolume(int volume)
{
  if (volume < 0) volume = 0;
  if (volume > 128) volume = 128;
  synth.SetVolume(volume / 128.0);
}

NativeMidiSong *native_midi_loadsong_RW(SDL_RWops *rw)
{
  NativeMidiSong *song = new NativeMidiSong;
  song->store = new MidiEventsStore;
  status_t res = song->store->Import(rw);

  if (res != B_OK)
  {
    snprintf(lasterr, sizeof lasterr, "Cannot Import() midi file: status_t=%d", res);
    delete song->store;
    delete song;
    return NULL;
  }
  return song;
}

NativeMidiSong *native_midi_loadsong(const char *midifile)
{
  SDL_RWops *rw = SDL_RWFromFile(midifile, "rb");
  if (!rw) return NULL;
  NativeMidiSong *song = native_midi_loadsong_RW(rw);
  SDL_RWclose(rw);
  return song;
}

void native_midi_freesong(NativeMidiSong *song)
{
  if (song == NULL) return;
  song->store->Stop();
  song->store->Disconnect(&synth);
  if (currentSong == song) 
  {
    currentSong = NULL;
  }
  delete song->store;
  delete song; song = 0;
}
void native_midi_start(NativeMidiSong *song)
{
  native_midi_stop();
  song->store->Connect(&synth);
  song->store->Start();
  currentSong = song;
}
void native_midi_stop()
{
  if (currentSong == NULL) return;
  currentSong->store->Stop();
  currentSong->store->Disconnect(&synth);
  while (currentSong->store->IsPlaying())
    usleep(1000);
  currentSong = NULL;
}
int native_midi_active()
{
  if (currentSong == NULL) return 0;
  return currentSong->store->CurrentEvent() < currentSong->store->CountEvents();
}

const char* native_midi_error(void)
{
  return lasterr;
}

#endif /* __HAIKU__ */
