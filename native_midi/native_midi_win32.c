/*
    native_midi:  Hardware Midi support for the SDL_mixer library
    Copyright (C) 2000,2001  Florian 'Proff' Schulze

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

/* everything below is currently one very big bad hack ;) Proff */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "native_midi.h"

#define XCHG_SHORT(x) ((((x)&0xFF)<<8) | (((x)>>8)&0xFF))
# define XCHG_LONG(x) ((((x)&0xFF)<<24) | \
		      (((x)&0xFF00)<<8) | \
		      (((x)&0xFF0000)>>8) | \
		      (((x)>>24)&0xFF))

#define LE_SHORT(x) x
#define LE_LONG(x) x
#define BE_SHORT(x) XCHG_SHORT(x)
#define BE_LONG(x) XCHG_LONG(x)

static UINT MidiDevice=MIDI_MAPPER;
static HMIDISTRM hMidiStream;
static NativeMidiSong *currentsong;

static int getvl(NativeMidiSong *song)
{
  int l=0;
  byte c;
  for (;;)
  {
    c=song->mididata.track[song->CurrentTrack].data[song->CurrentPos];
    song->CurrentPos++;
    l += (c & 0x7f);
    if (!(c & 0x80)) 
      return l;
    l<<=7;
  }
}

static void AddEvent(NativeMidiSong *song, DWORD at, DWORD type, byte event, byte a, byte b)
{
  MIDIEVENT *CurEvent;

  if ((song->BytesRecorded[song->CurrentTrack]+(int)sizeof(MIDIEVENT))>=song->BufferSize[song->CurrentTrack])
  {
    song->BufferSize[song->CurrentTrack]+=100*sizeof(MIDIEVENT);
    song->MidiEvents[song->CurrentTrack]=realloc(song->MidiEvents[song->CurrentTrack],song->BufferSize[song->CurrentTrack]);
  }
  CurEvent=(MIDIEVENT *)((byte *)song->MidiEvents[song->CurrentTrack]+song->BytesRecorded[song->CurrentTrack]);
  memset(CurEvent,0,sizeof(MIDIEVENT));
  CurEvent->dwDeltaTime=at;
  CurEvent->dwEvent=event+(a<<8)+(b<<16)+(type<<24);
  song->BytesRecorded[song->CurrentTrack]+=3*sizeof(DWORD);
}

static void MidiTracktoStream(NativeMidiSong *song)
{
  DWORD atime,len;
  byte event,type,a,b,c;
  byte laststatus,lastchan;

  song->CurrentPos=0;
  laststatus=0;
  lastchan=0;
  atime=0;
  for (;;)
  {
    if (song->CurrentPos>=song->mididata.track[song->CurrentTrack].len)
      return;
    atime+=getvl(song);
    event=song->mididata.track[song->CurrentTrack].data[song->CurrentPos];
    song->CurrentPos++;
    if(event==0xF0 || event == 0xF7) // SysEx event
    {
      len=getvl(song);
      song->CurrentPos+=len;
    }
    else if(event==0xFF) // Meta event
    {
      type=song->mididata.track[song->CurrentTrack].data[song->CurrentPos];
      song->CurrentPos++;
      len=getvl(song);
      switch(type)
        {
        case 0x2f:
          return;
        case 0x51: // Tempo
          a=song->mididata.track[song->CurrentTrack].data[song->CurrentPos];
          song->CurrentPos++;
          b=song->mididata.track[song->CurrentTrack].data[song->CurrentPos];
          song->CurrentPos++;
          c=song->mididata.track[song->CurrentTrack].data[song->CurrentPos];
          song->CurrentPos++;
          AddEvent(song, atime, MEVT_TEMPO, c, b, a);
          break;
        default:
          song->CurrentPos+=len;
          break;
        }
    }
    else
    {
      a=event;
      if (a & 0x80) // status byte
      {
        lastchan=a & 0x0F;
        laststatus=(a>>4) & 0x07;
        a=song->mididata.track[song->CurrentTrack].data[song->CurrentPos];
        song->CurrentPos++;
        a &= 0x7F;
      }
      switch(laststatus)
      {
      case 0: // Note off
        b=song->mididata.track[song->CurrentTrack].data[song->CurrentPos];
        song->CurrentPos++;
        b &= 0x7F;
        AddEvent(song, atime, MEVT_SHORTMSG, (byte)((laststatus<<4)+lastchan+0x80), a, b);
        break;

      case 1: // Note on
        b=song->mididata.track[song->CurrentTrack].data[song->CurrentPos];
        song->CurrentPos++;
        b &= 0x7F;
        AddEvent(song, atime, MEVT_SHORTMSG, (byte)((laststatus<<4)+lastchan+0x80), a, b);
        break;

      case 2: // Key Pressure
        b=song->mididata.track[song->CurrentTrack].data[song->CurrentPos];
        song->CurrentPos++;
        b &= 0x7F;
        AddEvent(song, atime, MEVT_SHORTMSG, (byte)((laststatus<<4)+lastchan+0x80), a, b);
        break;

      case 3: // Control change
        b=song->mididata.track[song->CurrentTrack].data[song->CurrentPos];
        song->CurrentPos++;
        b &= 0x7F;
        AddEvent(song, atime, MEVT_SHORTMSG, (byte)((laststatus<<4)+lastchan+0x80), a, b);
        break;

      case 4: // Program change
        a &= 0x7f;
        AddEvent(song, atime, MEVT_SHORTMSG, (byte)((laststatus<<4)+lastchan+0x80), a, 0);
        break;

      case 5: // Channel pressure
        a &= 0x7f;
        AddEvent(song, atime, MEVT_SHORTMSG, (byte)((laststatus<<4)+lastchan+0x80), a, 0);
        break;

      case 6: // Pitch wheel
        b=song->mididata.track[song->CurrentTrack].data[song->CurrentPos];
        song->CurrentPos++;
        b &= 0x7F;
        AddEvent(song, atime, MEVT_SHORTMSG, (byte)((laststatus<<4)+lastchan+0x80), a, b);
        break;

      default: 
        break;
      }
    }
  }
}

static int BlockOut(NativeMidiSong *song)
{
  MMRESULT err;
  int BlockSize;

  if ((song->MusicLoaded) && (song->NewEvents))
  {
    // proff 12/8/98: Added for savety
    midiOutUnprepareHeader(hMidiStream,&song->MidiStreamHdr,sizeof(MIDIHDR));
    if (song->NewPos>=song->NewSize)
      return 0;
    BlockSize=(song->NewSize-song->NewPos);
    if (BlockSize<=0)
      return 0;
    if (BlockSize>36000)
      BlockSize=36000;
    song->MidiStreamHdr.lpData=(void *)((byte *)song->NewEvents+song->NewPos);
    song->NewPos+=BlockSize;
    song->MidiStreamHdr.dwBufferLength=BlockSize;
    song->MidiStreamHdr.dwBytesRecorded=BlockSize;
    song->MidiStreamHdr.dwFlags=0;
//    lprintf(LO_DEBUG,"Data: %p, Size: %i\n",MidiStreamHdr.lpData,BlockSize);
    err=midiOutPrepareHeader(hMidiStream,&song->MidiStreamHdr,sizeof(MIDIHDR));
    if (err!=MMSYSERR_NOERROR)
      return 0;
    err=midiStreamOut(hMidiStream,&song->MidiStreamHdr,sizeof(MIDIHDR));
      return 0;
  }
  return 1;
}

static void MIDItoStream(NativeMidiSong *song)
{
  int BufferPos[MIDI_TRACKS];
  MIDIEVENT *CurEvent;
  MIDIEVENT *NewEvent;
  int lTime;
  int Dummy;
  int Track;

  //if (!hMidiStream)
    //return;
  song->NewSize=0;
  for (song->CurrentTrack=0;song->CurrentTrack<MIDI_TRACKS;song->CurrentTrack++)
  {
    song->MidiEvents[song->CurrentTrack]=NULL;
    song->BytesRecorded[song->CurrentTrack]=0;
    song->BufferSize[song->CurrentTrack]=0;
    MidiTracktoStream(song);
    song->NewSize+=song->BytesRecorded[song->CurrentTrack];
    BufferPos[song->CurrentTrack]=0;
  }
  song->NewEvents=realloc(song->NewEvents,song->NewSize);
  if (song->NewEvents)
  {
    song->NewPos=0;
    while (1)
    {
      lTime=INT_MAX;
      Track=-1;
      for (song->CurrentTrack=MIDI_TRACKS-1;song->CurrentTrack>=0;song->CurrentTrack--)
      {
        if ((song->BytesRecorded[song->CurrentTrack]>0) && (BufferPos[song->CurrentTrack]<song->BytesRecorded[song->CurrentTrack]))
          CurEvent=(MIDIEVENT *)((byte *)song->MidiEvents[song->CurrentTrack]+BufferPos[song->CurrentTrack]);
        else 
          continue;
        if ((int)CurEvent->dwDeltaTime<=lTime)
        {
          lTime=CurEvent->dwDeltaTime;
          Track=song->CurrentTrack;
        }
      }
      if (Track==-1)
        break;
      else
      {
        CurEvent=(MIDIEVENT *)((byte *)song->MidiEvents[Track]+BufferPos[Track]);
        BufferPos[Track]+=12;
        NewEvent=(MIDIEVENT *)((byte *)song->NewEvents+song->NewPos);
        memcpy(NewEvent,CurEvent,12);
        song->NewPos+=12;
      }
    }
    song->NewPos=0;
    lTime=0;
    while (song->NewPos<song->NewSize)
    {
      NewEvent=(MIDIEVENT *)((byte *)song->NewEvents+song->NewPos);
      Dummy=NewEvent->dwDeltaTime;
      NewEvent->dwDeltaTime-=lTime;
      lTime=Dummy;
      if ((song->NewPos+12)>=song->NewSize)
        NewEvent->dwEvent |= MEVT_F_CALLBACK;
      song->NewPos+=12;
    }
    song->NewPos=0;
    song->MusicLoaded=1;
    //BlockOut(song);
  }
  for (song->CurrentTrack=0;song->CurrentTrack<MIDI_TRACKS;song->CurrentTrack++)
  {
    if (song->MidiEvents[song->CurrentTrack])
      free(song->MidiEvents[song->CurrentTrack]);
  }
}

void CALLBACK MidiProc( HMIDIIN hMidi, UINT uMsg, DWORD dwInstance,
                        DWORD dwParam1, DWORD dwParam2 )
{
    switch( uMsg )
    {
    case MOM_DONE:
      if ((currentsong->MusicLoaded) && ((DWORD)dwParam1 == (DWORD)&currentsong->MidiStreamHdr))
        BlockOut(currentsong);
      break;
    case MOM_POSITIONCB:
      if ((currentsong->MusicLoaded) && ((DWORD)dwParam1 == (DWORD)&currentsong->MidiStreamHdr))
        currentsong->MusicPlaying=0;
      break;
    default:
      break;
    }
}

int native_midi_init()
{
  MMRESULT merr;
  HMIDISTRM MidiStream;

  merr=midiStreamOpen(&MidiStream,&MidiDevice,1,(DWORD)&MidiProc,0,CALLBACK_FUNCTION);
  if (merr!=MMSYSERR_NOERROR)
    MidiStream=0;
  midiStreamClose(MidiStream);
  if (!MidiStream)
    return 0;
  else
    return 1;
}

typedef struct
{
  char  ID[4];
  int   size;
  short format;
  short tracks;
  short deltatime;
} fmidihdr;

typedef struct
{
  char  ID[4];
  int   size;
} fmiditrack;

static void load_mididata(MIDI *mididata, FILE *fp)
{
  fmidihdr midihdr;
  fmiditrack trackhdr;
  int i;

  if (!mididata)
    return;
  if (!fp)
    return;
  fread(&midihdr,1,14,fp);
  mididata->divisions=BE_SHORT(midihdr.deltatime);
  for (i=0;i<BE_SHORT(midihdr.tracks);i++)
  {
    fread(&trackhdr,1,8,fp);
    mididata->track[i].len=BE_LONG(trackhdr.size);
    mididata->track[i].data=malloc(mididata->track[i].len);
    if (mididata->track[i].data)
      fread(mididata->track[i].data,1,mididata->track[i].len,fp);
  }
  mididata->loaded=1;
}

NativeMidiSong *native_midi_loadsong(char *midifile)
{
  FILE *fp;
  NativeMidiSong *newsong;

  newsong=malloc(sizeof(NativeMidiSong));
  if (!newsong)
    return NULL;
  memset(newsong,0,sizeof(NativeMidiSong));
  /* Open the file */
  fp = fopen(midifile, "rb");
  if ( fp != NULL )
  {
    newsong->mididata.loaded=0;
    load_mididata(&newsong->mididata, fp);
    if (!newsong->mididata.loaded)
    {
      free(newsong);
      fclose(fp);
      return NULL;
    }
    fclose(fp);
  }
  else
  {
    free(newsong);
    return NULL;
  }
  MIDItoStream(newsong);

  return newsong;
}

void native_midi_freesong(NativeMidiSong *song)
{
  if (hMidiStream)
  {
    midiStreamStop(hMidiStream);
    midiStreamClose(hMidiStream);
  }
  if (song)
  {
    if (song->NewEvents)
      free(song->NewEvents);
    free(song);
  }
}

void native_midi_start(NativeMidiSong *song)
{
  MMRESULT merr;
  MIDIPROPTIMEDIV mptd;

  native_midi_stop();
  if (!hMidiStream)
  {
    merr=midiStreamOpen(&hMidiStream,&MidiDevice,1,(DWORD)&MidiProc,0,CALLBACK_FUNCTION);
    if (merr!=MMSYSERR_NOERROR)
    {
      hMidiStream=0;
      return;
    }
    //midiStreamStop(hMidiStream);
//    MusicLoop=looping;
    currentsong=song;
    currentsong->NewPos=0;
    currentsong->MusicPlaying=1;
    mptd.cbStruct=sizeof(MIDIPROPTIMEDIV);
    mptd.dwTimeDiv=currentsong->mididata.divisions;
    merr=midiStreamProperty(hMidiStream,(LPBYTE)&mptd,MIDIPROP_SET | MIDIPROP_TIMEDIV);
    BlockOut(song);
    merr=midiStreamRestart(hMidiStream);
  }
}

void native_midi_stop()
{
  if (!hMidiStream)
    return;
  midiStreamStop(hMidiStream);
  midiStreamClose(hMidiStream);
  currentsong=NULL;
  hMidiStream = 0;
}

int native_midi_active()
{
  return currentsong->MusicPlaying;
}

void native_midi_setvolume(int volume)
{
}

char *native_midi_error()
{
  return "";
}

