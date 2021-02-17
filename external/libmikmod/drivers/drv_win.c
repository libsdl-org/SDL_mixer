/*	MikMod sound library
	(c) 1998, 1999, 2000 Miodrag Vallat and others - see file AUTHORS for
	complete list.

	This library is free software; you can redistribute it and/or modify
	it under the terms of the GNU Library General Public License as
	published by the Free Software Foundation; either version 2 of
	the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
	02111-1307, USA.
*/

/*==============================================================================

  $Id: drv_win.c,v 1.1.1.1 2004/06/01 12:16:17 raph Exp $

  Driver for output on win32 platforms using the multimedia API

==============================================================================*/

/*

	Written by Bjornar Henden <bhenden@online.no>

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mikmod_internals.h"

#ifdef DRV_WIN

#include <windows.h>

#if defined(_MSC_VER)
#pragma comment(lib,"winmm.lib")
#if (_MSC_VER < 1300)
typedef DWORD DWORD_PTR;
#endif
#endif

#define MINBUFFERS		2			/* min number of buffers */
#define MAXBUFFERS		16			/* max number of buffers */
#define MINBUFFERSIZE_MS	50			/* min buffer size in milliseconds */
#define MAXBUFFERSIZE_MS	1000			/* max buffer size in milliseconds */

#define DEFAULT_NUMBUFFERS	2
#define DEFAULT_BUFFERSIZE_MS	120

static HWAVEOUT	hwaveout;
static WAVEHDR	header[MAXBUFFERS];
static HPSTR	buffer[MAXBUFFERS];		/* pointers to buffers */
static WORD	buffersout;				/* number of buffers playing/about to be played */
static WORD	nextbuffer;				/* next buffer to be mixed */
static ULONG	buffersize;				/* buffer size in bytes */
static ULONG	buffersize_ms = DEFAULT_BUFFERSIZE_MS;	/* configured buffer size in milliseconds */
static ULONG	numbuffers = DEFAULT_NUMBUFFERS;	/* configured number of buffers */

static void WIN_CommandLine(CHAR *cmdline)
{
	CHAR *ptr;

	if((ptr = MD_GetAtom("buffer",cmdline,0)) != NULL) {
		buffersize_ms = atoi(ptr);
		if(buffersize_ms < MINBUFFERSIZE_MS || buffersize_ms > MAXBUFFERSIZE_MS)
			buffersize_ms = DEFAULT_BUFFERSIZE_MS;
		free(ptr);
	}
	if((ptr = MD_GetAtom("count",cmdline,0)) != NULL) {
		numbuffers = atoi(ptr);
		if(numbuffers < MINBUFFERS || numbuffers > MAXBUFFERS)
			numbuffers = DEFAULT_NUMBUFFERS;
		free(ptr);
	}
}

/* converts Windows error to libmikmod error */
static int WIN_GetError(MMRESULT mmr)
{
	switch (mmr) {
		case MMSYSERR_INVALHANDLE:
			return MMERR_WINMM_HANDLE;
		case MMSYSERR_NODRIVER:
			return MMERR_OPENING_AUDIO;
		case MMSYSERR_NOMEM:
			return MMERR_OUT_OF_MEMORY;
		case MMSYSERR_ALLOCATED:
			return MMERR_WINMM_ALLOCATED;
		case MMSYSERR_BADDEVICEID:
			return MMERR_WINMM_DEVICEID;
		case WAVERR_BADFORMAT:
			return MMERR_WINMM_FORMAT;
		default:
			return MMERR_WINMM_UNKNOWN; /* should hopefully not happen */
	}
}

static BOOL WIN_IsThere(void)
{
	return waveOutGetNumDevs()>0?1:0;
}

static void CALLBACK WIN_CallBack(HWAVEOUT hwo,UINT uMsg,DWORD_PTR dwInstance,DWORD_PTR dwParam1,DWORD_PTR dwParam2)
{
	if (uMsg==WOM_DONE) --buffersout;
}

static int WIN_Init(void)
{
	WAVEFORMATEX	wfe;
	WORD			samplesize;
	MMRESULT		mmr;
	int		n;

	samplesize=1;
	if (md_mode&DMODE_STEREO) samplesize<<=1;
	if (md_mode&DMODE_16BITS) samplesize<<=1;

	wfe.wFormatTag=WAVE_FORMAT_PCM;
	wfe.nChannels=md_mode&DMODE_STEREO?2:1;
	wfe.nSamplesPerSec=md_mixfreq;
	wfe.nAvgBytesPerSec=md_mixfreq*samplesize;
	wfe.nBlockAlign=samplesize;
	wfe.wBitsPerSample=md_mode&DMODE_16BITS?16:8;
	wfe.cbSize=sizeof(wfe);

	mmr=waveOutOpen(&hwaveout,WAVE_MAPPER,&wfe,(DWORD_PTR)WIN_CallBack,0,CALLBACK_FUNCTION);
	if (mmr!=MMSYSERR_NOERROR) {
		_mm_errno=WIN_GetError(mmr);
		return 1;
	}

	buffersize=md_mixfreq*samplesize*buffersize_ms/1000;

	for (n=0;n<numbuffers;n++) {
		buffer[n]=_mm_malloc(buffersize);
		header[n].lpData=buffer[n];
		header[n].dwBufferLength=buffersize;
		mmr=waveOutPrepareHeader(hwaveout,&header[n],sizeof(WAVEHDR));
		if (!buffer[n]||mmr!=MMSYSERR_NOERROR) {
			if (!buffer[n])
				_mm_errno=MMERR_OUT_OF_MEMORY;
			else
				_mm_errno=WIN_GetError(mmr);
			return 1;
		}
	}

	md_mode|=DMODE_SOFT_MUSIC|DMODE_SOFT_SNDFX;
	buffersout=nextbuffer=0;
	return VC_Init();
}

static void WIN_Exit(void)
{
	int n;

	VC_Exit();
	if (hwaveout) {
		for (n=0;n<numbuffers;n++) {
			if (header[n].dwFlags&WHDR_PREPARED)
				waveOutUnprepareHeader(hwaveout,&header[n],sizeof(WAVEHDR));
			_mm_free(buffer[n]);
		}
		while (waveOutClose(hwaveout)==WAVERR_STILLPLAYING) Sleep(10);
		hwaveout=NULL;
	}
}

static void WIN_Update(void)
{
	ULONG done;

	while (buffersout<numbuffers) {
		done=VC_WriteBytes((SBYTE*)buffer[nextbuffer],buffersize);
		if (!done) break;
		header[nextbuffer].dwBufferLength=done;
		waveOutWrite(hwaveout,&header[nextbuffer],sizeof(WAVEHDR));
		if (++nextbuffer>=numbuffers) nextbuffer%=numbuffers;
		++buffersout;
	}
}

static void WIN_PlayStop(void)
{
	if (hwaveout) waveOutReset(hwaveout);
	VC_PlayStop();
}

MIKMODAPI MDRIVER drv_win={
	NULL,
	"Windows waveform-audio",
	"Windows waveform-audio driver v0.1",
	0,255,
	"winmm",

	WIN_CommandLine,
	WIN_IsThere,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	WIN_Init,
	WIN_Exit,
	NULL,
	VC_SetNumVoices,
	VC_PlayStart,
	WIN_PlayStop,
	WIN_Update,
	NULL,
	VC_VoiceSetVolume,
	VC_VoiceGetVolume,
	VC_VoiceSetFrequency,
	VC_VoiceGetFrequency,
	VC_VoiceSetPanning,
	VC_VoiceGetPanning,
	VC_VoicePlay,
	VC_VoiceStop,
	VC_VoiceStopped,
	VC_VoiceGetPosition,
	VC_VoiceRealVolume
};

#else

MISSING(drv_win);

#endif

/* ex:set ts=4: */
