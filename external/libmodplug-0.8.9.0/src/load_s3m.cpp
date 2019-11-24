/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (endian and char fixes for PPC)
*/

#include "stdafx.h"
#include "sndfile.h"
#include "tables.h"

#ifdef _MSC_VER
//#pragma warning(disable:4244)
#endif

//////////////////////////////////////////////////////
// ScreamTracker S3M file support

#pragma pack(1)
typedef struct tagS3MSAMPLESTRUCT
{
	BYTE type;
	CHAR dosname[12];
	BYTE hmem;
	WORD memseg;
	DWORD length;
	DWORD loopbegin;
	DWORD loopend;
	BYTE vol;
	BYTE bReserved;
	BYTE pack;
	BYTE flags;
	DWORD finetune;
	DWORD dwReserved;
	WORD intgp;
	WORD int512;
	DWORD lastused;
	CHAR name[28];
	CHAR scrs[4];
} S3MSAMPLESTRUCT;

typedef struct tagS3MFILEHEADER
{
	CHAR name[28];
	BYTE b1A;
	BYTE type;
	WORD reserved1;
	WORD ordnum;
	WORD insnum;
	WORD patnum;
	WORD flags;
	WORD cwtv;
	WORD version;
	DWORD scrm;	// "SCRM" = 0x4D524353
	BYTE globalvol;
	BYTE speed;
	BYTE tempo;
	BYTE mastervol;
	BYTE ultraclicks;
	BYTE panning_present;
	BYTE reserved2[8];
	WORD special;
	BYTE channels[32];
} S3MFILEHEADER;
#pragma pack()


void CSoundFile::S3MConvert(MODCOMMAND *m, BOOL bIT) const
//--------------------------------------------------------
{
	UINT command = m->command;
	UINT param = m->param;
	switch (command + 0x40)
	{
	case 'A':	command = CMD_SPEED; break;
	case 'B':	command = CMD_POSITIONJUMP; break;
	case 'C':	command = CMD_PATTERNBREAK; if (!bIT) param = (param >> 4) * 10 + (param & 0x0F); break;
	case 'D':	command = CMD_VOLUMESLIDE; break;
	case 'E':	command = CMD_PORTAMENTODOWN; break;
	case 'F':	command = CMD_PORTAMENTOUP; break;
	case 'G':	command = CMD_TONEPORTAMENTO; break;
	case 'H':	command = CMD_VIBRATO; break;
	case 'I':	command = CMD_TREMOR; break;
	case 'J':	command = CMD_ARPEGGIO; break;
	case 'K':	command = CMD_VIBRATOVOL; break;
	case 'L':	command = CMD_TONEPORTAVOL; break;
	case 'M':	command = CMD_CHANNELVOLUME; break;
	case 'N':	command = CMD_CHANNELVOLSLIDE; break;
	case 'O':	command = CMD_OFFSET; break;
	case 'P':	command = CMD_PANNINGSLIDE; break;
	case 'Q':	command = CMD_RETRIG; break;
	case 'R':	command = CMD_TREMOLO; break;
	case 'S':	command = CMD_S3MCMDEX; break;
	case 'T':	command = CMD_TEMPO; break;
	case 'U':	command = CMD_FINEVIBRATO; break;
	case 'V':	command = CMD_GLOBALVOLUME; break;
	case 'W':	command = CMD_GLOBALVOLSLIDE; break;
	case 'X':	command = CMD_PANNING8; break;
	case 'Y':	command = CMD_PANBRELLO; break;
	case 'Z':	command = CMD_MIDI; break;
	default:	command = 0;
	}
	m->command = command;
	m->param = param;
}

static DWORD boundInput(DWORD input, DWORD smin, DWORD smax)
{
	if (input > smax) input = smax;
	else if (input < smin) input = 0;
	return(input);
}

BOOL CSoundFile::ReadS3M(const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	UINT insnum,patnum,nins,npat;
	DWORD insfile[MAX_SAMPLES];
	WORD ptr[256];
	DWORD dwMemPos;
	BYTE insflags[MAX_SAMPLES], inspack[MAX_SAMPLES];

	if ((!lpStream) || (dwMemLength <= sizeof(S3MFILEHEADER)+sizeof(S3MSAMPLESTRUCT)+64)) return FALSE;
	S3MFILEHEADER psfh = *(S3MFILEHEADER *)lpStream;

	psfh.reserved1 = bswapLE16(psfh.reserved1);
	psfh.ordnum = bswapLE16(psfh.ordnum);
	psfh.insnum = bswapLE16(psfh.insnum);
	psfh.patnum = bswapLE16(psfh.patnum);
	psfh.flags = bswapLE16(psfh.flags);
	psfh.cwtv = bswapLE16(psfh.cwtv);
	psfh.version = bswapLE16(psfh.version);
	psfh.scrm = bswapLE32(psfh.scrm);
	psfh.special = bswapLE16(psfh.special);

	if (psfh.scrm != 0x4D524353) return FALSE;
	dwMemPos = 0x60;
	m_nType = MOD_TYPE_S3M;
	memset(m_szNames,0,sizeof(m_szNames));
	memcpy(m_szNames[0], psfh.name, 28);
	// Speed
	m_nDefaultSpeed = psfh.speed;
	if (m_nDefaultSpeed < 1) m_nDefaultSpeed = 6;
	if (m_nDefaultSpeed > 0x1F) m_nDefaultSpeed = 0x1F;
	// Tempo
	m_nDefaultTempo = psfh.tempo;
	if (m_nDefaultTempo < 40) m_nDefaultTempo = 40;
	if (m_nDefaultTempo > 240) m_nDefaultTempo = 240;
	// Global Volume
	m_nDefaultGlobalVolume = psfh.globalvol << 2;
	if ((!m_nDefaultGlobalVolume) || (m_nDefaultGlobalVolume > 256)) m_nDefaultGlobalVolume = 256;
	m_nSongPreAmp = psfh.mastervol & 0x7F;
	// Channels
	m_nChannels = 4;
	for (UINT ich=0; ich<32; ich++)
	{
		ChnSettings[ich].nPan = 128;
		ChnSettings[ich].nVolume = 64;

		ChnSettings[ich].dwFlags = CHN_MUTE;
		if (psfh.channels[ich] != 0xFF)
		{
			m_nChannels = ich+1;
			UINT b = psfh.channels[ich] & 0x0F;
			ChnSettings[ich].nPan = (b & 8) ? 0xC0 : 0x40;
			ChnSettings[ich].dwFlags = 0;
		}
	}
	if (m_nChannels < 4) m_nChannels = 4;
	if ((psfh.cwtv < 0x1320) || (psfh.flags & 0x40)) m_dwSongFlags |= SONG_FASTVOLSLIDES;
	// Reading pattern order
	UINT iord = psfh.ordnum;
	if (iord<1) iord = 1;
	if (iord > MAX_ORDERS) iord = MAX_ORDERS;
	if (iord)
	{
		memcpy(Order, lpStream+dwMemPos, iord);
		dwMemPos += iord;
	}
	if ((iord & 1) && (lpStream[dwMemPos] == 0xFF)) dwMemPos++;
	// Reading file pointers
	insnum = nins = psfh.insnum;
	if (insnum >= MAX_SAMPLES) insnum = MAX_SAMPLES-1;
	m_nSamples = insnum;
	patnum = npat = psfh.patnum;
	if (patnum > MAX_PATTERNS) patnum = MAX_PATTERNS;
	memset(ptr, 0, sizeof(ptr));

	// Ignore file if it has a corrupted header.
	if (nins+npat > 256) return FALSE;

	if (nins+npat)
	{
		memcpy(ptr, lpStream+dwMemPos, 2*(nins+npat));
		dwMemPos += 2*(nins+npat);
		for (UINT j = 0; j < (nins+npat); ++j) {
		        ptr[j] = bswapLE16(ptr[j]);
		}
		if (psfh.panning_present == 252)
		{
			const BYTE *chnpan = lpStream+dwMemPos;
			for (UINT i=0; i<32; i++) if (chnpan[i] & 0x20)
			{
				ChnSettings[i].nPan = ((chnpan[i] & 0x0F) << 4) + 8;
			}
		}
	}
	if (!m_nChannels) return TRUE;
	// Reading instrument headers
	memset(insfile, 0, sizeof(insfile));
	for (UINT iSmp=1; iSmp<=insnum; iSmp++)
	{
		UINT nInd = ((DWORD)ptr[iSmp-1])*16;
		if ((!nInd) || (nInd + 0x50 > dwMemLength)) {
			// initialize basic variables.
			insflags[iSmp-1] = 0;
			inspack[iSmp-1] = 0;
			continue;
		}
		S3MSAMPLESTRUCT pSmp;
		memcpy(&pSmp, lpStream+nInd, 0x50);
		memcpy(Ins[iSmp].name, pSmp.dosname, 12);
		insflags[iSmp-1] = pSmp.flags;
		inspack[iSmp-1] = pSmp.pack;
		memcpy(m_szNames[iSmp], pSmp.name, 28);
		m_szNames[iSmp][28] = 0;
		if ((pSmp.type==1) && (pSmp.scrs[2]=='R') && (pSmp.scrs[3]=='S'))
		{
			Ins[iSmp].nLength = boundInput(bswapLE32(pSmp.length), 4, MAX_SAMPLE_LENGTH);
			Ins[iSmp].nLoopStart = boundInput(bswapLE32(pSmp.loopbegin), 4, Ins[iSmp].nLength - 1);
			Ins[iSmp].nLoopEnd = boundInput(bswapLE32(pSmp.loopend), 4, Ins[iSmp].nLength);
			Ins[iSmp].nVolume = boundInput(pSmp.vol, 0, 64) << 2;
			Ins[iSmp].nGlobalVol = 64;
			if (pSmp.flags&1) Ins[iSmp].uFlags |= CHN_LOOP;
			UINT j = bswapLE32(pSmp.finetune);
			if (!j) j = 8363;
			if (j < 1024) j = 1024;
			Ins[iSmp].nC4Speed = j;
			insfile[iSmp] = (pSmp.hmem << 20) + (bswapLE16(pSmp.memseg) << 4);
			// offset is invalid - ignore this sample.
			if (insfile[iSmp] > dwMemLength) insfile[iSmp] = 0;
			else if (insfile[iSmp]) {
				// ignore duplicate samples.
				for (int z=iSmp-1; z>=0; z--)
					if (insfile[iSmp] == insfile[z])
						insfile[iSmp] = 0;
			}
			if ((Ins[iSmp].nLoopStart >= Ins[iSmp].nLoopEnd) || (Ins[iSmp].nLoopEnd - Ins[iSmp].nLoopStart < 8))
				Ins[iSmp].nLoopStart = Ins[iSmp].nLoopEnd = 0;
			Ins[iSmp].nPan = 0x80;
		}
	}
	// Reading patterns
	for (UINT iPat=0; iPat<patnum; iPat++)
	{
		UINT nInd = ((DWORD)ptr[nins+iPat]) << 4;
		if (nInd + 0x40 > dwMemLength) continue;
		WORD len = bswapLE16(*((WORD *)(lpStream+nInd)));
		nInd += 2;
		PatternSize[iPat] = 64;
		if ((!len) || (nInd + len > dwMemLength - 6)
		 || ((Patterns[iPat] = AllocatePattern(64, m_nChannels)) == NULL)) continue;
		LPBYTE src = (LPBYTE)(lpStream+nInd);
		// Unpacking pattern
		MODCOMMAND *p = Patterns[iPat];
		UINT row = 0;
		UINT j = 0;
		while (j < len)
		{
			BYTE b = src[j++];
			if (!b)
			{
				if (++row >= 64) break;
			} else
			{
				UINT chn = b & 0x1F;
				if (chn < m_nChannels)
				{
					MODCOMMAND *m = &p[row*m_nChannels+chn];
					if (b & 0x20)
					{
						m->note = src[j++];
						if (m->note < 0xF0) m->note = (m->note & 0x0F) + 12*(m->note >> 4) + 13;
						else if (m->note == 0xFF) m->note = 0;
						m->instr = src[j++];
					}
					if (b & 0x40)
					{
						UINT vol = src[j++];
						if ((vol >= 128) && (vol <= 192))
						{
							vol -= 128;
							m->volcmd = VOLCMD_PANNING;
						} else
						{
							if (vol > 64) vol = 64;
							m->volcmd = VOLCMD_VOLUME;
						}
						m->vol = vol;
					}
					if (b & 0x80)
					{
						m->command = src[j++];
						m->param = src[j++];
						if (m->command) S3MConvert(m, FALSE);
					}
				} else
				{
					if (b & 0x20) j += 2;
					if (b & 0x40) j++;
					if (b & 0x80) j += 2;
				}
				if (j >= len) break;
			}
		}
	}
	// Reading samples
	for (UINT iRaw=1; iRaw<=insnum; iRaw++) if ((Ins[iRaw].nLength) && (insfile[iRaw]))
	{
		UINT flags = (psfh.version == 1) ? RS_PCM8S : RS_PCM8U;
		if (insflags[iRaw-1] & 4) flags += 5;
		if (insflags[iRaw-1] & 2) flags |= RSF_STEREO;
		if (inspack[iRaw-1] == 4) flags = RS_ADPCM4;
		dwMemPos = insfile[iRaw];
		if (dwMemPos < dwMemLength)
			ReadSample(&Ins[iRaw], flags, (LPSTR)(lpStream + dwMemPos), dwMemLength - dwMemPos);
	}
	m_nMinPeriod = 64;
	m_nMaxPeriod = 32767;
	if (psfh.flags & 0x10) m_dwSongFlags |= SONG_AMIGALIMITS;
	return TRUE;
}
