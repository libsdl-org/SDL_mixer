/* splayFP.c
 * An example on how to use libmikmod to play a module from a FILE*
 *
 * (C) 2004, Raphael Assenat (raph@raphnet.net)
 *
 * This example is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRENTY; without event the implied warrenty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <signal.h>
#include <stdio.h>
#include <mikmod.h>

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
  FILE *fptr;

  if (argc < 2) {
    fprintf(stderr, "Usage: ./splayFP file\n");
    return 1;
  }

  /* open the file */
  fptr = fopen(argv[1], "rb");
  if (fptr == NULL) {
    perror("fopen");
    return 1;
  }

#ifdef _MIKMOD_AMIGA
  amiga_sysinit ();
#endif
  if (libmikmod_init() < 0) {
    fclose(fptr);
    return 1;
  }
  signals_init();

  /* load module */
  module = Player_LoadFP(fptr, 64, 0);
  /* close the file:
   * mikmod is done with it.  */
  fclose(fptr);

  if (module) {
    /* start module */
    printf("Playing %s\n", module->songname);
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
