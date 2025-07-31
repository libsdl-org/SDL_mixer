/*

    TiMidity -- Experimental MIDI to WAVE converter
    Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

    sbk.h: SoundFont(tm) file format
    Copyright (C) 1996,1997 Takashi Iwai

    This program is free software; you can redistribute it and/or modify
    it under the terms of the Perl Artistic License, available in COPYING.
*/

#ifndef SBK_H_DEF
#define SBK_H_DEF

typedef struct _tchunk {
	char id[4];
	Sint32 size;
} tchunk;

typedef struct _tsbkheader {
	char riff[4];	/* RIFF */
	Sint32 size;	/* size of sbk after there bytes */
	char sfbk[4];	/* sfbk id */
} tsbkheader;

typedef struct _tsamplenames {
	char name[20];
} tsamplenames;

typedef struct _tpresethdr {
	char name[20];
	Uint16 preset, bank, bagNdx;
	/*int lib, genre, morphology;*/ /* reserved */
} tpresethdr;

typedef struct _tsampleinfo {
	Sint32 startsample, endsample;
	Sint32 startloop, endloop;
	/* ver.2 additional info */
	Sint32 samplerate;
	Uint8 originalPitch;
	Uint8 pitchCorrection;
	Uint16 samplelink;
	Uint16 sampletype;  /*1=mono, 2=right, 4=left, 8=linked, $8000=ROM*/
} tsampleinfo;

typedef struct _tinsthdr {
	char name[20];
	Uint16 bagNdx;
} tinsthdr;

typedef struct _tgenrec {
	Sint16 oper;
	Sint16 amount;
} tgenrec;


typedef struct _SFInfo {
	Uint16 version, minorversion;
	Sint32 samplepos, samplesize;

	int nrsamples;
	tsamplenames *samplenames;

	int nrpresets;
	tpresethdr *presethdr;

	int nrinfos;
	tsampleinfo *sampleinfo;

	int nrinsts;
	tinsthdr *insthdr;

	int nrpbags, nribags;
	Uint16 *presetbag, *instbag;

	int nrpgens, nrigens;
	tgenrec *presetgen, *instgen;

	/*tsbkheader sbkh;*/

	/*char *sf_name;*/

	int in_rom;
} SFInfo;


/*----------------------------------------------------------------
 * functions
 *----------------------------------------------------------------*/

#define load_sbk TIMI_NAMESPACE(load_sbk)
#define free_sbk TIMI_NAMESPACE(free_sbk)

int load_sbk(SDL_IOStream *io, SFInfo *sf);
void free_sbk(SFInfo *sf);

#endif
