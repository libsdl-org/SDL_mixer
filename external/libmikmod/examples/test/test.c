/* simple app to run under valgrind for testing, etc... */
#include <signal.h>
#include <mikmod.h>

static int libmikmod_init(void)
{
  /* initialize MikMod threads */
  MikMod_InitThreads ();

  /* register only 'nosound' driver. */
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

  if (argc < 2) {
    fprintf(stderr, "Usage: ./splay file\n");
    return 1;
  }

  if (libmikmod_init() < 0) {
    return 1;
  }
  signals_init();

  /* load module */
  module = Player_Load(argv[1], 64, 0);
  if (module) {
    /* start module */
    printf("Playing %s (%d chn)\n", module->songname, (int) module->numchn);

    module->loop = 0; /* disable in-module loops */
    Player_Start(module);

    while (!quit && Player_Active()) {
    /* call update without usleep() or something: we only registered null driver */
      MikMod_Update();
    }

    Player_Stop();
    Player_Free(module);
  }
  else
    fprintf(stderr, "Could not load module, reason: %s\n",
            MikMod_strerror(MikMod_errno));

  signals_deinit();
  libmikmod_deinit();

  return 0;
}
