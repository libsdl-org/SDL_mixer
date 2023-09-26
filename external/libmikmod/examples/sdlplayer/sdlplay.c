/* sdlplay.c: A very basic example on how to use libmikmod to play
 * a module using SDL to output audio: SDL2 or SDL-1.2 can be used.
 *
 * This file is in public domain.
 */

#include <stdio.h>
#include <signal.h>
#include "SDL.h"
#include <mikmod.h>


static void SDLCALL fill_audio(void *udata, Uint8 *stream, int len)
{
  (void) udata; /* not used. */
  VC_WriteBytes((SBYTE *)stream, len);
}

static int sdl_init(void)
{
  SDL_AudioSpec spec;

  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    fprintf(stderr, "Can't initialize SDL: %s\n", SDL_GetError());
    return -1;
  }

  spec.freq = 44100;
  spec.format = AUDIO_S16;
  spec.channels = 2;
  spec.samples = 2048;
  spec.callback = fill_audio;
  spec.userdata = NULL;

  if (SDL_OpenAudio(&spec, NULL) < 0) {
    fprintf(stderr, "Can't initialize SDL audio: %s\n", SDL_GetError());
    SDL_Quit();
    return -1;
  }

  return 0;
}

static void sdl_deinit(void)
{
  SDL_CloseAudio();
  SDL_Quit();
}

static int libmikmod_init(void)
{
  /* initialize MikMod threads */
  MikMod_InitThreads ();

  /* register only 'nosound' driver: we
   * will be using SDL for audio output. */
  MikMod_RegisterDriver(&drv_nos);

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
  int i;

  if (argc < 2) {
    fprintf(stderr, "Usage: ./sdlplay file\n");
    return 1;
  }

  if (sdl_init() < 0) {
    return 1;
  }
  if (libmikmod_init() < 0) {
    sdl_deinit();
    return 1;
  }
  signals_init();

  for (i = 1; !quit && i < argc; ++i) {
    module = Player_Load(argv[i], 64, 0);
    if (!module) {
      fprintf(stderr, "Could not load %s: %s\n",
              argv[i], MikMod_strerror(MikMod_errno));
      continue;
    }
    /* start module */
    printf("Playing %s (%d chn)\n", module->songname, (int) module->numchn);
    Player_Start(module);
    SDL_PauseAudio(0);

    while (!quit && Player_Active()) {
      SDL_Delay(10);
    }

    SDL_PauseAudio(1);
    Player_Stop();
    Player_Free(module);
  }

  signals_deinit();
  libmikmod_deinit();
  sdl_deinit();

  return 0;
}
