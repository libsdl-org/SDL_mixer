/*

    TiMidity -- Experimental MIDI to WAVE converter
    Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

    readsbk.c: read soundfont file
    Copyright (C) 1996,1997 Takashi Iwai

    This program is free software; you can redistribute it and/or modify
    it under the terms of the Perl Artistic License, available in COPYING.
*/

#include <SDL3/SDL.h>

#include "timidity.h"
#include "options.h"
#include "common.h"
#include "sbk.h"

/*----------------------------------------------------------------
 * function prototypes
 *----------------------------------------------------------------*/

#define NEW(type,nums)	(type*)SDL_calloc((nums), sizeof(type))

static int READCHUNK(tchunk *vp, SDL_IOStream *io)
{
	if (SDL_ReadIO(io, vp, 8) != 8) return -1;
	vp->size = SDL_Swap32LE(vp->size);
	return 1;
}

static int READDW(Sint32 *vp, SDL_IOStream *io)
{
	if (SDL_ReadIO(io, vp, 4) != 4) return -1;
	*vp = SDL_Swap32LE(*vp);
	return 1;
}

static int READW(Uint16 *vp, SDL_IOStream *io)
{
	if (SDL_ReadIO(io, vp, 2) != 2) return -1;
	*vp = SDL_Swap16LE(*vp);
	return 1;
}

static int READSTR(char *str, SDL_IOStream *io)
{
	int n;
	if (SDL_ReadIO(io, str, 20) != 20) return -1;
	str[19] = '\0';
	n = (int) SDL_strlen(str);
	while (n > 0 && str[n - 1] == ' ')
		n--;
	str[n] = '\0';
	return n;
}

#define READID(var,io)	SDL_ReadIO(io, var, 4)
#define READB(var,io)	SDL_ReadIO(io, var, 1)
#define SKIPB(io)	SDL_SeekIO(io, 1, SDL_IO_SEEK_CUR);
#define SKIPW(io)	SDL_SeekIO(io, 2, SDL_IO_SEEK_CUR);
#define SKIPDW(io)	SDL_SeekIO(io, 4, SDL_IO_SEEK_CUR);

static int getchunk(const char *id);
static void process_chunk(int id, int s, SFInfo *sf, SDL_IOStream *io);
static void load_sample_names(int size, SFInfo *sf, SDL_IOStream *io);
static void load_preset_header(int size, SFInfo *sf, SDL_IOStream *io);
static void load_inst_header(int size, SFInfo *sf, SDL_IOStream *io);
static void load_bag(int size, SFInfo *sf, SDL_IOStream *io, int *totalp, Uint16 **bufp);
static void load_gen(int size, SFInfo *sf, SDL_IOStream *io, int *totalp, tgenrec **bufp);
static void load_sample_info(int size, SFInfo *sf, SDL_IOStream *io);


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

int load_sbk(SDL_IOStream *io, SFInfo *sf)
{
	tchunk chunk, subchunk;
	Sint64 len;

	len = SDL_GetIOSize(io);
	if (len < 32) /* better?? */
		return -1;

	READCHUNK(&chunk, io);
	if (getchunk(chunk.id) != RIFF_ID) return -1;
	if (chunk.size != len - 8) return -1;

	READID(chunk.id, io);
	if (getchunk(chunk.id) != SFBK_ID) return -1;

	sf->in_rom = 1;
	while (SDL_GetIOStatus(io) != SDL_IO_STATUS_EOF) {
		READID(chunk.id, io);
		switch (getchunk(chunk.id)) {
		case LIST_ID:
			READDW(&chunk.size, io);
			READID(subchunk.id, io);
			process_chunk(getchunk(subchunk.id), chunk.size - 4, sf, io);
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


static void load_sample_names(int size, SFInfo *sf, SDL_IOStream *io)
{
	int i;
	sf->nrsamples = size / 20;
	sf->samplenames = NEW(tsamplenames, sf->nrsamples);
	for (i = 0; i < sf->nrsamples; i++) {
		READSTR(sf->samplenames[i].name, io);
	}
}

static void load_preset_header(int size, SFInfo *sf, SDL_IOStream *io)
{
	int i;
	sf->nrpresets = size / 38;
	sf->presethdr = NEW(tpresethdr, sf->nrpresets);
	for (i = 0; i < sf->nrpresets; i++) {
		READSTR(sf->presethdr[i].name, io);
		READW(&sf->presethdr[i].preset, io);
		READW(&sf->presethdr[i].bank, io);
		READW(&sf->presethdr[i].bagNdx, io);
		SKIPDW(io); /* lib */
		SKIPDW(io); /* genre */
		SKIPDW(io); /* morph */
	}
}

static void load_inst_header(int size, SFInfo *sf, SDL_IOStream *io)
{
	int i;

	sf->nrinsts = size / 22;
	sf->insthdr = NEW(tinsthdr, sf->nrinsts);
	for (i = 0; i < sf->nrinsts; i++) {
		READSTR(sf->insthdr[i].name, io);
		READW(&sf->insthdr[i].bagNdx, io);
	}
}

static void load_bag(int size, SFInfo *sf, SDL_IOStream *io, int *totalp, Uint16 **bufp)
{
	Uint16 *buf;
	int i;

	(void) sf;
	debugval("bagsize", size);
	size /= 4;
	buf = NEW(Uint16, size);
	for (i = 0; i < size; i++) {
		READW(&buf[i], io);
		SKIPW(io); /* mod */
	}
	*totalp = size;
	*bufp = buf;
}

static void load_gen(int size, SFInfo *sf, SDL_IOStream *io, int *totalp, tgenrec **bufp)
{
	tgenrec *buf;
	int i;

	(void) sf;
	debugval("gensize", size);
	size /= 4;
	buf = NEW(tgenrec, size);
	for (i = 0; i < size; i++) {
		READW((Uint16 *)&buf[i].oper, io);
		READW((Uint16 *)&buf[i].amount, io);
	}
	*totalp = size;
	*bufp = buf;
}

static void load_sample_info(int size, SFInfo *sf, SDL_IOStream *io)
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
			READSTR(sf->samplenames[i].name, io);
		READDW(&sf->sampleinfo[i].startsample, io);
		READDW(&sf->sampleinfo[i].endsample, io);
		READDW(&sf->sampleinfo[i].startloop, io);
		READDW(&sf->sampleinfo[i].endloop, io);
		if (sf->version > 1) {
			READDW(&sf->sampleinfo[i].samplerate, io);
			READB(&sf->sampleinfo[i].originalPitch, io);
			READB(&sf->sampleinfo[i].pitchCorrection, io);
			READW(&sf->sampleinfo[i].samplelink, io);
			READW(&sf->sampleinfo[i].sampletype, io);
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

static void process_chunk(int id, int s, SFInfo *sf, SDL_IOStream *io)
{
	int cid;
	tchunk subchunk;

	(void) s;

	switch (id) {
	case INFO_ID:
		READCHUNK(&subchunk, io);
		while ((cid = getchunk(subchunk.id)) != LIST_ID) {
			switch (cid) {
			case IFIL_ID:
				READW(&sf->version, io);
				READW(&sf->minorversion, io);
				break;
			/*
			case INAM_ID:
				sf->sf_name = (char *)SDL_malloc(subchunk.size + 1);
				SDL_ReadIO(io, sf->sf_name, subchunk.size);
				sf->sf_name[subchunk.size] = 0;
				break;
			*/
			default:
				SDL_SeekIO(io, subchunk.size, SDL_IO_SEEK_CUR);
				break;
			}
			READCHUNK(&subchunk, io);
			if (SDL_GetIOStatus(io) == SDL_IO_STATUS_EOF)
				return;
		}
		SDL_SeekIO(io, -8, SDL_IO_SEEK_CUR); /* seek back */
		break;

	case SDTA_ID:
		READCHUNK(&subchunk, io);
		while ((cid = getchunk(subchunk.id)) != LIST_ID) {
			switch (cid) {
			case SNAM_ID:
				if (sf->version > 1) {
					SNDDBG(("**** version 2 has obsolete format??\n"));
					SDL_SeekIO(io, subchunk.size, SDL_IO_SEEK_CUR);
				} else
					load_sample_names(subchunk.size, sf, io);
				break;
			case SMPL_ID:
				sf->samplepos = SDL_TellIO(io);
				sf->samplesize = subchunk.size;
				SDL_SeekIO(io, subchunk.size, SDL_IO_SEEK_CUR);
			}
			READCHUNK(&subchunk, io);
			if (SDL_GetIOStatus(io) == SDL_IO_STATUS_EOF)
				return;
		}
		SDL_SeekIO(io, -8, SDL_IO_SEEK_CUR); /* seek back */
		break;

	case PDTA_ID:
		READCHUNK(&subchunk, io);
		while ((cid = getchunk(subchunk.id)) != LIST_ID) {
			switch (cid) {
			case PHDR_ID:
				load_preset_header(subchunk.size, sf, io);
				break;

			case PBAG_ID:
				load_bag(subchunk.size, sf, io,
					 &sf->nrpbags, &sf->presetbag);
				break;

			case PMOD_ID: /* ignored */
				SDL_SeekIO(io, subchunk.size, SDL_IO_SEEK_CUR);
				break;

			case PGEN_ID:
				load_gen(subchunk.size, sf, io,
					 &sf->nrpgens, &sf->presetgen);
				break;

			case INST_ID:
				load_inst_header(subchunk.size, sf, io);
				break;

			case IBAG_ID:
				load_bag(subchunk.size, sf, io,
					 &sf->nribags, &sf->instbag);
				break;

			case IMOD_ID: /* ingored */
				SDL_SeekIO(io, subchunk.size, SDL_IO_SEEK_CUR);
				break;

			case IGEN_ID:
				load_gen(subchunk.size, sf, io,
					 &sf->nrigens, &sf->instgen);
				break;

			case SHDR_ID:
				load_sample_info(subchunk.size, sf, io);
				break;

			default:
				SNDDBG(("unknown id\n"));
				SDL_SeekIO(io, subchunk.size, SDL_IO_SEEK_CUR);
				break;
			}
			READCHUNK(&subchunk, io);
			if (SDL_GetIOStatus(io) == SDL_IO_STATUS_EOF) {
				debugid("file", "EOF");
				return;
			}
		}
		SDL_SeekIO(io, -8, SDL_IO_SEEK_CUR); /* rewind */
		break;
	}
}

