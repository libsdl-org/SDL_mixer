/* umxplay.c (C) 2022, Ozkan Sezer <sezero@users.sourceforge.net>
 * Shows how to initialize the library, load an Unreal UMX music
 * module using a custom MREADER, and play it.
 *
 * This example is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRENTY; without event the implied warrenty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <signal.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <mikmod.h>
#include "umxload.h"

#if defined(_WIN32)
#define MikMod_Sleep(ns) Sleep(ns / 1000)
#elif defined(_MIKMOD_AMIGA)
void amiga_sysinit (void);
void amiga_usleep (unsigned long timeout);
#define MikMod_Sleep(ns) amiga_usleep(ns)
#else
#include <unistd.h>  /* for usleep() */
#define MikMod_Sleep(ns) usleep(ns)
#endif

typedef struct _UMXREADER {
  MREADER core;
  /* MREADER in libmikmod <= 3.2.0-beta2 doesn't have iobase */
  long iobase,prev_iobase;
  long start, length, pos;
  FILE *file;
} UMXREADER;

static UMXREADER umxreader;

static BOOL UMX_Eof(MREADER* reader)
{
  UMXREADER* r = (UMXREADER*)reader;
  if (r->pos >= r->length)
    return -1;
  return 0;
}

static BOOL UMX_Read(MREADER* reader,void* ptr,size_t size)
{
  UMXREADER* r = (UMXREADER*)reader;
  size_t ret;
  if (!size || size > LONG_MAX) return 0;
  if ((long)size > r->length - r->pos) /* just read to end */
    size = r->length - r->pos;
  ret = fread(ptr, 1, size, r->file);
  r->pos += ret;
  return !!ret;
}

static int UMX_Get(MREADER* reader)
{
  UMXREADER* r = (UMXREADER*)reader;
  if (r->pos >= r->length)
    return EOF;
  r->pos += 1;
  return fgetc(r->file);
}

static int UMX_Seek(MREADER* reader,long offset,int whence)
{
  UMXREADER* r = (UMXREADER*)reader;
  int ret;

  switch (whence) {
  case SEEK_SET:
    break;
  case SEEK_CUR:
    offset += r->pos;
    break;
  case SEEK_END:
    offset = r->length + offset;
    break;
  default: return -1;
  }

  if (offset < 0) return -1;
  if (offset > r->length) /* just seek to end */
    offset = r->length;
  ret = fseek(r->file, r->start + offset, SEEK_SET);
  if (ret < 0)
    return ret;
  r->pos = offset;
  return 0;
}

static long UMX_Tell(MREADER* reader)
{
  return ((UMXREADER*)reader)->pos;
}

static int libmikmod_init(void)
{
  /* initialize MikMod threads */
  MikMod_InitThreads ();

  /* register all the drivers */
  MikMod_RegisterAllDrivers();

  /* register all the module loaders */
  MikMod_RegisterAllLoaders();

  /* init the library */
  md_mode |= DMODE_SOFT_MUSIC;
  md_mixfreq = 44100;
  if (MikMod_Init("")) {
    fprintf(stderr, "Could not initialize sound, reason: %s\n",
            MikMod_strerror(MikMod_errno));
    return -1;
  }
  return 0;
}

static void libmikmod_deinit(void)
{
  MikMod_Exit();
}

static int quit = 0;
static void my_sighandler (int sig)
{
  (void) sig;
  quit = 1;
}

static void signals_init(void)
{
  /* handle Ctrl-C, etc. */
  #ifdef SIGBREAK
  signal(SIGBREAK, my_sighandler);
  #endif
  signal(SIGINT, my_sighandler);
  signal(SIGTERM, my_sighandler);
}

static void signals_deinit(void)
{
  /* restore signals. */
  #ifdef SIGBREAK
  signal(SIGBREAK, SIG_DFL);
  #endif
  signal(SIGINT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
}

int main(int argc, char **argv)
{
  MODULE *module;
  FILE *f;
  int type;
  int ofs, size;

  if (argc < 2) {
    fprintf(stderr, "Usage: ./umxplay filename\n");
    return 1;
  }

  f = fopen(argv[1], "rb");
  if (f == NULL) {
    perror("fopen");
    return 1;
  }

  ofs = size = 0;

  type = process_upkg(f, &ofs, &size);
  if (type < 0) {
    fclose(f);
    fprintf(stderr, "%s: unrecognized umx\n", argv[1]);
    return 1;
  }

  fprintf(stdout, "%s: %s data @ 0x%x, %d bytes\n",
          argv[1], mustype[type], ofs, size);

  /* need more than libmikmod to play wav or mp2 */
  if (type > UMUSIC_MOD) {
    fclose(f);
    fprintf(stderr, "Not playing %s data\n", mustype[type]);
    return 1;
  }

  /* seek to music start */
  fseek(f, ofs, SEEK_SET);

#ifdef _MIKMOD_AMIGA
  amiga_sysinit ();
#endif
  if (libmikmod_init() < 0) {
    return 1;
  }
  signals_init();

  /* load module */
  memset(&umxreader, 0, sizeof(UMXREADER));
  umxreader.core.Seek = UMX_Seek;
  umxreader.core.Tell = UMX_Tell;
  umxreader.core.Read = UMX_Read;
  umxreader.core.Get = UMX_Get;
  umxreader.core.Eof = UMX_Eof;
  /* hack the reader's start pos and length members so
   * that only the relevant data is accessed from now on */
  umxreader.start = ofs;
  umxreader.length = size;
  umxreader.pos = 0;
  umxreader.file = f;
  module = Player_LoadGeneric((MREADER *)&umxreader, 64, 0);
  /* close the file:
   * mikmod is done with it.  */
  fclose(f);

  if (module) {
    /* start module */
    printf("Playing %s (%d chn)\n", module->songname, (int) module->numchn);
    Player_Start(module);

    while (!quit && Player_Active()) {
      MikMod_Sleep(10000);
      MikMod_Update();
    }

    Player_Stop();
    Player_Free(module);
  } else
    fprintf(stderr, "Could not load module, reason: %s\n",
            MikMod_strerror(MikMod_errno));

  signals_deinit();
  libmikmod_deinit();

  return 0;
}
