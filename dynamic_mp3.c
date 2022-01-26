/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2012 Sam Lantinga <slouken@libsdl.org>

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
*/

#ifdef MP3_MUSIC

#include "SDL_loadso.h"

#include "dynamic_mp3.h"

mpg123_loader mpg123 = {
	0, NULL
};

#ifdef MPG123_DYNAMIC
#define FUNCTION_LOADER(FUNC, SIG) \
	mpg123.FUNC = (SIG) SDL_LoadFunction(mpg123.handle, #FUNC); \
	if (mpg123.FUNC == NULL) { goto initerr; }
#else
#define FUNCTION_LOADER(FUNC, SIG) \
	mpg123.FUNC = FUNC;
#endif

int Mix_InitMP3(void)
{
	if (mpg123.loaded == 0) {
#ifdef MPG123_DYNAMIC
		mpg123.handle = SDL_LoadObject(MPG123_DYNAMIC);
		if (mpg123.handle == NULL) {
			return -1;
		}
#endif
		FUNCTION_LOADER(mpg123_close, int (*)(mpg123_handle *mh))
		FUNCTION_LOADER(mpg123_delete, void (*)(mpg123_handle *mh))
		FUNCTION_LOADER(mpg123_exit, void (*)(void))
		FUNCTION_LOADER(mpg123_format, int (*)( mpg123_handle *mh, long rate, int channels, int encodings ))
		FUNCTION_LOADER(mpg123_format_none, int (*)(mpg123_handle *mh))
		FUNCTION_LOADER(mpg123_getformat, int (*)( mpg123_handle *mh, long *rate, int *channels, int *encoding ))
		FUNCTION_LOADER(mpg123_init, int (*)(void))
		FUNCTION_LOADER(mpg123_new, mpg123_handle *(*)(const char* decoder, int *error))
		FUNCTION_LOADER(mpg123_open_handle, int (*)(mpg123_handle *mh, void *iohandle))
		FUNCTION_LOADER(mpg123_plain_strerror, const char* (*)(int errcode))
		FUNCTION_LOADER(mpg123_rates, void (*)(const long **list, size_t *number))
#if (MPG123_API_VERSION >= 45) /* api (but not abi) change as of mpg123-1.26.0 */
		FUNCTION_LOADER(mpg123_read, int (*)(mpg123_handle *mh, void *outmemory, size_t outmemsize, size_t *done ))
#else
		FUNCTION_LOADER(mpg123_read, int (*)(mpg123_handle *mh, unsigned char *outmemory, size_t outmemsize, size_t *done ))
#endif
		FUNCTION_LOADER(mpg123_replace_reader_handle, int (*)( mpg123_handle *mh, ssize_t (*r_read) (void *, void *, size_t), off_t (*r_lseek)(void *, off_t, int), void (*cleanup)(void*) ))
		FUNCTION_LOADER(mpg123_seek, off_t (*)( mpg123_handle *mh, off_t sampleoff, int whence ))
		FUNCTION_LOADER(mpg123_strerror, const char* (*)(mpg123_handle *mh))
		if (mpg123.mpg123_init() != MPG123_OK) {
#ifdef MPG123_DYNAMIC
			initerr:
			SDL_UnloadObject(mpg123.handle);
#endif
			return -1;
		}
	}
	++mpg123.loaded;

	return 0;
}

void Mix_QuitMP3(void)
{
	if (mpg123.loaded == 0) {
		return;
	}
	if (mpg123.loaded == 1) {
		mpg123.mpg123_exit();
#ifdef MPG123_DYNAMIC
		SDL_UnloadObject(mpg123.handle);
#endif
	}
	--mpg123.loaded;
}

#endif /* MP3_MUSIC */
