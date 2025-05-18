/*
    SDL_mixer:    An audio mixer library based on the SDL library
    Copyright (C) 1997-2017 Sam Lantinga <slouken@libsdl.org>

    This software is provided 'as-is', without any express or implied
    warranty.    In no event will the authors be held liable for any damages
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
*/

#ifdef MP3_MUSIC

#include <mpg123.h>
#include "SDL_rwops.h"
#include "mp3utils.h"

typedef struct
{
    struct mp3file_t mp3file;
    int freesrc;

    SDL_AudioSpec mixer;

    int playing;
    int volume;

    mpg123_handle* handle;

    int gotformat;
    SDL_AudioCVT cvt;
    Uint8 buf[8192];
    size_t len_available;
    Uint8* snd_available;
}
mpg_data;

mpg_data* mpg_new_rw(SDL_RWops *src, SDL_AudioSpec* mixer, int freesrc);
void mpg_delete(mpg_data* m);

void mpg_start(mpg_data* m);
void mpg_stop(mpg_data* m);
int mpg_playing(mpg_data* m);

int mpg_get_samples(mpg_data* m, Uint8* stream, int len);
void mpg_seek(mpg_data* m, double seconds);
void mpg_volume(mpg_data* m, int volume);

#endif
