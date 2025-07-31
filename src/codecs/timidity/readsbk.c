/*

    TiMidity -- Experimental MIDI to WAVE converter
    Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

    readsbk.c: read soundfont file
    Copyright (C) 1996,1997 Takashi Iwai

    This program is free software; you can redistribute it and/or modify
    it under the terms of the Perl Artistic License, available in COPYING.
*/

#include "SDL.h"

#include "timidity.h"
#include "options.h"
#include "common.h"
#include "sbk.h"

/*----------------------------------------------------------------
 * function prototypes
 *----------------------------------------------------------------*/

#define NEW(type,nums)	(type*)SDL_calloc((nums), sizeof(type))

static int READCHUNK(tchunk *vp, SDL_RWops *rw)
{
	if (SDL_RWread(rw, vp, 1, 8) != 8) return -1;
	vp->size = SDL_SwapLE32(vp->size);
	return 1;
}

static int READDW(Sint32 *vp, SDL_RWops *rw)
{
	if (SDL_RWread(rw, vp, 1, 4) != 4) return -1;
	*vp = SDL_SwapLE32(*vp);
	return 1;
}

static int READW(Uint16 *vp, SDL_RWops *rw)
{
	if (SDL_RWread(rw, vp, 1, 2) != 2) return -1;
	*vp = SDL_SwapLE16(*vp);
	return 1;
}

static int READSTR(char *str, SDL_RWops *rw)
{
	int n;
	if (SDL_RWread(rw, str, 1, 20) != 20) return -1;
	str[19] = '\0';
	n = (int) SDL_strlen(str);
	while (n > 0 && str[n - 1] == ' ')
		n--;
	str[n] = '\0';
	return n;
}

#define READID(var,rw)	SDL_RWread(rw, var, 1, 4)
#define READB(var,rw)	SDL_RWread(rw, var, 1, 1)
#define SKIPB(rw)	SDL_RWseek(rw, 1, RW_SEEK_CUR);
#define SKIPW(rw)	SDL_RWseek(rw, 2, RW_SEEK_CUR);
#define SKIPDW(rw)	SDL_RWseek(rw, 4, RW_SEEK_CUR);

static int getchunk(const char *id);
static void process_chunk(int id, int s, SFInfo *sf, SDL_RWops *rw);
static void load_sample_names(int size, SFInfo *sf, SDL_RWops *rw);
static void load_preset_header(int size, SFInfo *sf, SDL_RWops *rw);
static void load_inst_header(int size, SFInfo *sf, SDL_RWops *rw);
static void load_bag(int size, SFInfo *sf, SDL_RWops *rw, int *totalp, Uint16 **bufp);
static void load_gen(int size, SFInfo *sf, SDL_RWops *rw, int *totalp, tgenrec **bufp);
static void load_sample_info(int size, SFInfo *sf, SDL_RWops *rw);


enum {
	/* level 0 */
	UNKN_ID, RIFF_ID, LIST_ID, SFBK_ID,
	/* level 1 */
	INFO_ID, SDTA_ID, PDTA_ID,
	/* info stuff */
	IFIL_ID, ISNG_ID, IROM_ID, INAM_ID, IVER_ID, IPRD_ID, ICOP_ID,
	ICRD_ID, IENG_ID, ISFT_ID, ICMT_ID,
	/* sample data stuff */
	SNAM_ID, SMPL_ID,
	/* preset stuff */
	PHDR_ID, PBAG_ID, PMOD_ID, PGEN_ID,
	/* inst stuff */
	INST_ID, IBAG_ID, IMOD_ID, IGEN_ID,
	/* sample header */
	SHDR_ID
};


/*----------------------------------------------------------------
 * debug routine
 *----------------------------------------------------------------*/

#if 0
static void debugid(const char *tag, const char *p)
{
	char buf[5]; SDL_strlcpy(buf, p, 5);
	SDL_Log("[%s:%s]", tag, buf);
}

static void debugname(const char *tag, const char *p)
{
	char buf[21]; SDL_strlcpy(buf, p, 21);
	SDL_Log("[%s:%s]", tag, buf);
}

static void debugval(const char *tag, int v)
{
	SDL_Log("[%s:%d]", tag, v);
}
#else
#define debugid(t,s) /**/
#define debugname(t,s) /**/
#define debugval(t,v) /**/
#endif


/*----------------------------------------------------------------
 * load sbk file
 *----------------------------------------------------------------*/

int load_sbk(SDL_RWops *rw, SFInfo *sf)
{
	const Sint64 rwend = SDL_RWsize(rw);
	tchunk chunk, subchunk;

	if (rwend < 32) /* better?? */
		return -1;

	READCHUNK(&chunk, rw);
	if (getchunk(chunk.id) != RIFF_ID) return -1;
	if (chunk.size != rwend - 8) return -1;

	READID(chunk.id, rw);
	if (getchunk(chunk.id) != SFBK_ID) return -1;

	sf->in_rom = 1;
	while (SDL_RWtell(rw) < rwend) {
		READID(chunk.id, rw);
		switch (getchunk(chunk.id)) {
		case LIST_ID:
			READDW(&chunk.size, rw);
			READID(subchunk.id, rw);
			process_chunk(getchunk(subchunk.id), chunk.size - 4, sf, rw);
			break;
		}
	}

	return 0;
}


/*----------------------------------------------------------------
 * free buffer
 *----------------------------------------------------------------*/

void free_sbk(SFInfo *sf)
{
	SDL_free(sf->samplenames);
	SDL_free(sf->presethdr);
	SDL_free(sf->sampleinfo);
	SDL_free(sf->insthdr);
	SDL_free(sf->presetbag);
	SDL_free(sf->instbag);
	SDL_free(sf->presetgen);
	SDL_free(sf->instgen);
	/*SDL_free(sf->sf_name);*/
	SDL_memset(sf, 0, sizeof(*sf));
}



/*----------------------------------------------------------------
 * get id value
 *----------------------------------------------------------------*/

static int getchunk(const char *id)
{
	static struct idstring {
		const char *str;
		int id;
	} idlist[] = {
		{"RIFF", RIFF_ID},
		{"LIST", LIST_ID},
		{"sfbk", SFBK_ID},
		{"INFO", INFO_ID},
		{"sdta", SDTA_ID},
		{"snam", SNAM_ID},
		{"smpl", SMPL_ID},
		{"pdta", PDTA_ID},
		{"phdr", PHDR_ID},
		{"pbag", PBAG_ID},
		{"pmod", PMOD_ID},
		{"pgen", PGEN_ID},
		{"inst", INST_ID},
		{"ibag", IBAG_ID},
		{"imod", IMOD_ID},
		{"igen", IGEN_ID},
		{"shdr", SHDR_ID},
		{"ifil", IFIL_ID},
		{"isng", ISNG_ID},
		{"irom", IROM_ID},
		{"iver", IVER_ID},
		{"INAM", INAM_ID},
		{"IPRD", IPRD_ID},
		{"ICOP", ICOP_ID},
		{"ICRD", ICRD_ID},
		{"IENG", IENG_ID},
		{"ISFT", ISFT_ID},
		{"ICMT", ICMT_ID},
	};
	static const int listsize = (int) sizeof(idlist)/sizeof(idlist[0]);

	int i;

	for (i = 0; i < listsize; i++) {
		if (SDL_memcmp(id, idlist[i].str, 4) == 0) {
			debugid("ok", id);
			return idlist[i].id;
		}
	}

	debugid("xx", id);
	return UNKN_ID;
}


static void load_sample_names(int size, SFInfo *sf, SDL_RWops *rw)
{
	int i;
	sf->nrsamples = size / 20;
	sf->samplenames = NEW(tsamplenames, sf->nrsamples);
	for (i = 0; i < sf->nrsamples; i++) {
		READSTR(sf->samplenames[i].name, rw);
	}
}

static void load_preset_header(int size, SFInfo *sf, SDL_RWops *rw)
{
	int i;
	sf->nrpresets = size / 38;
	sf->presethdr = NEW(tpresethdr, sf->nrpresets);
	for (i = 0; i < sf->nrpresets; i++) {
		READSTR(sf->presethdr[i].name, rw);
		READW(&sf->presethdr[i].preset, rw);
		READW(&sf->presethdr[i].bank, rw);
		READW(&sf->presethdr[i].bagNdx, rw);
		SKIPDW(rw); /* lib */
		SKIPDW(rw); /* genre */
		SKIPDW(rw); /* morph */
	}
}

static void load_inst_header(int size, SFInfo *sf, SDL_RWops *rw)
{
	int i;

	sf->nrinsts = size / 22;
	sf->insthdr = NEW(tinsthdr, sf->nrinsts);
	for (i = 0; i < sf->nrinsts; i++) {
		READSTR(sf->insthdr[i].name, rw);
		READW(&sf->insthdr[i].bagNdx, rw);
	}
}

static void load_bag(int size, SFInfo *sf, SDL_RWops *rw, int *totalp, Uint16 **bufp)
{
	Uint16 *buf;
	int i;

	(void) sf;
	debugval("bagsize", size);
	size /= 4;
	buf = NEW(Uint16, size);
	for (i = 0; i < size; i++) {
		READW(&buf[i], rw);
		SKIPW(rw); /* mod */
	}
	*totalp = size;
	*bufp = buf;
}

static void load_gen(int size, SFInfo *sf, SDL_RWops *rw, int *totalp, tgenrec **bufp)
{
	tgenrec *buf;
	int i;

	(void) sf;
	debugval("gensize", size);
	size /= 4;
	buf = NEW(tgenrec, size);
	for (i = 0; i < size; i++) {
		READW((Uint16 *)&buf[i].oper, rw);
		READW((Uint16 *)&buf[i].amount, rw);
	}
	*totalp = size;
	*bufp = buf;
}

static void load_sample_info(int size, SFInfo *sf, SDL_RWops *rw)
{
	int i;

	debugval("infosize", size);
	if (sf->version > 1) {
		sf->nrinfos = size / 46;
		sf->nrsamples = sf->nrinfos;
		sf->sampleinfo = NEW(tsampleinfo, sf->nrinfos);
		sf->samplenames = NEW(tsamplenames, sf->nrsamples);
	}
	else  {
		sf->nrinfos = size / 16;
		sf->sampleinfo = NEW(tsampleinfo, sf->nrinfos);
	}

	for (i = 0; i < sf->nrinfos; i++) {
		if (sf->version > 1)
			READSTR(sf->samplenames[i].name, rw);
		READDW(&sf->sampleinfo[i].startsample, rw);
		READDW(&sf->sampleinfo[i].endsample, rw);
		READDW(&sf->sampleinfo[i].startloop, rw);
		READDW(&sf->sampleinfo[i].endloop, rw);
		if (sf->version > 1) {
			READDW(&sf->sampleinfo[i].samplerate, rw);
			READB(&sf->sampleinfo[i].originalPitch, rw);
			READB(&sf->sampleinfo[i].pitchCorrection, rw);
			READW(&sf->sampleinfo[i].samplelink, rw);
			READW(&sf->sampleinfo[i].sampletype, rw);
		} else {
			if (sf->sampleinfo[i].startsample == 0)
				sf->in_rom = 0;
			sf->sampleinfo[i].startloop++;
			sf->sampleinfo[i].endloop += 2;
			sf->sampleinfo[i].samplerate = 44100;
			sf->sampleinfo[i].originalPitch = 60;
			sf->sampleinfo[i].pitchCorrection = 0;
			sf->sampleinfo[i].samplelink = 0;
			if (sf->in_rom)
				sf->sampleinfo[i].sampletype = 0x8001;
			else
				sf->sampleinfo[i].sampletype = 1;
		}
	}
}

static void process_chunk(int id, int s, SFInfo *sf, SDL_RWops *rw)
{
	const Sint64 rwend = SDL_RWsize(rw);
	int cid;
	tchunk subchunk;

	(void) s;

	switch (id) {
	case INFO_ID:
		READCHUNK(&subchunk, rw);
		while ((cid = getchunk(subchunk.id)) != LIST_ID) {
			switch (cid) {
			case IFIL_ID:
				READW(&sf->version, rw);
				READW(&sf->minorversion, rw);
				break;
			/*
			case INAM_ID:
				sf->sf_name = (char *)SDL_malloc(subchunk.size + 1);
				SDL_RWread(rw, sf->sf_name, 1, subchunk.size);
				sf->sf_name[subchunk.size] = 0;
				break;
			*/
			default:
				SDL_RWseek(rw, subchunk.size, RW_SEEK_CUR);
				break;
			}
			READCHUNK(&subchunk, rw);
			if (SDL_RWtell(rw) >= rwend)
				return;
		}
		SDL_RWseek(rw, -8, RW_SEEK_CUR); /* seek back */
		break;

	case SDTA_ID:
		READCHUNK(&subchunk, rw);
		while ((cid = getchunk(subchunk.id)) != LIST_ID) {
			switch (cid) {
			case SNAM_ID:
				if (sf->version > 1) {
					SNDDBG(("**** version 2 has obsolete format??\n"));
					SDL_RWseek(rw, subchunk.size, RW_SEEK_CUR);
				} else
					load_sample_names(subchunk.size, sf, rw);
				break;
			case SMPL_ID:
				sf->samplepos = SDL_RWtell(rw);
				sf->samplesize = subchunk.size;
				SDL_RWseek(rw, subchunk.size, RW_SEEK_CUR);
			}
			READCHUNK(&subchunk, rw);
			if (SDL_RWtell(rw) >= rwend)
				return;
		}
		SDL_RWseek(rw, -8, RW_SEEK_CUR); /* seek back */
		break;

	case PDTA_ID:
		READCHUNK(&subchunk, rw);
		while ((cid = getchunk(subchunk.id)) != LIST_ID) {
			switch (cid) {
			case PHDR_ID:
				load_preset_header(subchunk.size, sf, rw);
				break;

			case PBAG_ID:
				load_bag(subchunk.size, sf, rw,
					 &sf->nrpbags, &sf->presetbag);
				break;

			case PMOD_ID: /* ignored */
				SDL_RWseek(rw, subchunk.size, RW_SEEK_CUR);
				break;

			case PGEN_ID:
				load_gen(subchunk.size, sf, rw,
					 &sf->nrpgens, &sf->presetgen);
				break;

			case INST_ID:
				load_inst_header(subchunk.size, sf, rw);
				break;

			case IBAG_ID:
				load_bag(subchunk.size, sf, rw,
					 &sf->nribags, &sf->instbag);
				break;

			case IMOD_ID: /* ingored */
				SDL_RWseek(rw, subchunk.size, RW_SEEK_CUR);
				break;

			case IGEN_ID:
				load_gen(subchunk.size, sf, rw,
					 &sf->nrigens, &sf->instgen);
				break;

			case SHDR_ID:
				load_sample_info(subchunk.size, sf, rw);
				break;

			default:
				SNDDBG(("unknown id\n"));
				SDL_RWseek(rw, subchunk.size, RW_SEEK_CUR);
				break;
			}
			READCHUNK(&subchunk, rw);
			if (SDL_RWtell(rw) >= rwend) {
				debugid("file", "EOF");
				return;
			}
		}
		SDL_RWseek(rw, -8, RW_SEEK_CUR); /* rewind */
		break;
	}
}

