/*
    SDL_mixer:  An audio mixer library based on the SDL library
    Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    This file by Ryan C. Gordon (icculus@linuxgames.com)

    These are some internally supported special effects that use SDL_mixer's
    effect callback API. They are meant for speed over quality.  :)
*/

/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_endian.h"

#define __MIX_INTERNAL_EFFECT__
#include "effects_internal.h"

/* profile code:
    #include <sys/time.h>
    #include <unistd.h>
    struct timeval tv1;
    struct timeval tv2;
    
    gettimeofday(&tv1, NULL);

        ... do your thing here ...

    gettimeofday(&tv2, NULL);
    printf("%ld\n", tv2.tv_usec - tv1.tv_usec);
*/


/*
 * Positional effects...panning, distance attenuation, etc.
 */

typedef struct _Eff_positionargs
{
    volatile float left_f;
    volatile float right_f;
    volatile Uint8 left_u8;
    volatile Uint8 right_u8;
    volatile float distance_f;
    volatile Uint8 distance_u8;
    volatile int in_use;
    volatile int channels;
} position_args;

static position_args **pos_args_array = NULL;
static position_args *pos_args_global = NULL;
static int position_channels = 0;

/* This just frees up the callback-specific data. */
static void _Eff_PositionDone(int channel, void *udata)
{
    if (channel < 0) {
        if (pos_args_global != NULL) {
            free(pos_args_global);
            pos_args_global = NULL;
        }
    }

    else if (pos_args_array[channel] != NULL) {
        free(pos_args_array[channel]);
        pos_args_array[channel] = NULL;
    }
}


static void _Eff_position_u8(int chan, void *stream, int len, void *udata)
{
    volatile position_args *args = (volatile position_args *) udata;
    Uint8 *ptr = (Uint8 *) stream;
    int i;

        /*
         * if there's only a mono channnel (the only way we wouldn't have
         *  a len divisible by 2 here), then left_f and right_f are always
         *  1.0, and are therefore throwaways.
         */
    if (len % sizeof (Uint16) != 0) {
        *(ptr++) = (Uint8) (((float) *ptr) * args->distance_f);
        len--;
    }

    for (i = 0; i < len; i += sizeof (Uint8) * 2) {
        *(ptr++) = (Uint8)((((float) *ptr) * args->left_f) * args->distance_f);
        *(ptr++) = (Uint8)((((float) *ptr) * args->right_f) * args->distance_f);
    }
}


/*
 * This one runs about 10.1 times faster than the non-table version, with
 *  no loss in quality. It does, however, require 64k of memory for the
 *  lookup table. Also, this will only update position information once per
 *  call; the non-table version always checks the arguments for each sample,
 *  in case the user has called Mix_SetPanning() or whatnot again while this
 *  callback is running.
 */
static void _Eff_position_table_u8(int chan, void *stream, int len, void *udata)
{
    volatile position_args *args = (volatile position_args *) udata;
    Uint8 *ptr = (Uint8 *) stream;
    Uint32 *p;
    int i;
    Uint8 *l = ((Uint8 *) _Eff_volume_table) + (256 * args->left_u8);
    Uint8 *r = ((Uint8 *) _Eff_volume_table) + (256 * args->right_u8);
    Uint8 *d = ((Uint8 *) _Eff_volume_table) + (256 * args->distance_u8);

        /*
         * if there's only a mono channnel, then l[] and r[] are always
         *  volume 255, and are therefore throwaways. Still, we have to
         *  be sure not to overrun the audio buffer...
         */
    while (len % sizeof (Uint32) != 0) {
        *(ptr++) = d[l[*ptr]];
        if (args->channels == 2)
            *(ptr++) = d[r[*ptr]];
        len -= args->channels;
    }

    p = (Uint32 *) ptr;

    for (i = 0; i < len; i += sizeof (Uint32)) {
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        *(p++) = (d[l[(*p & 0xFF000000) >> 24]] << 24) |
                 (d[r[(*p & 0x00FF0000) >> 16]] << 16) |
                 (d[l[(*p & 0x0000FF00) >>  8]] <<  8) |
                 (d[r[(*p & 0x000000FF)      ]]      ) ;
#else
        *(p++) = (d[r[(*p & 0xFF000000) >> 24]] << 24) |
                 (d[l[(*p & 0x00FF0000) >> 16]] << 16) |
                 (d[r[(*p & 0x0000FF00) >>  8]] <<  8) |
                 (d[l[(*p & 0x000000FF)      ]]      ) ;
#endif
    }
}


static void _Eff_position_s8(int chan, void *stream, int len, void *udata)
{
    volatile position_args *args = (volatile position_args *) udata;
    Sint8 *ptr = (Sint8 *) stream;
    int i;

        /*
         * if there's only a mono channnel (the only way we wouldn't have
         *  a len divisible by 2 here), then left_f and right_f are always
         *  1.0, and are therefore throwaways.
         */
    if (len % sizeof (Sint16) != 0) {
        *(ptr++) = (Sint8) (((float) *ptr) * args->distance_f);
        len--;
    }

    for (i = 0; i < len; i += sizeof (Sint8) * 2) {
        *(ptr++) = (Sint8)((((float) *ptr) * args->left_f) * args->distance_f);
        *(ptr++) = (Sint8)((((float) *ptr) * args->right_f) * args->distance_f);
    }
}


/*
 * This one runs about 10.1 times faster than the non-table version, with
 *  no loss in quality. It does, however, require 64k of memory for the
 *  lookup table. Also, this will only update position information once per
 *  call; the non-table version always checks the arguments for each sample,
 *  in case the user has called Mix_SetPanning() or whatnot again while this
 *  callback is running.
 */
static void _Eff_position_table_s8(int chan, void *stream, int len, void *udata)
{
    volatile position_args *args = (volatile position_args *) udata;
    Sint8 *ptr = (Sint8 *) stream;
    Uint32 *p;
    int i;
    Sint8 *l = ((Sint8 *) _Eff_volume_table) + (256 * args->left_u8);
    Sint8 *r = ((Sint8 *) _Eff_volume_table) + (256 * args->right_u8);
    Sint8 *d = ((Sint8 *) _Eff_volume_table) + (256 * args->distance_u8);


    while (len % sizeof (Uint32) != 0) {
        *(ptr++) = d[l[*ptr]];
        if (args->channels > 1)
            *(ptr++) = d[r[*ptr]];
        len -= args->channels;
    }

    p = (Uint32 *) ptr;

    for (i = 0; i < len; i += sizeof (Uint32)) {
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        *(p++) = (d[l[((Sint16)(Sint8)((*p & 0xFF000000) >> 24))+128]] << 24) |
                 (d[r[((Sint16)(Sint8)((*p & 0x00FF0000) >> 16))+128]] << 16) |
                 (d[l[((Sint16)(Sint8)((*p & 0x0000FF00) >>  8))+128]] <<  8) |
                 (d[r[((Sint16)(Sint8)((*p & 0x000000FF)      ))+128]]      ) ;
#else
        *(p++) = (d[r[((Sint16)(Sint8)((*p & 0xFF000000) >> 24))+128]] << 24) |
                 (d[l[((Sint16)(Sint8)((*p & 0x00FF0000) >> 16))+128]] << 16) |
                 (d[r[((Sint16)(Sint8)((*p & 0x0000FF00) >>  8))+128]] <<  8) |
                 (d[l[((Sint16)(Sint8)((*p & 0x000000FF)      ))+128]]      ) ;
#endif
    }


}


/* !!! FIXME : Optimize the code for 16-bit samples? */

static void _Eff_position_u16lsb(int chan, void *stream, int len, void *udata)
{
    volatile position_args *args = (volatile position_args *) udata;
    Uint16 *ptr = (Uint16 *) stream;
    int i;

    for (i = 0; i < len; i += sizeof (Uint16) * 2) {
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        Uint16 swapl = (Uint16) ((((float) SDL_Swap16(*(ptr))) *
                                    args->left_f) * args->distance_f);
        Uint16 swapr = (Uint16) ((((float) SDL_Swap16(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        *(ptr++) = (Uint16) SDL_Swap16(swapl);
        *(ptr++) = (Uint16) SDL_Swap16(swapr);
#else
        *(ptr++) = (Uint16) ((((float) *ptr)*args->left_f)*args->distance_f);
        *(ptr++) = (Uint16) ((((float) *ptr)*args->right_f)*args->distance_f);
#endif
    }
}

static void _Eff_position_s16lsb(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 2 channels. */
    volatile position_args *args = (volatile position_args *) udata;
    Sint16 *ptr = (Sint16 *) stream;
    int i;

    for (i = 0; i < len; i += sizeof (Sint16) * 2) {
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        Sint16 swapl = (Sint16) ((((float) SDL_Swap16(*(ptr))) *
                                    args->left_f) * args->distance_f);
        Sint16 swapr = (Sint16) ((((float) SDL_Swap16(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        *(ptr++) = (Sint16) SDL_Swap16(swapl);
        *(ptr++) = (Sint16) SDL_Swap16(swapr);
#else
        *(ptr++) = (Sint16) ((((float) *ptr)*args->left_f)*args->distance_f);
        *(ptr++) = (Sint16) ((((float) *ptr)*args->right_f)*args->distance_f);
#endif
    }
}

static void _Eff_position_u16msb(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 2 channels. */
    volatile position_args *args = (volatile position_args *) udata;
    Uint16 *ptr = (Uint16 *) stream;
    int i;

    for (i = 0; i < len; i += sizeof (Sint16) * 2) {
#if (SDL_BYTEORDER == SDL_LIL_ENDIAN)
        Uint16 swapl = (Uint16) ((((float) SDL_Swap16(*(ptr))) *
                                    args->left_f) * args->distance_f);
        Uint16 swapr = (Uint16) ((((float) SDL_Swap16(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        *(ptr++) = (Uint16) SDL_Swap16(swapl);
        *(ptr++) = (Uint16) SDL_Swap16(swapr);
#else
        *(ptr++) = (Uint16) ((((float) *ptr)*args->left_f)*args->distance_f);
        *(ptr++) = (Uint16) ((((float) *ptr)*args->right_f)*args->distance_f);
#endif
    }
}

static void _Eff_position_s16msb(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 2 channels. */
    volatile position_args *args = (volatile position_args *) udata;
    Sint16 *ptr = (Sint16 *) stream;
    int i;

    for (i = 0; i < len; i += sizeof (Sint16) * 2) {
#if (SDL_BYTEORDER == SDL_LIL_ENDIAN)
        Sint16 swapl = (Sint16) ((((float) SDL_Swap16(*(ptr))) *
                                    args->left_f) * args->distance_f);
        Sint16 swapr = (Sint16) ((((float) SDL_Swap16(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        *(ptr++) = (Sint16) SDL_Swap16(swapl);
        *(ptr++) = (Sint16) SDL_Swap16(swapr);
#else
        *(ptr++) = (Sint16) ((((float) *ptr)*args->left_f)*args->distance_f);
        *(ptr++) = (Sint16) ((((float) *ptr)*args->right_f)*args->distance_f);
#endif
    }
}


static void init_position_args(position_args *args)
{
    memset(args, '\0', sizeof (position_args));
    args->in_use = 0;
    args->left_u8 = args->right_u8 = args->distance_u8 = 255;
    args->left_f  = args->right_f  = args->distance_f  = 1.0f;
    Mix_QuerySpec(NULL, NULL, (int *) &args->channels);
}


static position_args *get_position_arg(int channel)
{
    void *rc;
    int i;

    if (channel < 0) {
        if (pos_args_global == NULL) {
            pos_args_global = malloc(sizeof (position_args));
            if (pos_args_global == NULL) {
                Mix_SetError("Out of memory");
                return(NULL);
            }
            init_position_args(pos_args_global);
        }

        return(pos_args_global);
    }

    if (channel >= position_channels) {
        rc = realloc(pos_args_array, (channel + 1) * sizeof (position_args *));
        if (rc == NULL) {
            Mix_SetError("Out of memory");
            return(NULL);
        }
        pos_args_array = (position_args **) rc;
        for (i = position_channels; i <= channel; i++) {
            pos_args_array[i] = NULL;
        }
        position_channels = channel + 1;
    }

    if (pos_args_array[channel] == NULL) {
        pos_args_array[channel] = (position_args *)malloc(sizeof(position_args));
        if (pos_args_array[channel] == NULL) {
            Mix_SetError("Out of memory");
            return(NULL);
        }
        init_position_args(pos_args_array[channel]);
    }

    return(pos_args_array[channel]);
}


static Mix_EffectFunc_t get_position_effect_func(Uint16 format)
{
    Mix_EffectFunc_t f = NULL;

    switch (format) {
        case AUDIO_U8:
            f = (_Eff_build_volume_table_u8()) ? _Eff_position_table_u8 :
                                                 _Eff_position_u8;
            break;

        case AUDIO_S8:
            f = (_Eff_build_volume_table_s8()) ? _Eff_position_table_s8 :
                                                 _Eff_position_s8;
            break;

        case AUDIO_U16LSB:
            f = _Eff_position_u16lsb;
            break;

        case AUDIO_S16LSB:
            f = _Eff_position_s16lsb;
            break;

        case AUDIO_U16MSB:
            f = _Eff_position_u16msb;
            break;

        case AUDIO_S16MSB:
            f = _Eff_position_s16msb;
            break;

        default:
            Mix_SetError("Unsupported audio format");
    }

    return(f);
}


int Mix_SetPanning(int channel, Uint8 left, Uint8 right)
{
    Mix_EffectFunc_t f = NULL;
    int channels;
    Uint16 format;
    position_args *args = NULL;

    Mix_QuerySpec(NULL, &format, &channels);

    if (channels != 2)    /* it's a no-op; we call that successful. */
        return(1);

    f = get_position_effect_func(format);
    if (f == NULL)
        return(0);

    args = get_position_arg(channel);
    if (!args)
        return(0);

        /* it's a no-op; unregister the effect, if it's registered. */
    if ((args->distance_u8 == 255) && (left == 255) &&
        (right == 255) && (args->in_use))
    {
        return(Mix_UnregisterEffect(channel, f));
    }

    args->left_u8 = left;
    args->right_u8 = right;
    args->left_f = ((float) left) / 255.0f;
    args->right_f = ((float) right) / 255.0f;
    if (!args->in_use) {
        args->in_use = 1;
        return(Mix_RegisterEffect(channel, f, _Eff_PositionDone, (void *) args));
    }

    return(1);
}


int Mix_SetDistance(int channel, Uint8 distance)
{
    Mix_EffectFunc_t f = NULL;
    Uint16 format;
    position_args *args = NULL;

    Mix_QuerySpec(NULL, &format, NULL);
    f = get_position_effect_func(format);
    if (f == NULL)
        return(0);

    args = get_position_arg(channel);
    if (!args)
        return(0);

    distance = 255 - distance;  /* flip it to our scale. */

        /* it's a no-op; unregister the effect, if it's registered. */
    if ((distance == 255) && (args->left_u8 == 255) &&
        (args->right_u8 == 255) && (args->in_use))
    {
        return(Mix_UnregisterEffect(channel, f));
    }

    args->distance_u8 = distance;
    args->distance_f = ((float) distance) / 255.0f;
    if (!args->in_use) {
        args->in_use = 1;
        return(Mix_RegisterEffect(channel, f, _Eff_PositionDone, (void *) args));
    }

    return(1);
}


int Mix_SetPosition(int channel, Sint16 angle, Uint8 distance)
{
    Mix_EffectFunc_t f = NULL;
    Uint16 format;
    int channels;
    position_args *args = NULL;
    Uint8 left = 255, right = 255;

    Mix_QuerySpec(NULL, &format, &channels);
    f = get_position_effect_func(format);
    if (f == NULL)
        return(0);

        /* unwind the angle...it'll be between 0 and 359. */
    while (angle >= 360) angle -= 360;
    while (angle < 0) angle += 360;

    args = get_position_arg(channel);
    if (!args)
        return(0);

        /* it's a no-op; unregister the effect, if it's registered. */
    if ((!distance) && (!angle) && (args->in_use))
        return(Mix_UnregisterEffect(channel, f));

    if (channels == 2)
    {
        /*
         * We only attenuate by position if the angle falls on the far side
         *  of center; That is, an angle that's due north would not attenuate
         *  either channel. Due west attenuates the right channel to 0.0, and
         *  due east attenuates the left channel to 0.0. Slightly east of
         *  center attenuates the left channel a little, and the right channel
         *  not at all. I think of this as occlusion by one's own head.  :)
         *
         *   ...so, we split our angle circle into four quadrants...
         */
        if (angle < 90) {
            left = 255 - ((Uint8) (255.0f * (((float) angle) / 89.0f)));
        } else if (angle < 180) {
            left = (Uint8) (255.0f * (((float) (angle - 90)) / 89.0f));
        } else if (angle < 270) {
            right = 255 - ((Uint8) (255.0f * (((float) (angle - 180)) / 89.0f)));
        } else {
            right = (Uint8) (255.0f * (((float) (angle - 270)) / 89.0f));
        }
    }

    distance = 255 - distance;  /* flip it to scale Mix_SetDistance() uses. */

    args->left_u8 = left;
    args->left_f = ((float) left) / 255.0f;
    args->right_u8 = right;
    args->right_f = ((float) right) / 255.0f;
    args->distance_u8 = distance;
    args->distance_f = ((float) distance) / 255.0f;
    if (!args->in_use) {
        args->in_use = 1;
        return(Mix_RegisterEffect(channel, f, _Eff_PositionDone, (void *) args));
    }

    return(1);
}


/* end of effects_position.c ... */

