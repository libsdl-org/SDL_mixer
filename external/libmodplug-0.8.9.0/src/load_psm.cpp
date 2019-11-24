/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/


///////////////////////////////////////////////////
//
// PSM module loader
//
///////////////////////////////////////////////////
#include "stdafx.h"
#include "sndfile.h"

//#define PSM_LOG

#define PSM_ID_NEW	0x204d5350
#define PSM_ID_OLD	0xfe4d5350
#define IFFID_FILE	0x454c4946
#define IFFID_TITL	0x4c544954
#define IFFID_SDFT	0x54464453
#define IFFID_PBOD	0x444f4250
#define IFFID_SONG	0x474e4f53
#define IFFID_PATT	0x54544150
#define IFFID_DSMP	0x504d5344
#define IFFID_OPLH	0x484c504f

#pragma pack(1)

typedef struct _PSMCHUNK
{
	DWORD id;
	DWORD len;
	DWORD listid;
} PSMCHUNK;

void swap_PSMCHUNK(PSMCHUNK* p){
	p->id = bswapLE32(p->id);
	p->len = bswapLE32(p->len);
	p->listid = bswapLE32(p->listid);
}

typedef struct _PSMSONGHDR
{
	CHAR songname[8];	// "MAINSONG"
	BYTE reserved1;
	BYTE reserved2;
	BYTE channels;
} PSMSONGHDR;

typedef struct _PSMPATTERN
{
	DWORD size;
	DWORD name;
	WORD rows;
	WORD reserved1;
	BYTE data[4];
} PSMPATTERN;

void swap_PSMPATTERN(PSMPATTERN* p){
	p->size = bswapLE32(p->size);
	p->name = bswapLE32(p->name);
	p->rows = bswapLE16(p->rows);
}

typedef struct _PSMSAMPLE
{
	BYTE flags;
	CHAR songname[8];
	DWORD smpid;
	CHAR samplename[34];
	DWORD reserved1;
	BYTE reserved2;
	BYTE insno;
	BYTE reserved3;
	DWORD length;
	DWORD loopstart;
	DWORD loopend;
	WORD reserved4;
	BYTE defvol;
	DWORD reserved5;
	DWORD samplerate;
	BYTE reserved6[19];
} PSMSAMPLE;

#pragma pack()

static void swap_PSMSAMPLE(PSMSAMPLE* p){
	p->smpid = bswapLE32(p->smpid);
	p->length = bswapLE32(p->length);
	p->loopstart = bswapLE32(p->loopstart);
	p->loopend = bswapLE32(p->loopend);
	p->samplerate = bswapLE32(p->samplerate);
}


BOOL CSoundFile::ReadPSM(LPCBYTE lpStream, DWORD dwMemLength)
//-----------------------------------------------------------
{
	PSMCHUNK pfh = *(const PSMCHUNK *)lpStream;
	DWORD dwMemPos, dwSongPos;
	DWORD smpnames[MAX_SAMPLES];
	DWORD patptrs[MAX_PATTERNS];
	BYTE samplemap[MAX_SAMPLES];
	UINT nPatterns;

	if (dwMemLength < 256) return FALSE;

	// Swap chunk
	swap_PSMCHUNK(&pfh);

	// Chunk0: "PSM ",filesize,"FILE"
	if (pfh.id == PSM_ID_OLD)
	{
	#ifdef PSM_LOG
		Log("Old PSM format not supported\n");
	#endif
		return FALSE;
	}
	if ((pfh.id != PSM_ID_NEW) || (pfh.len+12 > dwMemLength) || (pfh.listid != IFFID_FILE)) return FALSE;
	m_nType = MOD_TYPE_PSM;
	m_nChannels = 16;
	m_nSamples = 0;
	nPatterns = 0;
	dwMemPos = 12;
	dwSongPos = 0;
	for (UINT iChPan=0; iChPan<16; iChPan++)
	{
		UINT pan = (((iChPan & 3) == 1) || ((iChPan&3)==2)) ? 0xC0 : 0x40;
		ChnSettings[iChPan].nPan = pan;
	}
	while (dwMemPos+8 < dwMemLength)
	{
		PSMCHUNK pchunk = *(const PSMCHUNK *)(lpStream+dwMemPos);
		swap_PSMCHUNK(&pchunk);
		if ((pchunk.len >= dwMemLength - 8) || (dwMemPos + pchunk.len + 8 > dwMemLength)) break;
		dwMemPos += 8;
		PUCHAR pdata = (PUCHAR)(lpStream+dwMemPos);
		ULONG len = pchunk.len;
		if (len) switch(pchunk.id)
		{
		// "TITL": Song title
		case IFFID_TITL:
			if (!pdata[0]) { pdata++; len--; }
			memcpy(m_szNames[0], pdata, (len>31) ? 31 : len);
			m_szNames[0][31] = 0;
			break;
		// "PBOD": Pattern
		case IFFID_PBOD:
			if ((len >= 12) && (nPatterns < MAX_PATTERNS))
			{
				patptrs[nPatterns++] = dwMemPos-8;
			}
			break;
		// "SONG": Song description
		case IFFID_SONG:
			if ((len >= sizeof(PSMSONGHDR)+8) && (!dwSongPos))
			{
				dwSongPos = dwMemPos - 8;
			}
			break;
		// "DSMP": Sample Data
		case IFFID_DSMP:
			if ((len >= sizeof(PSMSAMPLE)) && (m_nSamples+1 < MAX_SAMPLES))
			{
				m_nSamples++;
				MODINSTRUMENT *pins = &Ins[m_nSamples];
				PSMSAMPLE psmp = *(PSMSAMPLE *)pdata;
				swap_PSMSAMPLE(&psmp);
				smpnames[m_nSamples] = psmp.smpid;
				memcpy(m_szNames[m_nSamples], psmp.samplename, 31);
				m_szNames[m_nSamples][31] = 0;
				samplemap[m_nSamples-1] = (BYTE)m_nSamples;
				// Init sample
				pins->nGlobalVol = 0x40;
				pins->nC4Speed = psmp.samplerate;
				pins->nLength = psmp.length;
				pins->nLoopStart = psmp.loopstart;
				pins->nLoopEnd = psmp.loopend;
				pins->nPan = 128;
				pins->nVolume = (psmp.defvol+1) * 2;
				pins->uFlags = (psmp.flags & 0x80) ? CHN_LOOP : 0;
				if (pins->nLoopStart > 0) pins->nLoopStart--;
				// Point to sample data
				pdata += 0x60;
				len -= 0x60;
				// Load sample data
				if ((pins->nLength > 3) && (len > 3))
				{
					ReadSample(pins, RS_PCM8D, (LPCSTR)pdata, len);
				} else
				{
					pins->nLength = 0;
				}
			}
			break;
	#if 0
		default:
			{
				CHAR s[8], s2[64];
				*(DWORD *)s = pchunk.id;
				s[4] = 0;
				wsprintf(s2, "%s: %4d bytes @ %4d\n", s, pchunk.len, dwMemPos);
				OutputDebugString(s2);
			}
	#endif
		}
		dwMemPos += pchunk.len;
	}
	// Step #1: convert song structure
	const PSMSONGHDR *pSong = (const PSMSONGHDR *)(lpStream+dwSongPos+8);
	if ((!dwSongPos) || (pSong->channels < 2) || (pSong->channels > 32)) return TRUE;
	m_nChannels = pSong->channels;
	// Valid song header -> convert attached chunks
	{
		DWORD dwSongEnd = dwSongPos + 8 + *(DWORD *)(lpStream+dwSongPos+4);
		dwMemPos = dwSongPos + 8 + 11; // sizeof(PSMCHUNK)+sizeof(PSMSONGHDR)
		while (dwMemPos + 8 < dwSongEnd)
		{
			PSMCHUNK pchunk = *(const PSMCHUNK *)(lpStream+dwMemPos);
			swap_PSMCHUNK(&pchunk);
			dwMemPos += 8;
			if ((pchunk.len > dwSongEnd) || (dwMemPos + pchunk.len > dwSongEnd)) break;
			PUCHAR pdata = (PUCHAR)(lpStream+dwMemPos);
			ULONG len = pchunk.len;
			switch(pchunk.id)
			{
			case IFFID_OPLH:
				if (len >= 0x20)
				{
					UINT pos = len - 3;
					while (pos > 5)
					{
						BOOL bFound = FALSE;
						pos -= 5;
						DWORD dwName = *(DWORD *)(pdata+pos);
						for (UINT i=0; i<nPatterns; i++)
						{
							DWORD dwPatName = ((const PSMPATTERN *)(lpStream+patptrs[i]+8))->name;
							if (dwName == dwPatName)
							{
								bFound = TRUE;
								break;
							}
						}
						if ((!bFound) && (pdata[pos+1] > 0) && (pdata[pos+1] <= 0x10)
						 && (pdata[pos+3] > 0x40) && (pdata[pos+3] < 0xC0))
						{
							m_nDefaultSpeed = pdata[pos+1];
							m_nDefaultTempo = pdata[pos+3];
							break;
						}
					}
					UINT iOrd = 0;
					while ((pos+5<len) && (iOrd < MAX_ORDERS))
					{
						DWORD dwName = *(DWORD *)(pdata+pos);
						for (UINT i=0; i<nPatterns; i++)
						{
							DWORD dwPatName = ((const PSMPATTERN *)(lpStream+patptrs[i]+8))->name;
							if (dwName == dwPatName)
							{
								Order[iOrd++] = i;
								break;
							}
						}
						pos += 5;
					}
				}
				break;
			}
			dwMemPos += pchunk.len;
		}
	}

	// Step #2: convert patterns
	for (UINT nPat=0; nPat<nPatterns; nPat++)
	{
		PSMPATTERN pPsmPat = *(const PSMPATTERN *)(lpStream+patptrs[nPat]+8);
		swap_PSMPATTERN(&pPsmPat);
		ULONG len = *(DWORD *)(lpStream+patptrs[nPat]+4) - 12;
		UINT nRows = pPsmPat.rows;
		if (len > pPsmPat.size) len = pPsmPat.size;
		if ((nRows < 64) || (nRows > 256)) nRows = 64;
		PatternSize[nPat] = nRows;
		if ((Patterns[nPat] = AllocatePattern(nRows, m_nChannels)) == NULL) break;
		MODCOMMAND *m = Patterns[nPat];
		BYTE *p = pPsmPat.data;
		MODCOMMAND *sp, dummy;
		UINT pos = 0;
		UINT row = 0;
		UINT rowlim;
	#ifdef PSM_LOG
		Log("Pattern %d at offset 0x%04X\n", nPat, (DWORD)(p - (BYTE *)lpStream));
	#endif
		UINT flags, ch;
		rowlim = bswapLE16(pPsmPat.reserved1)-2;
		while ((row < nRows) && (pos+3 < len))
		{
			if ((pos+1) >= rowlim) {
				pos = rowlim;
				rowlim = (((int)p[pos+1])<<8)
					| ((int)p[pos+0]);
				m += m_nChannels;
				row++;
				rowlim += pos;
				pos += 2;
			}
			if (row >= nRows) continue;

			flags = p[pos++];
			ch = p[pos++];
			if (ch >= m_nChannels) {
				sp = &dummy;
			} else {
				sp = &m[ch];
			}
			// Note + Instr
			if ((flags & 0x80) && (pos+1 < len))
			{
				UINT note = p[pos++];
				note = (note>>4)*12+(note&0x0f)+12+1;
				if (note > 0x80) note = 0;
				sp->note = note;
			}
			if ((flags & 0x40) && (pos+1 < len))
			{
				UINT nins = p[pos++];
			#ifdef PSM_LOG
				//if (!nPat) Log("note+ins: %02X.%02X\n", note, nins);
				if ((!nPat) && (nins >= m_nSamples)) Log("WARNING: invalid instrument number (%d)\n", nins);
			#endif
				sp->instr = samplemap[nins];
			}
			// Volume
			if ((flags & 0x20) && (pos < len))
			{
				sp->volcmd = VOLCMD_VOLUME;
				sp->vol = p[pos++] / 2;
			}
			// Effect
			if ((flags & 0x10) && (pos+1 < len))
			{
				UINT command = p[pos++];
				UINT param = p[pos++];
				// Convert effects
				switch(command)
				{
				// 01: fine volslide up
				case 0x01:	command = CMD_VOLUMESLIDE; param |= 0x0f;
						if (param == 15) param=31;
						break;
				// 02: volslide up
				case 0x02:	command = CMD_VOLUMESLIDE; param>>=1; param<<=4; break;
				// 03: fine volslide down
				case 0x03:	command = CMD_VOLUMESLIDE; param>>=4; param |= 0xf0;
						if (param == 240) param=241;
						break;
				// 04: fine volslide down
				case 0x04:	command = CMD_VOLUMESLIDE; param>>=4; param |= 0xf0; break;
				// 0C: portamento up
				case 0x0C:	command = CMD_PORTAMENTOUP; param = (param+1)/2; break;
				// 0E: portamento down
				case 0x0E:	command = CMD_PORTAMENTODOWN; param = (param+1)/2; break;
				// 0F: tone portamento
				case 0x0F:	command = CMD_TONEPORTAMENTO; param = param/4; break;
				// 15: vibrato
				case 0x15:	command = CMD_VIBRATO; break;
				// 29: sample offset
				case 0x29:	pos += 2; break;
				// 2A: retrigger note
				case 0x2A:	command = CMD_RETRIG; break;
				// 33: Position Jump
				case 0x33:	command = CMD_POSITIONJUMP; break;
				// 34: Pattern break
				case 0x34:	command = CMD_PATTERNBREAK; break;
				// 3D: speed
				case 0x3D:	command = CMD_SPEED; break;
				// 3E: tempo
				case 0x3E:	command = CMD_TEMPO; break;
				// Unknown
				default:
				#ifdef PSM_LOG
					Log("Unknown PSM effect pat=%d row=%d ch=%d: %02X.%02X\n", nPat, row, ch, command, param);
				#endif
					command = param = 0;
				}
				sp->command = (BYTE)command;
				sp->param = (BYTE)param;
			}
		}
	#ifdef PSM_LOG
		if (pos < len)
		{
			Log("Pattern %d: %d/%d[%d] rows (%d bytes) -> %d bytes left\n", nPat, row, nRows, pPsmPat.rows, pPsmPat.size, len-pos);
		}
	#endif
	}

	// Done (finally!)
	return TRUE;
}
