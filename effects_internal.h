#ifndef _INCLUDE_EFFECTS_INTERNAL_H_
#define _INCLUDE_EFFECTS_INTERNAL_H_

#ifndef __MIX_INTERNAL_EFFECT__
#error You should not include this file or use these functions.
#endif

#include "SDL_mixer.h"

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

extern int _Mix_effects_max_speed;
extern void *_Eff_volume_table;
void *_Eff_build_volume_table_u8(void);
void *_Eff_build_volume_table_s8(void);

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
}
#endif


#endif

