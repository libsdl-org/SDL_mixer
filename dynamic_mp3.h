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

#include <mpg123.h>

typedef struct {
	int loaded;
	void *handle;

	int (*mpg123_close)(mpg123_handle *mh);
	void (*mpg123_delete)(mpg123_handle *mh);
	void (*mpg123_exit)(void);
	int (*mpg123_format)( mpg123_handle *mh, long rate, int channels, int encodings );
	int (*mpg123_format_none)(mpg123_handle *mh);
	int (*mpg123_getformat)( mpg123_handle *mh, long *rate, int *channels, int *encoding );
	int (*mpg123_init)(void);
	mpg123_handle *(*mpg123_new)(const char* decoder, int *error);
	int (*mpg123_open_handle)(mpg123_handle *mh, void *iohandle);
	const char* (*mpg123_plain_strerror)(int errcode);
	void (*mpg123_rates)(const long **list, size_t *number);
#if (MPG123_API_VERSION >= 45) /* api (but not abi) change as of mpg123-1.26.0 */
	int (*mpg123_read)(mpg123_handle *mh, void *outmemory, size_t outmemsize, size_t *done );
#else
	int (*mpg123_read)(mpg123_handle *mh, unsigned char *outmemory, size_t outmemsize, size_t *done );
#endif
	int (*mpg123_replace_reader_handle)( mpg123_handle *mh, ssize_t (*r_read) (void *, void *, size_t), off_t (*r_lseek)(void *, off_t, int), void (*cleanup)(void*) );
	off_t (*mpg123_seek)( mpg123_handle *mh, off_t sampleoff, int whence );
	const char* (*mpg123_strerror)(mpg123_handle *mh);
} mpg123_loader;

extern mpg123_loader mpg123;

extern int Mix_InitMP3 (void);
extern void Mix_QuitMP3 (void);

#endif /* MUSIC_MP3 */
