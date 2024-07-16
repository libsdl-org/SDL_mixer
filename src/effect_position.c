/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2024 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  This file by Ryan C. Gordon (icculus@icculus.org)

  These are some internally supported special effects that use SDL_mixer's
  effect callback API. They are meant for speed over quality.  :)
*/

#include <SDL3/SDL_endian.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "mixer.h"

#define MIX_INTERNAL_EFFECT__
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
    float left_f;
    float right_f;
    Uint8 left_u8;
    Uint8 right_u8;
    float left_rear_f;
    float right_rear_f;
    float center_f;
    float lfe_f;
    Uint8 left_rear_u8;
    Uint8 right_rear_u8;
    Uint8 center_u8;
    Uint8 lfe_u8;
    float distance_f;
    Uint8 distance_u8;
    Sint16 room_angle;
    int in_use;
    int channels;
} position_args;

static position_args **pos_args_array = NULL;
static position_args *pos_args_global = NULL;
static int position_channels = 0;

void _Eff_PositionDeinit(void)
{
    int i;
    for (i = 0; i < position_channels; i++) {
        SDL_free(pos_args_array[i]);
    }

    position_channels = 0;

    SDL_free(pos_args_global);
    pos_args_global = NULL;
    SDL_free(pos_args_array);
    pos_args_array = NULL;
}


/* This just frees up the callback-specific data. */
static void SDLCALL _Eff_PositionDone(int channel, void *udata)
{
    (void)udata;

    if (channel < 0) {
        if (pos_args_global != NULL) {
            SDL_free(pos_args_global);
            pos_args_global = NULL;
        }
    }
    else if (pos_args_array[channel] != NULL) {
        SDL_free(pos_args_array[channel]);
        pos_args_array[channel] = NULL;
    }
}

static void SDLCALL _Eff_position_u8(int chan, void *stream, int len, void *udata)
{
    Uint8 *ptr = (Uint8 *) stream;
    const float dist_f = ((position_args *)udata)->distance_f;
    const float left_f = ((position_args *)udata)->left_f;
    const float right_f = ((position_args *)udata)->right_f;
    int i;

    (void)chan;

    /*
     * if there's only a mono channnel (the only way we wouldn't have
     *  a len divisible by 2 here), then left_f and right_f are always
     *  1.0, and are therefore throwaways.
     */
    if (len % (int)sizeof(Uint16) != 0) {
        *ptr = (Uint8) (((float) *ptr) * dist_f);
        ptr++;
        len--;
    }

    if (((position_args *)udata)->room_angle == 180) {
      for (i = 0; i < len; i += sizeof(Uint8) * 2) {
        /* must adjust the sample so that 0 is the center */
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * right_f) * dist_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * left_f) * dist_f) + 128);
        ptr++;
      }
    }
    else {
      for (i = 0; i < len; i += sizeof(Uint8) * 2) {
        /* must adjust the sample so that 0 is the center */
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * left_f) * dist_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * right_f) * dist_f) + 128);
        ptr++;
      }
    }
}

static void SDLCALL _Eff_position_u8_c4(int chan, void *stream, int len, void *udata)
{
    position_args *args = (position_args *) udata;
    Uint8 *ptr = (Uint8 *) stream;
    int i;

    (void)chan;

    /*
     * if there's only a mono channnel (the only way we wouldn't have
     *  a len divisible by 2 here), then left_f and right_f are always
     *  1.0, and are therefore throwaways.
     */
    if (len % (int)sizeof(Uint16) != 0) {
        *ptr = (Uint8) (((float) *ptr) * args->distance_f);
        ptr++;
        len--;
    }

    if (args->room_angle == 0) {
      for (i = 0; i < len; i += sizeof(Uint8) * 4) {
        /* must adjust the sample so that 0 is the center */
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_rear_f) * args->distance_f) + 128);
        ptr++;
      }
    }
    else if (args->room_angle == 90) {
      for (i = 0; i < len; i += sizeof(Uint8) * 4) {
        /* must adjust the sample so that 0 is the center */
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_rear_f) * args->distance_f) + 128);
        ptr++;
      }
    }
    else if (args->room_angle == 180) {
      for (i = 0; i < len; i += sizeof(Uint8) * 4) {
        /* must adjust the sample so that 0 is the center */
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_f) * args->distance_f) + 128);
        ptr++;
      }
    }
    else if (args->room_angle == 270) {
      for (i = 0; i < len; i += sizeof(Uint8) * 4) {
        /* must adjust the sample so that 0 is the center */
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_f) * args->distance_f) + 128);
        ptr++;
      }
    }
}

static void SDLCALL _Eff_position_u8_c6(int chan, void *stream, int len, void *udata)
{
    position_args *args = (position_args *) udata;
    Uint8 *ptr = (Uint8 *) stream;
    int i;

    (void)chan;
    (void)len;

    /*
     * if there's only a mono channnel (the only way we wouldn't have
     *  a len divisible by 2 here), then left_f and right_f are always
     *  1.0, and are therefore throwaways.
     */
    if (len % (int)sizeof(Uint16) != 0) {
        *ptr = (Uint8) (((float) *ptr) * args->distance_f);
        ptr++;
        len--;
    }

    if (args->room_angle == 0) {
      for (i = 0; i < len; i += sizeof(Uint8) * 6) {
        /* must adjust the sample so that 0 is the center */
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->center_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->lfe_f) * args->distance_f) + 128);
        ptr++;
      }
    }
    else if (args->room_angle == 90) {
      for (i = 0; i < len; i += sizeof(Uint8) * 6) {
        /* must adjust the sample so that 0 is the center */
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_rear_f) * args->distance_f/2) + 128)
            + (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_f) * args->distance_f/2) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->lfe_f) * args->distance_f) + 128);
        ptr++;
      }
    }
    else if (args->room_angle == 180) {
      for (i = 0; i < len; i += sizeof(Uint8) * 6) {
        /* must adjust the sample so that 0 is the center */
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_rear_f) * args->distance_f/2) + 128)
            + (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_rear_f) * args->distance_f/2) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->lfe_f) * args->distance_f) + 128);
        ptr++;
      }
    }
    else if (args->room_angle == 270) {
      for (i = 0; i < len; i += sizeof(Uint8) * 6) {
        /* must adjust the sample so that 0 is the center */
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_f) * args->distance_f/2) + 128)
            + (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_rear_f) * args->distance_f/2) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->lfe_f) * args->distance_f) + 128);
        ptr++;
      }
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
static void SDLCALL _Eff_position_table_u8(int chan, void *stream, int len, void *udata)
{
    position_args *args = (position_args *) udata;
    Uint8 *ptr = (Uint8 *) stream;
    Uint32 *p;
    int i;
    Uint8 *l = ((Uint8 *) _Eff_volume_table) + (256 * args->left_u8);
    Uint8 *r = ((Uint8 *) _Eff_volume_table) + (256 * args->right_u8);
    Uint8 *d = ((Uint8 *) _Eff_volume_table) + (256 * args->distance_u8);

    (void)chan;

    if (args->room_angle == 180) {
        Uint8 *temp = l;
        l = r;
        r = temp;
    }
    /*
     * if there's only a mono channnel, then l[] and r[] are always
     *  volume 255, and are therefore throwaways. Still, we have to
     *  be sure not to overrun the audio buffer...
     */
    while (len % (int)sizeof(Uint32) != 0) {
        *ptr = d[l[*ptr]];
        ptr++;
        if (args->channels > 1) {
            *ptr = d[r[*ptr]];
            ptr++;
        }
        len -= args->channels;
    }

    p = (Uint32 *) ptr;

    for (i = 0; i < len; i += sizeof(Uint32)) {
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        *p = (d[l[(*p & 0xFF000000) >> 24]] << 24) |
             (d[r[(*p & 0x00FF0000) >> 16]] << 16) |
             (d[l[(*p & 0x0000FF00) >>  8]] <<  8) |
             (d[r[(*p & 0x000000FF)      ]]      ) ;
#else
        *p = (d[r[(*p & 0xFF000000) >> 24]] << 24) |
             (d[l[(*p & 0x00FF0000) >> 16]] << 16) |
             (d[r[(*p & 0x0000FF00) >>  8]] <<  8) |
             (d[l[(*p & 0x000000FF)      ]]      ) ;
#endif
        ++p;
    }
}


static void SDLCALL _Eff_position_s8(int chan, void *stream, int len, void *udata)
{
    Sint8 *ptr = (Sint8 *) stream;
    const float dist_f = ((position_args *)udata)->distance_f;
    const float left_f = ((position_args *)udata)->left_f;
    const float right_f = ((position_args *)udata)->right_f;
    int i;

    (void)chan;

    /*
     * if there's only a mono channnel (the only way we wouldn't have
     *  a len divisible by 2 here), then left_f and right_f are always
     *  1.0, and are therefore throwaways.
     */
    if (len % (int)sizeof(Sint16) != 0) {
        *ptr = (Sint8) (((float) *ptr) * dist_f);
        ptr++;
        len--;
    }

    if (((position_args *)udata)->room_angle == 180) {
      for (i = 0; i < len; i += sizeof(Sint8) * 2) {
        *ptr = (Sint8)((((float) *ptr) * right_f) * dist_f);
        ptr++;
        *ptr = (Sint8)((((float) *ptr) * left_f) * dist_f);
        ptr++;
      }
    }
    else {
      for (i = 0; i < len; i += sizeof(Sint8) * 2) {
        *ptr = (Sint8)((((float) *ptr) * left_f) * dist_f);
        ptr++;
        *ptr = (Sint8)((((float) *ptr) * right_f) * dist_f);
        ptr++;
      }
    }
}
static void SDLCALL _Eff_position_s8_c4(int chan, void *stream, int len, void *udata)
{
    position_args *args = (position_args *) udata;
    Sint8 *ptr = (Sint8 *) stream;
    int i;

    (void)chan;

    /*
     * if there's only a mono channnel (the only way we wouldn't have
     *  a len divisible by 2 here), then left_f and right_f are always
     *  1.0, and are therefore throwaways.
     */
    if (len % (int)sizeof(Sint16) != 0) {
        *ptr = (Sint8) (((float) *ptr) * args->distance_f);
        ptr++;
        len--;
    }

    for (i = 0; i < len; i += sizeof(Sint8) * 4) {
      switch (args->room_angle) {
      case 0:
        *ptr = (Sint8)((((float) *ptr) * args->left_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_rear_f) * args->distance_f); ptr++;
        break;
      case 90:
        *ptr = (Sint8)((((float) *ptr) * args->right_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_rear_f) * args->distance_f); ptr++;
        break;
      case 180:
        *ptr = (Sint8)((((float) *ptr) * args->right_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_f) * args->distance_f); ptr++;
        break;
      case 270:
        *ptr = (Sint8)((((float) *ptr) * args->left_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_f) * args->distance_f); ptr++;
        break;
      }
    }
}

static void SDLCALL _Eff_position_s8_c6(int chan, void *stream, int len, void *udata)
{
    position_args *args = (position_args *) udata;
    Sint8 *ptr = (Sint8 *) stream;
    int i;

    (void)chan;

    /*
     * if there's only a mono channnel (the only way we wouldn't have
     *  a len divisible by 2 here), then left_f and right_f are always
     *  1.0, and are therefore throwaways.
     */
    if (len % (int)sizeof(Sint16) != 0) {
        *ptr = (Sint8) (((float) *ptr) * args->distance_f);
        ptr++;
        len--;
    }

    for (i = 0; i < len; i += sizeof(Sint8) * 6) {
      switch (args->room_angle) {
      case 0:
        *ptr = (Sint8)((((float) *ptr) * args->left_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->center_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->lfe_f) * args->distance_f); ptr++;
        break;
      case 90:
        *ptr = (Sint8)((((float) *ptr) * args->right_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_rear_f) * args->distance_f / 2)
           + (Sint8)((((float) *ptr) * args->right_f) * args->distance_f / 2); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->lfe_f) * args->distance_f); ptr++;
        break;
      case 180:
        *ptr = (Sint8)((((float) *ptr) * args->right_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_rear_f) * args->distance_f / 2)
           + (Sint8)((((float) *ptr) * args->left_rear_f) * args->distance_f / 2); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->lfe_f) * args->distance_f); ptr++;
        break;
      case 270:
        *ptr = (Sint8)((((float) *ptr) * args->left_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_f) * args->distance_f / 2)
           + (Sint8)((((float) *ptr) * args->left_rear_f) * args->distance_f / 2); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->lfe_f) * args->distance_f); ptr++;
        break;
      }
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
static void SDLCALL _Eff_position_table_s8(int chan, void *stream, int len, void *udata)
{
    position_args *args = (position_args *) udata;
    Sint8 *ptr = (Sint8 *) stream;
    Uint32 *p;
    int i;
    Sint8 *l = ((Sint8 *) _Eff_volume_table) + (256 * args->left_u8);
    Sint8 *r = ((Sint8 *) _Eff_volume_table) + (256 * args->right_u8);
    Sint8 *d = ((Sint8 *) _Eff_volume_table) + (256 * args->distance_u8);

    (void)chan;

    if (args->room_angle == 180) {
        Sint8 *temp = l;
        l = r;
        r = temp;
    }

    while (len % (int)sizeof(Uint32) != 0) {
        *ptr = d[l[*ptr]];
        ptr++;
        if (args->channels > 1) {
            *ptr = d[r[*ptr]];
            ptr++;
        }
        len -= args->channels;
    }

    p = (Uint32 *) ptr;

    for (i = 0; i < len; i += sizeof(Uint32)) {
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        *p = (d[l[((Sint16)(Sint8)((*p & 0xFF000000) >> 24))+128]] << 24) |
             (d[r[((Sint16)(Sint8)((*p & 0x00FF0000) >> 16))+128]] << 16) |
             (d[l[((Sint16)(Sint8)((*p & 0x0000FF00) >>  8))+128]] <<  8) |
             (d[r[((Sint16)(Sint8)((*p & 0x000000FF)      ))+128]]      ) ;
#else
        *p = (d[r[((Sint16)(Sint8)((*p & 0xFF000000) >> 24))+128]] << 24) |
             (d[l[((Sint16)(Sint8)((*p & 0x00FF0000) >> 16))+128]] << 16) |
             (d[r[((Sint16)(Sint8)((*p & 0x0000FF00) >>  8))+128]] <<  8) |
             (d[l[((Sint16)(Sint8)((*p & 0x000000FF)      ))+128]]      ) ;
#endif
        ++p;
    }
}


/* !!! FIXME : Optimize the code for 16-bit samples? */

static void SDLCALL _Eff_position_s16lsb(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 2 channels. */
    Sint16 *ptr = (Sint16 *) stream;
    const SDL_bool opp = ((position_args *)udata)->room_angle == 180 ? SDL_TRUE : SDL_FALSE;
    const float dist_f = ((position_args *)udata)->distance_f;
    const float left_f = ((position_args *)udata)->left_f;
    const float right_f = ((position_args *)udata)->right_f;
    int i;

    (void)chan;

#if 0
    if (len % (int)(sizeof(Sint16) * 2)) {
        fprintf(stderr,"Not an even number of frames! len=%d\n", len);
        return;
    }
#endif

    for (i = 0; i < len; i += sizeof(Sint16) * 2) {
        Sint16 swapl = (Sint16) ((((float) (Sint16) SDL_Swap16LE(*(ptr+0))) *
                                    left_f) * dist_f);
        Sint16 swapr = (Sint16) ((((float) (Sint16) SDL_Swap16LE(*(ptr+1))) *
                                    right_f) * dist_f);
        if (opp) {
            *(ptr++) = (Sint16) SDL_Swap16LE(swapr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swapl);
        }
        else {
            *(ptr++) = (Sint16) SDL_Swap16LE(swapl);
            *(ptr++) = (Sint16) SDL_Swap16LE(swapr);
        }
    }
}
static void SDLCALL _Eff_position_s16lsb_c4(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 4 channels. */
    position_args *args = (position_args *) udata;
    Sint16 *ptr = (Sint16 *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof(Sint16) * 4) {
        Sint16 swapl = (Sint16) ((((float) (Sint16) SDL_Swap16LE(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint16 swapr = (Sint16) ((((float) (Sint16) SDL_Swap16LE(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        Sint16 swaplr = (Sint16) ((((float) (Sint16) SDL_Swap16LE(*(ptr+1))) *
                                    args->left_rear_f) * args->distance_f);
        Sint16 swaprr = (Sint16) ((((float) (Sint16) SDL_Swap16LE(*(ptr+2))) *
                                    args->right_rear_f) * args->distance_f);
        switch (args->room_angle) {
        case 0:
            *(ptr++) = (Sint16) SDL_Swap16LE(swapl);
            *(ptr++) = (Sint16) SDL_Swap16LE(swapr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swaplr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swaprr);
            break;
        case 90:
            *(ptr++) = (Sint16) SDL_Swap16LE(swapr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swaprr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swapl);
            *(ptr++) = (Sint16) SDL_Swap16LE(swaplr);
            break;
        case 180:
            *(ptr++) = (Sint16) SDL_Swap16LE(swaprr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swaplr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swapr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swapl);
            break;
        case 270:
            *(ptr++) = (Sint16) SDL_Swap16LE(swaplr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swapl);
            *(ptr++) = (Sint16) SDL_Swap16LE(swaprr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swapr);
            break;
        }
    }
}

static void SDLCALL _Eff_position_s16lsb_c6(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 6 channels. */
    position_args *args = (position_args *) udata;
    Sint16 *ptr = (Sint16 *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof(Sint16) * 6) {
        Sint16 swapl = (Sint16) ((((float) (Sint16) SDL_Swap16LE(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint16 swapr = (Sint16) ((((float) (Sint16) SDL_Swap16LE(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        Sint16 swaplr = (Sint16) ((((float) (Sint16) SDL_Swap16LE(*(ptr+2))) *
                                    args->left_rear_f) * args->distance_f);
        Sint16 swaprr = (Sint16) ((((float) (Sint16) SDL_Swap16LE(*(ptr+3))) *
                                    args->right_rear_f) * args->distance_f);
        Sint16 swapce = (Sint16) ((((float) (Sint16) SDL_Swap16LE(*(ptr+4))) *
                                    args->center_f) * args->distance_f);
        Sint16 swapwf = (Sint16) ((((float) (Sint16) SDL_Swap16LE(*(ptr+5))) *
                                    args->lfe_f) * args->distance_f);
        switch (args->room_angle) {
        case 0:
            *(ptr++) = (Sint16) SDL_Swap16LE(swapl);
            *(ptr++) = (Sint16) SDL_Swap16LE(swapr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swaplr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swaprr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swapce);
            *(ptr++) = (Sint16) SDL_Swap16LE(swapwf);
            break;
        case 90:
            *(ptr++) = (Sint16) SDL_Swap16LE(swapr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swaprr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swapl);
            *(ptr++) = (Sint16) SDL_Swap16LE(swaplr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swapr)/2 + (Sint16) SDL_Swap16LE(swaprr)/2;
            *(ptr++) = (Sint16) SDL_Swap16LE(swapwf);
            break;
        case 180:
            *(ptr++) = (Sint16) SDL_Swap16LE(swaprr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swaplr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swapr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swapl);
            *(ptr++) = (Sint16) SDL_Swap16LE(swaprr)/2 + (Sint16) SDL_Swap16LE(swaplr)/2;
            *(ptr++) = (Sint16) SDL_Swap16LE(swapwf);
            break;
        case 270:
            *(ptr++) = (Sint16) SDL_Swap16LE(swaplr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swapl);
            *(ptr++) = (Sint16) SDL_Swap16LE(swaprr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swapr);
            *(ptr++) = (Sint16) SDL_Swap16LE(swapl)/2 + (Sint16) SDL_Swap16LE(swaplr)/2;
            *(ptr++) = (Sint16) SDL_Swap16LE(swapwf);
            break;
        }
    }
}

static void SDLCALL _Eff_position_s16msb(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 2 channels. */
    Sint16 *ptr = (Sint16 *) stream;
    const float dist_f = ((position_args *)udata)->distance_f;
    const float left_f = ((position_args *)udata)->left_f;
    const float right_f = ((position_args *)udata)->right_f;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof(Sint16) * 2) {
        Sint16 swapl = (Sint16) ((((float) (Sint16) SDL_Swap16BE(*(ptr+0))) *
                                    left_f) * dist_f);
        Sint16 swapr = (Sint16) ((((float) (Sint16) SDL_Swap16BE(*(ptr+1))) *
                                    right_f) * dist_f);
        *(ptr++) = (Sint16) SDL_Swap16BE(swapl);
        *(ptr++) = (Sint16) SDL_Swap16BE(swapr);
    }
}

static void SDLCALL _Eff_position_s16msb_c4(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 4 channels. */
    position_args *args = (position_args *) udata;
    Sint16 *ptr = (Sint16 *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof(Sint16) * 4) {
        Sint16 swapl = (Sint16) ((((float) (Sint16) SDL_Swap16BE(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint16 swapr = (Sint16) ((((float) (Sint16) SDL_Swap16BE(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        Sint16 swaplr = (Sint16) ((((float) (Sint16) SDL_Swap16BE(*(ptr+2))) *
                                    args->left_rear_f) * args->distance_f);
        Sint16 swaprr = (Sint16) ((((float) (Sint16) SDL_Swap16BE(*(ptr+3))) *
                                    args->right_rear_f) * args->distance_f);
        switch (args->room_angle) {
        case 0:
            *(ptr++) = (Sint16) SDL_Swap16BE(swapl);
            *(ptr++) = (Sint16) SDL_Swap16BE(swapr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swaplr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swaprr);
            break;
        case 90:
            *(ptr++) = (Sint16) SDL_Swap16BE(swapr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swaprr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swapl);
            *(ptr++) = (Sint16) SDL_Swap16BE(swaplr);
            break;
        case 180:
            *(ptr++) = (Sint16) SDL_Swap16BE(swaprr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swaplr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swapr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swapl);
            break;
        case 270:
            *(ptr++) = (Sint16) SDL_Swap16BE(swaplr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swapl);
            *(ptr++) = (Sint16) SDL_Swap16BE(swaprr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swapr);
            break;
        }
    }
}

static void SDLCALL _Eff_position_s16msb_c6(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 6 channels. */
    position_args *args = (position_args *) udata;
    Sint16 *ptr = (Sint16 *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof(Sint16) * 6) {
        Sint16 swapl = (Sint16) ((((float) (Sint16) SDL_Swap16BE(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint16 swapr = (Sint16) ((((float) (Sint16) SDL_Swap16BE(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        Sint16 swaplr = (Sint16) ((((float) (Sint16) SDL_Swap16BE(*(ptr+2))) *
                                    args->left_rear_f) * args->distance_f);
        Sint16 swaprr = (Sint16) ((((float) (Sint16) SDL_Swap16BE(*(ptr+3))) *
                                    args->right_rear_f) * args->distance_f);
        Sint16 swapce = (Sint16) ((((float) (Sint16) SDL_Swap16BE(*(ptr+4))) *
                                    args->center_f) * args->distance_f);
        Sint16 swapwf = (Sint16) ((((float) (Sint16) SDL_Swap16BE(*(ptr+5))) *
                                    args->lfe_f) * args->distance_f);

        switch (args->room_angle) {
        case 0:
            *(ptr++) = (Sint16) SDL_Swap16BE(swapl);
            *(ptr++) = (Sint16) SDL_Swap16BE(swapr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swaplr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swaprr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swapce);
            *(ptr++) = (Sint16) SDL_Swap16BE(swapwf);
            break;
        case 90:
            *(ptr++) = (Sint16) SDL_Swap16BE(swapr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swaprr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swapl);
            *(ptr++) = (Sint16) SDL_Swap16BE(swaplr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swapr)/2 + (Sint16) SDL_Swap16BE(swaprr)/2;
            *(ptr++) = (Sint16) SDL_Swap16BE(swapwf);
            break;
        case 180:
            *(ptr++) = (Sint16) SDL_Swap16BE(swaprr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swaplr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swapr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swapl);
            *(ptr++) = (Sint16) SDL_Swap16BE(swaprr)/2 + (Sint16) SDL_Swap16BE(swaplr)/2;
            *(ptr++) = (Sint16) SDL_Swap16BE(swapwf);
            break;
        case 270:
            *(ptr++) = (Sint16) SDL_Swap16BE(swaplr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swapl);
            *(ptr++) = (Sint16) SDL_Swap16BE(swaprr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swapr);
            *(ptr++) = (Sint16) SDL_Swap16BE(swapl)/2 + (Sint16) SDL_Swap16BE(swaplr)/2;
            *(ptr++) = (Sint16) SDL_Swap16BE(swapwf);
            break;
        }
    }
}

static void SDLCALL _Eff_position_s32lsb(int chan, void *stream, int len, void *udata)
{
    /* 32 signed bits (lsb) * 2 channels. */
    Sint32 *ptr = (Sint32 *) stream;
    const SDL_bool opp = ((position_args *)udata)->room_angle == 180 ? SDL_TRUE : SDL_FALSE;
    const float dist_f = ((position_args *)udata)->distance_f;
    const float left_f = ((position_args *)udata)->left_f;
    const float right_f = ((position_args *)udata)->right_f;
    int i;

    (void)chan;

#if 0
    if (len % (int)(sizeof(Sint32) * 2)) {
        fprintf(stderr,"Not an even number of frames! len=%d\n", len);
        return;
    }
#endif

    for (i = 0; i < len; i += sizeof(Sint32) * 2) {
        Sint32 swapl = (Sint32) ((((float) (Sint32) SDL_Swap32LE(*(ptr+0))) *
                                    left_f) * dist_f);
        Sint32 swapr = (Sint32) ((((float) (Sint32) SDL_Swap32LE(*(ptr+1))) *
                                    right_f) * dist_f);
        if (opp) {
            *(ptr++) = (Sint32) SDL_Swap32LE(swapr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swapl);
        }
        else {
            *(ptr++) = (Sint32) SDL_Swap32LE(swapl);
            *(ptr++) = (Sint32) SDL_Swap32LE(swapr);
        }
    }
}

static void SDLCALL _Eff_position_s32lsb_c4(int chan, void *stream, int len, void *udata)
{
    /* 32 signed bits (lsb) * 4 channels. */
    position_args *args = (position_args *) udata;
    Sint32 *ptr = (Sint32 *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof(Sint32) * 4) {
        Sint32 swapl = (Sint32) ((((float) (Sint32) SDL_Swap32LE(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint32 swapr = (Sint32) ((((float) (Sint32) SDL_Swap32LE(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        Sint32 swaplr = (Sint32) ((((float) (Sint32) SDL_Swap32LE(*(ptr+1))) *
                                    args->left_rear_f) * args->distance_f);
        Sint32 swaprr = (Sint32) ((((float) (Sint32) SDL_Swap32LE(*(ptr+2))) *
                                    args->right_rear_f) * args->distance_f);
        switch (args->room_angle) {
        case 0:
            *(ptr++) = (Sint32) SDL_Swap32LE(swapl);
            *(ptr++) = (Sint32) SDL_Swap32LE(swapr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swaplr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swaprr);
            break;
        case 90:
            *(ptr++) = (Sint32) SDL_Swap32LE(swapr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swaprr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swapl);
            *(ptr++) = (Sint32) SDL_Swap32LE(swaplr);
            break;
        case 180:
            *(ptr++) = (Sint32) SDL_Swap32LE(swaprr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swaplr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swapr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swapl);
            break;
        case 270:
            *(ptr++) = (Sint32) SDL_Swap32LE(swaplr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swapl);
            *(ptr++) = (Sint32) SDL_Swap32LE(swaprr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swapr);
            break;
        }
    }
}

static void SDLCALL _Eff_position_s32lsb_c6(int chan, void *stream, int len, void *udata)
{
    /* 32 signed bits (lsb) * 6 channels. */
    position_args *args = (position_args *) udata;
    Sint32 *ptr = (Sint32 *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof(Sint32) * 6) {
        Sint32 swapl = (Sint32) ((((float) (Sint32) SDL_Swap32LE(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint32 swapr = (Sint32) ((((float) (Sint32) SDL_Swap32LE(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        Sint32 swaplr = (Sint32) ((((float) (Sint32) SDL_Swap32LE(*(ptr+2))) *
                                    args->left_rear_f) * args->distance_f);
        Sint32 swaprr = (Sint32) ((((float) (Sint32) SDL_Swap32LE(*(ptr+3))) *
                                    args->right_rear_f) * args->distance_f);
        Sint32 swapce = (Sint32) ((((float) (Sint32) SDL_Swap32LE(*(ptr+4))) *
                                    args->center_f) * args->distance_f);
        Sint32 swapwf = (Sint32) ((((float) (Sint32) SDL_Swap32LE(*(ptr+5))) *
                                    args->lfe_f) * args->distance_f);
        switch (args->room_angle) {
        case 0:
            *(ptr++) = (Sint32) SDL_Swap32LE(swapl);
            *(ptr++) = (Sint32) SDL_Swap32LE(swapr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swaplr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swaprr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swapce);
            *(ptr++) = (Sint32) SDL_Swap32LE(swapwf);
            break;
        case 90:
            *(ptr++) = (Sint32) SDL_Swap32LE(swapr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swaprr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swapl);
            *(ptr++) = (Sint32) SDL_Swap32LE(swaplr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swapr)/2 + (Sint32) SDL_Swap32LE(swaprr)/2;
            *(ptr++) = (Sint32) SDL_Swap32LE(swapwf);
            break;
        case 180:
            *(ptr++) = (Sint32) SDL_Swap32LE(swaprr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swaplr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swapr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swapl);
            *(ptr++) = (Sint32) SDL_Swap32LE(swaprr)/2 + (Sint32) SDL_Swap32LE(swaplr)/2;
            *(ptr++) = (Sint32) SDL_Swap32LE(swapwf);
            break;
        case 270:
            *(ptr++) = (Sint32) SDL_Swap32LE(swaplr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swapl);
            *(ptr++) = (Sint32) SDL_Swap32LE(swaprr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swapr);
            *(ptr++) = (Sint32) SDL_Swap32LE(swapl)/2 + (Sint32) SDL_Swap32LE(swaplr)/2;
            *(ptr++) = (Sint32) SDL_Swap32LE(swapwf);
            break;
        }
    }
}

static void SDLCALL _Eff_position_s32msb(int chan, void *stream, int len, void *udata)
{
    /* 32 signed bits (lsb) * 2 channels. */
    Sint32 *ptr = (Sint32 *) stream;
    const float dist_f = ((position_args *)udata)->distance_f;
    const float left_f = ((position_args *)udata)->left_f;
    const float right_f = ((position_args *)udata)->right_f;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof(Sint32) * 2) {
        Sint32 swapl = (Sint32) ((((float) (Sint32) SDL_Swap32BE(*(ptr+0))) *
                                    left_f) * dist_f);
        Sint32 swapr = (Sint32) ((((float) (Sint32) SDL_Swap32BE(*(ptr+1))) *
                                    right_f) * dist_f);
        *(ptr++) = (Sint32) SDL_Swap32BE(swapl);
        *(ptr++) = (Sint32) SDL_Swap32BE(swapr);
    }
}

static void SDLCALL _Eff_position_s32msb_c4(int chan, void *stream, int len, void *udata)
{
    /* 32 signed bits (lsb) * 4 channels. */
    position_args *args = (position_args *) udata;
    Sint32 *ptr = (Sint32 *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof(Sint32) * 4) {
        Sint32 swapl = (Sint32) ((((float) (Sint32) SDL_Swap32BE(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint32 swapr = (Sint32) ((((float) (Sint32) SDL_Swap32BE(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        Sint32 swaplr = (Sint32) ((((float) (Sint32) SDL_Swap32BE(*(ptr+2))) *
                                    args->left_rear_f) * args->distance_f);
        Sint32 swaprr = (Sint32) ((((float) (Sint32) SDL_Swap32BE(*(ptr+3))) *
                                    args->right_rear_f) * args->distance_f);
        switch (args->room_angle) {
        case 0:
            *(ptr++) = (Sint32) SDL_Swap32BE(swapl);
            *(ptr++) = (Sint32) SDL_Swap32BE(swapr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swaplr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swaprr);
            break;
        case 90:
            *(ptr++) = (Sint32) SDL_Swap32BE(swapr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swaprr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swapl);
            *(ptr++) = (Sint32) SDL_Swap32BE(swaplr);
            break;
        case 180:
            *(ptr++) = (Sint32) SDL_Swap32BE(swaprr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swaplr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swapr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swapl);
            break;
        case 270:
            *(ptr++) = (Sint32) SDL_Swap32BE(swaplr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swapl);
            *(ptr++) = (Sint32) SDL_Swap32BE(swaprr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swapr);
            break;
        }
    }
}

static void SDLCALL _Eff_position_s32msb_c6(int chan, void *stream, int len, void *udata)
{
    /* 32 signed bits (lsb) * 6 channels. */
    position_args *args = (position_args *) udata;
    Sint32 *ptr = (Sint32 *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof(Sint32) * 6) {
        Sint32 swapl = (Sint32) ((((float) (Sint32) SDL_Swap32BE(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint32 swapr = (Sint32) ((((float) (Sint32) SDL_Swap32BE(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        Sint32 swaplr = (Sint32) ((((float) (Sint32) SDL_Swap32BE(*(ptr+2))) *
                                    args->left_rear_f) * args->distance_f);
        Sint32 swaprr = (Sint32) ((((float) (Sint32) SDL_Swap32BE(*(ptr+3))) *
                                    args->right_rear_f) * args->distance_f);
        Sint32 swapce = (Sint32) ((((float) (Sint32) SDL_Swap32BE(*(ptr+4))) *
                                    args->center_f) * args->distance_f);
        Sint32 swapwf = (Sint32) ((((float) (Sint32) SDL_Swap32BE(*(ptr+5))) *
                                    args->lfe_f) * args->distance_f);

        switch (args->room_angle) {
        case 0:
            *(ptr++) = (Sint32) SDL_Swap32BE(swapl);
            *(ptr++) = (Sint32) SDL_Swap32BE(swapr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swaplr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swaprr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swapce);
            *(ptr++) = (Sint32) SDL_Swap32BE(swapwf);
            break;
        case 90:
            *(ptr++) = (Sint32) SDL_Swap32BE(swapr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swaprr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swapl);
            *(ptr++) = (Sint32) SDL_Swap32BE(swaplr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swapr)/2 + (Sint32) SDL_Swap32BE(swaprr)/2;
            *(ptr++) = (Sint32) SDL_Swap32BE(swapwf);
            break;
        case 180:
            *(ptr++) = (Sint32) SDL_Swap32BE(swaprr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swaplr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swapr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swapl);
            *(ptr++) = (Sint32) SDL_Swap32BE(swaprr)/2 + (Sint32) SDL_Swap32BE(swaplr)/2;
            *(ptr++) = (Sint32) SDL_Swap32BE(swapwf);
            break;
        case 270:
            *(ptr++) = (Sint32) SDL_Swap32BE(swaplr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swapl);
            *(ptr++) = (Sint32) SDL_Swap32BE(swaprr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swapr);
            *(ptr++) = (Sint32) SDL_Swap32BE(swapl)/2 + (Sint32) SDL_Swap32BE(swaplr)/2;
            *(ptr++) = (Sint32) SDL_Swap32BE(swapwf);
            break;
        }
    }
}

static void SDLCALL _Eff_position_f32sys(int chan, void *stream, int len, void *udata)
{
    /* float * 2 channels. */
    float *ptr = (float *) stream;
    const float dist_f = ((position_args *)udata)->distance_f;
    const float left_f = ((position_args *)udata)->left_f;
    const float right_f = ((position_args *)udata)->right_f;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof(float) * 2) {
        float swapl = ((*(ptr+0) * left_f) * dist_f);
        float swapr = ((*(ptr+1) * right_f) * dist_f);
        *(ptr++) = swapl;
        *(ptr++) = swapr;
    }
}

static void SDLCALL _Eff_position_f32sys_c4(int chan, void *stream, int len, void *udata)
{
    /* float * 4 channels. */
    position_args *args = (position_args *) udata;
    float *ptr = (float *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof(float) * 4) {
        float swapl = ((*(ptr+0) * args->left_f) * args->distance_f);
        float swapr = ((*(ptr+1) * args->right_f) * args->distance_f);
        float swaplr = ((*(ptr+2) * args->left_rear_f) * args->distance_f);
        float swaprr = ((*(ptr+3) * args->right_rear_f) * args->distance_f);
        switch (args->room_angle) {
        case 0:
            *(ptr++) = swapl;
            *(ptr++) = swapr;
            *(ptr++) = swaplr;
            *(ptr++) = swaprr;
            break;
        case 90:
            *(ptr++) = swapr;
            *(ptr++) = swaprr;
            *(ptr++) = swapl;
            *(ptr++) = swaplr;
            break;
        case 180:
            *(ptr++) = swaprr;
            *(ptr++) = swaplr;
            *(ptr++) = swapr;
            *(ptr++) = swapl;
            break;
        case 270:
            *(ptr++) = swaplr;
            *(ptr++) = swapl;
            *(ptr++) = swaprr;
            *(ptr++) = swapr;
            break;
        }
    }
}

static void SDLCALL _Eff_position_f32sys_c6(int chan, void *stream, int len, void *udata)
{
    /* float * 6 channels. */
    position_args *args = (position_args *) udata;
    float *ptr = (float *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof(float) * 6) {
        float swapl = ((*(ptr+0) * args->left_f) * args->distance_f);
        float swapr = ((*(ptr+1) * args->right_f) * args->distance_f);
        float swaplr = ((*(ptr+2) * args->left_rear_f) * args->distance_f);
        float swaprr = ((*(ptr+3) * args->right_rear_f) * args->distance_f);
        float swapce = ((*(ptr+4) * args->center_f) * args->distance_f);
        float swapwf = ((*(ptr+5) * args->lfe_f) * args->distance_f);

        switch (args->room_angle) {
        case 0:
            *(ptr++) = swapl;
            *(ptr++) = swapr;
            *(ptr++) = swaplr;
            *(ptr++) = swaprr;
            *(ptr++) = swapce;
            *(ptr++) = swapwf;
            break;
        case 90:
            *(ptr++) = swapr;
            *(ptr++) = swaprr;
            *(ptr++) = swapl;
            *(ptr++) = swaplr;
            *(ptr++) = swapr/2.0f + swaprr/2.0f;
            *(ptr++) = swapwf;
            break;
        case 180:
            *(ptr++) = swaprr;
            *(ptr++) = swaplr;
            *(ptr++) = swapr;
            *(ptr++) = swapl;
            *(ptr++) = swaprr/2.0f + swaplr/2.0f;
            *(ptr++) = swapwf;
            break;
        case 270:
            *(ptr++) = swaplr;
            *(ptr++) = swapl;
            *(ptr++) = swaprr;
            *(ptr++) = swapr;
            *(ptr++) = swapl/2.0f + swaplr/2.0f;
            *(ptr++) = swapwf;
            break;
        }
    }
}

static void init_position_args(position_args *args)
{
    SDL_memset(args, '\0', sizeof(position_args));
    args->in_use = 0;
    args->room_angle = 0;
    args->left_u8 = args->right_u8 = args->distance_u8 = 255;
    args->left_f  = args->right_f  = args->distance_f  = 1.0f;
    args->left_rear_u8 = args->right_rear_u8 = args->center_u8 = args->lfe_u8 = 255;
    args->left_rear_f = args->right_rear_f = args->center_f = args->lfe_f = 1.0f;
    Mix_QuerySpec(NULL, NULL, (int *) &args->channels);
}

static position_args *get_position_arg(int channel)
{
    void *rc;
    int i;

    if (channel < 0) {
        if (pos_args_global == NULL) {
            pos_args_global = SDL_malloc(sizeof(position_args));
            if (pos_args_global == NULL) {
                return NULL;
            }
            init_position_args(pos_args_global);
        }

        return pos_args_global;
    }

    if (channel >= position_channels) {
        rc = SDL_realloc(pos_args_array, (size_t)(channel + 1) * sizeof(position_args *));
        if (rc == NULL) {
            return NULL;
        }
        pos_args_array = (position_args **) rc;
        for (i = position_channels; i <= channel; i++) {
            pos_args_array[i] = NULL;
        }
        position_channels = channel + 1;
    }

    if (pos_args_array[channel] == NULL) {
        pos_args_array[channel] = (position_args *)SDL_malloc(sizeof(position_args));
        if (pos_args_array[channel] == NULL) {
            return NULL;
        }
        init_position_args(pos_args_array[channel]);
    }

    return pos_args_array[channel];
}

static Mix_EffectFunc_t get_position_effect_func(SDL_AudioFormat format, int channels)
{
    Mix_EffectFunc_t f = NULL;

    switch (format) {
        case SDL_AUDIO_U8 :
            switch (channels) {
            case 1:
            case 2:
                f = (_Eff_build_volume_table_u8()) ? _Eff_position_table_u8 :
                                                     _Eff_position_u8;
                break;
            case 4:
                f = _Eff_position_u8_c4;
                break;
            case 6:
                f = _Eff_position_u8_c6;
                break;
            default:
                Mix_SetError("Unsupported audio channels");
                break;
            }
            break;

        case SDL_AUDIO_S8 :
            switch (channels) {
            case 1:
            case 2:
                f = (_Eff_build_volume_table_s8()) ? _Eff_position_table_s8 :
                                                     _Eff_position_s8;
                break;
            case 4:
                f = _Eff_position_s8_c4;
                break;
            case 6:
                f = _Eff_position_s8_c6;
                break;
            default:
                Mix_SetError("Unsupported audio channels");
                break;
            }
            break;

        case SDL_AUDIO_S16LE:
            switch (channels) {
            case 1:
            case 2:
                f = _Eff_position_s16lsb;
                break;
            case 4:
                f = _Eff_position_s16lsb_c4;
                break;
            case 6:
                f = _Eff_position_s16lsb_c6;
                break;
            default:
                Mix_SetError("Unsupported audio channels");
                break;
            }
            break;

        case SDL_AUDIO_S16BE:
            switch (channels) {
            case 1:
            case 2:
                f = _Eff_position_s16msb;
                break;
            case 4:
                f = _Eff_position_s16msb_c4;
                break;
            case 6:
                f = _Eff_position_s16msb_c6;
                break;
            default:
                Mix_SetError("Unsupported audio channels");
                break;
            }
            break;

        case SDL_AUDIO_S32BE:
            switch (channels) {
            case 1:
            case 2:
                f = _Eff_position_s32msb;
                break;
            case 4:
                f = _Eff_position_s32msb_c4;
                break;
            case 6:
                f = _Eff_position_s32msb_c6;
                break;
            default:
                Mix_SetError("Unsupported audio channels");
                break;
            }
            break;

        case SDL_AUDIO_S32LE:
            switch (channels) {
            case 1:
            case 2:
                f = _Eff_position_s32lsb;
                break;
            case 4:
                f = _Eff_position_s32lsb_c4;
                break;
            case 6:
                f = _Eff_position_s32lsb_c6;
                break;
            default:
                Mix_SetError("Unsupported audio channels");
                break;
            }
            break;

        case SDL_AUDIO_F32:
            switch (channels) {
            case 1:
            case 2:
                f = _Eff_position_f32sys;
                break;
            case 4:
                f = _Eff_position_f32sys_c4;
                break;
            case 6:
                f = _Eff_position_f32sys_c6;
                break;
            default:
                Mix_SetError("Unsupported audio channels");
                break;
            }
            break;

        default:
            Mix_SetError("Unsupported audio format");
            break;
    }

    return f;
}

static Uint8 speaker_amplitude[6];

static void set_amplitudes(int channels, int angle, int room_angle)
{
    int left = 255, right = 255;
    int left_rear = 255, right_rear = 255, center = 255;

    /* our only caller Mix_SetPosition() already makes angle between 0 and 359. */

    if (channels == 2) {
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
            left = 255 - ((int) (255.0f * (((float) angle) / 89.0f)));
        } else if (angle < 180) {
            left = (int) (255.0f * (((float) (angle - 90)) / 89.0f));
        } else if (angle < 270) {
            right = 255 - ((int) (255.0f * (((float) (angle - 180)) / 89.0f)));
        } else {
            right = (int) (255.0f * (((float) (angle - 270)) / 89.0f));
        }
    }

    if (channels == 4 || channels == 6) {
        /*
         *  An angle that's due north does not attenuate the center channel.
         *  An angle in the first quadrant, 0-90, does not attenuate the RF.
         *
         *   ...so, we split our angle circle into 8 ...
         *
         *             CE
         *             0
         *     LF      |         RF
         *             |
         *  270<-------|----------->90
         *             |
         *     LR      |         RR
         *            180
         *
         */
        if (angle < 45) {
            left = ((int) (255.0f * (((float) (180 - angle)) / 179.0f)));
            left_rear = 255 - ((int) (255.0f * (((float) (angle + 45)) / 89.0f)));
            right_rear = 255 - ((int) (255.0f * (((float) (90 - angle)) / 179.0f)));
        } else if (angle < 90) {
            center = ((int) (255.0f * (((float) (225 - angle)) / 179.0f)));
            left = ((int) (255.0f * (((float) (180 - angle)) / 179.0f)));
            left_rear = 255 - ((int) (255.0f * (((float) (135 - angle)) / 89.0f)));
            right_rear = ((int) (255.0f * (((float) (90 + angle)) / 179.0f)));
        } else if (angle < 135) {
            center = ((int) (255.0f * (((float) (225 - angle)) / 179.0f)));
            left = 255 - ((int) (255.0f * (((float) (angle - 45)) / 89.0f)));
            right = ((int) (255.0f * (((float) (270 - angle)) / 179.0f)));
            left_rear = ((int) (255.0f * (((float) (angle)) / 179.0f)));
        } else if (angle < 180) {
            center = 255 - ((int) (255.0f * (((float) (angle - 90)) / 89.0f)));
            left = 255 - ((int) (255.0f * (((float) (225 - angle)) / 89.0f)));
            right = ((int) (255.0f * (((float) (270 - angle)) / 179.0f)));
            left_rear = ((int) (255.0f * (((float) (angle)) / 179.0f)));
        } else if (angle < 225) {
            center = 255 - ((int) (255.0f * (((float) (270 - angle)) / 89.0f)));
            left = ((int) (255.0f * (((float) (angle - 90)) / 179.0f)));
            right = 255 - ((int) (255.0f * (((float) (angle - 135)) / 89.0f)));
            right_rear = ((int) (255.0f * (((float) (360 - angle)) / 179.0f)));
        } else if (angle < 270) {
            center = ((int) (255.0f * (((float) (angle - 135)) / 179.0f)));
            left = ((int) (255.0f * (((float) (angle - 90)) / 179.0f)));
            right = 255 - ((int) (255.0f * (((float) (315 - angle)) / 89.0f)));
            right_rear = ((int) (255.0f * (((float) (360 - angle)) / 179.0f)));
        } else if (angle < 315) {
            center = ((int) (255.0f * (((float) (angle - 135)) / 179.0f)));
            right = ((int) (255.0f * (((float) (angle - 180)) / 179.0f)));
            left_rear = ((int) (255.0f * (((float) (450 - angle)) / 179.0f)));
            right_rear = 255 - ((int) (255.0f * (((float) (angle - 225)) / 89.0f)));
        } else {
            right = ((int) (255.0f * (((float) (angle - 180)) / 179.0f)));
            left_rear = ((int) (255.0f * (((float) (450 - angle)) / 179.0f)));
            right_rear = 255 - ((int) (255.0f * (((float) (405 - angle)) / 89.0f)));
        }
    }

    if (left < 0) left = 0;
    if (left > 255) left = 255;
    if (right < 0) right = 0;
    if (right > 255) right = 255;
    if (left_rear < 0) left_rear = 0;
    if (left_rear > 255) left_rear = 255;
    if (right_rear < 0) right_rear = 0;
    if (right_rear > 255) right_rear = 255;
    if (center < 0) center = 0;
    if (center > 255) center = 255;

    if (room_angle == 90) {
        speaker_amplitude[0] = (Uint8)left_rear;
        speaker_amplitude[1] = (Uint8)left;
        speaker_amplitude[2] = (Uint8)right_rear;
        speaker_amplitude[3] = (Uint8)right;
    }
    else if (room_angle == 180) {
        if (channels == 2) {
            speaker_amplitude[0] = (Uint8)right;
            speaker_amplitude[1] = (Uint8)left;
        }
        else {
            speaker_amplitude[0] = (Uint8)right_rear;
            speaker_amplitude[1] = (Uint8)left_rear;
            speaker_amplitude[2] = (Uint8)right;
            speaker_amplitude[3] = (Uint8)left;
        }
    }
    else if (room_angle == 270) {
        speaker_amplitude[0] = (Uint8)right;
        speaker_amplitude[1] = (Uint8)right_rear;
        speaker_amplitude[2] = (Uint8)left;
        speaker_amplitude[3] = (Uint8)left_rear;
    }
    else {
        speaker_amplitude[0] = (Uint8)left;
        speaker_amplitude[1] = (Uint8)right;
        speaker_amplitude[2] = (Uint8)left_rear;
        speaker_amplitude[3] = (Uint8)right_rear;
    }
    speaker_amplitude[4] = (Uint8)center;
    speaker_amplitude[5] = 255;
}

int Mix_SetPosition(int channel, Sint16 angle, Uint8 distance);

int Mix_SetPanning(int channel, Uint8 left, Uint8 right)
{
    Mix_EffectFunc_t f = NULL;
    int channels;
    SDL_AudioFormat format;
    position_args *args = NULL;
    int retval = 1;

    Mix_QuerySpec(NULL, &format, &channels);

    if (channels != 2 && channels != 4 && channels != 6)    /* it's a no-op; we call that successful. */
        return 1;

    if (channels > 2) {
        /* left = right = 255 => angle = 0, to unregister effect as when channels = 2 */
        /* left = 255 =>  angle = -90;  left = 0 => angle = +89 */
        int angle = 0;
        if ((left != 255) || (right != 255)) {
            angle = (int)left;
            angle = 127 - angle;
            angle = -angle;
            angle = angle * 90 / 128; /* Make it larger for more effect? */
        }
        return Mix_SetPosition(channel, angle, 0);
    }

    f = get_position_effect_func(format, channels);
    if (f == NULL)
        return 0;

    Mix_LockAudio();
    args = get_position_arg(channel);
    if (!args) {
        Mix_UnlockAudio();
        return 0;
    }

        /* it's a no-op; unregister the effect, if it's registered. */
    if ((args->distance_u8 == 255) && (left == 255) && (right == 255)) {
        if (args->in_use) {
            retval = _Mix_UnregisterEffect_locked(channel, f);
            Mix_UnlockAudio();
            return retval;
        } else {
            Mix_UnlockAudio();
            return 1;
        }
    }

    args->left_u8 = left;
    args->left_f = ((float) left) / 255.0f;
    args->right_u8 = right;
    args->right_f = ((float) right) / 255.0f;
    args->room_angle = 0;

    if (!args->in_use) {
        args->in_use = 1;
        retval=_Mix_RegisterEffect_locked(channel, f, _Eff_PositionDone, (void*)args);
    }

    Mix_UnlockAudio();
    return retval;
}

int Mix_SetDistance(int channel, Uint8 distance)
{
    Mix_EffectFunc_t f = NULL;
    SDL_AudioFormat format;
    position_args *args = NULL;
    int channels;
    int retval = 1;

    Mix_QuerySpec(NULL, &format, &channels);
    f = get_position_effect_func(format, channels);
    if (f == NULL)
        return 0;

    Mix_LockAudio();
    args = get_position_arg(channel);
    if (!args) {
        Mix_UnlockAudio();
        return 0;
    }

    distance = 255 - distance;  /* flip it to our scale. */

    /* it's a no-op; unregister the effect, if it's registered. */
    if ((distance == 255) && (args->left_u8 == 255) && (args->right_u8 == 255)) {
        if (args->in_use) {
            retval = _Mix_UnregisterEffect_locked(channel, f);
            Mix_UnlockAudio();
            return retval;
        } else {
            Mix_UnlockAudio();
            return 1;
        }
    }

    args->distance_u8 = distance;
    args->distance_f = ((float) distance) / 255.0f;
    if (!args->in_use) {
        args->in_use = 1;
        retval = _Mix_RegisterEffect_locked(channel, f, _Eff_PositionDone, (void *) args);
    }

    Mix_UnlockAudio();
    return retval;
}

int Mix_SetPosition(int channel, Sint16 angle, Uint8 distance)
{
    Mix_EffectFunc_t f = NULL;
    SDL_AudioFormat format;
    int channels;
    position_args *args = NULL;
    Sint16 room_angle = 0;
    int retval = 1;

    Mix_QuerySpec(NULL, &format, &channels);
    f = get_position_effect_func(format, channels);
    if (f == NULL)
        return 0;

    /* make angle between 0 and 359. */
    angle %= 360;
    if (angle < 0) angle += 360;

    Mix_LockAudio();
    args = get_position_arg(channel);
    if (!args) {
        Mix_UnlockAudio();
        return 0;
    }

    /* it's a no-op; unregister the effect, if it's registered. */
    if ((!distance) && (!angle)) {
        if (args->in_use) {
            retval = _Mix_UnregisterEffect_locked(channel, f);
            Mix_UnlockAudio();
            return retval;
        } else {
            Mix_UnlockAudio();
            return 1;
        }
    }

    if (channels == 2) {
        if (angle > 180)
            room_angle = 180; /* exchange left and right channels */
        else room_angle = 0;
    }

    if (channels == 4 || channels == 6) {
        if (angle > 315) room_angle = 0;
        else if (angle > 225) room_angle = 270;
        else if (angle > 135) room_angle = 180;
        else if (angle >  45) room_angle = 90;
        else room_angle = 0;
    }

    distance = 255 - distance;  /* flip it to scale Mix_SetDistance() uses. */

    set_amplitudes(channels, angle, room_angle);

    args->left_u8 = speaker_amplitude[0];
    args->left_f = ((float) speaker_amplitude[0]) / 255.0f;
    args->right_u8 = speaker_amplitude[1];
    args->right_f = ((float) speaker_amplitude[1]) / 255.0f;
    args->left_rear_u8 = speaker_amplitude[2];
    args->left_rear_f = ((float) speaker_amplitude[2]) / 255.0f;
    args->right_rear_u8 = speaker_amplitude[3];
    args->right_rear_f = ((float) speaker_amplitude[3]) / 255.0f;
    args->center_u8 = speaker_amplitude[4];
    args->center_f = ((float) speaker_amplitude[4]) / 255.0f;
    args->lfe_u8 = speaker_amplitude[5];
    args->lfe_f = ((float) speaker_amplitude[5]) / 255.0f;
    args->distance_u8 = distance;
    args->distance_f = ((float) distance) / 255.0f;
    args->room_angle = room_angle;
    if (!args->in_use) {
        args->in_use = 1;
        retval = _Mix_RegisterEffect_locked(channel, f, _Eff_PositionDone, (void *) args);
    }

    Mix_UnlockAudio();
    return retval;
}

/* end of effects_position.c ... */

/* vi: set ts=4 sw=4 expandtab: */
