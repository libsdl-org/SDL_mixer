/* config.h.in.  Generated for macintosh builds.  */

/* Define if you have the ANSI C header files.  */
/* #undef STDC_HEADERS */

/* disable the high quality mixer (build only with the standart mixer) */
/* #define NO_HQMIXER */

/* Define if you want support for output to stdout */
/* #undef DRV_STDOUT */

/* Define if your system supports binary pipes (i.e. Unix) */
/* #undef DRV_PIPE */

/* Define if you want a raw pcm data file writer driver */
#define DRV_RAW 1

/* Define if you want a .wav file writer driver */
#define DRV_WAV 1

/* Define if the Macintosh driver is compiled */
#define DRV_MAC 1

/* Define if you want a debug version of the library */
/* #undef MIKMOD_DEBUG */

/* Define if you have the srandom function.  */
/* #undef HAVE_SRANDOM */

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <malloc.h> header file.  */
/* #undef HAVE_MALLOC_H */

/* Define if you have the <sys/ioctl.h> header file.  */
/* #undef HAVE_SYS_IOCTL_H */

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

