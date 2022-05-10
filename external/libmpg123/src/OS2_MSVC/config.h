/* hand-edited libmpg123 config for OS/2 (EMX or Watcom), Win32/Watcom, and Visual Studio 6 */

/* Define if your architecture wants/needs/can use attribute_align_arg and
   alignment checks. It is for 32bit x86... */
#define ABI_ALIGN_FUN 1

/* Define if .balign is present. */
#define ASMALIGN_BALIGN 1

/* Define if .align just takes byte count. */
/* #undef ASMALIGN_BYTE */

/* Define if .align takes 3 for alignment of 2^3=8 bytes instead of 8. */
/* #undef ASMALIGN_EXP */

/* Define if __attribute__((aligned(16))) shall be used */
#if defined(__GNUC__) && (__GNUC__ > 2) /* possibly need binutils >= 2.11.2 too */
#define CCALIGN 1
#endif

/* Define to indicate that float storage follows IEEE754. */
#define IEEE_FLOAT 1

/* Define to use Unicode for Windows */
/* #undef WANT_WIN32_UNICODE */

/* Use EFBIG as substitude for EOVERFLOW */
#if !(defined(__INNOTEK_LIBC__) || defined(__KLIBC__))
#define EOVERFLOW EFBIG
#endif

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1
/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <signal.h> header file. */
#define HAVE_SIGNAL_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <strings.h> header file. */
#if defined(__WATCOMC__) && (__WATCOMC__ >= 1240)
#define HAVE_STRINGS_H 1
#endif

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/select.h> header file. */
#ifdef __EMX__ /* !! */
#define HAVE_SYS_SELECT_H 1
#endif

/* Define to 1 if you have the <sys/time.h> header file. */
#ifdef __EMX__ /* !! */
#define HAVE_SYS_TIME_H 1
#endif

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#ifdef __EMX__ /* !! */
#define HAVE_UNISTD_H 1
#endif

/* Define to 1 if you have the `random' function. */
/* #undef HAVE_RANDOM */

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define if debugging is enabled. */
/* #undef DEBUG */

/* Define to disable error messages in combination with a return value (the
   return is left intact). */
#define NO_ERETURN 1

/* Define to disable error messages. */
#define NO_ERRORMSG 1

/* Define to disable warning messages. */
#define NO_WARNING 1

/* Define to disable all messages from library. */
#define LIBMPG123_QUIET 1

/* Define if network support is enabled. */
/* #undef NETWORK */

/* Define if frame index should be used. */
#define FRAME_INDEX 1

/* Define if gapless is enabled. */
#define GAPLESS 1

/* size of the frame index seek table */
#define INDEX_SIZE 1000

/* Define to use proper rounding. */
/* #undef ACCURATE_ROUNDING */

/* Define for new Huffman decoding scheme. */
#define USE_NEW_HUFFTABLE 1

/* Define to disable 16 bit integer output. */
/* #undef NO_16BIT */

/* Define to disable 32 bit and 24 bit integer output. */
/* #undef NO_32BIT */

/* Define to disable 8 bit integer output. */
/* #undef NO_8BIT */

/* Define to disable downsampled decoding. */
/* #undef NO_DOWNSAMPLE */

/* Define to disable equalizer. */
#define NO_EQUALIZER 1

/* Define to disable feeder and buffered readers. */
/* #undef NO_FEEDER */

/* Define to disable ICY handling. */
#define NO_ICY 1

/* Define to disable ID3v2 parsing. */
/* #undef NO_ID3V2 */

/* Define to disable layer I. */
/* #undef NO_LAYER1 */

/* Define to disable layer II. */
/* #undef NO_LAYER2 */

/* Define to disable layer III. */
/* #undef NO_LAYER3 */

/* Define to disable ntom resampling. */
/* #undef NO_NTOM */

/* Define to disable real output. */
/* #undef NO_REAL */

/* Define to disable string functions. */
/* #undef NO_STRING */

/* Define for post-processed 32 bit formats. */
/* #undef NO_SYNTH32 */

/* The size of `int32_t', as computed by sizeof. */
#define SIZEOF_INT32_T 4

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* The size of `off_t', as computed by sizeof. */
#define SIZEOF_OFF_T 4

/* The size of `size_t', as computed by sizeof. */
#define SIZEOF_SIZE_T 4

/* The size of `ssize_t', as computed by sizeof. */
#define SIZEOF_SSIZE_T 4

/* Define to the native offset type (long or actually off_t). */
#define lfs_alias_t off_t

/* Define this to the size of native offset type in bits, used for LFS alias
   functions. */
#define LFS_ALIAS_BITS 32

#if defined(_MSC_VER) && !defined(__cplusplus)
#define inline __inline
#endif
