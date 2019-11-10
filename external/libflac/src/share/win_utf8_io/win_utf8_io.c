/* libFLAC - Free Lossless Audio Codec library
 * Copyright (C) 2013  Xiph.Org Foundation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of the Xiph.org Foundation nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h> /* for WideCharToMultiByte and MultiByteToWideChar */

#include "share/compat.h"
#include "share/win_utf8_io.h"

static UINT win_utf8_io_codepage = CP_ACP;

/* convert UTF-8 back to WCHAR. Caller is responsible for freeing memory */
static
wchar_t *wchar_from_utf8(const char *str)
{
	wchar_t *widestr;
	int len;

	if (!str) return NULL;
	len=(int)strlen(str)+1;
	if ((widestr = (wchar_t *)malloc(len*sizeof(wchar_t))) != NULL) {
		if (MultiByteToWideChar(win_utf8_io_codepage, 0, str, len, widestr, len) == 0) {
			if (MultiByteToWideChar(CP_ACP, 0, str, len, widestr, len) == 0) { /* try conversion from Ansi in case the initial UTF-8 conversion had failed */
				free(widestr);
				widestr = NULL;
			}
		}
	}

	return widestr;
}

/* file functions */

FILE *fopen_utf8(const char *filename, const char *mode)
{
	wchar_t *wname = NULL;
	wchar_t *wmode = NULL;
	FILE *f = NULL;

	while (1) {
		if (!(wname = wchar_from_utf8(filename))) break;
		if (!(wmode = wchar_from_utf8(mode))) break;
		f = _wfopen(wname, wmode);
		break;
	}
	if (wname) free(wname);
	if (wmode) free(wmode);

	return f;
}

#ifdef FLAC_METADATA_INTERFACES
int _stat64_utf8(const char *path, struct __stat64 *buffer)
{
	wchar_t *wpath;
	int ret;

	if (!(wpath = wchar_from_utf8(path))) return -1;
	ret = _wstat64(wpath, buffer);
	free(wpath);

	return ret;
}

int chmod_utf8(const char *filename, int pmode)
{
	wchar_t *wname;
	int ret;

	if (!(wname = wchar_from_utf8(filename))) return -1;
	ret = _wchmod(wname, pmode);
	free(wname);

	return ret;
}

int utime_utf8(const char *filename, struct utimbuf *times)
{
	wchar_t *wname;
	struct __utimbuf64 ut;
	int ret;

	if (sizeof(*times) == sizeof(ut)) {
		memcpy(&ut, times, sizeof(ut));
	} else {
		ut.actime = times->actime;
		ut.modtime = times->modtime;
	}

	if (!(wname = wchar_from_utf8(filename))) return -1;
	ret = _wutime64(wname, &ut);
	free(wname);

	return ret;
}

int unlink_utf8(const char *filename)
{
	wchar_t *wname;
	int ret;

	if (!(wname = wchar_from_utf8(filename))) return -1;
	ret = _wunlink(wname);
	free(wname);

	return ret;
}

int rename_utf8(const char *oldname, const char *newname)
{
	wchar_t *wold = NULL;
	wchar_t *wnew = NULL;
	int ret = -1;

	while (1) {
		if (!(wold = wchar_from_utf8(oldname))) break;
		if (!(wnew = wchar_from_utf8(newname))) break;
		ret = _wrename(wold, wnew);
		break;
	}
	if (wold) free(wold);
	if (wnew) free(wnew);

	return ret;
}
#endif /* FLAC_METADATA_ITERATORS */

