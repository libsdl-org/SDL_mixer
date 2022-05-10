/*	MikMod sound library
	(c) 1998, 1999, 2000, 2001, 2002 Miodrag Vallat and others - see file
	AUTHORS for complete list.

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

  $Id: load_med.c,v 1.1.1.1 2004/06/01 12:16:17 raph Exp $

  Amiga MED module loader

==============================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#include <string.h>

#include "mikmod_internals.h"

#ifdef SUNOS
extern int fprintf(FILE *, const char *, ...);
#endif

/*========== Module information */

#define MEDNOTECNT 120

typedef struct MEDHEADER {
	ULONG id;
	ULONG modlen;
	ULONG MEDSONGP;				/* struct MEDSONG *song; */
	UWORD psecnum;				/* for the player routine, MMD2 only */
	UWORD pseq;					/*  "   "   "   " */
	ULONG MEDBlockPP;			/* struct MEDBlock **blockarr; */
	ULONG reserved1;
	ULONG MEDINSTHEADERPP;		/* struct MEDINSTHEADER **smplarr; */
	ULONG reserved2;
	ULONG MEDEXPP;				/* struct MEDEXP *expdata; */
	ULONG reserved3;
	UWORD pstate;				/* some data for the player routine */
	UWORD pblock;
	UWORD pline;
	UWORD pseqnum;
	SWORD actplayline;
	UBYTE counter;
	UBYTE extra_songs;			/* number of songs - 1 */
} MEDHEADER;

typedef struct MEDSAMPLE {
	UWORD rep, replen;			/* offs: 0(s), 2(s) */
	UBYTE midich;				/* offs: 4(s) */
	UBYTE midipreset;			/* offs: 5(s) */
	UBYTE svol;					/* offs: 6(s) */
	SBYTE strans;				/* offs: 7(s) */
} MEDSAMPLE;

typedef struct MEDSONG {
	MEDSAMPLE sample[63];		/* 63 * 8 bytes = 504 bytes */
	UWORD numblocks;			/* offs: 504 */
	UWORD songlen;				/* offs: 506 */
	UBYTE playseq[256];			/* offs: 508 */
	UWORD deftempo;				/* offs: 764 */
	SBYTE playtransp;			/* offs: 766 */
	UBYTE flags;				/* offs: 767 */
	UBYTE flags2;				/* offs: 768 */
	UBYTE tempo2;				/* offs: 769 */
	UBYTE trkvol[16];			/* offs: 770 */
	UBYTE mastervol;			/* offs: 786 */
	UBYTE numsamples;			/* offs: 787 */
} MEDSONG;

typedef struct MEDEXP {
	ULONG nextmod;				/* pointer to next module */
	ULONG exp_smp;				/* pointer to MEDINSTEXT array */
	UWORD s_ext_entries;
	UWORD s_ext_entrsz;
	ULONG annotxt;				/* pointer to annotation text */
	ULONG annolen;
	ULONG iinfo;				/* pointer to MEDINSTINFO array */
	UWORD i_ext_entries;
	UWORD i_ext_entrsz;
	ULONG jumpmask;
	ULONG rgbtable;
	ULONG channelsplit;
	ULONG n_info;
	ULONG songname;				/* pointer to songname */
	ULONG songnamelen;
	ULONG dumps;
	ULONG reserved2[7];
} MEDEXP;

typedef struct MMD0NOTE {
	UBYTE a, b, c;
} MMD0NOTE;

typedef struct MMD1NOTE {
	UBYTE a, b, c, d;
} MMD1NOTE;

typedef struct MEDINSTHEADER {
	ULONG length;
	SWORD type;
	/* Followed by actual data */
} MEDINSTHEADER;

typedef struct MEDINSTEXT {
	UBYTE hold;
	UBYTE decay;
	UBYTE suppress_midi_off;
	SBYTE finetune;
} MEDINSTEXT;

typedef struct MEDINSTINFO {
	UBYTE name[40];
} MEDINSTINFO;

enum MEDINSTTYPE {
	INST_HYBRID	= -2,
	INST_SYNTH	= -1,
	INST_SAMPLE	= 0,
	INST_IFFOCT_5	= 1,
	INST_IFFOCT_3	= 2,
	INST_IFFOCT_2	= 3,
	INST_IFFOCT_4	= 4,
	INST_IFFOCT_6	= 5,
	INST_IFFOCT_7	= 6,
	INST_EXTSAMPLE	= 7
};

/*========== Loader variables */

#define MMD0_string 0x4D4D4430
#define MMD1_string 0x4D4D4431

static MEDHEADER *mh = NULL;
static MEDSONG *ms = NULL;
static MEDEXP *me = NULL;
static ULONG *ba = NULL;
static MMD0NOTE *mmd0pat = NULL;
static MMD1NOTE *mmd1pat = NULL;

static UBYTE medversion;
static BOOL decimalvolumes;
static BOOL bpmtempos;
static BOOL is8channel;
static UWORD rowsperbeat;

#define d0note(row,col) mmd0pat[((row)*(UWORD)of.numchn)+(col)]
#define d1note(row,col) mmd1pat[((row)*(UWORD)of.numchn)+(col)]

static CHAR MED_Version[] = "OctaMED (MMDx)";

/*========== Loader code */

static BOOL MED_Test(void)
{
	UBYTE id[4];

	if (!_mm_read_UBYTES(id, 4, modreader))
		return 0;
	if ((!memcmp(id, "MMD0", 4)) || (!memcmp(id, "MMD1", 4)))
		return 1;
	return 0;
}

static BOOL MED_Init(void)
{
	if (!(me = (MEDEXP *)_mm_malloc(sizeof(MEDEXP))))
		return 0;
	if (!(mh = (MEDHEADER *)_mm_malloc(sizeof(MEDHEADER))))
		return 0;
	if (!(ms = (MEDSONG *)_mm_malloc(sizeof(MEDSONG))))
		return 0;
	return 1;
}

static void MED_Cleanup(void)
{
	_mm_free(me);
	_mm_free(mh);
	_mm_free(ms);
	_mm_free(ba);
	_mm_free(mmd0pat);
	_mm_free(mmd1pat);
}

static UWORD MED_ConvertTempo(UWORD tempo)
{
	/* MED tempos 1-10 are compatibility tempos that are converted to different values.
	   These were determined by testing with OctaMED 2.00 and roughly correspond to the
	   formula: (195 + speed/2) / speed. Despite being "tracker compatible" these really
	   are supposed to change the tempo and NOT the speed. These are tempo-mode only. */
	static const UBYTE tempocompat[11] =
	{
		0, 195, 97, 65, 49, 39, 32, 28, 24, 22, 20
	};

	/* MEDs with 8 channels do something completely different with their tempos.
	   This behavior completely overrides normal timing when it is enabled.
	   NOTE: the tempos used for this are directly from OctaMED Soundstudio 2, but
	   in older versions the playback speeds differed slightly between NTSC and PAL.
	   This table seems to have been intended to be a compromise between the two.*/
	static const UBYTE tempo8channel[11] =
	{
		0, 179, 164, 152, 141, 131, 123, 116, 110, 104, 99
	};

	ULONG result;

	if (is8channel) {
		tempo = tempo < 10 ? tempo : 10;
		return tempo8channel[tempo];
	}

	if (bpmtempos) {
		/* Convert MED BPM into ProTracker-compatible BPM. All that really needs to be done
		   here is the BPM needs to be multiplied by (rows per beat)/(PT rows per beat = 4).
		   BPM mode doesn't have compatibility tempos like tempo mode but OctaMED does
		   something unusual with BPM<=2 that was found in electrosound 64.med. */
		result = (tempo > 2) ? ((ULONG)tempo * rowsperbeat + 2) / 4 : 125;
	} else {
		if (tempo >= 1 && tempo <= 10)
			tempo = tempocompat[tempo];

		/* Convert MED tempo into ProTracker-compatble BPM. */
		result = ((ULONG)tempo * 125) / 33;
	}

	return result < 65535 ? result : 65535;
}

static void EffectCvt(UBYTE note, UBYTE eff, UBYTE dat)
{
	switch (eff) {
	  /* 0x0: arpeggio */
	  case 0x1:				/* portamento up (PT compatible, ignore 0) */
		if (dat)
			UniPTEffect(0x1, dat);
		break;
	  case 0x2:				/* portamento down (PT compatible, ignore 0) */
		if (dat)
			UniPTEffect(0x2, dat);
		break;
	  /* 0x3: tone portamento */
	  case 0x4:				/* vibrato (~2x the speed/depth of PT vibrato) */
		UniWriteByte(UNI_MEDEFFECT_VIB);
		UniWriteByte((dat & 0xf0) >> 3);
		UniWriteByte((dat & 0x0f) << 1);
		break;
	  case 0x5:				/* tone portamento + volslide (MMD0: old vibrato) */
		if (medversion == 0) {
			/* Approximate conversion, probably wrong.
			   The entire param is depth and the rate is fixed. */
			UniWriteByte(UNI_MEDEFFECT_VIB);
			UniWriteByte(0x16);
			UniWriteByte((dat + 3) >> 2);
			break;
		}
		UniPTEffect(eff, dat);
		break;
	  /* 0x6: vibrato + volslide */
	  /* 0x7: tremolo */
	  case 0x8:				/* set hold/decay (FIXME- hold/decay not implemented) */
		break;
	  case 0x9:				/* set speed */
		/* TODO: Rarely MED modules request values of 0x00 or >0x20. In most Amiga versions
		   these behave as speed=(param & 0x1F) ? (param & 0x1F) : 32. From Soundstudio 2
		   up, these have different behavior. Since the docs/UI insist you shouldn't use
		   these values and not many modules use these, just ignore them for now. */
		if (dat >= 0x01 && dat <= 0x20) {
			UniEffect(UNI_S3MEFFECTA, dat);
		}
		break;
	  /* 0xa: volslide */
	  /* 0xb: position jump */
	  case 0xc:
		if (decimalvolumes)
			dat = (dat >> 4) * 10 + (dat & 0xf);
		UniPTEffect(0xc, dat);
		break;
	  case 0xa:
	  case 0xd:				/* same as PT volslide */
		/* if both nibbles are set, a slide up is performed. */
		if ((dat & 0xf) && (dat & 0xf0))
			dat &= 0xf0;
		UniPTEffect(0xa, dat);
		break;
	  case 0xe:				/* synth jump (FIXME- synth instruments not implemented) */
		break;
	  case 0xf:
		switch (dat) {
		  case 0:			/* patternbreak */
			UniPTEffect(0xd, 0);
			break;
		  case 0xf1:			/* play note twice */
			/* Note: OctaMED 6.00d and up will not play FF1/FF3 effects when
			   they are used on a line without a note. Since MMD2/MMD3 support is
			   theoretical at this point, allow these unconditionally for now. */
			UniWriteByte(UNI_MEDEFFECTF1);
			break;
		  case 0xf2:			/* delay note */
			UniWriteByte(UNI_MEDEFFECTF2);
			break;
		  case 0xf3:			/* play note three times */
			UniWriteByte(UNI_MEDEFFECTF3);
			break;
		  case 0xf8:			/* hardware filter off */
			UniPTEffect(0xe, 0x01);
			break;
		  case 0xf9:			/* hardware filter on */
			UniPTEffect(0xe, 0x00);
			break;
		  case 0xfd:			/* set pitch */
			UniWriteByte(UNI_MEDEFFECT_FD);
			break;
		  case 0xfe:			/* stop playing */
			UniPTEffect(0xb, of.numpat);
			break;
		  case 0xff:			/* note cut */
			UniPTEffect(0xc, 0);
			break;
		  default:
			if (dat <= 240)
				UniEffect(UNI_MEDSPEED, MED_ConvertTempo(dat));
		}
		break;
	  case 0x11:				/* fine portamento up */
		/* fine portamento of 0 does nothing. */
		if (dat)
			UniEffect(UNI_XMEFFECTE1, dat);
		break;
	  case 0x12:				/* fine portamento down */
		if (dat)
			UniEffect(UNI_XMEFFECTE2, dat);
		break;
	  case 0x14:				/* vibrato (PT compatible depth, ~2x speed) */
		UniWriteByte(UNI_MEDEFFECT_VIB);
		UniWriteByte((dat & 0xf0) >> 3);
		UniWriteByte((dat & 0x0f));
		break;
	  case 0x15:				/* set finetune */
		/* Valid values are 0x0 to 0x7 and 0xF8 to 0xFF. */
		if (dat <= 0x7 || dat >= 0xF8)
			UniPTEffect(0xe, 0x50 | (dat & 0xF));
		break;
	  case 0x16:				/* loop */
		UniEffect(UNI_MEDEFFECT_16, dat);
		break;
	  case 0x18:				/* cut note */
		UniEffect(UNI_MEDEFFECT_18, dat);
		break;
	  case 0x19:				/* sample offset */
		UniPTEffect(0x9, dat);
		break;
	  case 0x1a:				/* fine volslide up */
		/* volslide of 0 does nothing. */
		if (dat)
			UniEffect(UNI_XMEFFECTEA, dat);
		break;
	  case 0x1b:				/* fine volslide down */
		if (dat)
			UniEffect(UNI_XMEFFECTEB, dat);
		break;
	  case 0x1d:				/* pattern break */
		UniPTEffect(0xd, dat);
		break;
	  case 0x1e:				/* pattern delay */
		UniEffect(UNI_MEDEFFECT_1E, dat);
		break;
	  case 0x1f:				/* combined delay-retrigger */
		/* This effect does nothing on lines without a note. */
		if (note)
			UniEffect(UNI_MEDEFFECT_1F, dat);
		break;
	  default:
		if (eff < 0x10)
			UniPTEffect(eff, dat);
#ifdef MIKMOD_DEBUG
		else
			fprintf(stderr, "unsupported OctaMED command %u\n", eff);
#endif
		break;
	}
}

static UBYTE *MED_Convert1(int count, int col)
{
	int t;
	UBYTE inst, note, eff, dat;
	MMD1NOTE *n;

	UniReset();
	for (t = 0; t < count; t++) {
		n = &d1note(t, col);

		note = n->a & 0x7f;
		inst = n->b & 0x3f;
		eff = n->c;
		dat = n->d;

		if (inst)
			UniInstrument(inst - 1);
		if (note)
			UniNote(note - 1);
		EffectCvt(note, eff, dat);
		UniNewline();
	}
	return UniDup();
}

static UBYTE *MED_Convert0(int count, int col)
{
	int t;
	UBYTE a, b, inst, note, eff, dat;
	MMD0NOTE *n;

	UniReset();
	for (t = 0; t < count; t++) {
		n = &d0note(t, col);
		a = n->a;
		b = n->b;

		note = a & 0x3f;
		a >>= 6;
		a = ((a & 1) << 1) | (a >> 1);
		inst = (b >> 4) | (a << 4);
		eff = b & 0xf;
		dat = n->c;

		if (inst)
			UniInstrument(inst - 1);
		if (note)
			UniNote(note - 1);
		EffectCvt(note, eff, dat);
		UniNewline();
	}
	return UniDup();
}

static BOOL LoadMEDPatterns(void)
{
	int t, row, col;
	UWORD numtracks, numlines, maxlines = 0, track = 0;
	MMD0NOTE *mmdp;

	/* first, scan patterns to see how many channels are used */
	for (t = 0; t < of.numpat; t++) {
		_mm_fseek(modreader, ba[t], SEEK_SET);
		numtracks = _mm_read_UBYTE(modreader);
		numlines = _mm_read_UBYTE(modreader);

		if (numtracks > of.numchn)
			of.numchn = numtracks;
		if (numlines > maxlines)
			maxlines = numlines;
		/* sanity check */
		if (numtracks > 64)
			return 0;
	}
	/* sanity check */
	if (! of.numchn)	/* docs say 4, 8, 12 or 16 */
		return 0;

	of.numtrk = of.numpat * of.numchn;
	if (!AllocTracks())
		return 0;
	if (!AllocPatterns())
		return 0;

	if (!(mmd0pat = (MMD0NOTE *)_mm_calloc(of.numchn * (maxlines + 1), sizeof(MMD0NOTE))))
		return 0;

	/* second read: read and convert patterns */
	for (t = 0; t < of.numpat; t++) {
		_mm_fseek(modreader, ba[t], SEEK_SET);
		numtracks = _mm_read_UBYTE(modreader);
		numlines = _mm_read_UBYTE(modreader);

		of.pattrows[t] = ++numlines;
		memset(mmdp = mmd0pat, 0, of.numchn * maxlines * sizeof(MMD0NOTE));
		for (row = numlines; row; row--) {
			for (col = numtracks; col; col--, mmdp++) {
				mmdp->a = _mm_read_UBYTE(modreader);
				mmdp->b = _mm_read_UBYTE(modreader);
				mmdp->c = _mm_read_UBYTE(modreader);
			}
			/* Skip tracks this block doesn't use. */
			for (col = numtracks; col < of.numchn; col++, mmdp++) {}
		}

		for (col = 0; col < of.numchn; col++)
			of.tracks[track++] = MED_Convert0(numlines, col);
	}
	return 1;
}

static BOOL LoadMMD1Patterns(void)
{
	int t, row, col;
	UWORD numtracks, numlines, maxlines = 0, track = 0;
	MMD1NOTE *mmdp;

	/* first, scan patterns to see how many channels are used */
	for (t = 0; t < of.numpat; t++) {
		_mm_fseek(modreader, ba[t], SEEK_SET);
		numtracks = _mm_read_M_UWORD(modreader);
		numlines = _mm_read_M_UWORD(modreader);
		if (numtracks > of.numchn)
			of.numchn = numtracks;
		if (numlines > maxlines)
			maxlines = numlines;
		/* sanity check */
		if (numtracks > 64)
			return 0;
		if (numlines >= 3200) /* per docs */
			return 0;
	}
	/* sanity check */
	if (! of.numchn)	/* docs say 4, 8, 12 or 16 */
		return 0;

	of.numtrk = of.numpat * of.numchn;
	if (!AllocTracks())
		return 0;
	if (!AllocPatterns())
		return 0;

	if (!(mmd1pat = (MMD1NOTE *)_mm_calloc(of.numchn * (maxlines + 1), sizeof(MMD1NOTE))))
		return 0;

	/* second read: really read and convert patterns */
	for (t = 0; t < of.numpat; t++) {
		_mm_fseek(modreader, ba[t], SEEK_SET);
		numtracks = _mm_read_M_UWORD(modreader);
		numlines = _mm_read_M_UWORD(modreader);

		_mm_fseek(modreader, sizeof(ULONG), SEEK_CUR);
		of.pattrows[t] = ++numlines;
		memset(mmdp = mmd1pat, 0, of.numchn * maxlines * sizeof(MMD1NOTE));

		for (row = numlines; row; row--) {
			for (col = numtracks; col; col--, mmdp++) {
				mmdp->a = _mm_read_UBYTE(modreader);
				mmdp->b = _mm_read_UBYTE(modreader);
				mmdp->c = _mm_read_UBYTE(modreader);
				mmdp->d = _mm_read_UBYTE(modreader);
			}
			/* Skip tracks this block doesn't use. */
			for (col = numtracks; col < of.numchn; col++, mmdp++) {}
		}

		for (col = 0; col < of.numchn; col++)
			of.tracks[track++] = MED_Convert1(numlines, col);
	}
	return 1;
}

static BOOL MED_Load(BOOL curious)
{
	int t, i;
	ULONG sa[64];
	MEDINSTHEADER s;
	INSTRUMENT *d;
	SAMPLE *q;
	MEDSAMPLE *mss;

	/* try to read module header */
	mh->id = _mm_read_M_ULONG(modreader);
	mh->modlen = _mm_read_M_ULONG(modreader);
	mh->MEDSONGP = _mm_read_M_ULONG(modreader);
	mh->psecnum = _mm_read_M_UWORD(modreader);
	mh->pseq = _mm_read_M_UWORD(modreader);
	mh->MEDBlockPP = _mm_read_M_ULONG(modreader);
	mh->reserved1 = _mm_read_M_ULONG(modreader);
	mh->MEDINSTHEADERPP = _mm_read_M_ULONG(modreader);
	mh->reserved2 = _mm_read_M_ULONG(modreader);
	mh->MEDEXPP = _mm_read_M_ULONG(modreader);
	mh->reserved3 = _mm_read_M_ULONG(modreader);
	mh->pstate = _mm_read_M_UWORD(modreader);
	mh->pblock = _mm_read_M_UWORD(modreader);
	mh->pline = _mm_read_M_UWORD(modreader);
	mh->pseqnum = _mm_read_M_UWORD(modreader);
	mh->actplayline = _mm_read_M_SWORD(modreader);
	mh->counter = _mm_read_UBYTE(modreader);
	mh->extra_songs = _mm_read_UBYTE(modreader);

	/* Seek to MEDSONG struct */
	_mm_fseek(modreader, mh->MEDSONGP, SEEK_SET);

	/* Load the MED Song Header */
	mss = ms->sample;			/* load the sample data first */
	for (t = 63; t; t--, mss++) {
		mss->rep = _mm_read_M_UWORD(modreader);
		mss->replen = _mm_read_M_UWORD(modreader);
		mss->midich = _mm_read_UBYTE(modreader);
		mss->midipreset = _mm_read_UBYTE(modreader);
		mss->svol = _mm_read_UBYTE(modreader);
		mss->strans = _mm_read_SBYTE(modreader);
	}

	ms->numblocks = _mm_read_M_UWORD(modreader);
	ms->songlen = _mm_read_M_UWORD(modreader);
	_mm_read_UBYTES(ms->playseq, 256, modreader);
	/* sanity check */
	if (ms->numblocks > 255 || ms->songlen > 256) {
		_mm_errno = MMERR_NOT_A_MODULE;
		return 0;
	}
	ms->deftempo = _mm_read_M_UWORD(modreader);
	ms->playtransp = _mm_read_SBYTE(modreader);
	ms->flags = _mm_read_UBYTE(modreader);
	ms->flags2 = _mm_read_UBYTE(modreader);
	ms->tempo2 = _mm_read_UBYTE(modreader);
	_mm_read_UBYTES(ms->trkvol, 16, modreader);
	ms->mastervol = _mm_read_UBYTE(modreader);
	ms->numsamples = _mm_read_UBYTE(modreader);
	/* sanity check */
	if (ms->numsamples > 64) {
		_mm_errno = MMERR_NOT_A_MODULE;
		return 0;
	}

	/* check for a bad header */
	if (_mm_eof(modreader)) {
		_mm_errno = MMERR_LOADING_HEADER;
		return 0;
	}

	/* load extension structure */
	if (mh->MEDEXPP) {
		_mm_fseek(modreader, mh->MEDEXPP, SEEK_SET);
		me->nextmod = _mm_read_M_ULONG(modreader);
		me->exp_smp = _mm_read_M_ULONG(modreader);
		me->s_ext_entries = _mm_read_M_UWORD(modreader);
		me->s_ext_entrsz = _mm_read_M_UWORD(modreader);
		me->annotxt = _mm_read_M_ULONG(modreader);
		me->annolen = _mm_read_M_ULONG(modreader);
		me->iinfo = _mm_read_M_ULONG(modreader);
		me->i_ext_entries = _mm_read_M_UWORD(modreader);
		me->i_ext_entrsz = _mm_read_M_UWORD(modreader);
		me->jumpmask = _mm_read_M_ULONG(modreader);
		me->rgbtable = _mm_read_M_ULONG(modreader);
		me->channelsplit = _mm_read_M_ULONG(modreader);
		me->n_info = _mm_read_M_ULONG(modreader);
		me->songname = _mm_read_M_ULONG(modreader);
		me->songnamelen = _mm_read_M_ULONG(modreader);
		me->dumps = _mm_read_M_ULONG(modreader);
		/* sanity check */
		if (me->annolen > 0xffff) {
			_mm_errno = MMERR_NOT_A_MODULE;
			return 0;
		}
		/* truncate insane songnamelen (fail instead?) */
		if (me->songnamelen > 256)
			me->songnamelen = 256;
	}

	/* seek to and read the samplepointer array */
	_mm_fseek(modreader, mh->MEDINSTHEADERPP, SEEK_SET);
	if (!_mm_read_M_ULONGS(sa, ms->numsamples, modreader)) {
		_mm_errno = MMERR_LOADING_HEADER;
		return 0;
	}

	/* alloc and read the blockpointer array */
	if (!(ba = (ULONG *)_mm_calloc(ms->numblocks, sizeof(ULONG))))
		return 0;
	_mm_fseek(modreader, mh->MEDBlockPP, SEEK_SET);
	if (!_mm_read_M_ULONGS(ba, ms->numblocks, modreader)) {
		_mm_errno = MMERR_LOADING_HEADER;
		return 0;
	}

	/* copy song positions */
	if (!AllocPositions(ms->songlen))
		return 0;
	for (t = 0; t < ms->songlen; t++) {
		of.positions[t] = ms->playseq[t];
		if (of.positions[t]>ms->numblocks) { /* SANITIY CHECK */
		/*	fprintf(stderr,"positions[%d]=%d > numpat=%d\n",t,of.positions[t],ms->numblocks);*/
			_mm_errno = MMERR_LOADING_HEADER;
			return 0;
		}
	}

	decimalvolumes = (ms->flags & 0x10) ? 0 : 1;
	is8channel = (ms->flags & 0x40) ? 1 : 0;
	bpmtempos = (ms->flags2 & 0x20) ? 1 : 0;

	if (bpmtempos) {
		rowsperbeat = (ms->flags2 & 0x1f) + 1;
		of.initspeed = ms->tempo2;
		of.inittempo = MED_ConvertTempo(ms->deftempo);
	} else {
		of.initspeed = ms->tempo2;
		of.inittempo = ms->deftempo ? MED_ConvertTempo(ms->deftempo) : 128;
	}
	of.flags |= UF_HIGHBPM;
	MED_Version[12] = mh->id;
	of.modtype = _mm_strdup(MED_Version);
	of.numchn = 0;				/* will be counted later */
	of.numpat = ms->numblocks;
	of.numpos = ms->songlen;
	of.numins = ms->numsamples;
	of.numsmp = of.numins;
	of.reppos = 0;
	if ((mh->MEDEXPP) && (me->songname) && (me->songnamelen)) {
		char *name;

		_mm_fseek(modreader, me->songname, SEEK_SET);
		name = (char *) _mm_malloc(me->songnamelen);
		_mm_read_UBYTES(name, me->songnamelen, modreader);
		of.songname = DupStr(name, me->songnamelen, 1);
		free(name);
	} else
		of.songname = DupStr(NULL, 0, 0);
	if ((mh->MEDEXPP) && (me->annotxt) && (me->annolen)) {
		_mm_fseek(modreader, me->annotxt, SEEK_SET);
		ReadComment(me->annolen);
	}

	/* TODO: should do an initial scan for IFFOCT instruments to determine the
	   actual number of samples (instead of assuming 1-to-1). */
	if (!AllocSamples() || !AllocInstruments())
		return 0;

	of.flags |= UF_INST;
	q = of.samples;
	d = of.instruments;
	for (t = 0; t < of.numins; t++) {
		q->flags = SF_SIGNED;
		q->volume = 64;
		s.type = INST_SAMPLE;
		if (sa[t]) {
			_mm_fseek(modreader, sa[t], SEEK_SET);
			s.length = _mm_read_M_ULONG(modreader);
			s.type = _mm_read_M_SWORD(modreader);

			switch (s.type) {
			  case INST_SAMPLE:
			  case INST_EXTSAMPLE:
				break;

			  default:
#ifdef MIKMOD_DEBUG
				fprintf(stderr, "\rNon-sample instruments not supported in MED loader yet\n");
#endif
				if (!curious) {
					_mm_errno = MMERR_MED_SYNTHSAMPLES;
					return 0;
				}
				s.length = 0;
			}

			if (_mm_eof(modreader)) {
				_mm_errno = MMERR_LOADING_SAMPLEINFO;
				return 0;
			}

			q->length = s.length;
			q->seekpos = _mm_ftell(modreader);
			q->loopstart = ms->sample[t].rep << 1;
			q->loopend = q->loopstart + (ms->sample[t].replen << 1);

			if (ms->sample[t].replen > 1)
				q->flags |= SF_LOOP;

			if(ms->sample[t].svol <= 64)
				q->volume = ms->sample[t].svol;

			/* don't load sample if length>='MMD0'...
			   such kluges make libmikmod's code unique !!! */
			if (q->length >= MMD0_string)
				q->length = 0;
		} else
			q->length = 0;

		if ((mh->MEDEXPP) && (me->exp_smp) &&
			(t < me->s_ext_entries) && (me->s_ext_entrsz >= 4)) {
			MEDINSTEXT ie;

			_mm_fseek(modreader, me->exp_smp + t * me->s_ext_entrsz,
					  SEEK_SET);
			ie.hold = _mm_read_UBYTE(modreader);
			ie.decay = _mm_read_UBYTE(modreader);
			ie.suppress_midi_off = _mm_read_UBYTE(modreader);
			ie.finetune = _mm_read_SBYTE(modreader);

			q->speed = finetune[ie.finetune & 0xf];
		} else
			q->speed = 8363;

		if ((mh->MEDEXPP) && (me->iinfo) &&
			(t < me->i_ext_entries) && (me->i_ext_entrsz >= 40)) {
			MEDINSTINFO ii;

			_mm_fseek(modreader, me->iinfo + t * me->i_ext_entrsz, SEEK_SET);
			_mm_read_UBYTES(ii.name, 40, modreader);
			q->samplename = DupStr((char*)ii.name, 40, 1);
			d->insname = DupStr((char*)ii.name, 40, 1);
		} else {
			q->samplename = NULL;
			d->insname = NULL;
		}

		/* Instrument transpose tables. */
		for (i = 0; i < MEDNOTECNT; i++) {
			int note = i + 3 * OCTAVE + ms->sample[t].strans + ms->playtransp;

			/* TODO: IFFOCT instruments... */
			switch (s.type) {
			  case INST_EXTSAMPLE:
				/* TODO: not clear if this has the same wrapping behavior as regular samples.
				   This is a MMD2/MMD3 extension so it has not been tested. */
				note -= 2 * OCTAVE;
				/* fall-through */

			  case INST_SAMPLE:
				/* TODO: in MMD2/MMD3 mixing mode, these wrapping transforms don't apply. */
				if (note >= 10 * OCTAVE) {
					/* Buggy octaves 8 through A wrap to 2 octaves below octave 1.
					   Technically they're also a finetune step higher but that's safe
					   to ignore. */
					note -= 9 * OCTAVE;
				} else if (note >= 6 * OCTAVE) {
					/* Octaves 4 through 7 repeat octave 3. */
					note = (note % 12) + 5 * OCTAVE;
				}
				d->samplenumber[i] = t;
				d->samplenote[i] = note<0 ? 0 : note>255 ? 255 : note;
				break;
			}
		}

		q++;
		d++;
	}

	if (mh->id == MMD0_string) {
		medversion = 0;
		if (!LoadMEDPatterns()) {
			_mm_errno = MMERR_LOADING_PATTERN;
			return 0;
		}
	} else if (mh->id == MMD1_string) {
		medversion = 1;
		if (!LoadMMD1Patterns()) {
			_mm_errno = MMERR_LOADING_PATTERN;
			return 0;
		}
	} else {
		_mm_errno = MMERR_NOT_A_MODULE;
		return 0;
	}
	return 1;
}

static CHAR *MED_LoadTitle(void)
{
	ULONG posit, namelen;
	CHAR *name, *retvalue = NULL;

	_mm_fseek(modreader, 0x20, SEEK_SET);
	posit = _mm_read_M_ULONG(modreader);

	if (posit) {
		_mm_fseek(modreader, posit + 0x2C, SEEK_SET);
		posit = _mm_read_M_ULONG(modreader);
		namelen = _mm_read_M_ULONG(modreader);

		_mm_fseek(modreader, posit, SEEK_SET);
		name = (CHAR*) _mm_malloc(namelen);
		_mm_read_UBYTES(name, namelen, modreader);
		retvalue = DupStr(name, namelen, 1);
		free(name);
	}

	return retvalue;
}

/*========== Loader information */

MIKMODAPI MLOADER load_med = {
	NULL,
	"MED",
	"MED (OctaMED)",
	MED_Init,
	MED_Test,
	MED_Load,
	MED_Cleanup,
	MED_LoadTitle
};

/* ex:set ts=4: */
