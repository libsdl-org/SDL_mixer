/*	MikMod sound library
	(c) 1998, 1999 Miodrag Vallat and others - see file AUTHORS for
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

  $Id$

  The Protracker Player Driver

  The protracker driver supports all base Protracker 3.x commands and features.

==============================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdarg.h>
#ifdef SUNOS
extern long int random(void);
#endif
#ifdef SRANDOM_IN_MATH_H
#include <math.h>
#else
#include <stdlib.h>
#endif

#include "mikmod_internals.h"

/* Set forbid to 1 when you want to modify any of the pf->sngpos, pf->patpos etc
   variables and clear it when you're done. This prevents getting strange
   results due to intermediate interrupts. */

		MODULE *pf=NULL; /* modfile being played */
static	SWORD mp_channel; /* channel we're working on */
static	MP_CONTROL *a;    /* current AUDTMP we're working on */
static  int explicitslides;

static	UWORD oldperiods[OCTAVE*2]={
	1712*16,1664*16,1616*16,1570*16,1524*16,1480*16,
	1438*16,1396*16,1356*16,1318*16,1280*16,1244*16,
	1208*16,1174*16,1140*16,1108*16,1076*16,1046*16,
	1016*16, 988*16, 960*16, 932*16, 906*16, 880*16
};

static	UBYTE VibratoTable[32]={
	  0, 24, 49, 74, 97,120,141,161,180,197,212,224,235,244,250,253,
	255,253,250,244,235,224,212,197,180,161,141,120, 97, 74, 49, 24
};

static	UBYTE avibtab[128]={
	 0, 1, 3, 4, 6, 7, 9,10,12,14,15,17,18,20,21,23,
	24,25,27,28,30,31,32,34,35,36,38,39,40,41,42,44,
	45,46,47,48,49,50,51,52,53,54,54,55,56,57,57,58,
	59,59,60,60,61,61,62,62,62,63,63,63,63,63,63,63,
	64,63,63,63,63,63,63,63,62,62,62,61,61,60,60,59,
	59,58,57,57,56,55,54,54,53,52,51,50,49,48,47,46,
	45,44,42,41,40,39,38,36,35,34,32,31,30,28,27,25,
	24,23,21,20,18,17,15,14,12,10, 9, 7, 6, 4, 3, 1
};

/* Triton's linear periods to frequency translation table (for XM modules) */
static	ULONG lintab[768]={
	535232,534749,534266,533784,533303,532822,532341,531861,
	531381,530902,530423,529944,529466,528988,528511,528034,
	527558,527082,526607,526131,525657,525183,524709,524236,
	523763,523290,522818,522346,521875,521404,520934,520464,
	519994,519525,519057,518588,518121,517653,517186,516720,
	516253,515788,515322,514858,514393,513929,513465,513002,
	512539,512077,511615,511154,510692,510232,509771,509312,
	508852,508393,507934,507476,507018,506561,506104,505647,
	505191,504735,504280,503825,503371,502917,502463,502010,
	501557,501104,500652,500201,499749,499298,498848,498398,
	497948,497499,497050,496602,496154,495706,495259,494812,
	494366,493920,493474,493029,492585,492140,491696,491253,
	490809,490367,489924,489482,489041,488600,488159,487718,
	487278,486839,486400,485961,485522,485084,484647,484210,
	483773,483336,482900,482465,482029,481595,481160,480726,
	480292,479859,479426,478994,478562,478130,477699,477268,
	476837,476407,475977,475548,475119,474690,474262,473834,
	473407,472979,472553,472126,471701,471275,470850,470425,
	470001,469577,469153,468730,468307,467884,467462,467041,
	466619,466198,465778,465358,464938,464518,464099,463681,
	463262,462844,462427,462010,461593,461177,460760,460345,
	459930,459515,459100,458686,458272,457859,457446,457033,
	456621,456209,455797,455386,454975,454565,454155,453745,
	453336,452927,452518,452110,451702,451294,450887,450481,
	450074,449668,449262,448857,448452,448048,447644,447240,
	446836,446433,446030,445628,445226,444824,444423,444022,
	443622,443221,442821,442422,442023,441624,441226,440828,
	440430,440033,439636,439239,438843,438447,438051,437656,
	437261,436867,436473,436079,435686,435293,434900,434508,
	434116,433724,433333,432942,432551,432161,431771,431382,
	430992,430604,430215,429827,429439,429052,428665,428278,
	427892,427506,427120,426735,426350,425965,425581,425197,
	424813,424430,424047,423665,423283,422901,422519,422138,
	421757,421377,420997,420617,420237,419858,419479,419101,
	418723,418345,417968,417591,417214,416838,416462,416086,
	415711,415336,414961,414586,414212,413839,413465,413092,
	412720,412347,411975,411604,411232,410862,410491,410121,
	409751,409381,409012,408643,408274,407906,407538,407170,
	406803,406436,406069,405703,405337,404971,404606,404241,
	403876,403512,403148,402784,402421,402058,401695,401333,
	400970,400609,400247,399886,399525,399165,398805,398445,
	398086,397727,397368,397009,396651,396293,395936,395579,
	395222,394865,394509,394153,393798,393442,393087,392733,
	392378,392024,391671,391317,390964,390612,390259,389907,
	389556,389204,388853,388502,388152,387802,387452,387102,
	386753,386404,386056,385707,385359,385012,384664,384317,
	383971,383624,383278,382932,382587,382242,381897,381552,
	381208,380864,380521,380177,379834,379492,379149,378807,
	378466,378124,377783,377442,377102,376762,376422,376082,
	375743,375404,375065,374727,374389,374051,373714,373377,
	373040,372703,372367,372031,371695,371360,371025,370690,
	370356,370022,369688,369355,369021,368688,368356,368023,
	367691,367360,367028,366697,366366,366036,365706,365376,
	365046,364717,364388,364059,363731,363403,363075,362747,
	362420,362093,361766,361440,361114,360788,360463,360137,
	359813,359488,359164,358840,358516,358193,357869,357547,
	357224,356902,356580,356258,355937,355616,355295,354974,
	354654,354334,354014,353695,353376,353057,352739,352420,
	352103,351785,351468,351150,350834,350517,350201,349885,
	349569,349254,348939,348624,348310,347995,347682,347368,
	347055,346741,346429,346116,345804,345492,345180,344869,
	344558,344247,343936,343626,343316,343006,342697,342388,
	342079,341770,341462,341154,340846,340539,340231,339924,
	339618,339311,339005,338700,338394,338089,337784,337479,
	337175,336870,336566,336263,335959,335656,335354,335051,
	334749,334447,334145,333844,333542,333242,332941,332641,
	332341,332041,331741,331442,331143,330844,330546,330247,
	329950,329652,329355,329057,328761,328464,328168,327872,
	327576,327280,326985,326690,326395,326101,325807,325513,
	325219,324926,324633,324340,324047,323755,323463,323171,
	322879,322588,322297,322006,321716,321426,321136,320846,
	320557,320267,319978,319690,319401,319113,318825,318538,
	318250,317963,317676,317390,317103,316817,316532,316246,
	315961,315676,315391,315106,314822,314538,314254,313971,
	313688,313405,313122,312839,312557,312275,311994,311712,
	311431,311150,310869,310589,310309,310029,309749,309470,
	309190,308911,308633,308354,308076,307798,307521,307243,
	306966,306689,306412,306136,305860,305584,305308,305033,
	304758,304483,304208,303934,303659,303385,303112,302838,
	302565,302292,302019,301747,301475,301203,300931,300660,
	300388,300117,299847,299576,299306,299036,298766,298497,
	298227,297958,297689,297421,297153,296884,296617,296349,
	296082,295815,295548,295281,295015,294749,294483,294217,
	293952,293686,293421,293157,292892,292628,292364,292100,
	291837,291574,291311,291048,290785,290523,290261,289999,
	289737,289476,289215,288954,288693,288433,288173,287913,
	287653,287393,287134,286875,286616,286358,286099,285841,
	285583,285326,285068,284811,284554,284298,284041,283785,
	283529,283273,283017,282762,282507,282252,281998,281743,
	281489,281235,280981,280728,280475,280222,279969,279716,
	279464,279212,278960,278708,278457,278206,277955,277704,
	277453,277203,276953,276703,276453,276204,275955,275706,
	275457,275209,274960,274712,274465,274217,273970,273722,
	273476,273229,272982,272736,272490,272244,271999,271753,
	271508,271263,271018,270774,270530,270286,270042,269798,
	269555,269312,269069,268826,268583,268341,268099,267857
};

#define LOGFAC 2*16
static	UWORD logtab[104]={
	LOGFAC*907,LOGFAC*900,LOGFAC*894,LOGFAC*887,
	LOGFAC*881,LOGFAC*875,LOGFAC*868,LOGFAC*862,
	LOGFAC*856,LOGFAC*850,LOGFAC*844,LOGFAC*838,
	LOGFAC*832,LOGFAC*826,LOGFAC*820,LOGFAC*814,
	LOGFAC*808,LOGFAC*802,LOGFAC*796,LOGFAC*791,
	LOGFAC*785,LOGFAC*779,LOGFAC*774,LOGFAC*768,
	LOGFAC*762,LOGFAC*757,LOGFAC*752,LOGFAC*746,
	LOGFAC*741,LOGFAC*736,LOGFAC*730,LOGFAC*725,
	LOGFAC*720,LOGFAC*715,LOGFAC*709,LOGFAC*704,
	LOGFAC*699,LOGFAC*694,LOGFAC*689,LOGFAC*684,
	LOGFAC*678,LOGFAC*675,LOGFAC*670,LOGFAC*665,
	LOGFAC*660,LOGFAC*655,LOGFAC*651,LOGFAC*646,
	LOGFAC*640,LOGFAC*636,LOGFAC*632,LOGFAC*628,
	LOGFAC*623,LOGFAC*619,LOGFAC*614,LOGFAC*610,
	LOGFAC*604,LOGFAC*601,LOGFAC*597,LOGFAC*592,
	LOGFAC*588,LOGFAC*584,LOGFAC*580,LOGFAC*575,
	LOGFAC*570,LOGFAC*567,LOGFAC*563,LOGFAC*559,
	LOGFAC*555,LOGFAC*551,LOGFAC*547,LOGFAC*543,
	LOGFAC*538,LOGFAC*535,LOGFAC*532,LOGFAC*528,
	LOGFAC*524,LOGFAC*520,LOGFAC*516,LOGFAC*513,
	LOGFAC*508,LOGFAC*505,LOGFAC*502,LOGFAC*498,
	LOGFAC*494,LOGFAC*491,LOGFAC*487,LOGFAC*484,
	LOGFAC*480,LOGFAC*477,LOGFAC*474,LOGFAC*470,
	LOGFAC*467,LOGFAC*463,LOGFAC*460,LOGFAC*457,
	LOGFAC*453,LOGFAC*450,LOGFAC*447,LOGFAC*443,
	LOGFAC*440,LOGFAC*437,LOGFAC*434,LOGFAC*431
};

static	SBYTE PanbrelloTable[256]={
	  0,  2,  3,  5,  6,  8,  9, 11, 12, 14, 16, 17, 19, 20, 22, 23,
	 24, 26, 27, 29, 30, 32, 33, 34, 36, 37, 38, 39, 41, 42, 43, 44,
	 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 58, 59,
	 59, 60, 60, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 64, 64,
	 64, 64, 64, 64, 64, 64, 63, 63, 63, 62, 62, 62, 61, 61, 60, 60,
	 59, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46,
	 45, 44, 43, 42, 41, 39, 38, 37, 36, 34, 33, 32, 30, 29, 27, 26,
	 24, 23, 22, 20, 19, 17, 16, 14, 12, 11,  9,  8,  6,  5,  3,  2,
	  0,- 2,- 3,- 5,- 6,- 8,- 9,-11,-12,-14,-16,-17,-19,-20,-22,-23,
	-24,-26,-27,-29,-30,-32,-33,-34,-36,-37,-38,-39,-41,-42,-43,-44,
	-45,-46,-47,-48,-49,-50,-51,-52,-53,-54,-55,-56,-56,-57,-58,-59,
	-59,-60,-60,-61,-61,-62,-62,-62,-63,-63,-63,-64,-64,-64,-64,-64,
	-64,-64,-64,-64,-64,-64,-63,-63,-63,-62,-62,-62,-61,-61,-60,-60,
	-59,-59,-58,-57,-56,-56,-55,-54,-53,-52,-51,-50,-49,-48,-47,-46,
	-45,-44,-43,-42,-41,-39,-38,-37,-36,-34,-33,-32,-30,-29,-27,-26,
	-24,-23,-22,-20,-19,-17,-16,-14,-12,-11,- 9,- 8,- 6,- 5,- 3,- 2
};

/* returns a random value between 0 and ceil-1, ceil must be a power of two */
static int getrandom(int ceil)
{
#ifdef HAVE_SRANDOM
	return random()&(ceil-1);
#else
	return (int)((rand()*ceil)/(RAND_MAX+1.0));
#endif
}

/*	New Note Action Scoring System :
	--------------------------------
1)	total-volume (fadevol, chanvol, volume) is the main scorer.
2)	a looping sample is a bonus x2
3)	a foreground channel is a bonus x4
4)	an active envelope with keyoff is a handicap -x2                          */
static int MP_FindEmptyChannel(void)
{
	MP_VOICE *a;
	ULONG t,k,tvol,pp;

	for (t=0;t<md_sngchn;t++)
		if (((pf->voice[t].kick==KICK_ABSENT)||(pf->voice[t].kick==KICK_ENV))&&
		   Voice_Stopped_internal(t))
			return t;

	tvol=0xffffffUL;t=0;a=pf->voice;
	for (k=0;k<md_sngchn;k++,a++)
		if ((a->kick==KICK_ABSENT)||(a->kick==KICK_ENV)) {
			pp=a->totalvol<<((a->s->flags&SF_LOOP)?1:0);
			if ((a->master)&&(a==a->master->slave))
				pp<<=2;

			if (pp<tvol) {
				tvol=pp;
				t=k;
			}
		}

	if (tvol>8000*7) return -1;
	return t;
}

static SWORD Interpolate(SWORD p,SWORD p1,SWORD p2,SWORD v1,SWORD v2)
{
	if ((p1==p2)||(p==p1)) return v1;
	return v1+((SLONG)((p-p1)*(v2-v1))/(p2-p1));
}

UWORD getlinearperiod(UWORD note,ULONG fine)
{
	UWORD t;

	t=(20L*OCTAVE+2-note)*32L-(fine>>1);
	return t;
}

static UWORD getlogperiod(UWORD note,ULONG fine)
{
	UWORD n,o;
	UWORD p1,p2;
	ULONG i;

	n=note%(2*OCTAVE);
	o=note/(2*OCTAVE);
	i=(n<<2)+(fine>>4); /* n*8 + fine/16 */

	p1=logtab[i];
	p2=logtab[i+1];

	return (Interpolate(fine>>4,0,15,p1,p2)>>o);
}

static UWORD getoldperiod(UWORD note,ULONG speed)
{
	UWORD n,o;

	if (!speed) {
#ifdef MIKMOD_DEBUG
		fprintf(stderr,"\rmplayer: getoldperiod() called with note=%d, speed=0 !\n",note);
#endif
		return 4242; /* <- prevent divide overflow.. (42 hehe) */
	}

	n=note%(2*OCTAVE);
	o=note/(2*OCTAVE);
	return ((8363L*(ULONG)oldperiods[n])>>o)/speed;
}

static UWORD GetPeriod(UWORD note,ULONG speed)
{
	if (pf->flags & UF_XMPERIODS)
		return (pf->flags&UF_LINEAR)?getlinearperiod(note,speed):getlogperiod(note,speed);

	return getoldperiod(note,speed);
}

static SWORD InterpolateEnv(SWORD p,ENVPT *a,ENVPT *b)
{
	return (Interpolate(p,a->pos,b->pos,a->val,b->val));
}

static SWORD DoPan(SWORD envpan,SWORD pan)
{
	int newpan;

	newpan=pan+(((envpan-PAN_CENTER)*(128-abs(pan-PAN_CENTER)))/128);

	return (newpan<PAN_LEFT)?PAN_LEFT:(newpan>PAN_RIGHT?PAN_RIGHT:newpan);
}

static void StartEnvelope(ENVPR *t,UBYTE flg,UBYTE pts,UBYTE susbeg,UBYTE susend,UBYTE beg,UBYTE end,ENVPT *p,UBYTE keyoff)
{
	t->flg=flg;
	t->pts=pts;
	t->susbeg=susbeg;
	t->susend=susend;
	t->beg=beg;
	t->end=end;
	t->env=p;
	t->p=0;
	t->a=0;
	t->b=((t->flg&EF_SUSTAIN)&&(!(keyoff&KEY_OFF)))?0:1;

	/* Imago Orpheus sometimes stores an extra initial point in the envelope */
	if ((t->pts>=2)&&(t->env[0].pos==t->env[1].pos)) {
		t->a++;t->b++;
	}

	if (t->b>=t->pts) t->b=t->pts-1;
}

/* This procedure processes all envelope types, include volume, pitch, and
   panning.  Envelopes are defined by a set of points, each with a magnitude
   [relating either to volume, panning position, or pitch modifier] and a tick
   position.

   Envelopes work in the following manner:

   (a) Each tick the envelope is moved a point further in its progression. For
       an accurate progression, magnitudes between two envelope points are
       interpolated.

   (b) When progression reaches a defined point on the envelope, values are
       shifted to interpolate between this point and the next, and checks for
       loops or envelope end are done.

   Misc:
     Sustain loops are loops that are only active as long as the keyoff flag is
     clear.  When a volume envelope terminates, so does the current fadeout.  */
static SWORD ProcessEnvelope(ENVPR *t,SWORD v,UBYTE keyoff)
{
	if (t->flg & EF_ON) {
		UBYTE a,b; /* actual points in the envelope */
		UWORD p; /* the 'tick counter' - real point being played */

		a=t->a;
		b=t->b;
		p=t->p;

		/* if sustain loop on one point (XM type), don't move and don't
		   interpolate when the point is reached */
		if ((t->flg & EF_SUSTAIN)&&(t->susbeg==t->susend)&&
		   (!(keyoff&KEY_OFF))&&(p==t->env[t->susbeg].pos))
			v=t->env[t->susbeg].val;
		else {
			/* compute the current envelope value between points a and b */
			if (a==b)
				v=t->env[a].val;
			else
				v=InterpolateEnv(p,&t->env[a],&t->env[b]);

			p++;
			/* pointer reached point b? */
			if (p>=t->env[b].pos) {
				a=b++; /* shift points a and b */

				/* Check for loops, sustain loops, or end of envelope. */
				if ((t->flg&EF_SUSTAIN)&&(!(keyoff&KEY_OFF))&&(b>t->susend)) {
					a=t->susbeg;
					b=(t->susbeg==t->susend)?a:a+1;
					p=t->env[a].pos;
				} else
				  if ((t->flg & EF_LOOP)&&(b>t->end)) {
					a=t->beg;
					b=(t->beg==t->end)?a:a+1;
					p=t->env[a].pos;
				} else {
					if (b>=t->pts) {
						if ((t->flg & EF_VOLENV)&&(mp_channel!=-1)) {
							pf->voice[mp_channel].keyoff|=KEY_FADE;
							if (!v)
								pf->voice[mp_channel].fadevol=0;
						}
						b--;p--;
					}
				}
			}
			t->a=a;
			t->b=b;
			t->p=p;
		}
	}
	return v;
}

/* XM linear period to frequency conversion */
ULONG getfrequency(UBYTE flags,ULONG period)
{
	if (flags & UF_LINEAR)
		return lintab[period%768]>>(period/768);
	else
		return (8363L*1712L)/(period?period:1);
}

/*========== Protracker effects */

static void DoEEffects(UBYTE dat)
{
	UBYTE nib=dat&0xf;

	switch (dat>>4) {
		case 0x0: /* hardware filter toggle, not supported */
			break;
		case 0x1: /* fineslide up */
			if (a->period)
				if (!pf->vbtick) a->tmpperiod-=(nib<<2);
			break;
		case 0x2: /* fineslide dn */
			if (a->period)
				if (!pf->vbtick) a->tmpperiod+=(nib<<2);
			break;
		case 0x3: /* glissando ctrl */
			a->glissando=nib;
			break;
		case 0x4: /* set vibrato waveform */
			a->wavecontrol&=0xf0;
			a->wavecontrol|=nib;
			break;
		case 0x5: /* set finetune */
			if (a->period) {
				if (pf->flags&UF_XMPERIODS)
					a->speed=nib+128;
				else
					a->speed=finetune[nib];
				a->tmpperiod=GetPeriod((UWORD)a->note<<1,a->speed);
			}
			break;
		case 0x6: /* set patternloop */
			if (pf->vbtick) break;
			if (nib) { /* set reppos or repcnt ? */
				/* set repcnt, so check if repcnt already is set, which means we
				   are already looping */
				if (a->pat_repcnt)
					a->pat_repcnt--; /* already looping, decrease counter */
				else {
#if 0
					/* this would make walker.xm, shipped with Xsoundtracker,
					   play correctly, but it's better to remain compatible
					   with FT2 */
					if ((!(pf->flags&UF_NOWRAP))||(a->pat_reppos!=POS_NONE))
#endif
						a->pat_repcnt=nib; /* not yet looping, so set repcnt */
				}

				if (a->pat_repcnt) { /* jump to reppos if repcnt>0 */
					if (a->pat_reppos==POS_NONE)
						a->pat_reppos=pf->patpos-1;
					if (a->pat_reppos==-1) {
						pf->pat_repcrazy=1;
						pf->patpos=0;
					} else
						pf->patpos=a->pat_reppos;
				} else a->pat_reppos=POS_NONE;
			} else
				a->pat_reppos=pf->patpos-1; /* set reppos - can be (-1) */
			break;
		case 0x7: /* set tremolo waveform */
			a->wavecontrol&=0x0f;
			a->wavecontrol|=nib<<4;
			break;
		case 0x8: /* set panning */
			if (pf->panflag) {
				if (nib<=8) nib<<=4;
				else nib*=17;
				a->panning=pf->panning[mp_channel]=nib;
			}
			break;
		case 0x9: /* retrig note */
			/* only retrigger if data nibble > 0 */
			if (nib) {
				if (!a->retrig) {
					/* when retrig counter reaches 0, reset counter and restart
					   the sample */
					if (a->period) a->kick=KICK_NOTE;
					a->retrig=nib;
				}
				a->retrig--; /* countdown */
			}
			break;
		case 0xa: /* fine volume slide up */
			if (pf->vbtick) break;
			a->tmpvolume+=nib;
			if (a->tmpvolume>64) a->tmpvolume=64;
			break;
		case 0xb: /* fine volume slide dn  */
			if (pf->vbtick) break;
			a->tmpvolume-=nib;
			if (a->tmpvolume<0) a->tmpvolume=0;
			break;
		case 0xc: /* cut note */
			/* When pf->vbtick reaches the cut-note value, turn the volume to
			   zero ( Just like on the amiga) */
			if (pf->vbtick>=nib)
				a->tmpvolume=0; /* just turn the volume down */
			break;
		case 0xd: /* note delay */
			/* delay the start of the sample until pf->vbtick==nib */
			if (!pf->vbtick)
				a->notedelay=nib;
			else if (a->notedelay)
				a->notedelay--;
			break;
		case 0xe: /* pattern delay */
			if (pf->vbtick) break;
			if (!pf->patdly2) pf->patdly=nib+1; /* only once, when vbtick=0 */
			break;
		case 0xf: /* invert loop, not supported  */
			break;
	}
}

static void DoVibrato(void)
{
	UBYTE q;
	UWORD temp=0;

	q=(a->vibpos>>2)&0x1f;

	switch (a->wavecontrol&3) {
		case 0: /* sine */
			temp=VibratoTable[q];
			break;
		case 1: /* ramp down */
			q<<=3;
			if (a->vibpos<0) q=255-q;
			temp=q;
			break;
		case 2: /* square wave */
			temp=255;
			break;
		case 3: /* random wave */
			temp=getrandom(256);
			break;
	}

	temp*=a->vibdepth;
	temp>>=7;temp<<=2;

	if (a->vibpos>=0)
		a->period=a->tmpperiod+temp;
	else
		a->period=a->tmpperiod-temp;

	if (pf->vbtick) a->vibpos+=a->vibspd;
}

static void DoTremolo(void)
{
	UBYTE q;
	UWORD temp=0;

	q=(a->trmpos>>2)&0x1f;

	switch ((a->wavecontrol>>4)&3) {
		case 0: /* sine */
			temp=VibratoTable[q];
			break;
		case 1: /* ramp down */
			q<<=3;
			if (a->trmpos<0) q=255-q;
			temp=q;
			break;
		case 2: /* square wave */
			temp=255;
			break;
		case 3: /* random wave */
			temp=getrandom(256);
			break;
	}
	temp*=a->trmdepth;
	temp>>=6;

	if (a->trmpos>=0) {
		a->volume=a->tmpvolume+temp;
		if (a->volume>64) a->volume=64;
	} else {
		a->volume=a->tmpvolume-temp;
		if (a->volume<0) a->volume=0;
	}

	if (pf->vbtick) a->trmpos+=a->trmspd;
}

static void DoVolSlide(UBYTE dat)
{
	if (!pf->vbtick) return;

	if (dat&0xf) {
		a->tmpvolume-=(dat&0x0f);
		if (a->tmpvolume<0) a->tmpvolume=0;
	} else {
		a->tmpvolume+=(dat>>4);
		if (a->tmpvolume>64) a->tmpvolume=64;
	}
}

static void DoToneSlide(void)
{
	if (pf->vbtick) {
		int dist;

		/* We have to slide a->period towards a->wantedperiod, so compute the
		   difference between those two values */
		dist=a->period-a->wantedperiod;

		/* if they are equal or if portamentospeed is too big ...*/
		if ((!dist)||a->portspeed>abs(dist))
			/* ...make tmpperiod equal tperiod */
			a->tmpperiod=a->period=a->wantedperiod;
		else if (dist>0) {
			a->tmpperiod-=a->portspeed;
			a->period-=a->portspeed; /* dist>0, slide up */
		} else {
			a->tmpperiod+=a->portspeed;
			a->period+=a->portspeed; /* dist<0, slide down */
		}
	} else
		a->tmpperiod=a->period;
}

static void DoArpeggio(UBYTE dat)
{
	UBYTE note=a->note;

	if (dat) {
		switch (pf->vbtick%3) {
			case 1:
				note+=(dat>>4);  break;
			case 2:
				note+=(dat&0xf); break;
		}
		a->period=GetPeriod((UWORD)note<<1,a->speed);
		a->ownper=1;
	}
}

/*========== Scream Tracker effects */

static void DoS3MVolSlide(UBYTE inf)
{
	UBYTE lo, hi;

	explicitslides=1;

	if (inf) a->s3mvolslide=inf;
	else inf=a->s3mvolslide;

	lo=inf&0xf;
	hi=inf>>4;

	if (!lo) {
		if ((pf->vbtick)||(pf->flags&UF_S3MSLIDES)) a->tmpvolume+=hi;
	} else
	  if (!hi) {
		if ((pf->vbtick)||(pf->flags&UF_S3MSLIDES)) a->tmpvolume-=lo;
	} else
	  if (lo==0xf) {
		if (!pf->vbtick) a->tmpvolume+=(hi?hi:0xf);
	} else
	  if (hi==0xf) {
		if (!pf->vbtick) a->tmpvolume-=(lo?lo:0xf);
	} else
	  return;

	if (a->tmpvolume<0) a->tmpvolume=0;
	else if (a->tmpvolume>64) a->tmpvolume=64;
}

static void DoS3MSlideDn(UBYTE inf)
{
	UBYTE hi,lo;

	if (inf) a->slidespeed=inf;
	else inf=a->slidespeed;

	hi=inf>>4;
	lo=inf&0xf;

	if (hi==0xf) {
		if (!pf->vbtick) a->tmpperiod+=(UWORD)lo<<2;
	} else
	  if (hi==0xe) {
		if (!pf->vbtick) a->tmpperiod+=lo;
	} else {
		if (pf->vbtick) a->tmpperiod+=(UWORD)inf<<2;
	}
}

static void DoS3MSlideUp(UBYTE inf)
{
	UBYTE hi,lo;

	if (inf) a->slidespeed=inf;
	else inf=a->slidespeed;

	hi=inf>>4;
	lo=inf&0xf;

	if (hi==0xf) {
		if (!pf->vbtick) a->tmpperiod-=(UWORD)lo<<2;
	} else
	  if (hi==0xe) {
		if (!pf->vbtick) a->tmpperiod-=lo;
	} else {
		if (pf->vbtick) a->tmpperiod-=(UWORD)inf<<2;
	}
}

static void DoS3MTremor(UBYTE inf)
{
	UBYTE on,off;

	if (inf)
		a->s3mtronof=inf;
	else {
		inf=a->s3mtronof;
		if (!inf) return;
	}

	if (!pf->vbtick) return;

	on=(inf>>4)+1;
	off=(inf&0xf)+1;
	a->s3mtremor%=(on+off);
	a->volume=(a->s3mtremor<on)?a->tmpvolume:0;
	a->s3mtremor++;
}

static void DoS3MRetrig(UBYTE inf)
{
	if (inf) {
		a->s3mrtgslide=inf>>4;
		a->s3mrtgspeed=inf&0xf;
	}

	/* only retrigger if low nibble > 0 */
	if ( a->s3mrtgspeed>0) {
		if (!a->retrig) {
			/* when retrig counter reaches 0, reset counter and restart the
			   sample */
			if (a->kick!=KICK_NOTE) a->kick=KICK_KEYOFF;
			a->retrig=a->s3mrtgspeed;

			if ((pf->vbtick)||(pf->flags&UF_S3MSLIDES)) {
				switch (a->s3mrtgslide) {
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
						a->tmpvolume-=(1<<(a->s3mrtgslide-1));
						break;
					case 6:
						a->tmpvolume=(2*a->tmpvolume)/3;
						break;
					case 7:
						a->tmpvolume>>=1;
						break;
					case 9:
					case 0xa:
					case 0xb:
					case 0xc:
					case 0xd:
						a->tmpvolume+=(1<<(a->s3mrtgslide-9));
						break;
					case 0xe:
						a->tmpvolume=(3*a->tmpvolume)>>1;
						break;
					case 0xf:
						a->tmpvolume=a->tmpvolume<<1;
						break;
				}
				if (a->tmpvolume<0) a->tmpvolume=0;
				else if (a->tmpvolume>64) a->tmpvolume=64;
			}
		}
		a->retrig--; /* countdown  */
	}
}

static void DoS3MSpeed(UBYTE speed)
{
	if (pf->vbtick||pf->patdly2) return;

	if (speed>128) speed-=128;
	if (speed) {
		pf->sngspd=speed;
		pf->vbtick=0;
	}
}

static void DoS3MTempo(UBYTE tempo)
{
	if (pf->vbtick||pf->patdly2) return;

	pf->bpm=(tempo<32)?32:tempo;
}

static void DoS3MFineVibrato(void)
{
	UBYTE q;
	UWORD temp=0;

	q=(a->vibpos>>2)&0x1f;

	switch (a->wavecontrol&3) {
		case 0: /* sine */
			temp=VibratoTable[q];
			break;
		case 1: /* ramp down */
			q<<=3;
			if (a->vibpos<0) q=255-q;
			temp=q;
			break;
		case 2: /* square wave */
			temp=255;
			break;
		case 3: /* random */
			temp=getrandom(256);
			break;
	}

	temp*=a->vibdepth;
	temp>>=8;

	if (a->vibpos>=0)
		a->period=a->tmpperiod+temp;
	else
		a->period=a->tmpperiod-temp;

	a->vibpos+=a->vibspd;
}

static void DoS3MTremolo(void)
{
	UBYTE q;
	UWORD temp=0;

	q=(a->trmpos>>2)&0x1f;

	switch ((a->wavecontrol>>4)&3) {
		case 0: /* sine */
			temp=VibratoTable[q];
			break;
		case 1: /* ramp down */
			q<<=3;
			if (a->trmpos<0) q=255-q;
			temp=q;
			break;
		case 2: /* square wave */
			temp=255;
			break;
		case 3: /* random */
			temp=getrandom(256);
			break;
	}

	temp*=a->trmdepth;
	temp>>=7;

	if (a->trmpos>=0) {
		a->volume=a->tmpvolume+temp;
		if (a->volume>64) a->volume=64;
	} else {
		a->volume=a->tmpvolume-temp;
		if (a->volume<0) a->volume=0;
	}

	if (pf->vbtick) a->trmpos+=a->trmspd;
}

/*========== Fast Tracker effects */

static void DoXMVolSlide(UBYTE inf)
{
	UBYTE lo,hi;

	explicitslides=2;

	if (inf) a->s3mvolslide=inf;
	else inf=a->s3mvolslide;

	if (!pf->vbtick) return;

	lo=inf&0xf;
	hi=inf>>4;

	if (!hi) {
		a->tmpvolume-=lo;
		if (a->tmpvolume<0) a->tmpvolume=0;
	} else {
		a->tmpvolume+=hi;
		if (a->tmpvolume>64) a->tmpvolume=64;
	}
}

static void DoXMGlobalSlide(UBYTE inf)
{
	if (pf->vbtick) {
		if (inf) pf->globalslide=inf;
		else inf=pf->globalslide;
		if (inf & 0xf0) inf&=0xf0;
		pf->volume=pf->volume+((inf>>4)-(inf&0xf))*2;

		if (pf->volume<0) pf->volume=0;
		else if (pf->volume>128) pf->volume=128;
	}
}

static void DoXMPanSlide(UBYTE inf)
{
	UBYTE lo,hi;
	SWORD pan;

	if (inf) a->pansspd=inf;
	else inf=a->pansspd;

	if (!pf->vbtick) return;

	lo=inf&0xf;
	hi=inf>>4;

	/* slide right has absolute priority */
	if (hi) lo=0;

	pan=((a->panning==PAN_SURROUND)?PAN_CENTER:a->panning)+hi-lo;

	a->panning=(pan<PAN_LEFT)?PAN_LEFT:(pan>PAN_RIGHT?PAN_RIGHT:pan);
}

static void DoXMExtraFineSlideUp(UBYTE inf)
{
	if (!pf->vbtick) {
		a->period-=inf;
		a->tmpperiod-=inf;
	}
}

static void DoXMExtraFineSlideDown(UBYTE inf)
{
	if (!pf->vbtick) {
		a->period+=inf;
		a->tmpperiod+=inf;
	}
}

/*========== Impulse Tracker effects */

static void DoITChanVolSlide(UBYTE inf)
{
	UBYTE lo, hi;

	if (inf) a->chanvolslide=inf;
	inf=a->chanvolslide;

	lo=inf&0xf;
	hi=inf>>4;

	if (!hi)
		a->chanvol-=lo;
	else
	  if (!lo) {
		a->chanvol+=hi;
	} else
	  if (hi==0xf) {
		if (!pf->vbtick) a->chanvol-=lo;
	} else
	  if (lo==0xf) {
		if (!pf->vbtick) a->chanvol+=hi;
	}

	if (a->chanvol<0) a->chanvol=0;
	if (a->chanvol>64) a->chanvol=64;
}

static void DoITGlobalSlide(UBYTE inf)
{
	UBYTE lo,hi;

	if (inf) pf->globalslide=inf;
	inf=pf->globalslide;

	lo=inf&0xf;
	hi=inf>>4;

	if (!lo) {
		if (pf->vbtick) pf->volume+=hi;
	} else
	  if (!hi) {
		if (pf->vbtick) pf->volume-=lo;
	} else
	  if (lo==0xf) {
		if (!pf->vbtick) pf->volume+=hi;
	} else
	  if (hi==0xf) {
		if (!pf->vbtick) pf->volume-=lo;
	}

	if (pf->volume<0)   pf->volume=0;
	if (pf->volume>128) pf->volume=128;
}

static void DoITPanSlide(UBYTE inf)
{
	UBYTE lo,hi;
	SWORD pan;

	if (inf) a->pansspd=inf;
	else inf=a->pansspd;

	lo=inf&0xf;
	hi=inf>>4;

	pan=(a->panning==PAN_SURROUND)?PAN_CENTER:a->panning;

	if (!hi)
		pan+=lo<<2;
	else
	  if (!lo) {
		pan-=hi<<2;
	} else
	  if (hi==0xf) {
		if (!pf->vbtick) pan+=lo<<2;
	} else
	  if (lo==0xf) {
		if (!pf->vbtick) pan-=hi<<2;
	}
	a->panning=/*pf->panning[mp_channel]=*/
	  (pan<PAN_LEFT)?PAN_LEFT:(pan>PAN_RIGHT?PAN_RIGHT:pan);
}

static void DoITTempo(UBYTE tempo)
{
	SWORD temp=pf->bpm;

	if (pf->vbtick||pf->patdly2) return;

	if (tempo&0x10)
		temp+=(tempo&0x0f);
	else
		temp-=tempo;

	pf->bpm=(temp>255)?255:(temp<1?1:temp);
}

static void DoITVibrato(void)
{
	UBYTE q;
	UWORD temp=0;

	q=(a->vibpos>>2)&0x1f;

	switch (a->wavecontrol&3) {
		case 0: /* sine */
			temp=VibratoTable[q];
			break;
		case 1: /* square wave */
			temp=255;
			break;
		case 2: /* ramp down */
			q<<=3;
			if (a->vibpos<0) q=255-q;
			temp=q;
			break;
		case 3: /* random */
			temp=getrandom(256);
			break;
	}

	temp*=a->vibdepth;
	temp>>=8;
	temp<<=2;

	if (a->vibpos>=0)
		a->period=a->tmpperiod+temp;
	else
		a->period=a->tmpperiod-temp;

	a->vibpos+=a->vibspd;
}

static void DoITFineVibrato(void)
{
	UBYTE q;
	UWORD temp=0;

	q=(a->vibpos>>2)&0x1f;

	switch (a->wavecontrol&3) {
		case 0: /* sine */
			temp=VibratoTable[q];
			break;
		case 1: /* square wave */
			temp=255;
			break;
		case 2: /* ramp down */
			q<<=3;
			if (a->vibpos<0) q=255-q;
			temp=q;
			break;
		case 3: /* random */
			temp=getrandom(256);
			break;
	}

	temp*=a->vibdepth;
	temp>>=8;

	if (a->vibpos>=0)
		a->period=a->tmpperiod+temp;
	else
		a->period=a->tmpperiod-temp;

	a->vibpos+=a->vibspd;
}

static void DoITTremor(UBYTE inf)
{
	UBYTE on,off;

	if (inf)
		a->s3mtronof=inf;
	else {
		inf=a->s3mtronof;
		if (!inf) return;
	}

	if (!pf->vbtick) return;

	on=(inf>>4);
	off=(inf&0xf);

	a->s3mtremor%=(on+off);
	a->volume=(a->s3mtremor<on)?a->tmpvolume:0;
	a->s3mtremor++;
}

static void DoITPanbrello(void)
{
	UBYTE q;
	SLONG temp=0;

	q=a->panbpos;

	switch (a->panbwave) {
		case 0: /* sine */
			temp=PanbrelloTable[q];
			break;
		case 1: /* square wave */
			temp=(q<0x80)?64:0;
			break;
		case 2: /* ramp down */
			q<<=3;
			temp=q;
			break;
		case 3: /* random */
			if (a->panbpos>=a->panbspd) {
				a->panbpos=0;
				temp=getrandom(256);
			}
	}

	temp*=a->panbdepth;
	temp=(temp/8)+pf->panning[mp_channel];

	a->panning=(temp<PAN_LEFT)?PAN_LEFT:(temp>PAN_RIGHT?PAN_RIGHT:temp);
	a->panbpos+=a->panbspd;
}

static void DoITToneSlide(void)
{
	/* if we don't come from another note, ignore the slide and play the note
	   as is */
	if (!a->oldnote) return;

	if (pf->vbtick) {
		int dist;

		/* We have to slide a->period towards a->wantedperiod, compute the
		   difference between those two values */
		dist=a->period-a->wantedperiod;

	    /* if they are equal or if portamentospeed is too big... */
		if ((!dist)||((a->portspeed<<2)>abs(dist)))
			/* ... make tmpperiod equal tperiod */
			a->tmpperiod=a->period=a->wantedperiod;
		else
		  if (dist>0) {
			a->tmpperiod-=a->portspeed<<2;
			a->period-=a->portspeed<<2; /* dist>0 slide up */
		} else {
			a->tmpperiod+=a->portspeed<<2;
			a->period+=a->portspeed<<2; /* dist<0 slide down */
		}
	} else
		a->tmpperiod=a->period;
}

static void DoNNAEffects(UBYTE dat);
/* Impulse/Scream Tracker Sxx effects.
   All Sxx effects share the same memory space. */
static void DoSSEffects(UBYTE dat)
{
	UBYTE inf,c;

	inf=dat&0xf;
	c=dat>>4;

	if (!dat) {
		c=a->sseffect;
		inf=a->ssdata;
	} else {
		a->sseffect=c;
		a->ssdata=inf;
	}

	switch (c) {
		case SS_GLISSANDO: /* S1x set glissando voice */
			DoEEffects(0x30|inf);
			break;
		case SS_FINETUNE: /* S2x set finetune */
			DoEEffects(0x50|inf);
			break;
		case SS_VIBWAVE: /* S3x set vibrato waveform */
			DoEEffects(0x40|inf);
			break;
		case SS_TREMWAVE: /* S4x set tremolo waveform */
			DoEEffects(0x70|inf);
			break;
		case SS_PANWAVE: /* S5x panbrello */
			a->panbwave=inf;
			break;
		case SS_FRAMEDELAY: /* S6x delay x number of frames (patdly) */
			DoEEffects(0xe0|inf);
			break;
		case SS_S7EFFECTS: /* S7x instrument / NNA commands */
			DoNNAEffects(inf);
			break;
		case SS_PANNING: /* S8x set panning position */
			DoEEffects(0x80 | inf);
			break;
		case SS_SURROUND: /* S9x set surround Sound */
			if (pf->panflag)
				a->panning=pf->panning[mp_channel]=PAN_SURROUND;
			break;
		case SS_HIOFFSET: /* SAy set high order sample offset yxx00h */
			if (!pf->vbtick) {
				a->hioffset=inf<<16;
				a->start=a->hioffset|a->soffset;

				if ((a->s)&&(a->start>a->s->length))
					a->start=a->s->flags&(SF_LOOP|SF_BIDI)?a->s->loopstart:a->s->length;
			}
			break;
		case SS_PATLOOP: /* SBx pattern loop */
			DoEEffects(0x60|inf);
			break;
		case SS_NOTECUT: /* SCx notecut */
			DoEEffects(0xC0|inf);
			break;
		case SS_NOTEDELAY: /* SDx notedelay */
			DoEEffects(0xD0|inf);
			break;
		case SS_PATDELAY: /* SEx patterndelay */
			DoEEffects(0xE0|inf);
			break;
	}
}

/* Impulse Tracker Volume/Pan Column effects.
   All volume/pan column effects share the same memory space. */
static void DoVolEffects(UBYTE c)
{
	UBYTE inf=UniGetByte();

	if ((!c)&&(!inf)) {
		c=a->voleffect;
		inf=a->voldata;
	} else {
		a->voleffect=c;
		a->voldata=inf;
	}

	if (c)
	  switch (c) {
		case VOL_VOLUME:
			if (pf->vbtick) break;
			if (inf>64) inf=64;
			a->tmpvolume=inf;
			break;
		case VOL_PANNING:
			if (pf->panflag)
				a->panning=/*pf->panning[mp_channel]=*/inf;
			break;
		case VOL_VOLSLIDE:
			DoS3MVolSlide(inf);
			break;
		case VOL_PITCHSLIDEDN:
			if (a->period)
				DoS3MSlideDn(inf);
			break;
		case VOL_PITCHSLIDEUP:
			if (a->period)
				DoS3MSlideUp(inf);
			break;
		case VOL_PORTAMENTO:
			if (inf) a->slidespeed=inf;
			if (a->period) {
				if ((!pf->vbtick)||(a->newsamp)){
					a->kick=KICK_NOTE;
					a->start=-1;
				} else
					a->kick=(a->kick==KICK_NOTE)?KICK_ENV:KICK_ABSENT;
				DoITToneSlide();
				a->ownper=1;
			}
			break;
		case VOL_VIBRATO:
			if (!pf->vbtick) {
				if (inf&0x0f) a->vibdepth=inf&0xf;
				if (inf&0xf0) a->vibspd=(inf&0xf0)>>2;
			}
			if (a->period) {
				DoITVibrato();
				a->ownper=1;
			}
			break;
	}
}

/*========== UltraTracker effects */

static void DoULTSampleOffset(void)
{
	UWORD offset=UniGetWord();

	if (offset)
		a->ultoffset=offset;

	a->start=a->ultoffset<<2;
	if ((a->s)&&(a->start>a->s->length))
		a->start=a->s->flags&(SF_LOOP|SF_BIDI)?a->s->loopstart:a->s->length;
}

/*========== OctaMED effects */

static void DoMEDSpeed(void)
{
	UWORD speed=UniGetWord();

	pf->bpm=speed;
}

/*========== General player functions */

static void pt_playeffects(void)
{
	UBYTE dat,c;

	while((c=UniGetByte())) {
		int oldsliding=a->sliding;

		a->sliding=0;
		switch (c) {
			case UNI_NOTE:
			case UNI_INSTRUMENT:
				a->sliding=oldsliding;
				UniSkipOpcode(c);
				break;
			case UNI_PTEFFECT0:
				dat=UniGetByte();
				if (!pf->vbtick) {
					if ((!dat)&&(pf->flags&UF_ARPMEM)) dat=a->arpmem;
					a->arpmem=dat;
				}
				if (a->period)
					DoArpeggio(a->arpmem);
				break;
			case UNI_PTEFFECT1:
				dat=UniGetByte();
				if ((!pf->vbtick)&&(dat)) a->slidespeed=(UWORD)dat<<2;
				if (a->period)
					if (pf->vbtick) a->tmpperiod-=a->slidespeed;
				break;
			case UNI_PTEFFECT2:
				dat=UniGetByte();
				if ((!pf->vbtick)&&(dat)) a->slidespeed=(UWORD)dat<<2;
				if (a->period)
					if (pf->vbtick) a->tmpperiod+=a->slidespeed;
				break;
			case UNI_PTEFFECT3:
				dat=UniGetByte();
				if ((!pf->vbtick)&&(dat)) a->portspeed=(UWORD)dat<<2;
				if (a->period) {
					if (!a->fadevol)
						a->kick=(a->kick==KICK_NOTE)?KICK_NOTE:KICK_KEYOFF;
					else
						a->kick=(a->kick==KICK_NOTE)?KICK_ENV:KICK_ABSENT;
					DoToneSlide();
					a->ownper=1;
				}
				break;
			case UNI_PTEFFECT4:
			case UNI_XMEFFECT4:
				dat=UniGetByte();
				if (!pf->vbtick) {
					if (dat&0x0f) a->vibdepth=dat&0xf;
					if (dat&0xf0) a->vibspd=(dat&0xf0)>>2;
				} else
				if (a->period) {
					DoVibrato();
					a->ownper=1;
				}
				break;
			case UNI_PTEFFECT5:
				dat=UniGetByte();
				if (a->period) {
					if (!a->fadevol)
						a->kick=(a->kick==KICK_NOTE)?KICK_NOTE:KICK_KEYOFF;
					else
						a->kick=(a->kick==KICK_NOTE)?KICK_ENV:KICK_ABSENT;
					DoToneSlide();
					a->ownper=1;
				}
				DoVolSlide(dat);
				break;
			case UNI_PTEFFECT6:
				dat=UniGetByte();
				if ((a->period)&&(pf->vbtick)) {
					DoVibrato();
					a->ownper=1;
				}
				DoVolSlide(dat);
				break;
			case UNI_PTEFFECT7:
				dat=UniGetByte();
				if (!pf->vbtick) {
					if (dat&0x0f) a->trmdepth=dat&0xf;
					if (dat&0xf0) a->trmspd=(dat&0xf0)>>2;
				}
				if (a->period) {
					DoTremolo();
					a->ownvol=1;
				}
				break;
			case UNI_PTEFFECT8:
				dat=UniGetByte();
				if (pf->panflag)
					a->panning=pf->panning[mp_channel]=dat;
				break;
			case UNI_PTEFFECT9:
				dat=UniGetByte();
				if (!pf->vbtick) {
					if (dat) a->soffset=(UWORD)dat<<8;
					a->start=a->hioffset|a->soffset;

					if ((a->s)&&(a->start>a->s->length))
						a->start=a->s->flags&(SF_LOOP|SF_BIDI)?a->s->loopstart:a->s->length;
				}
				break;
			case UNI_PTEFFECTA:
				DoVolSlide(UniGetByte());
				break;
			case UNI_PTEFFECTB:
				dat=UniGetByte();
				if ((pf->vbtick)||(pf->patdly2)) break;
				/* Vincent Voois uses a nasty trick in "Universal Bolero" */
				if (dat==pf->sngpos && pf->patbrk==pf->patpos) break;
				if ((!pf->loop)&&(!pf->patbrk)&&((dat<pf->sngpos)||
				   ((pf->sngpos==pf->numpos-1)&&(!pf->patbrk))||
				   ((dat==pf->sngpos)&&(pf->flags&UF_NOWRAP)))) {
					/* if we don't loop, better not to skip the end of the
					   pattern, after all... so:
					pf->patbrk=0; */
					pf->posjmp=3;
				} else {
					/* if we were fading, adjust... */
					if (pf->sngpos==(pf->numpos-1))
						pf->volume=pf->initvolume>128?128:pf->initvolume;
					pf->sngpos=dat;
					pf->posjmp=2;
					pf->patpos=0;
				}
				break;
			case UNI_PTEFFECTC:
				dat=UniGetByte();
				if (pf->vbtick) break;
				if (dat==(UBYTE)-1) a->anote=dat=0; /* note cut */
				else if (dat>64) dat=64;
				a->tmpvolume=dat;
				break;
			case UNI_PTEFFECTD:
				dat=UniGetByte();
				if ((pf->vbtick)||(pf->patdly2)) break;
				if ((pf->positions[pf->sngpos]!=255)&&
				   (dat>pf->pattrows[pf->positions[pf->sngpos]]))
					dat=pf->pattrows[pf->positions[pf->sngpos]];
				pf->patbrk=dat;
				if (!pf->posjmp) {
					/* don't ask me to explain this code - it makes
					   backwards.s3m and children.xm (heretic's version) play
					   correctly, among others. Take that for granted, or write
					   the page of comments yourself... you might need some
					   aspirin - Miod */
					if ((pf->sngpos==pf->numpos-1)&&(dat)&&((pf->loop)||
					               (pf->positions[pf->sngpos]==(pf->numpat-1)
									&& !(pf->flags&UF_NOWRAP)))) {
						pf->sngpos=0;
						pf->posjmp=2;
					} else
						pf->posjmp=3;
				}
				break;
			case UNI_PTEFFECTE:
				DoEEffects(UniGetByte());
				break;
			case UNI_PTEFFECTF:
				dat=UniGetByte();
				if (pf->vbtick||pf->patdly2) break;
				if (pf->extspd&&(dat>=0x20))
					pf->bpm=dat;
				else
				  if (dat) {
					pf->sngspd=(dat>32)?32:dat;
					pf->vbtick=0;
				}
				break;
			case UNI_S3MEFFECTA:
				DoS3MSpeed(UniGetByte());
				break;
			case UNI_S3MEFFECTD:
				DoS3MVolSlide(UniGetByte());
				break;
			case UNI_S3MEFFECTE:
				dat=UniGetByte();
				if (a->period)
					DoS3MSlideDn(dat);
				break;
			case UNI_S3MEFFECTF:
				dat=UniGetByte();
				if (a->period)
					DoS3MSlideUp(dat);
				break;
			case UNI_S3MEFFECTI:
				DoS3MTremor(UniGetByte());
				a->ownvol=1;
				break;
			case UNI_S3MEFFECTQ:
				dat=UniGetByte();
				if (a->period)
					DoS3MRetrig(dat);
				break;
			case UNI_S3MEFFECTR:
				dat=UniGetByte();
				if (!pf->vbtick) {
					if (dat&0x0f) a->trmdepth=dat&0xf;
					if (dat&0xf0) a->trmspd=(dat&0xf0)>>2;
				}
				DoS3MTremolo();
				a->ownvol=1;
				break;
			case UNI_S3MEFFECTT:
				DoS3MTempo(UniGetByte());
				break;
			case UNI_S3MEFFECTU:
				dat=UniGetByte();
				if (!pf->vbtick) {
					if (dat&0x0f) a->vibdepth=dat&0xf;
					if (dat&0xf0) a->vibspd=(dat&0xf0)>>2;
				} else
				if (a->period) {
					DoS3MFineVibrato();
					a->ownper=1;
				}
				break;
			case UNI_KEYOFF:
				a->keyoff|=KEY_OFF;
				if ((!(a->volflg&EF_ON))||(a->volflg&EF_LOOP))
					a->keyoff=KEY_KILL;
				break;
			case UNI_KEYFADE:
				dat=UniGetByte();
				if ((pf->vbtick>=dat)||(pf->vbtick==pf->sngspd-1)) {
					a->keyoff=KEY_KILL;
					if (!(a->volflg&EF_ON))
						a->fadevol=0;
				}
				break;
			case UNI_VOLEFFECTS:
				DoVolEffects(UniGetByte());
				break;
			case UNI_XMEFFECTA:
				DoXMVolSlide(UniGetByte());
				break;
			case UNI_XMEFFECTE1: /* XM fineslide up */
				dat=UniGetByte();
				if (!pf->vbtick) {
					if (dat) a->fportupspd=dat;
					if (a->period)
						a->tmpperiod-=(a->fportupspd<<2);
				}
				break;
			case UNI_XMEFFECTE2: /* XM fineslide dn */
				dat=UniGetByte();
				if (!pf->vbtick) {
					if (dat) a->fportdnspd=dat;
					if (a->period)
						a->tmpperiod+=(a->fportdnspd<<2);
				}
				break;
			case UNI_XMEFFECTEA: /* fine volume slide up */
				dat=UniGetByte();
				if (!pf->vbtick)
					if (dat) a->fslideupspd=dat;
				a->tmpvolume+=a->fslideupspd;
				if (a->tmpvolume>64) a->tmpvolume=64;
				break;
			case UNI_XMEFFECTEB: /* fine volume slide dn */
				dat=UniGetByte();
				if (!pf->vbtick)
					if (dat) a->fslidednspd=dat;
				a->tmpvolume-=a->fslidednspd;
				if (a->tmpvolume<0) a->tmpvolume=0;
				break;
			case UNI_XMEFFECTG:
				pf->volume=UniGetByte()<<1;
				if (pf->volume>128) pf->volume=128;
				break;
			case UNI_XMEFFECTH:
				DoXMGlobalSlide(UniGetByte());
				break;
			case UNI_XMEFFECTL:
				dat=UniGetByte();
				if ((!pf->vbtick)&&(a->i)) {
					UWORD points;
					INSTRUMENT *i=a->i;
					MP_VOICE *aout;

					if ((aout=a->slave)) {
						points=i->volenv[i->volpts-1].pos;
						aout->venv.p=aout->venv.env[(dat>points)?points:dat].pos;
						points=i->panenv[i->panpts-1].pos;
						aout->penv.p=aout->penv.env[(dat>points)?points:dat].pos;
					}
				}
				break;
			case UNI_XMEFFECTP:
				dat=UniGetByte();
				if (pf->panflag)
					DoXMPanSlide(dat);
				break;
			case UNI_XMEFFECTX1:
				dat=UniGetByte();
				if (dat) a->ffportupspd=dat;
				else dat=a->ffportupspd;
				if (a->period) {
					DoXMExtraFineSlideUp(dat);
					a->ownper=1;
				}
				break;
			case UNI_XMEFFECTX2:
				dat=UniGetByte();
				if (dat) a->ffportdnspd=dat;
				else dat=a->ffportdnspd;
				if (a->period) {
					DoXMExtraFineSlideDown(dat);
					a->ownper=1;
				}
				break;
			case UNI_ITEFFECTG:
				dat=UniGetByte();
				if (dat) {
					a->portspeed=dat;
				}
				if (a->period) {
					if ((!pf->vbtick)&&(a->newsamp)){
						a->kick=KICK_NOTE;
						a->start=-1;
					} else
						a->kick=(a->kick==KICK_NOTE)?KICK_ENV:KICK_ABSENT;
					DoITToneSlide();
					a->ownper=1;
				}
				break;
			case UNI_ITEFFECTH: /* IT vibrato */
				dat=UniGetByte();
				if (!pf->vbtick) {
					if (dat&0x0f) a->vibdepth=dat&0xf;
					if (dat&0xf0) a->vibspd=(dat&0xf0)>>2;
				}
				if (a->period) {
					DoITVibrato();
					a->ownper=1;
				}
				break;
			case UNI_ITEFFECTI: /* IT tremor */
				DoITTremor(UniGetByte());
				a->ownvol=1;
				break;
			case UNI_ITEFFECTM:
				a->chanvol=UniGetByte();
				if (a->chanvol>64) a->chanvol=64;
				else if (a->chanvol<0) a->chanvol=0;
				break;
			case UNI_ITEFFECTN: /* slide / fineslide channel volume */
				DoITChanVolSlide(UniGetByte());
				break;
			case UNI_ITEFFECTP:  /* slide / fineslide channel panning */
				dat=UniGetByte();
				if (pf->panflag)
					DoITPanSlide(dat);
				break;
			case UNI_ITEFFECTT: /* slide / fineslide tempo */
				DoITTempo(UniGetByte());
				break;
			case UNI_ITEFFECTU: /* fine vibrato */
				dat=UniGetByte();
				if (!pf->vbtick) {
					if (dat&0x0f) a->vibdepth=dat&0xf;
					if (dat&0xf0) a->vibspd=(dat&0xf0)>>2;
				}
				if (a->period) {
					DoITFineVibrato();
					a->ownper=1;
				}
				break;
			case UNI_ITEFFECTW: /* slide / fineslide global volume */
				DoITGlobalSlide(UniGetByte());
				break;
			case UNI_ITEFFECTY: /* panbrello */
				dat=UniGetByte();
				if (!pf->vbtick) {
					if (dat&0x0f) a->panbdepth=(dat&0xf);
					if (dat&0xf0) a->panbspd=(dat&0xf0)>>4;
				}
				if (pf->panflag) DoITPanbrello();
				break;
			case UNI_ITEFFECTS0:
				DoSSEffects(UniGetByte());
				break;
			case UNI_ITEFFECTZ:
				/* FIXME not yet implemented */
				UniSkipOpcode(UNI_ITEFFECTZ);
				break;
			case UNI_ULTEFFECT9:
				DoULTSampleOffset();
				break;
			case UNI_MEDSPEED:
				DoMEDSpeed();
				break;
			case UNI_MEDEFFECTF1:
				DoEEffects(0x90|(pf->sngspd/2));
				break;
			case UNI_MEDEFFECTF2:
				DoEEffects(0xd0|(pf->sngspd/2));
				break;
			case UNI_MEDEFFECTF3:
				DoEEffects(0x90|(pf->sngspd/3));
				break;
			case UNI_XMEFFECTZ:
				Player_SetSynchroValue(UniGetByte());
				break;
			default:
				a->sliding=oldsliding;
				UniSkipOpcode(c);
				break;
		}
	}
}

static void DoNNAEffects(UBYTE dat)
{
	int t;
	MP_VOICE *aout;

	dat&=0xf;
	aout=(a->slave)?a->slave:NULL;

	switch (dat) {
		case 0x0: /* past note cut */
			for (t=0;t<md_sngchn;t++)
				if (pf->voice[t].master==a)
					pf->voice[t].fadevol=0;
			break;
		case 0x1: /* past note off */
			for (t=0;t<md_sngchn;t++)
				if (pf->voice[t].master==a) {
					pf->voice[t].keyoff|=KEY_OFF;
					if ((!(pf->voice[t].venv.flg & EF_ON))||
					   (pf->voice[t].venv.flg & EF_LOOP))
						pf->voice[t].keyoff=KEY_KILL;
				}
			break;
		case 0x2: /* past note fade */
			for (t=0;t<md_sngchn;t++)
				if (pf->voice[t].master==a)
					pf->voice[t].keyoff|=KEY_FADE;
			break;
		case 0x3: /* set NNA note cut */
			a->nna=(a->nna&~NNA_MASK)|NNA_CUT;
			break;
		case 0x4: /* set NNA note continue */
			a->nna=(a->nna&~NNA_MASK)|NNA_CONTINUE;
			break;
		case 0x5: /* set NNA note off */
			a->nna=(a->nna&~NNA_MASK)|NNA_OFF;
			break;
		case 0x6: /* set NNA note fade */
			a->nna=(a->nna&~NNA_MASK)|NNA_FADE;
			break;
		case 0x7: /* disable volume envelope */
			if (aout)
				aout->volflg&=~EF_ON;
			break;
		case 0x8: /* enable volume envelope  */
			if (aout)
				aout->volflg|=EF_ON;
			break;
		case 0x9: /* disable panning envelope */
			if (aout)
				aout->panflg&=~EF_ON;
			break;
		case 0xa: /* enable panning envelope */
			if (aout)
				aout->panflg|=EF_ON;
			break;
		case 0xb: /* disable pitch envelope */
			if (aout)
				aout->pitflg&=~EF_ON;
			break;
		case 0xc: /* enable pitch envelope */
			if (aout)
				aout->pitflg|=EF_ON;
			break;
	}
}

void pt_UpdateVoices(int max_volume)
{
	SWORD envpan,envvol,envpit;
	UWORD playperiod;
	SLONG vibval,vibdpt;
	ULONG tmpvol;

	MP_VOICE *aout;
	INSTRUMENT *i;
	SAMPLE *s;

	pf->totalchn=pf->realchn=0;
	for (mp_channel=0;mp_channel<md_sngchn;mp_channel++) {
		aout=&pf->voice[mp_channel];
		i=aout->i;
		s=aout->s;

		if ((!s)||(!s->length)) continue;

		if (aout->period<40) aout->period=40;
		else if (aout->period>50000) aout->period=50000;

		if ((aout->kick==KICK_NOTE)||(aout->kick==KICK_KEYOFF)) {
			Voice_Play_internal(mp_channel,s,(aout->start==-1)?((s->flags&SF_UST_LOOP)?s->loopstart:0):aout->start);
			aout->fadevol=32768;
			aout->aswppos=0;
		}

		if (i && ((aout->kick==KICK_NOTE)||(aout->kick==KICK_ENV))) {
			StartEnvelope(&aout->venv,aout->volflg,i->volpts,i->volsusbeg,
			  i->volsusend,i->volbeg,i->volend,i->volenv,aout->keyoff);
			StartEnvelope(&aout->penv,aout->panflg,i->panpts,i->pansusbeg,
			  i->pansusend,i->panbeg,i->panend,i->panenv,aout->keyoff);
			StartEnvelope(&aout->cenv,aout->pitflg,i->pitpts,i->pitsusbeg,
			  i->pitsusend,i->pitbeg,i->pitend,i->pitenv,aout->keyoff);
			if (aout->cenv.flg&EF_ON)
				aout->masterperiod=GetPeriod((UWORD)aout->note<<1,aout->master->speed);
		}
		aout->kick=KICK_ABSENT;

		envvol=(!(aout->volflg & EF_ON))?256:
		        ProcessEnvelope(&aout->venv,256,aout->keyoff);
		envpan=(!(aout->panflg & EF_ON))?PAN_CENTER:
		        ProcessEnvelope(&aout->penv,PAN_CENTER,aout->keyoff);
		envpit=(!(aout->pitflg & EF_ON))?32:
		        ProcessEnvelope(&aout->cenv,32,aout->keyoff);

		tmpvol=aout->fadevol;    /* max 32768 */
		tmpvol*=aout->chanvol;    /* * max 64 */
		tmpvol*=aout->volume;     /* * max 256 */
		tmpvol/=16384L;           /* tmpvol is max 32768 */
		aout->totalvol=tmpvol>>2; /* totalvolume used to determine samplevolume */
		tmpvol*=envvol;           /* * max 256 */
		tmpvol*=pf->volume;       /* * max 128 */
		tmpvol/=4194304UL;
               tmpvol*=pf->initvolume;   /* * max 128 */
               tmpvol/=128;

		/* fade out */
		if (pf->sngpos>=pf->numpos) tmpvol=0;
		else
			tmpvol=(tmpvol*max_volume)/128;

		if ((aout->masterchn!=-1)&& pf->control[aout->masterchn].muted)
			Voice_SetVolume_internal(mp_channel,0);
		else {
			Voice_SetVolume_internal(mp_channel,tmpvol);
			if ((tmpvol)&&(aout->master)&&(aout->master->slave==aout))
				pf->realchn++;
			pf->totalchn++;
		}

		if (aout->panning==PAN_SURROUND)
			Voice_SetPanning_internal(mp_channel,PAN_SURROUND);
		else
			if ((pf->panflag)&&(aout->penv.flg & EF_ON))
				Voice_SetPanning_internal(mp_channel,DoPan(envpan,aout->panning));
			else
				Voice_SetPanning_internal(mp_channel,aout->panning);

		if (aout->period && s->vibdepth)
			switch (s->vibtype) {
				case 0:
					vibval=avibtab[s->avibpos&127];
					if (s->avibpos & 0x80) vibval=-vibval;
					break;
				case 1:
					vibval=64;
					if (s->avibpos & 0x80) vibval=-vibval;
					break;
				case 2:
					vibval=63-(((s->avibpos+128)&255)>>1);
					break;
				default:
					vibval=(((s->avibpos+128)&255)>>1)-64;
					break;
			}
		else
			vibval=0;

		if (s->vibflags & AV_IT) {
			if ((aout->aswppos>>8)<s->vibdepth) {
				aout->aswppos += s->vibsweep;
				vibdpt=aout->aswppos;
			} else
				vibdpt=s->vibdepth<<8;
			vibval=(vibval*vibdpt)>>16;
			if (aout->mflag) {
				if (!(pf->flags&UF_LINEAR)) vibval>>=1;
				aout->period-=vibval;
			}
		} else {
			/* do XM style auto-vibrato */
			if (!(aout->keyoff & KEY_OFF)) {
				if (aout->aswppos<s->vibsweep) {
					vibdpt=(aout->aswppos*s->vibdepth)/s->vibsweep;
					aout->aswppos++;
				} else
					vibdpt=s->vibdepth;
			} else {
				/* keyoff -> depth becomes 0 if final depth wasn't reached or
				   stays at final level if depth WAS reached */
				if (aout->aswppos>=s->vibsweep)
					vibdpt=s->vibdepth;
				else
					vibdpt=0;
			}
			vibval=(vibval*vibdpt)>>8;
			aout->period-=vibval;
		}

		/* update vibrato position */
		s->avibpos=(s->avibpos+s->vibrate)&0xff;

		/* process pitch envelope */
		playperiod=aout->period;

		if ((aout->pitflg&EF_ON)&&(envpit!=32)) {
			long p1;

			envpit-=32;
			if ((aout->note<<1)+envpit<=0) envpit=-(aout->note<<1);

			p1=GetPeriod(((UWORD)aout->note<<1)+envpit,aout->master->speed)-aout->masterperiod;
			if (p1>0) {
				if ((UWORD)(playperiod+p1)<=playperiod) {
					p1=0;
					aout->keyoff|=KEY_OFF;
				}
			} else if (p1<0) {
				if ((UWORD)(playperiod+p1)>=playperiod) {
					p1=0;
					aout->keyoff|=KEY_OFF;
				}
			}
			playperiod+=p1;
		}

		if (!aout->fadevol) { /* check for a dead note (fadevol=0) */
			Voice_Stop_internal(mp_channel);
			pf->totalchn--;
			if ((tmpvol)&&(aout->master)&&(aout->master->slave==aout))
				pf->realchn--;
		} else {
			Voice_SetFrequency_internal(mp_channel,
			                            getfrequency(pf->flags,playperiod));

			/* if keyfade, start substracting fadeoutspeed from fadevol: */
			if ((i)&&(aout->keyoff&KEY_FADE)) {
				if (aout->fadevol>=i->volfade)
					aout->fadevol-=i->volfade;
				else
					aout->fadevol=0;
			}
		}

		md_bpm=pf->bpm+pf->relspd;
		if (md_bpm<32) md_bpm=32;
		else if (md_bpm>255) md_bpm=255;
	}
}

/* Handles new notes or instruments */
void pt_Notes(void)
{
	UBYTE c,inst;
	int tr,funky; /* funky is set to indicate note or instrument change */

	for (mp_channel=0;mp_channel<pf->numchn;mp_channel++) {
		a=&pf->control[mp_channel];

		if (pf->sngpos>=pf->numpos) {
			tr=pf->numtrk;
			pf->numrow=0;
		} else {
			tr=pf->patterns[(pf->positions[pf->sngpos]*pf->numchn)+mp_channel];
			pf->numrow=pf->pattrows[pf->positions[pf->sngpos]];
		}

		a->row=(tr<pf->numtrk)?UniFindRow(pf->tracks[tr],pf->patpos):NULL;
		a->newsamp=0;
		if (!pf->vbtick) a->notedelay=0;

		if (!a->row) continue;
		UniSetRow(a->row);
		funky=0;

		while((c=UniGetByte()))
			switch (c) {
				case UNI_NOTE:
					funky|=1;
					a->oldnote=a->anote,a->anote=UniGetByte();
					a->kick =KICK_NOTE;
					a->start=-1;
					a->sliding=0;

					/* retrig tremolo and vibrato waves ? */
					if (!(a->wavecontrol & 0x80)) a->trmpos=0;
					if (!(a->wavecontrol & 0x08)) a->vibpos=0;
					if (!a->panbwave) a->panbpos=0;
					break;
				case UNI_INSTRUMENT:
					inst=UniGetByte();
					if (inst>=pf->numins) break; /* safety valve */
					funky|=2;
					a->i=(pf->flags & UF_INST)?&pf->instruments[inst]:NULL;
					a->retrig=0;
					a->s3mtremor=0;
					a->sample=inst;
					break;
				default:
					UniSkipOpcode(c);
					break;
			}

		if (funky) {
			INSTRUMENT *i;
			SAMPLE *s;

			i=a->i;
			if (i) {
				if (i->samplenumber[a->anote] >= pf->numsmp) continue;
				s=&pf->samples[i->samplenumber[a->anote]];
				a->note=i->samplenote[a->anote];
			} else {
				a->note=a->anote;
				s=&pf->samples[a->sample];
			}

			if (a->s!=s) {
				a->s=s;
				a->newsamp=a->period;
			}

			/* channel or instrument determined panning ? */
			a->panning=pf->panning[mp_channel];
			if (s->flags & SF_OWNPAN)
				a->panning=s->panning;
			else if ((i)&&(i->flags & IF_OWNPAN))
				a->panning=i->panning;

			a->handle=s->handle;
			a->speed=s->speed;

			if (i) {
				if ((pf->panflag)&&(i->flags & IF_PITCHPAN)
				   &&(a->panning!=PAN_SURROUND)){
					a->panning+=((a->anote-i->pitpancenter)*i->pitpansep)/8;
					if (a->panning<PAN_LEFT) a->panning=PAN_LEFT;
					else if (a->panning>PAN_RIGHT) a->panning=PAN_RIGHT;
				}
				a->pitflg=i->pitflg;
				a->volflg=i->volflg;
				a->panflg=i->panflg;
				a->nna=i->nnatype;
				a->dca=i->dca;
				a->dct=i->dct;
			} else {
				a->pitflg=0;
				a->volflg=0;
				a->panflg=0;
				a->nna=0;
				a->dca=0;
				a->dct=0;
			}

			if (funky&2) /* instrument change */ {
				/* IT random volume variations: 0:8 bit fixed, and one bit for
				   sign. */
				a->volume=a->tmpvolume=s->volume;
				if ((s)&&(i)) {
					if (i->rvolvar) {
						a->volume=a->tmpvolume=s->volume+
						  ((s->volume*((SLONG)i->rvolvar*(SLONG)getrandom(512)
						  ))/25600);
						if (a->volume<0) a->volume=a->tmpvolume=0;
						else if (a->volume>64) a->volume=a->tmpvolume=64;
					}
					if ((pf->panflag)&&(a->panning!=PAN_SURROUND)) {
						a->panning+=((a->panning*((SLONG)i->rpanvar*
						  (SLONG)getrandom(512)))/25600);
						if (a->panning<PAN_LEFT) a->panning=PAN_LEFT;
						else if (a->panning>PAN_RIGHT) a->panning=PAN_RIGHT;
					}
				}
			}

			a->wantedperiod=a->tmpperiod=GetPeriod((UWORD)a->note<<1,a->speed);
			a->keyoff=KEY_KICK;
		}
	}
}

/* Handles effects */
void pt_EffectsPass1(void)
{
	MP_VOICE *aout;

	for (mp_channel=0;mp_channel<pf->numchn;mp_channel++) {
		a=&pf->control[mp_channel];

		if ((aout=a->slave)) {
			a->fadevol=aout->fadevol;
			a->period=aout->period;
			if (a->kick==KICK_KEYOFF) a->keyoff=aout->keyoff;
		}

		if (!a->row) continue;
		UniSetRow(a->row);

		a->ownper=a->ownvol=0;
		explicitslides=0;
		pt_playeffects();

		/* continue volume slide if necessary for XM and IT */
		if (pf->flags&UF_BGSLIDES) {
			if (!explicitslides)
				switch (a->sliding) {
					case 1:
						DoS3MVolSlide(0);
						break;
					case 2:
						DoXMVolSlide(0);
						break;
				}
			else if (a->tmpvolume) a->sliding=explicitslides;
		}

		if (!a->ownper) a->period=a->tmpperiod;
		if (!a->ownvol) a->volume=a->tmpvolume;

		if (a->s) {
			if (a->i)
				a->outvolume=(a->volume*a->s->globvol*a->i->globvol)>>10;
			else
				a->outvolume=(a->volume*a->s->globvol)>>4;
			if (a->outvolume>256)a->volume=256;
			else if (a->outvolume<0) a->outvolume=0;
		}
	}
}

/* NNA management */
void pt_NNA(void)
{
	for (mp_channel=0;mp_channel<pf->numchn;mp_channel++) {
		a=&pf->control[mp_channel];

		if (a->kick==KICK_NOTE) {
			BOOL k=0;

			if (a->slave) {
				MP_VOICE *aout;

				aout=a->slave;
				if (aout->nna & NNA_MASK) {
					/* Make sure the old MP_VOICE channel knows it has no
					   master now ! */
					a->slave=NULL;
					/* assume the channel is taken by NNA */
					aout->mflag=0;

					switch (aout->nna) {
						case NNA_CONTINUE: /* continue note, do nothing */
							break;
						case NNA_OFF: /* note off */
							aout->keyoff|=KEY_OFF;
							if ((!(aout->volflg & EF_ON))||(aout->volflg & EF_LOOP))
								aout->keyoff=KEY_KILL;
							break;
						case NNA_FADE:
							aout->keyoff |= KEY_FADE;
							break;
					}
				}
			}

			if (a->dct!=DCT_OFF) {
				int t;

				for (t=0;t<md_sngchn;t++)
					if ((!Voice_Stopped_internal(t))&&
					   (pf->voice[t].masterchn==mp_channel)&&
					   (a->sample==pf->voice[t].sample)) {
						k=0;
						switch (a->dct) {
							case DCT_NOTE:
								if (a->note==pf->voice[t].note)
									k=1;
								break;
							case DCT_SAMPLE:
								if (a->handle==pf->voice[t].handle)
									k=1;
								break;
							case DCT_INST:
								k=1;
								break;
						}
						if (k)
							switch (a->dca) {
								case DCA_CUT:
									pf->voice[t].fadevol=0;
									break;
								case DCA_OFF:
									pf->voice[t].keyoff|=KEY_OFF;
									if ((!(pf->voice[t].volflg&EF_ON))||
									    (pf->voice[t].volflg&EF_LOOP))
										pf->voice[t].keyoff=KEY_KILL;
									break;
								case DCA_FADE:
									pf->voice[t].keyoff|=KEY_FADE;
									break;
							}
					}
			}
		} /* if (a->kick==KICK_NOTE) */
	}
}

/* Setup module and NNA voices */
void pt_SetupVoices(void)
{
	MP_VOICE *aout;

	for (mp_channel=0;mp_channel<pf->numchn;mp_channel++) {
		a=&pf->control[mp_channel];

		if (a->notedelay) continue;
		if (a->kick==KICK_NOTE) {
			/* if no channel was cut above, find an empty or quiet channel
			   here */
			if (pf->flags&UF_NNA) {
				if (!a->slave) {
					int newchn;

					if ((newchn=MP_FindEmptyChannel())!=-1)
						a->slave=&pf->voice[a->slavechn=newchn];
				}
			} else
				a->slave=&pf->voice[a->slavechn=mp_channel];

			/* assign parts of MP_VOICE only done for a KICK ! */
			if ((aout=a->slave)) {
				if (aout->mflag && aout->master) aout->master->slave=NULL;
				a->slave=aout;
				aout->master=a;
				aout->masterchn=mp_channel;
				aout->mflag=1;
			}
		} else
			aout=a->slave;

		if (aout) {
			aout->kick=a->kick;
			aout->i=a->i;
			aout->s=a->s;
			aout->sample=a->sample;
			aout->handle=a->handle;
			aout->period=a->period;
			aout->panning=a->panning;
			aout->chanvol=a->chanvol;
			aout->fadevol=a->fadevol;
			aout->start=a->start;
			aout->volflg=a->volflg;
			aout->panflg=a->panflg;
			aout->pitflg=a->pitflg;
			aout->volume=a->outvolume;
			aout->keyoff=a->keyoff;
			aout->note=a->note;
			aout->nna=a->nna;
		}
		a->kick=KICK_ABSENT;
	}
}

/* second effect pass */
void pt_EffectsPass2(void)
{
	UBYTE c;

	for (mp_channel=0;mp_channel<pf->numchn;mp_channel++) {
		a=&pf->control[mp_channel];

		if (!a->row) continue;
		UniSetRow(a->row);

		while((c=UniGetByte()))
			if (c==UNI_ITEFFECTS0) {
				c=UniGetByte();
				if ((c>>4)==SS_S7EFFECTS)
					DoNNAEffects(c&0xf);
			} else
				UniSkipOpcode(c);
	}
}

void Player_HandleTick(void)
{
	int max_volume;

#if 0
	/* don't handle the very first ticks, this allows the other hardware to
	   settle down so we don't loose any starting notes */
	if (isfirst) {
		isfirst--;
		return;
	}
#endif

	if ((!pf)||(pf->forbid)||(pf->sngpos>=pf->numpos)) return;

	/* update time counter (sngtime is in milliseconds (in fact 2^-10)) */
	pf->sngremainder+=(1<<9)*5; /* thus 2.5*(1<<10), since fps=0.4xtempo */
	pf->sngtime+=pf->sngremainder/pf->bpm;
	pf->sngremainder%=pf->bpm;

	if (++pf->vbtick>=pf->sngspd) {
		if (pf->pat_repcrazy)
			pf->pat_repcrazy=0; /* play 2 times row 0 */
		else
			pf->patpos++;
		pf->vbtick=0;

		/* process pattern-delay. pf->patdly2 is the counter and pf->patdly is
		   the command memory. */
		if (pf->patdly)
			pf->patdly2=pf->patdly,pf->patdly=0;
		if (pf->patdly2) {
			/* patterndelay active */
			if (--pf->patdly2)
				/* so turn back pf->patpos by 1 */
				if (pf->patpos) pf->patpos--;
		}

		/* do we have to get a new patternpointer ? (when pf->patpos reaches the
		   pattern size, or when a patternbreak is active) */
		if (((pf->patpos>=pf->numrow)&&(pf->numrow>0))&&(!pf->posjmp))
			pf->posjmp=3;

		if (pf->posjmp) {
			pf->patpos=pf->numrow?(pf->patbrk%pf->numrow):0;
			pf->pat_repcrazy=0;
			pf->sngpos+=(pf->posjmp-2);
			for (mp_channel=0;mp_channel<pf->numchn;mp_channel++)
				pf->control[mp_channel].pat_reppos=-1;

			pf->patbrk=pf->posjmp=0;
			/* handle the "---" (end of song) pattern since it can occur
			   *inside* the module in .IT and .S3M */
			if ((pf->sngpos>=pf->numpos)||(pf->positions[pf->sngpos]==255)) {
				if (!pf->wrap) return;
				if (!(pf->sngpos=pf->reppos)) {
				    pf->volume=pf->initvolume>128?128:pf->initvolume;
					pf->sngspd=pf->initspeed?(pf->initspeed<32?pf->initspeed:32):6;
					pf->bpm=pf->inittempo<32?32:pf->inittempo;
				}
			}
			if (pf->sngpos<0) pf->sngpos=pf->numpos-1;
		}

		if (!pf->patdly2)
			pt_Notes();
	}

	if (((pf->sngpos==pf->numpos-1)||(pf->positions[pf->sngpos+1]==255))&&
	    (pf->fadeout))
		max_volume=pf->numrow?((pf->numrow-pf->patpos)*128)/pf->numrow:0;
	else
		max_volume=128;

	pt_EffectsPass1();
	if (pf->flags&UF_NNA)
		pt_NNA();
	pt_SetupVoices();
	pt_EffectsPass2();

	/* now set up the actual hardware channel playback information */
	pt_UpdateVoices(max_volume);
}

static void Player_Init_internal(MODULE* mf)
{
	int t;

	for (t=0;t<mf->numchn;t++) {
		mf->control[t].chanvol=mf->chanvol[t];
		mf->control[t].panning=mf->panning[t];
	}

	mf->sngtime=0;
	mf->sngremainder=0;

	mf->pat_repcrazy=0;
	mf->sngpos=0;
	mf->sngspd=mf->initspeed?(mf->initspeed<32?mf->initspeed:32):6;
	mf->volume=mf->initvolume>128?128:mf->initvolume;

	mf->vbtick=mf->sngspd;
	mf->patdly=0;
	mf->patdly2=0;
	mf->bpm=mf->inittempo<32?32:mf->inittempo;
	mf->realchn=0;

	mf->patpos=0;
	mf->posjmp=2; /* make sure the player fetches the first note */
	mf->numrow=-1;
	mf->patbrk=0;
}

BOOL Player_Init(MODULE* mf)
{
	mf->extspd=1;
	mf->panflag=1;
	mf->wrap=0;
	mf->loop=1;
	mf->fadeout=0;

	mf->relspd=0;

	/* make sure the player doesn't start with garbage */
	if (!(mf->control=(MP_CONTROL*)_mm_calloc(mf->numchn,sizeof(MP_CONTROL))))
		return 1;
	if (!(mf->voice=(MP_VOICE*)_mm_calloc(md_sngchn,sizeof(MP_VOICE))))
		return 1;

	Player_Init_internal(mf);
	return 0;
}

void Player_Exit_internal(MODULE* mf)
{
	if (!mf) return;
	if (mf==pf) {
		Player_Stop_internal();
		pf=NULL;
	}
	if (mf->control) free(mf->control);
	if (mf->voice) free(mf->voice);
	mf->control=NULL;
	mf->voice=NULL;
}

void Player_Exit(MODULE* mf)
{
	MUTEX_LOCK(vars);
	Player_Exit_internal(mf);
	MUTEX_UNLOCK(vars);
}

void Player_SetVolume(SWORD volume)
{
	MUTEX_LOCK(vars);
	if (pf)
		pf->volume=pf->initvolume=(volume<0)?0:(volume>128)?128:volume;
	MUTEX_UNLOCK(vars);
}

MODULE* Player_GetModule(void)
{
	MODULE* result;

	MUTEX_LOCK(vars);
	result=pf;
	MUTEX_UNLOCK(vars);

	return result;
}

void Player_Start(MODULE *mf)
{
	int t;

	if (!mf) return;

	if (!MikMod_Active())
		MikMod_EnableOutput();

	mf->forbid=0;

	MUTEX_LOCK(vars);
	if (pf!=mf) {
		/* new song is being started, so completely stop out the old one. */
		if (pf) pf->forbid=1;
		for (t=0;t<md_sngchn;t++) Voice_Stop_internal(t);
	}
	pf=mf;
	MUTEX_UNLOCK(vars);
}

void Player_Stop_internal(void)
{
	if (!md_sfxchn) MikMod_DisableOutput_internal();
	if (pf) pf->forbid=1;
	pf=NULL;
}

void Player_Stop(void)
{
	MUTEX_LOCK(vars);
		Player_Stop_internal();
	MUTEX_UNLOCK(vars);
}

BOOL Player_Active(void)
{
	BOOL result=0;

	MUTEX_LOCK(vars);
	if (pf)
		result=(!(pf->sngpos>=pf->numpos));
	MUTEX_UNLOCK(vars);

	return result;
}

void Player_NextPosition(void)
{
	MUTEX_LOCK(vars);
	if (pf) {
		int t;

		pf->forbid=1;
		pf->posjmp=3;
		pf->patbrk=0;
		pf->vbtick=pf->sngspd;

		for (t=0;t<md_sngchn;t++) {
			Voice_Stop_internal(t);
			pf->voice[t].i=NULL;
			pf->voice[t].s=NULL;
		}
		for (t=0;t<pf->numchn;t++) {
			pf->control[t].i=NULL;
			pf->control[t].s=NULL;
		}
		pf->forbid=0;
	}
	MUTEX_UNLOCK(vars);
}

void Player_PrevPosition(void)
{
	MUTEX_LOCK(vars);
	if (pf) {
		int t;

		pf->forbid=1;
		pf->posjmp=1;
		pf->patbrk=0;
		pf->vbtick=pf->sngspd;

		for (t=0;t<md_sngchn;t++) {
			Voice_Stop_internal(t);
			pf->voice[t].i=NULL;
			pf->voice[t].s=NULL;
		}
		for (t=0;t<pf->numchn;t++) {
			pf->control[t].i=NULL;
			pf->control[t].s=NULL;
		}
		pf->forbid=0;
	}
	MUTEX_UNLOCK(vars);
}

void Player_SetPosition(UWORD pos)
{
	MUTEX_LOCK(vars);
	if (pf) {
		int t;

		pf->forbid=1;
		if (pos>=pf->numpos) pos=pf->numpos;
		pf->posjmp=2;
		pf->patbrk=0;
		pf->sngpos=pos;
		pf->vbtick=pf->sngspd;

		for (t=0;t<md_sngchn;t++) {
			Voice_Stop_internal(t);
			pf->voice[t].i=NULL;
			pf->voice[t].s=NULL;
		}
		for (t=0;t<pf->numchn;t++) {
			pf->control[t].i=NULL;
			pf->control[t].s=NULL;
		}
		pf->forbid=0;

		if (!pos)
			Player_Init_internal(pf);
	}
	MUTEX_UNLOCK(vars);
}

static void Player_Unmute_internal(SLONG arg1,va_list ap)
{
	SLONG t,arg2,arg3=0;

	if (pf) {
		switch (arg1) {
			case MUTE_INCLUSIVE:
				if (((!(arg2=va_arg(ap,SLONG)))&&(!(arg3=va_arg(ap,SLONG))))||
				   (arg2>arg3)||(arg3>=pf->numchn))
					return;
				for (;arg2<pf->numchn && arg2<=arg3;arg2++)
					pf->control[arg2].muted=0;
				break;
			case MUTE_EXCLUSIVE:
				if (((!(arg2=va_arg(ap,SLONG)))&&(!(arg3=va_arg(ap,SLONG))))||
				   (arg2>arg3)||(arg3>=pf->numchn))
					return;
				for (t=0;t<pf->numchn;t++) {
					if ((t>=arg2) && (t<=arg3)) continue;
					pf->control[t].muted=0;
				}
				break;
			default:
				if (arg1<pf->numchn) pf->control[arg1].muted=0;
				break;
		}
	}
}

void Player_Unmute(SLONG arg1, ...)
{
	va_list args;

	va_start(args,arg1);
	MUTEX_LOCK(vars);
	Player_Unmute_internal(arg1,args);
	MUTEX_UNLOCK(vars);
	va_end(args);
}

static void Player_Mute_internal(SLONG arg1,va_list ap)
{
	SLONG t,arg2,arg3=0;

	if (pf) {
		switch (arg1) {
			case MUTE_INCLUSIVE:
				if (((!(arg2=va_arg(ap,SLONG)))&&(!(arg3=va_arg(ap,SLONG))))||
				    (arg2>arg3)||(arg3>=pf->numchn))
					return;
				for (;arg2<pf->numchn && arg2<=arg3;arg2++)
					pf->control[arg2].muted=1;
				break;
			case MUTE_EXCLUSIVE:
				if (((!(arg2=va_arg(ap,SLONG)))&&(!(arg3=va_arg(ap,SLONG))))||
				    (arg2>arg3)||(arg3>=pf->numchn))
					return;
				for (t=0;t<pf->numchn;t++) {
					if ((t>=arg2) && (t<=arg3)) continue;
					pf->control[t].muted=1;
				}
				break;
			default:
				if (arg1<pf->numchn)
					pf->control[arg1].muted=1;
				break;
		}
	}
}

void Player_Mute(SLONG arg1,...)
{
	va_list args;

	va_start(args,arg1);
	MUTEX_LOCK(vars);
	Player_Mute_internal(arg1,args);
	MUTEX_UNLOCK(vars);
	va_end(args);
}

static void Player_ToggleMute_internal(SLONG arg1,va_list ap)
{
	SLONG arg2,arg3=0;
	ULONG t;

	if (pf) {
		switch (arg1) {
			case MUTE_INCLUSIVE:
				if (((!(arg2=va_arg(ap,SLONG)))&&(!(arg3=va_arg(ap,SLONG))))||
				    (arg2>arg3)||(arg3>=pf->numchn))
					return;
				for (;arg2<pf->numchn && arg2<=arg3;arg2++)
					pf->control[arg2].muted=1-pf->control[arg2].muted;
				break;
			case MUTE_EXCLUSIVE:
				if (((!(arg2=va_arg(ap,SLONG)))&&(!(arg3=va_arg(ap,SLONG))))||
				    (arg2>arg3)||(arg3>=pf->numchn))
					return;
				for (t=0;t<pf->numchn;t++) {
					if ((t>=arg2) && (t<=arg3)) continue;
					pf->control[t].muted=1-pf->control[t].muted;
				}
				break;
			default:
				if (arg1<pf->numchn)
					pf->control[arg1].muted=1-pf->control[arg1].muted;
				break;
		}
	}
}

void Player_ToggleMute(SLONG arg1,...)
{
	va_list args;

	va_start(args,arg1);
	MUTEX_LOCK(vars);
	Player_ToggleMute_internal(arg1,args);
	MUTEX_UNLOCK(vars);
	va_end(args);
}

BOOL Player_Muted(UBYTE chan)
{
	BOOL result=1;

	MUTEX_LOCK(vars);
	if (pf)
		result=(chan<pf->numchn)?pf->control[chan].muted:1;
	MUTEX_UNLOCK(vars);

	return result;
}

int Player_GetChannelVoice(UBYTE chan)
{
	int result=0;

	MUTEX_LOCK(vars);
	if (pf)
		result=(chan<pf->numchn)?pf->control[chan].slavechn:-1;
	MUTEX_UNLOCK(vars);

	return result;
}

UWORD Player_GetChannelPeriod(UBYTE chan)
{
	UWORD result=0;

	MUTEX_LOCK(vars);
    if (pf)
	    result=(chan<pf->numchn)?pf->control[chan].period:0;
	MUTEX_UNLOCK(vars);

	return result;
}

BOOL Player_Paused_internal(void)
{
	return pf?pf->forbid:1;
}

BOOL Player_Paused(void)
{
	BOOL result;

	MUTEX_LOCK(vars);
	result=Player_Paused_internal();
	MUTEX_UNLOCK(vars);

	return result;
}

void Player_TogglePause(void)
{
	MUTEX_LOCK(vars);
	if (pf)
		pf->forbid=1-pf->forbid;
	MUTEX_UNLOCK(vars);
}

void Player_SetSpeed(UWORD speed)
{
	MUTEX_LOCK(vars);
	if (pf)
		pf->sngspd=speed?(speed<32?speed:32):1;
	MUTEX_UNLOCK(vars);
}

void Player_SetTempo(UWORD tempo)
{
	if (tempo<32) tempo=32;

	MUTEX_LOCK(vars);
	if (pf) {
		if ((!(pf->flags&UF_HIGHBPM))&&(tempo>255)) tempo=255;
		pf->bpm=tempo;
	}
	MUTEX_UNLOCK(vars);
}

static int _pl_synchro_value;
void Player_SetSynchroValue(int i)
{
	_pl_synchro_value = i;
}

int Player_GetSynchroValue(void)
{
	return _pl_synchro_value;
}
	

/* ex:set ts=4: */
