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

  $Id: drv_ultra.c,v 1.1.1.1 2004/06/01 12:16:17 raph Exp $

  Driver for the Linux Ultrasound driver

==============================================================================*/

/*

	Written by Andy Lo A Foe <andy@alsa-project.org>

	Updated to work with later versions of both the ultrasound driver and
	libmikmod by C. Ray C. <crayc@pyro.net>

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mikmod_internals.h"

#ifdef DRV_ULTRA

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef MIKMOD_DYNAMIC
#include <dlfcn.h>
#endif
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <libgus.h>

#if !defined(GUS_INSTR_SIMPLE) || !defined(GUS_WAVE_BIDIR)
#error libgus version is too old
#endif
/* just in case */
#ifndef LIBGUS_VERSION_MAJOR
#define LIBGUS_VERSION_MAJOR 0x0003
#endif


#ifdef MIKMOD_DYNAMIC
/* runtime link with libgus */
static int (*_libgus_cards) (void);

#if LIBGUS_VERSION_MAJOR < 0x0004
static int (*_libgus_close) (int);
static int (*_libgus_do_flush) (void);
static void (*_libgus_do_tempo) (unsigned int);
static void (*_libgus_do_voice_frequency) (unsigned char, unsigned int);
static void (*_libgus_do_voice_pan) (unsigned char, unsigned short);
static void (*_libgus_do_voice_start) (unsigned char, unsigned int,
					unsigned int, unsigned short,
					unsigned short);
static void (*_libgus_do_voice_start_position) (unsigned char, unsigned int,
						unsigned int, unsigned short,
						unsigned short, unsigned int);
static void (*_libgus_do_voice_volume) (unsigned char, unsigned short);
static void (*_libgus_do_wait) (unsigned int);
static int (*_libgus_get_handle) (void);
static int (*_libgus_info) (gus_info_t *, int);
static int (*_libgus_memory_alloc) (gus_instrument_t *);
static int (*_libgus_memory_free) (gus_instrument_t *);
static int (*_libgus_memory_free_size) (void);
static int (*_libgus_memory_pack) (void);
static int (*_libgus_open) (int, size_t, int);
static int (*_libgus_queue_flush) (void);
static int (*_libgus_queue_read_set_size) (int);
static int (*_libgus_queue_write_set_size) (int);
static int (*_libgus_reset) (int, unsigned int);
static int (*_libgus_select) (int);
static int (*_libgus_timer_start) (void);
static int (*_libgus_timer_stop) (void);
static int (*_libgus_timer_tempo) (int);
#else
static int (*_libgus_close) (void*);
static int (*_libgus_do_flush) (void*);
static void (*_libgus_do_tempo) (void*, unsigned int);
static void (*_libgus_do_voice_frequency) (void*, unsigned char, unsigned int);
static void (*_libgus_do_voice_pan) (void*, unsigned char,unsigned short);
static void (*_libgus_do_voice_start) (void*, unsigned char, unsigned int,
					unsigned int, unsigned short,
					unsigned short);
static void (*_libgus_do_voice_start_position) (void*, unsigned char, unsigned int,
						unsigned int,unsigned short,
						unsigned short, unsigned int);
static void (*_libgus_do_voice_volume) (void*, unsigned char, unsigned short);
static void (*_libgus_do_wait) (void*, unsigned int);
static int (*_libgus_get_file_descriptor) (void*);
static int (*_libgus_info) (void*, gus_info_t*, int);
static int (*_libgus_memory_alloc) (void*, gus_instrument_t*);
static int (*_libgus_memory_free) (void*, gus_instrument_t*);
static int (*_libgus_memory_free_size) (void*);
static int (*_libgus_memory_pack) (void*);
static int (*_libgus_open) (void**, int, int, size_t, int);
static int (*_libgus_queue_flush) (void*);
static int (*_libgus_queue_read_set_size) (void*, int);
static int (*_libgus_queue_write_set_size) (void*, int);
static int (*_libgus_reset) (void*, int, unsigned int);
static int (*_libgus_timer_start)(void*);
static int (*_libgus_timer_stop) (void*);
static int (*_libgus_timer_tempo) (void*, int);
#endif
#ifndef HAVE_RTLD_GLOBAL
#define RTLD_GLOBAL (0)
#endif
static void *libgus = NULL;

#else
/* compile-time link with libgus */
#define _libgus_cards				gus_cards
#define _libgus_close				gus_close
#define _libgus_do_flush			gus_do_flush
#define _libgus_do_tempo			gus_do_tempo
#define _libgus_do_voice_frequency		gus_do_voice_frequency
#define _libgus_do_voice_pan			gus_do_voice_pan
#define _libgus_do_voice_start			gus_do_voice_start
#define _libgus_do_voice_start_position		gus_do_voice_start_position
#define _libgus_do_voice_volume			gus_do_voice_volume
#define _libgus_do_wait				gus_do_wait
#if LIBGUS_VERSION_MAJOR < 0x0004
#define _libgus_get_handle			gus_get_handle
#else
#define _libgus_get_file_descriptor		gus_get_file_descriptor
#endif
#define _libgus_info				gus_info
#define _libgus_memory_alloc			gus_memory_alloc
#define _libgus_memory_free			gus_memory_free
#define _libgus_memory_free_size		gus_memory_free_size
#define _libgus_memory_pack			gus_memory_pack
#define _libgus_open				gus_open
#define _libgus_queue_flush			gus_queue_flush
#define _libgus_queue_read_set_size		gus_queue_read_set_size
#define _libgus_queue_write_set_size		gus_queue_write_set_size
#define _libgus_reset				gus_reset
#if LIBGUS_VERSION_MAJOR < 0x0004
#define _libgus_select				gus_select
#endif
#define _libgus_timer_start			gus_timer_start
#define _libgus_timer_stop			gus_timer_stop
#define _libgus_timer_tempo			gus_timer_tempo
#endif

#define libgus_cards				_libgus_cards	/* same between v3 and v4 */
#define libgus_open				_libgus_open	/* different between v3 and v4: must use #ifdef */
#define libgus_close				_libgus_close	/* different between v3 and v4: must use #ifdef */
/* the following can be handled easily by macros: v4 only adds them the handle as the first param */
#if LIBGUS_VERSION_MAJOR < 0x0004
#define libgus_get_handle			_libgus_get_handle /* only in v3 */
#define libgus_do_flush				_libgus_do_flush
#define libgus_do_tempo				_libgus_do_tempo
#define libgus_do_voice_frequency		_libgus_do_voice_frequency
#define libgus_do_voice_pan			_libgus_do_voice_pan
#define libgus_do_voice_start			_libgus_do_voice_start
#define libgus_do_voice_start_position		_libgus_do_voice_start_position
#define libgus_do_voice_volume			_libgus_do_voice_volume
#define libgus_do_wait				_libgus_do_wait
#define libgus_info				_libgus_info
#define libgus_memory_alloc			_libgus_memory_alloc
#define libgus_memory_free			_libgus_memory_free
#define libgus_memory_free_size			_libgus_memory_free_size
#define libgus_memory_pack			_libgus_memory_pack
#define libgus_queue_flush			_libgus_queue_flush
#define libgus_queue_read_set_size		_libgus_queue_read_set_size
#define libgus_queue_write_set_size		_libgus_queue_write_set_size
#define libgus_reset				_libgus_reset
#define libgus_select				_libgus_select
#define libgus_timer_start			_libgus_timer_start
#define libgus_timer_stop			_libgus_timer_stop
#define libgus_timer_tempo			_libgus_timer_tempo
#else
#define libgus_get_file_descriptor		_libgus_get_file_descriptor /* only in v4 */
#define libgus_do_flush()			_libgus_do_flush(ultra_h)
#define libgus_do_tempo(t)			_libgus_do_tempo(ultra_h,t)
#define libgus_do_voice_frequency(a,b)		_libgus_do_voice_frequency(ultra_h,a,b)
#define libgus_do_voice_pan(a,b)		_libgus_do_voice_pan(ultra_h,a,b)
#define libgus_do_voice_start(a,b,c,d,e)	_libgus_do_voice_start(ultra_h,a,b,c,d,e)
#define libgus_do_voice_start_position(a,b,c,d,e,f) _libgus_do_voice_start_position(ultra_h,a,b,c,d,e,f)
#define libgus_do_voice_volume(a,b)		_libgus_do_voice_volume(ultra_h,a,b)
#define libgus_do_wait(a)			_libgus_do_wait(ultra_h,a)
#define libgus_info(a,b)			_libgus_info(ultra_h,a,b)
#define libgus_memory_alloc(a)			_libgus_memory_alloc(ultra_h,a)
#define libgus_memory_free(a)			_libgus_memory_free(ultra_h,a)
#define libgus_memory_free_size()		_libgus_memory_free_size(ultra_h)
#define libgus_memory_pack()			_libgus_memory_pack(ultra_h)
#define libgus_queue_flush()			_libgus_queue_flush(ultra_h)
#define libgus_queue_read_set_size(a)		_libgus_queue_read_set_size(ultra_h,a)
#define libgus_queue_write_set_size(a)		_libgus_queue_write_set_size(ultra_h,a)
#define libgus_reset(a,b)			_libgus_reset(ultra_h,a,b)
#define libgus_timer_start()			_libgus_timer_start(ultra_h)
#define libgus_timer_stop()			_libgus_timer_stop(ultra_h)
#define libgus_timer_tempo(a)			_libgus_timer_tempo(ultra_h,a)
#endif

#define MAX_INSTRUMENTS			128	/* Max. instruments loadable   */
#define GUS_CHANNELS			32	/* Max. GUS channels available */
#define SIZE_OF_SEQBUF		(8 * 1024)	/* Size of the sequence buffer */
#define ULTRA_PAN_MIDDLE	(16384 >> 1)	/* Middle balance position */

#define CH_FREQ	1
#define CH_VOL	2
#define CH_PAN	4

/*	This structure holds the information regarding a loaded sample, which
	is also kept in normal memory. */
typedef struct GUS_SAMPLE {
	SWORD *sample;
	ULONG length;
	ULONG loopstart;
	ULONG loopend;
	UWORD flags;
	UWORD active;
} GUS_SAMPLE;

/*	This structure holds the current state of a GUS voice channel. */
typedef struct GUS_VOICE {
	UBYTE kick;
	UBYTE active;
	UWORD flags;
	SWORD handle;
	ULONG start;
	ULONG size;
	ULONG reppos;
	ULONG repend;
	ULONG frq;
	int vol;
	int pan;

	int changes;
	time_t started;
} GUS_VOICE;

/* Global declarations follow */

static GUS_SAMPLE instrs[MAX_INSTRUMENTS];
static GUS_VOICE voices[GUS_CHANNELS];	/* channel status */
static int nr_instrs = 0;
static UWORD ultra_bpm;

static int ultra_dev = 0;	/* GUS index, if more than one card */
#if LIBGUS_VERSION_MAJOR < 0x0004
static int ultra_card = -1;	/* returned by gus_open(ultra_dev,,) - must be same as ultra_dev */
#else
static void* ultra_h = NULL;	/* GUS handle */
#endif
static int ultra_fd = -1;	/* GUS file descriptor */


#ifdef MIKMOD_DYNAMIC
static int Ultra_Link(void)
{
	if (libgus)
		return 0;

	/* load libgus.so */
#if LIBGUS_VERSION_MAJOR < 0x0004
	libgus = dlopen("libgus.so.3", RTLD_LAZY | RTLD_GLOBAL);
#else
	libgus = dlopen("libgus.so.4", RTLD_LAZY | RTLD_GLOBAL);
#endif
	if (!libgus) /* then this won't succeed either, but whatever.. */
		libgus = dlopen("libgus.so", RTLD_LAZY | RTLD_GLOBAL);
	if (!libgus)
		return 1;

	/* resolve function references */
#define IMPORT_SYMBOL(x) \
	if (!(_lib##x = dlsym(libgus, #x))) return 1
	IMPORT_SYMBOL(gus_cards);
	IMPORT_SYMBOL(gus_close);
	IMPORT_SYMBOL(gus_do_flush);
	IMPORT_SYMBOL(gus_do_tempo);
	IMPORT_SYMBOL(gus_do_voice_frequency);
	IMPORT_SYMBOL(gus_do_voice_pan);
	IMPORT_SYMBOL(gus_do_voice_start);
	IMPORT_SYMBOL(gus_do_voice_start_position);
	IMPORT_SYMBOL(gus_do_voice_volume);
	IMPORT_SYMBOL(gus_do_wait);
#if LIBGUS_VERSION_MAJOR < 0x0004
	IMPORT_SYMBOL(gus_get_handle);
#else
	IMPORT_SYMBOL(gus_get_file_descriptor);
#endif
	IMPORT_SYMBOL(gus_info);
	IMPORT_SYMBOL(gus_memory_alloc);
	IMPORT_SYMBOL(gus_memory_free);
	IMPORT_SYMBOL(gus_memory_free_size);
	IMPORT_SYMBOL(gus_memory_pack);
	IMPORT_SYMBOL(gus_open);
	IMPORT_SYMBOL(gus_queue_flush);
	IMPORT_SYMBOL(gus_queue_read_set_size);
	IMPORT_SYMBOL(gus_queue_write_set_size);
	IMPORT_SYMBOL(gus_reset);
#if LIBGUS_VERSION_MAJOR < 0x0004
	IMPORT_SYMBOL(gus_select);
#endif
	IMPORT_SYMBOL(gus_timer_start);
	IMPORT_SYMBOL(gus_timer_stop);
	IMPORT_SYMBOL(gus_timer_tempo);
#undef IMPORT_SYMBOL

	return 0;
}

static void Ultra_Unlink(void)
{
	_libgus_cards = NULL;
	_libgus_close = NULL;
	_libgus_do_flush = NULL;
	_libgus_do_tempo = NULL;
	_libgus_do_voice_frequency = NULL;
	_libgus_do_voice_pan = NULL;
	_libgus_do_voice_start = NULL;
	_libgus_do_voice_start_position = NULL;
	_libgus_do_voice_volume = NULL;
	_libgus_do_wait = NULL;
#if LIBGUS_VERSION_MAJOR < 0x0004
	_libgus_get_handle = NULL;
#else
	_libgus_get_file_descriptor = NULL;
#endif
	_libgus_info = NULL;
	_libgus_memory_alloc = NULL;
	_libgus_memory_free = NULL;
	_libgus_memory_free_size = NULL;
	_libgus_memory_pack = NULL;
	_libgus_open = NULL;
	_libgus_queue_flush = NULL;
	_libgus_queue_read_set_size = NULL;
	_libgus_queue_write_set_size = NULL;
	_libgus_reset = NULL;
#if LIBGUS_VERSION_MAJOR < 0x0004
	_libgus_select = NULL;
#endif
	_libgus_timer_start = NULL;
	_libgus_timer_stop = NULL;
	_libgus_timer_tempo = NULL;

	if (libgus) {
		dlclose(libgus);
		libgus = NULL;
	}
}
#endif

/* Checks for the presence of GUS cards */
static BOOL Ultra_IsThere(void)
{
	BOOL retval;

#ifdef MIKMOD_DYNAMIC
	if (Ultra_Link())
		return 0;
#endif
	retval = libgus_cards()? 1 : 0;
#ifdef MIKMOD_DYNAMIC
	Ultra_Unlink();
#endif
	return retval;
}

/* Loads all the samples into the GUS memory */
static BOOL loadsamples(void)
{
	int i;
	GUS_SAMPLE *smp;

	for (i = 0, smp = instrs; i < MAX_INSTRUMENTS; i++, smp++)
		if (smp->active) {
			ULONG length, loopstart, loopend;
			gus_instrument_t instrument;
			gus_layer_t layer;
			gus_wave_t wave;
			unsigned int type;

			/* convert position/length data from samples to bytes */
			length = smp->length;
			loopstart = smp->loopstart;
			loopend = smp->loopend;
			if (smp->flags & SF_16BITS) {
				length   <<=1;
				loopstart<<=1;
				loopend  <<=1;
			}

			memset(&instrument, 0, sizeof(instrument));
			memset(&layer, 0, sizeof(layer));
			memset(&wave, 0, sizeof(wave));

			instrument.mode=layer.mode=wave.mode=GUS_INSTR_SIMPLE;
			instrument.number.instrument=i;
			instrument.info.layer=&layer;
			layer.wave=&wave;
			type =	((smp->flags & SF_16BITS)?GUS_WAVE_16BIT:0)|
				((smp->flags & SF_DELTA) ?GUS_WAVE_DELTA:0)|
				((smp->flags & SF_LOOP)  ?GUS_WAVE_LOOP :0)|
				((smp->flags & SF_BIDI)  ?GUS_WAVE_BIDIR:0);

			wave.format = (unsigned char)type;
			wave.begin.ptr = (unsigned char*)smp->sample;
			wave.loop_start = loopstart<<4;
			wave.loop_end = loopend<<4;
			wave.size = length;

			if (smp->flags&SF_LOOP) {
				smp->sample[loopend] = smp->sample[loopstart];
				if ((smp->flags&SF_16BITS) && loopstart && loopend)
					smp->sample[loopend-1] = smp->sample[loopstart-1];
			}

			/* Download the sample to GUS RAM */
			if (libgus_memory_alloc(&instrument)) {
				_mm_errno = MMERR_SAMPLE_TOO_BIG;
				return 1;
			}
		}
	return 0;
}

/* Load a new sample into memory, but not in the GUS */
static SWORD Ultra_SampleLoad(struct SAMPLOAD *sload, int type)
{
	int handle;
	SAMPLE *s = sload->sample;
	ULONG length, loopstart, loopend;
	GUS_SAMPLE *smp;

	if (type == MD_SOFTWARE)
		return -1;

	if (s->length > MAX_SAMPLE_SIZE) {
		_mm_errno = MMERR_NOT_A_STREAM;/* better error? */
		return -1;
	}

	/* Find empty slot to put sample in */
	for (handle = 0; handle < MAX_INSTRUMENTS; handle++)
		if (!instrs[handle].active)
			break;

	if (handle == MAX_INSTRUMENTS) {
		_mm_errno = MMERR_OUT_OF_HANDLES;
		return -1;
	}

	length = s->length;
	loopstart = s->loopstart;
	loopend = s->loopend;

	smp = &instrs[handle];
	smp->length = length;
	smp->loopstart = loopstart;
	smp->loopend = loopend?loopend:length-1;
	smp->flags = s->flags;

	SL_SampleSigned(sload);

	if (!(smp->sample = (SWORD*)_mm_malloc((length+20)<<1))) {
		_mm_errno = MMERR_SAMPLE_TOO_BIG;
		return -1;
	}

	if (SL_Load(smp->sample,sload,s->length))
		return -1;

	smp->active = 1;
	nr_instrs++;

	return handle;
}

/* Discards a sample from the GUS memory, and from computer memory */
static void Ultra_SampleUnload(SWORD handle)
{
	GUS_SAMPLE *smp;

	if (handle >= MAX_INSTRUMENTS || handle < 0)
		return;

	smp = &instrs[handle];
	if (smp->active) {
		gus_instrument_t instrument;

		memset(&instrument, 0, sizeof(instrument));
		instrument.mode = GUS_INSTR_SIMPLE;
		instrument.number.instrument = handle;
		libgus_memory_free(&instrument);
		free(smp->sample);
		nr_instrs--;
		smp->active = 0;
	}
}

/* Reports available sample space */
static ULONG Ultra_SampleSpace(int type)
{
	if (type == MD_SOFTWARE)
		return 0;

	libgus_memory_pack();
	return (libgus_memory_free_size());
}

/* Reports the size of a sample */
static ULONG Ultra_SampleLength(int type, SAMPLE *s)
{
	if (!s)
		return 0;

	return (s->length*(s->flags&SF_16BITS?2:1))+16;
}

/* Initializes the driver */
static int Ultra_Init_internal(void)
{
	gus_info_t info;

	/* Check that requested settings are compatible with the GUS requirements */
	if((!(md_mode & DMODE_16BITS)) || (!(md_mode & DMODE_STEREO)) ||
	   (md_mixfreq!=44100)) {
		_mm_errno = MMERR_GUS_SETTINGS;
		return 1;
	}

	md_mode &= ~(DMODE_SOFT_MUSIC|DMODE_SOFT_SNDFX);

	ultra_dev = getenv("MM_ULTRA")? atoi(getenv("MM_ULTRA")) : 0;
#if LIBGUS_VERSION_MAJOR < 0x0004
	if ((ultra_card = libgus_open(ultra_dev, SIZE_OF_SEQBUF, 0)) < 0) {
		_mm_errno = (errno == ENOMEM)? MMERR_OUT_OF_MEMORY : MMERR_INVALID_DEVICE;
		return 1;
	}
	libgus_select(ultra_card);
	ultra_fd = libgus_get_handle();
#else
	if (libgus_open(&ultra_h, ultra_dev, 0, SIZE_OF_SEQBUF, GUS_OPEN_FLAG_NONE) < 0) {
		_mm_errno = (errno == ENOMEM)? MMERR_OUT_OF_MEMORY : MMERR_INVALID_DEVICE;
		return 1;
	}
	ultra_fd = libgus_get_file_descriptor(ultra_h);
#endif

	libgus_info(&info, 0);
#ifdef MIKMOD_DEBUG
	switch (info.version) {
	  case 0x24:
		fputs("GUS 2.4", stderr);
		break;
	  case 0x35:
		fputs("GUS 3.7 (flipped)", stderr);
		break;
	  case 0x37:
		fputs("GUS 3.7", stderr);
		break;
	  case 0x90:
		fputs("GUS ACE", stderr);
		break;
	  case 0xa0:
		fputs("GUS MAX 10", stderr);
		break;
	  case 0xa1:
		fputs("GUS MAX 11", stderr);
		break;
	  case 0x100:
		fputs("Interwave/GUS PnP", stderr);
		break;
	  default:
		fprintf(stderr, "Unknown GUS type %x", info.version);
		break;
	}
	fprintf(stderr, " with %dKb RAM on board\n", info.memory_size >> 10);
#endif

	return 0;
}

static int Ultra_Init(void)
{
#ifdef MIKMOD_DYNAMIC
	if (Ultra_Link()) {
		_mm_errno = MMERR_DYNAMIC_LINKING;
		return 1;
	}
#endif
	return Ultra_Init_internal();
}

/* Closes the driver */
static void Ultra_Exit_internal(void)
{
#if LIBGUS_VERSION_MAJOR < 0x0004
	if (ultra_card >= 0) {
		ultra_card = -1;
		libgus_close(ultra_dev);
	}
#else
	if (ultra_h) {
		libgus_close(ultra_h);
		ultra_h = NULL;
	}
#endif
	ultra_fd = -1;
}

static void Ultra_Exit(void)
{
	Ultra_Exit_internal();
#ifdef MIKMOD_DYNAMIC
	Ultra_Unlink();
#endif
}

/* Poor man's reset function */
static int Ultra_Reset(void)
{
	Ultra_Exit_internal();
	return Ultra_Init_internal();
}

static int Ultra_SetNumVoices(void)
{
	return 0;
}

/* Start playback */
static int Ultra_PlayStart(void)
{
	int t;

	for (t = 0; t < md_hardchn; t++) {
		voices[t].flags = 0;
		voices[t].handle = 0;
		voices[t].size = 0;
		voices[t].start = 0;
		voices[t].reppos = 0;
		voices[t].repend = 0;
		voices[t].changes = 0;
		voices[t].kick = 0;
		voices[t].frq = 10000;
		voices[t].vol = 64;
		voices[t].pan = ULTRA_PAN_MIDDLE;
		voices[t].active = 0;
	}

#if LIBGUS_VERSION_MAJOR < 0x0004
	libgus_select(ultra_card);
#endif
	if (libgus_reset(md_hardchn, 0) < 0) {
		_mm_errno = MMERR_GUS_RESET;
		return 1;
	}
	if (loadsamples())
		return 1;

	libgus_queue_write_set_size(1024);
	libgus_queue_read_set_size(128);

	if (libgus_timer_start() < 0) {
		_mm_errno = MMERR_GUS_TIMER;
		return 1;
	}
	libgus_timer_tempo(50);
	ultra_bpm = 0;

	for (t = 0; t < md_hardchn; t++) {
		libgus_do_voice_pan(t, ULTRA_PAN_MIDDLE);
		libgus_do_voice_volume(t, 64 << 8);
	}

	libgus_do_flush();

	return 0;
}

/* Stop playback */
static void Ultra_PlayStop(void)
{
	libgus_queue_flush();

	libgus_timer_stop();

	libgus_queue_write_set_size(0);
	libgus_queue_read_set_size(0);
}

/* Module player tick function */
static void ultraplayer(void)
{
	int t;
	struct GUS_VOICE *voice;

	md_player();

	for(t=0,voice=voices;t<md_numchn;t++,voice++) {
		if (voice->changes & CH_FREQ)
			libgus_do_voice_frequency(t, voice->frq);
		if (voice->changes & CH_VOL);
			libgus_do_voice_volume(t,voice->vol << 8);
		if (voice->changes & CH_PAN) {
			if (voice->pan == PAN_SURROUND)
				libgus_do_voice_pan(t, ULTRA_PAN_MIDDLE);
			else
				libgus_do_voice_pan(t, voice->pan << 6);
		}
		voice->changes = 0;
		if (voice->kick) {
			voice->kick = 0;
			if (voice->start > 0)
				libgus_do_voice_start_position(t, voice->handle,voice->frq,
								voice->vol << 8, voice->pan << 6,
								voice->start << 4);
			else
				libgus_do_voice_start(t, voice->handle, voice->frq,
							voice->vol << 8, voice->pan << 6);
		}
	}
	libgus_do_wait(1);
}

/* Play sound */
static void Ultra_Update(void)
{
	fd_set write_fds;
	int need_write;

	if (ultra_bpm != md_bpm) {
		libgus_do_tempo((md_bpm * 50) / 125);
		ultra_bpm = md_bpm;
	}

	ultraplayer();

	do {
		need_write = libgus_do_flush();

		FD_ZERO(&write_fds);
		do {
			FD_SET(ultra_fd, &write_fds);

			select(ultra_fd+1, NULL, &write_fds, NULL, NULL);
		} while (!FD_ISSET(ultra_fd, &write_fds));
	} while (need_write > 0);
}

/* Set the volume for the given voice */
static void Ultra_VoiceSetVolume(UBYTE voice, UWORD vol)
{
	if (voice < md_numchn)
		if (vol != voices[voice].vol) {
			voices[voice].vol = vol;
			voices[voice].changes |= CH_VOL;
		}
}

/* Returns the volume of the given voice */
static UWORD Ultra_VoiceGetVolume(UBYTE voice)
{
	return (voice < md_numchn) ? voices[voice].vol : 0;
}

/* Set the pitch for the given voice */
static void Ultra_VoiceSetFrequency(UBYTE voice, ULONG frq)
{
	if (voice < md_numchn)
		if (frq != voices[voice].frq) {
			voices[voice].frq = frq;
			voices[voice].changes |= CH_FREQ;
		}
}

/* Returns the frequency of the given voice */
static ULONG Ultra_VoiceGetFrequency(UBYTE voice)
{
	return (voice < md_numchn) ? voices[voice].frq : 0;
}

/* Set the panning position for the given voice */
static void Ultra_VoiceSetPanning(UBYTE voice, ULONG pan)
{
	if (voice < md_numchn)
		if (pan != voices[voice].pan) {
			voices[voice].pan = pan;
			voices[voice].changes |= CH_PAN;
		}
}

/* Returns the panning of the given voice */
static ULONG Ultra_VoiceGetPanning(UBYTE voice)
{
	return (voice < md_numchn) ? voices[voice].pan : 0;
}

/* Start a new sample on a voice */
static void Ultra_VoicePlay(UBYTE voice, SWORD handle, ULONG start,
							ULONG size, ULONG reppos, ULONG repend,
							UWORD flags)
{
	if ((voice >= md_numchn) || (start >= size))
		return;

	if (flags & SF_LOOP)
		if (repend > size)
			repend = size;

	voices[voice].flags = flags;
	voices[voice].handle = handle;
	voices[voice].start = start;
	voices[voice].size = size;
	voices[voice].reppos = reppos;
	voices[voice].repend = repend;
	voices[voice].kick = 1;
	voices[voice].active = 1;
	voices[voice].started = time(NULL);
}

/* Stops a voice */
static void Ultra_VoiceStop(UBYTE voice)
{
	if (voice < md_numchn)
		voices[voice].active = 0;
}

/* Returns whether a voice is stopped */
static BOOL Ultra_VoiceStopped(UBYTE voice)
{
	if (voice >= md_numchn)
		return 1;

	if (voices[voice].active) {
		/* is sample looping ? */
		if (voices[voice].flags & (SF_LOOP | SF_BIDI))
			return 0;
		else
		  if (time(NULL) - voices[voice].started >=
			  ((voices[voice].size - voices[voice].start + 44099)
			  / 44100)) {
			voices[voice].active = 0;
			return 1;
		}
		return 0;
	} else
		return 1;
}

/* Returns current voice position */
static SLONG Ultra_VoiceGetPosition(UBYTE voice)
{
	/* NOTE This information can not be determined. */
	return -1;
}

/* Returns voice real volume */
static ULONG Ultra_VoiceRealVolume(UBYTE voice)
{
	/* NOTE This information can not be accurately determined. */
	return 0;
}

MIKMODAPI MDRIVER drv_ultra={
	NULL,
	"Ultrasound driver",
	"Linux Ultrasound driver v1.12",
	GUS_CHANNELS,0,
	"ultra",
	NULL,
	Ultra_IsThere,
	Ultra_SampleLoad,
	Ultra_SampleUnload,
	Ultra_SampleSpace,
	Ultra_SampleLength,
	Ultra_Init,
	Ultra_Exit,
	Ultra_Reset,
	Ultra_SetNumVoices,
	Ultra_PlayStart,
	Ultra_PlayStop,
	Ultra_Update,
	NULL,
	Ultra_VoiceSetVolume,
	Ultra_VoiceGetVolume,
	Ultra_VoiceSetFrequency,
	Ultra_VoiceGetFrequency,
	Ultra_VoiceSetPanning,
	Ultra_VoiceGetPanning,
	Ultra_VoicePlay,
	Ultra_VoiceStop,
	Ultra_VoiceStopped,
	Ultra_VoiceGetPosition,
	Ultra_VoiceRealVolume
};
#else

MISSING(drv_ultra);

#endif /* DRV_ULTRA */

/* ex:set ts=8: */
