/* splayMEM.c
 * An example on how to use libmikmod to play a module,
 * but to load it with a custom MREADER.
 *
 * (C) 2004, Raphael Assenat (raph@raphnet.net)
 *
 * This example is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRENTY; without event the implied warrenty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <signal.h>
#include <stdlib.h>
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

#include "myloader.h"


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
  unsigned char *data_buf;
  long data_len;
  FILE *fptr;
  MREADER *mem_reader;

  if (argc < 2) {
    fprintf(stderr, "Usage: ./splayMEM file\n");
    return 1;
  }

  /* open the file */
  fptr = fopen(argv[1], "rb");
  if (fptr == NULL) {
    perror("fopen");
    return 1;
  }

  /* calculate the file size */
  fseek(fptr, 0, SEEK_END);
  data_len = ftell(fptr);
  fseek(fptr, 0, SEEK_SET);

  /* allocate a buffer and load the file into it */
  data_buf = malloc(data_len);
  if (data_buf == NULL) {
    perror("malloc");
    fclose(fptr);
    return 1;
  }
  if (fread(data_buf, data_len, 1, fptr) != 1) {
    perror("fread");
    fclose(fptr);
    free(data_buf);
    return 1;
  }
  fclose(fptr);

  /* Create the memory reader */
  mem_reader = my_new_mem_reader(data_buf, data_len);
  if (mem_reader == NULL) {
    free(data_buf);
    fprintf(stderr, "failed to create mem reader\n");
    return 1;
  }

#ifdef _MIKMOD_AMIGA
  amiga_sysinit ();
#endif
  if (libmikmod_init() < 0) {
    free(data_buf);
    return 1;
  }
  signals_init();

  /* load module */
  module = Player_LoadGeneric(mem_reader, 64, 0);
  /* free the buffer and reader:
   * mikmod is done with them.  */
  my_delete_mem_reader(mem_reader);
  free(data_buf);

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
