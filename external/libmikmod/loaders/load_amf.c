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

  $Id: load_amf.c,v 1.1.1.1 2004/06/01 12:16:17 raph Exp $

  DMP Advanced Module Format loader

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

/*========== Module structure */

typedef struct AMFHEADER {
	UBYTE id[3];			/* AMF file marker */
	UBYTE version;			/* upper major, lower nibble minor version number */
	CHAR  songname[32];		/* ASCIIZ songname */
	UBYTE numsamples;		/* number of samples saved */
	UBYTE numorders;
	UWORD numtracks;		/* number of tracks saved */
	UBYTE numchannels;		/* number of channels used  */
	SBYTE panpos[32];		/* voice pan positions */
	UBYTE songbpm;
	UBYTE songspd;
} AMFHEADER;

typedef struct AMFSAMPLE {
	UBYTE type;
	CHAR  samplename[32];
	CHAR  filename[13];
	ULONG offset;
	ULONG length;
	UWORD c2spd;
	UBYTE volume;
	ULONG reppos;
	ULONG repend;
} AMFSAMPLE;

typedef struct AMFNOTE {
	UBYTE note,instr,volume,fxcnt;
	UBYTE effect[3];
	SBYTE parameter[3];
} AMFNOTE;

/*========== Loader variables */

static AMFHEADER *mh = NULL;
#define AMFTEXTLEN 22
static CHAR AMF_Version[AMFTEXTLEN+1] = "DSMI Module Format 0.0";
static AMFNOTE *track = NULL;

/*========== Loader code */

static BOOL AMF_Test(void)
{
	UBYTE id[3],ver;

	if(!_mm_read_UBYTES(id,3,modreader)) return 0;
	if(memcmp(id,"AMF",3)) return 0;

	ver=_mm_read_UBYTE(modreader);
	if((ver>=8)&&(ver<=14)) return 1;
	return 0;
}

static BOOL AMF_Init(void)
{
	if(!(mh=(AMFHEADER*)_mm_malloc(sizeof(AMFHEADER)))) return 0;
	if(!(track=(AMFNOTE*)_mm_calloc(64,sizeof(AMFNOTE)))) return 0;

	return 1;
}

static void AMF_Cleanup(void)
{
	_mm_free(mh);
	_mm_free(track);
}

/* Some older version 1.0 AMFs contain an anomaly where the sample length is
 * a DWORD, the loop start is a WORD, and the loop end is missing. Later AMF
 * 1.0 modules and up contain all three as DWORDs, and earlier versions have
 * all three as WORDs. This function tries to detect this edge case in the
 * instruments table. This should only be called on 1.0 AMFs.
 */
static BOOL AMF_ScanV10Instruments(MREADER *r, unsigned int numins)
{
	SLONG resetpos;
	BOOL res = 0;
	char str[32];
	ULONG idx, len, start, end;
	UBYTE type, vol;
	UWORD c2spd;
	unsigned int i;

	resetpos = _mm_ftell(r);
	if(resetpos < 0) return 0;

	for(i = 0; i < numins; i++) {
		type  = _mm_read_UBYTE(r);   /* type: should be 0 or 1 */
		_mm_read_string(str, 32, r); /* name */
		_mm_read_string(str, 13, r); /* filename */
		idx   = _mm_read_I_ULONG(r); /* index (should be <= numins) */
		len   = _mm_read_I_ULONG(r);
		c2spd = _mm_read_I_UWORD(r); /* should be > 0 */
		vol   = _mm_read_UBYTE(r);   /* should be [0,0x40] */
		start = _mm_read_I_ULONG(r); /* should be <= len */
		end   = _mm_read_I_ULONG(r); /* should be <= len */

		if((type != 0 && type != 1) || (idx > numins) || (c2spd == 0) ||
		   (vol > 0x40) || (start > len) || (end > len)) {
			res = 1;
			break;
		}

	}
	_mm_fseek(r, resetpos, SEEK_SET);
	return res;
}

static BOOL AMF_UnpackTrack(MREADER *r)
{
	ULONG tracksize;
	UBYTE row,cmd;
	SBYTE arg;

	/* empty track */
	memset(track,0,64*sizeof(AMFNOTE));

	/* read packed track */
	if (r) {
		tracksize=_mm_read_I_UWORD(r);

		/* The original code in DSMI library read the byte,
		   but it is not used, so we won't either */
//		tracksize+=((ULONG)_mm_read_UBYTE(r))<<16;
		(void)_mm_read_UBYTE(r);

		if (tracksize)
			while(tracksize--) {
				row=_mm_read_UBYTE(r);
				cmd=_mm_read_UBYTE(r);
				arg=_mm_read_SBYTE(r);
				/* unexpected end of track */
				if(!tracksize) {
					if((row==0xff)&&(cmd==0xff)&&(arg==-1))
						break;
					/* the last triplet should be FF FF FF, but this is not
					   always the case... maybe a bug in m2amf ?
					else
						return 0;
					*/

				}
				/* invalid row (probably unexpected end of row) */
				if (row>=64) {
					_mm_fseek(modreader, tracksize * 3, SEEK_CUR);
					return 1;
				}
				if (cmd<0x7f) {
					/* note, vol */
					/* Note that 0xff values mean this note was not originally
					   accomanied by a volume event. The +1 here causes it to
					   overflow to 0, which will then (correctly) be ignored later. */
					track[row].note=cmd;
					track[row].volume=(UBYTE)arg+1;
				} else
				  if (cmd==0x7f) {
					/* AMF.TXT claims this should duplicate the previous row, but
					   this is a lie. This note value is used to communicate to
					   DSMI that the current playing note should be updated when
					   an instrument is used with no note. This can be ignored. */
				} else
				  if (cmd==0x80) {
					/* instr */
					track[row].instr=arg+1;
				} else
				  if (cmd==0x83) {
					/* volume without note */
					track[row].volume=(UBYTE)arg+1;
				} else
				  if (cmd==0xff) {
					/* apparently, some M2AMF version fail to estimate the
					   size of the compressed patterns correctly, and end
					   up with blanks, i.e. dead triplets. Those are marked
					   with cmd == 0xff. Let's ignore them. */
				} else
				  if(track[row].fxcnt<3) {
					/* effect, param */
					if(cmd>0x97) {
						/* Instead of failing, we just ignore unknown effects.
						   This will load the "escape from dulce base" module */
						continue;
					}
					track[row].effect[track[row].fxcnt]=cmd&0x7f;
					track[row].parameter[track[row].fxcnt]=arg;
					track[row].fxcnt++;
				} else
					return 0;
			}
	}
	return 1;
}

static UBYTE *AMF_ConvertTrack(void)
{
	int row,fx4memory=0;

	/* convert track */
	UniReset();
	for (row=0;row<64;row++) {
		if (track[row].instr)  UniInstrument(track[row].instr-1);
		if (track[row].note>OCTAVE) UniNote(track[row].note-OCTAVE);

		/* AMF effects */
		while(track[row].fxcnt--) {
			SBYTE inf=track[row].parameter[track[row].fxcnt];

			switch(track[row].effect[track[row].fxcnt]) {
				case 1: /* Set speed */
					UniEffect(UNI_S3MEFFECTA,inf);
					break;
				case 2: /* Volume slide */
					if(inf) {
						UniWriteByte(UNI_S3MEFFECTD);
						if (inf>=0)
							UniWriteByte((inf&0xf)<<4);
						else
							UniWriteByte((-inf)&0xf);
					}
					break;
				/* effect 3, set channel volume, done in UnpackTrack */
				case 4: /* Porta up/down */
					if(inf) {
						if(inf>0) {
							UniEffect(UNI_S3MEFFECTE,inf);
							fx4memory=UNI_S3MEFFECTE;
						} else {
							UniEffect(UNI_S3MEFFECTF,-inf);
							fx4memory=UNI_S3MEFFECTF;
						}
					} else if(fx4memory)
						UniEffect(fx4memory,0);
					break;
				/* effect 5, "Porta abs", not supported */
				case 6: /* Porta to note */
					UniEffect(UNI_ITEFFECTG,inf);
					break;
				case 7: /* Tremor */
					UniEffect(UNI_S3MEFFECTI,inf);
					break;
				case 8: /* Arpeggio */
					UniPTEffect(0x0,inf);
					break;
				case 9: /* Vibrato */
					UniPTEffect(0x4,inf);
					break;
				case 0xa: /* Porta + Volume slide */
					UniPTEffect(0x3,0);
					if(inf) {
						UniWriteByte(UNI_S3MEFFECTD);
						if (inf>=0)
							UniWriteByte((inf&0xf)<<4);
						else
							UniWriteByte((-inf)&0xf);
					}
					break;
				case 0xb: /* Vibrato + Volume slide */
					UniPTEffect(0x4,0);
					if(inf) {
						UniWriteByte(UNI_S3MEFFECTD);
						if (inf>=0)
							UniWriteByte((inf&0xf)<<4);
						else
							UniWriteByte((-inf)&0xf);
					}
					break;
				case 0xc: /* Pattern break (in hex) */
					UniPTEffect(0xd,inf);
					break;
				case 0xd: /* Pattern jump */
					UniPTEffect(0xb,inf);
					break;
				/* effect 0xe, "Sync", not supported */
				case 0xf: /* Retrig */
					UniEffect(UNI_S3MEFFECTQ,inf&0xf);
					break;
				case 0x10: /* Sample offset */
					UniPTEffect(0x9,inf);
					break;
				case 0x11: /* Fine volume slide */
					if(inf) {
						UniWriteByte(UNI_S3MEFFECTD);
						if (inf>=0)
							UniWriteByte((inf&0xf)<<4|0xf);
						else
							UniWriteByte(0xf0|((-inf)&0xf));
					}
					break;
				case 0x12: /* Fine portamento */
					if(inf) {
						if(inf>0) {
							UniEffect(UNI_S3MEFFECTE,0xf0|(inf&0xf));
							fx4memory=UNI_S3MEFFECTE;
						} else {
							UniEffect(UNI_S3MEFFECTF,0xf0|((-inf)&0xf));
							fx4memory=UNI_S3MEFFECTF;
						}
					} else if(fx4memory)
						UniEffect(fx4memory,0);
					break;
				case 0x13: /* Delay note */
					UniPTEffect(0xe,0xd0|(inf&0xf));
					break;
				case 0x14: /* Note cut */
					UniPTEffect(0xc,0);
					track[row].volume=0;
					break;
				case 0x15: /* Set tempo */
					UniEffect(UNI_S3MEFFECTT,inf);
					break;
				case 0x16: /* Extra fine portamento */
					if(inf) {
						if(inf>0) {
							UniEffect(UNI_S3MEFFECTE,0xe0|((inf>>2)&0xf));
							fx4memory=UNI_S3MEFFECTE;
						} else {
							UniEffect(UNI_S3MEFFECTF,0xe0|(((-inf)>>2)&0xf));
							fx4memory=UNI_S3MEFFECTF;
						}
					} else if(fx4memory)
						UniEffect(fx4memory,0);
					break;
				case 0x17: /* Panning */
					/* S3M pan, except offset by -64. */
					if (inf>64)
						UniEffect(UNI_ITEFFECTS0,0x91); /* surround */
					else
						UniPTEffect(0x8,(inf==64)?255:(inf+64)<<1);
					of.flags |= UF_PANNING;
					break;
			}

		}
		if (track[row].volume) UniVolEffect(VOL_VOLUME,track[row].volume-1);
		UniNewline();
	}
	return UniDup();
}

static BOOL AMF_Load(BOOL curious)
{
	int t,u,realtrackcnt,realsmpcnt,defaultpanning;
	AMFSAMPLE s;
	SAMPLE *q;
	UWORD *track_remap;
	ULONG samplepos, fileend;
	UBYTE channel_remap[16];
	BOOL no_loopend;

	/* try to read module header  */
	_mm_read_UBYTES(mh->id,3,modreader);
	mh->version     =_mm_read_UBYTE(modreader);

	/* For version 8, the song name is only 20 characters long and then come
	// some data, which I do not know what is. The original code by Otto Chrons
	// load the song name as 20 characters long and then it is overwritten again
	// it another function, where it loads 32 characters, no matter which version
	// it is. So we do the same here */
	_mm_read_string(mh->songname,32,modreader);

	mh->numsamples  =_mm_read_UBYTE(modreader);
	mh->numorders   =_mm_read_UBYTE(modreader);
	mh->numtracks   =_mm_read_I_UWORD(modreader);

	if(mh->version>=9)
		mh->numchannels=_mm_read_UBYTE(modreader);
	else
		mh->numchannels=4;

	if((!mh->numchannels)||(mh->numchannels>(mh->version>=12?32:16))) {
		_mm_errno=MMERR_NOT_A_MODULE;
		return 0;
	}

	if(mh->version>=11) {
		memset(mh->panpos,0,32);
		_mm_read_SBYTES(mh->panpos,(mh->version>=13)?32:16,modreader);
	} else if(mh->version>=9)
		_mm_read_UBYTES(channel_remap,16,modreader);

	if (mh->version>=13) {
		mh->songbpm=_mm_read_UBYTE(modreader);
		if(mh->songbpm<32) {
			_mm_errno=MMERR_NOT_A_MODULE;
			return 0;
		}
		mh->songspd=_mm_read_UBYTE(modreader);
		if(mh->songspd>32) {
			_mm_errno=MMERR_NOT_A_MODULE;
			return 0;
		}
	} else {
		mh->songbpm=125;
		mh->songspd=6;
	}

	if(_mm_eof(modreader)) {
		_mm_errno = MMERR_LOADING_HEADER;
		return 0;
	}

	/* set module variables */
	of.initspeed = mh->songspd;
	of.inittempo = mh->songbpm;
	AMF_Version[AMFTEXTLEN-3]='0'+(mh->version/10);
	AMF_Version[AMFTEXTLEN-1]='0'+(mh->version%10);
	of.modtype   = _mm_strdup(AMF_Version);
	of.numchn    = mh->numchannels;
	of.numtrk    = mh->numorders*mh->numchannels;
	if (mh->numtracks>of.numtrk)
		of.numtrk=mh->numtracks;
	of.numtrk++;	/* add room for extra, empty track */
	of.songname  = DupStr(mh->songname,32,1);
	of.numpos    = mh->numorders;
	of.numpat    = mh->numorders;
	of.reppos    = 0;
	of.flags    |= UF_S3MSLIDES;
	/* XXX whenever possible, we should try to determine the original format.
	   Here we assume it was S3M-style wrt bpmlimit... */
	of.bpmlimit = 32;

	/*
	 * Play with the panning table. Although the AMF format embeds a
	 * panning table, if the module was a MOD or an S3M with default
	 * panning and didn't use any panning commands, don't flag
	 * UF_PANNING, to use our preferred panning table for this case.
	 */
	defaultpanning = 1;

	if(mh->version>=11) {
		for (t = 0; t < 32; t++) {
			if (mh->panpos[t] > 64) {
				of.panning[t] = PAN_SURROUND;
				defaultpanning = 0;
			} else
				if (mh->panpos[t] == 64)
					of.panning[t] = PAN_RIGHT;
				else
					of.panning[t] = (mh->panpos[t] + 64) << 1;
		}
	}
	else
		defaultpanning = 0;

	if (defaultpanning) {
		for (t = 0; t < of.numchn; t++)
			if (of.panning[t] == (((t + 1) & 2) ? PAN_RIGHT : PAN_LEFT)) {
				defaultpanning = 0;	/* not MOD canonical panning */
				break;
			}
	}
	if (defaultpanning)
		of.flags |= UF_PANNING;

	of.numins=of.numsmp=mh->numsamples;

	if(!AllocPositions(of.numpos)) return 0;
	for(t=0;t<of.numpos;t++)
		of.positions[t]=t;

	if(!AllocTracks()) return 0;
	if(!AllocPatterns()) return 0;

	/* read AMF order table */
	for (t=0;t<of.numpat;t++) {
		if (mh->version>=14)
			/* track size */
			of.pattrows[t]=_mm_read_I_UWORD(modreader);
		if ((mh->version==9) || (mh->version==10)) {
			/* Only version 9 and 10 uses channel remap */
			for (u = 0; u < of.numchn; u++)
				of.patterns[t * of.numchn + channel_remap[u]] = _mm_read_I_UWORD(modreader);
		}
		else
			_mm_read_I_UWORDS(of.patterns + (t * of.numchn), of.numchn, modreader);
	}
	if(_mm_eof(modreader)) {
		_mm_errno = MMERR_LOADING_HEADER;
		return 0;
	}

	/* read sample information */
	if(!AllocSamples()) return 0;

	no_loopend = 0;
	if(mh->version == 10) {
		no_loopend = AMF_ScanV10Instruments(modreader, of.numins);
	}

	q=of.samples;
	for(t=0;t<of.numins;t++) {
		/* try to read sample info */
		s.type=_mm_read_UBYTE(modreader);
		_mm_read_string(s.samplename,32,modreader);
		_mm_read_string(s.filename,13,modreader);
		s.offset    =_mm_read_I_ULONG(modreader);

		if(mh->version>=10)
			s.length =_mm_read_I_ULONG(modreader);
		else
			s.length = _mm_read_I_UWORD(modreader);

		s.c2spd     =_mm_read_I_UWORD(modreader);
		if(s.c2spd==8368) s.c2spd=8363;
		s.volume    =_mm_read_UBYTE(modreader);
		/* "the tribal zone.amf" and "the way its gonna b.amf" by Maelcum
		 * are the only version 10 files I can find, and they have 32 bit
		 * reppos and repend, not 16. */
		if(mh->version>=10 && no_loopend==0) {/* was 11 */
			s.reppos    =_mm_read_I_ULONG(modreader);
			s.repend    =_mm_read_I_ULONG(modreader);
		} else if(mh->version==10) {
			/* Early AMF 1.0 modules have the upper two bytes of
			 * the loop start and the entire loop end truncated.
			 * libxmp cites "sweetdrm.amf" and "facing_n.amf", but
			 * these are currently missing. M2AMF 1.3 (from DMP 2.32)
			 * has been confirmed to output these, however. */
			s.reppos    =_mm_read_I_UWORD(modreader);
			s.repend    =s.length;
			/* There's not really a correct way to handle the loop
			 * end, but this makes unlooped samples work at least. */
			if(s.reppos==0)
				s.repend=0;
		} else {
			s.reppos    =_mm_read_I_UWORD(modreader);
			s.repend    =_mm_read_I_UWORD(modreader);
			if (s.repend==0xffff)
				s.repend=0;
		}

		if(_mm_eof(modreader)) {
			_mm_errno = MMERR_LOADING_SAMPLEINFO;
			return 0;
		}

		q->samplename = DupStr(s.samplename,32,1);
		q->speed     = s.c2spd;
		q->volume    = s.volume;
		if (s.type) {
			q->seekpos   = s.offset;
			q->length    = s.length;
			q->loopstart = s.reppos;
			q->loopend   = s.repend;
			if((s.repend-s.reppos)>2) q->flags |= SF_LOOP;
		}
		q++;
	}

	/* read track table */
	if(!(track_remap=(UWORD*)_mm_calloc(mh->numtracks+1,sizeof(UWORD))))
		return 0;
	_mm_read_I_UWORDS(track_remap+1,mh->numtracks,modreader);
	if(_mm_eof(modreader)) {
		free(track_remap);
		_mm_errno=MMERR_LOADING_TRACK;
		return 0;
	}

	for(realtrackcnt=t=0;t<=mh->numtracks;t++)
		if (realtrackcnt<track_remap[t])
			realtrackcnt=track_remap[t];
	if (realtrackcnt > (int)mh->numtracks) {
		free(track_remap);
		_mm_errno=MMERR_NOT_A_MODULE;
		return 0;
	}
	for(t=0;t<of.numpat*of.numchn;t++)
		of.patterns[t]=(of.patterns[t]<=mh->numtracks)?
		               track_remap[of.patterns[t]]-1:realtrackcnt;

	free(track_remap);

	/* unpack tracks */
	for(t=0;t<realtrackcnt;t++) {
		if(_mm_eof(modreader)) {
			_mm_errno = MMERR_LOADING_TRACK;
			return 0;
		}
		if (!AMF_UnpackTrack(modreader)) {
			_mm_errno = MMERR_LOADING_TRACK;
			return 0;
		}
		if(!(of.tracks[t]=AMF_ConvertTrack()))
			return 0;
	}
	/* add an extra void track */
	UniReset();
	for(t=0;t<64;t++) UniNewline();
	of.tracks[realtrackcnt++]=UniDup();
	for(t=realtrackcnt;t<of.numtrk;t++) of.tracks[t]=NULL;

	/* compute sample offsets */
	if(_mm_eof(modreader)) goto fail;
	samplepos=_mm_ftell(modreader);
	_mm_fseek(modreader,0,SEEK_END);
	fileend=_mm_ftell(modreader);
	_mm_fseek(modreader,samplepos,SEEK_SET);
	for(realsmpcnt=t=0;t<of.numsmp;t++)
		if(realsmpcnt<of.samples[t].seekpos)
			realsmpcnt=of.samples[t].seekpos;
	for(t=1;t<=realsmpcnt;t++) {
		q=of.samples;
		u=0;
		while(q->seekpos!=t) {
			if(++u==of.numsmp)
				goto fail;
			q++;
		}
		q->seekpos=samplepos;
		samplepos+=q->length;
	}
	if(samplepos>fileend)
		goto fail;

	return 1;
fail:
	_mm_errno = MMERR_LOADING_SAMPLEINFO;
	return 0;
}

static CHAR *AMF_LoadTitle(void)
{
	CHAR s[32];

	_mm_fseek(modreader,4,SEEK_SET);
	if(!_mm_read_UBYTES(s,32,modreader)) return NULL;

	return(DupStr(s,32,1));
}

/*========== Loader information */

MIKMODAPI MLOADER load_amf={
	NULL,
	"AMF",
	"AMF (DSMI Advanced Module Format)",
	AMF_Init,
	AMF_Test,
	AMF_Load,
	AMF_Cleanup,
	AMF_LoadTitle
};

/* ex:set ts=4: */
