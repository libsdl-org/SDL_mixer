dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])

AC_INIT([libmikmod],[3.1.22])
LIBMIKMOD_MAJOR_VERSION=3
LIBMIKMOD_MINOR_VERSION=1
LIBMIKMOD_MICRO_VERSION=22
LIBMIKMOD_VERSION=$LIBMIKMOD_MAJOR_VERSION.$LIBMIKMOD_MINOR_VERSION.$LIBMIKMOD_MICRO_VERSION

AC_CONFIG_AUX_DIR([autotools])
AM_INIT_AUTOMAKE([1.10 foreign subdir-objects])
AC_CONFIG_SRCDIR([include/mikmod_internals.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_LIBOBJ_DIR([posix])
AM_MAINTAINER_MODE

# Library versioning for libtool: CURRENT, REVISION, AGE
# - library source changed -> increment REVISION
# - interfaces added/removed/changed -> increment CURRENT, REVISION = 0
# - interfaces added -> increment AGE
# - interfaces removed -> AGE = 0
MIKMOD_LT_CURRENT=2
MIKMOD_LT_REVISION=7
MIKMOD_LT_AGE=0

AC_SUBST(MIKMOD_LT_CURRENT)
AC_SUBST(MIKMOD_LT_REVISION)
AC_SUBST(MIKMOD_LT_AGE)

# ===================
# build target:
# ===================

AC_CANONICAL_HOST

# these for convenience
libmikmod_mingw=no
libmikmod_cygwin=no
libmikmod_linux=no
libmikmod_darwin=no
libmikmod_netbsd=no
libmikmod_openbsd=no
libmikmod_os2=no
libmikmod_djgpp=no
libmikmod_amiga=no
libmikmod_haiku=no
libmikmod_irix=no
libmikmod_sun=no
libmikmod_aix=no
libmikmod_hpux=no

case $host_os in
	mingw*) libmikmod_mingw=yes ;;
	msys*) libmikmod_mingw=yes ;;
	cygwin*) libmikmod_cygwin=yes ;;
	linux*) libmikmod_linux=yes ;;
	darwin*) libmikmod_darwin=yes ;;
	netbsd*) libmikmod_netbsd=yes ;;
	openbsd*) libmikmod_openbsd=yes ;;
	emx*) libmikmod_os2=yes ;;
	*djgpp) libmikmod_djgpp=yes ;;
	amigaos*|aros*|morphos*)
		libmikmod_amiga=yes ;;
	haiku*) libmikmod_haiku=yes ;;
	hpux*) libmikmod_hpux=yes ;;
	irix*) libmikmod_irix=yes ;;
	aix*) AC_DEFINE([AIX], 1, [Define if your system is AIX 3.x - might be needed for 4.x too])
		libmikmod_aix=yes ;;
	sunos*|solaris*)
		if test "$cross_compiling" != yes
		then
		   case `uname -r` in
		     4*) AC_DEFINE([SUNOS], 1, [Define if your system is SunOS 4.x]) ;;
		   esac
		fi
		libmikmod_sun=yes ;;
esac

# ==============================================================
# libmikmod specific control variables and their default values.
# ==============================================================

libmikmod_hqmixer=yes

libmikmod_driverlist=nosound

libmikmod_driver_af=no
libmikmod_driver_aix=$libmikmod_aix
libmikmod_driver_alsa=$libmikmod_linux
libmikmod_driver_esd=yes
libmikmod_driver_hp=$libmikmod_hpux
libmikmod_driver_oss=yes
if test $libmikmod_openbsd = yes
then
  libmikmod_driver_oss=no
fi
libmikmod_driver_sam9407=no
libmikmod_driver_sgi=$libmikmod_irix
libmikmod_driver_sndio=$libmikmod_openbsd
libmikmod_driver_sun=$libmikmod_sun
if test $libmikmod_netbsd = yes
then
  libmikmod_driver_sun=yes
fi
libmikmod_driver_win=yes
libmikmod_driver_ds=yes
libmikmod_driver_ultra=no
libmikmod_driver_os2=$libmikmod_os2
libmikmod_driver_dart=$libmikmod_os2
libmikmod_driver_wav=yes
libmikmod_driver_raw=yes
libmikmod_driver_stdout=yes
libmikmod_driver_pipe=yes

libmikmod_dynload=yes
libmikmod_dynload_underscores=no
libmikmod_debug=no
libmikmod_threads=yes

libmikmod_unix=default

libmikmod_docs=yes

# =========================
# Configure script options.
# =========================

AC_ARG_ENABLE([hqmixer], [AS_HELP_STRING([--disable-hqmixer],[exclude support for high quality mixer [default=no]])],
[if test "$enableval" = "no"
then
  libmikmod_hqmixer=no
else
  libmikmod_hqmixer=yes
fi])

AC_ARG_ENABLE([alldrv], [AS_HELP_STRING([--disable-alldrv],[disable all of the audio drivers. using additional --enable-XXX options enables the specified drivers.])],
[if test "$enableval" = "no"
then
  libmikmod_driver_af=no
  libmikmod_driver_aix=no
  libmikmod_driver_alsa=no
  libmikmod_driver_esd=no
  libmikmod_driver_hp=no
  libmikmod_driver_oss=no
  libmikmod_driver_sam9407=no
  libmikmod_driver_sgi=no
  libmikmod_driver_sndio=no
  libmikmod_driver_sun=no
  libmikmod_driver_win=no
  libmikmod_driver_ds=no
  libmikmod_driver_ultra=no
  libmikmod_driver_os2=no
  libmikmod_driver_dart=no
  libmikmod_driver_wav=no
  libmikmod_driver_raw=no
  libmikmod_driver_stdout=no
  libmikmod_driver_pipe=no
fi])

AC_ARG_ENABLE([af], [AS_HELP_STRING([--enable-af],[include DEC AudioFile server driver [default=no]])],
[if test "$enableval" = "yes"
then
  libmikmod_driver_af=yes
else
  libmikmod_driver_af=no
fi])

AC_ARG_ENABLE([alsa], [AS_HELP_STRING([--enable-alsa],[Linux only: include ALSA driver [guessed]])],
[if test "$enableval" = "yes"
then
  libmikmod_driver_alsa=yes
else
  libmikmod_driver_alsa=no
fi])

AC_ARG_ENABLE([oss], [AS_HELP_STRING([--enable-oss],[include OSS driver [guessed]])],
[if test "$enableval" = "yes"
then
  libmikmod_driver_oss=yes
else
  libmikmod_driver_oss=no
fi])

AC_ARG_ENABLE([sunaudio], [AS_HELP_STRING([--enable-sunaudio],[For SunOS, Solaris, NetBSD or OpenBSD: include sunaudio driver [guessed]])],
[if test "$enableval" = "yes"
then
  libmikmod_driver_sun=yes
else
  libmikmod_driver_sun=no
fi])

AC_ARG_ENABLE([sndio], [AS_HELP_STRING([--enable-sndio],[OpenBSD only: include sndio driver [guessed]])],
[if test "$enableval" = "yes"
then
  libmikmod_driver_sndio=yes
else
  libmikmod_driver_sndio=no
fi])

AC_ARG_ENABLE([aix], [AS_HELP_STRING([--enable-aix],[AIX only: include AIX audio driver [guessed]])],
[if test "$enableval" = "yes"
then
  libmikmod_driver_aix=$libmikmod_aix
else
  libmikmod_driver_aix=no
fi])

AC_ARG_ENABLE([sgi], [AS_HELP_STRING([--enable-sgi],[IRIX only: include SGI audio driver [guessed]])],
[if test "$enableval" = "yes"
then
  libmikmod_driver_sgi=$libmikmod_irix
else
  libmikmod_driver_sgi=no
fi])

AC_ARG_ENABLE([hp], [AS_HELP_STRING([--enable-hp],[HP/UX only: include HP/UX audio driver [guessed]])],
[if test "$enableval" = "yes"
then
  libmikmod_driver_hp=$libmikmod_hpux
else
  libmikmod_driver_hp=no
fi])

AC_ARG_ENABLE([esd], [AS_HELP_STRING([--enable-esd],[include EsounD (Enlightened Sound Daemon) driver [guessed]])],
[if test "$enableval" = "yes"
then
  libmikmod_driver_esd=yes
else
  libmikmod_driver_esd=no
fi])

case $host_os in
  mingw*|emx*|*djgpp|amigaos*|aros*|morphos*)
     libmikmod_driver_oss=no
     libmikmod_driver_esd=no
     libmikmod_driver_nas=no
     libmikmod_driver_af=no
     # windows, dos, os2 don't need stdout or pipe.
     # so default to disabled, but still user-selectable.
     libmikmod_driver_pipe=no
     libmikmod_driver_stdout=no
    ;;
  *) ;;
esac

AC_ARG_ENABLE([wav], [AS_HELP_STRING([--enable-wav],[include support for output to a wav file [default=yes]])],
[if test "$enableval" = "yes"
then
  libmikmod_driver_wav=yes
else
  libmikmod_driver_wav=no
fi])

AC_ARG_ENABLE([raw], [AS_HELP_STRING([--enable-raw],[include support for output raw data to a file [default=yes]])],
[if test "$enableval" = "yes"
then
  libmikmod_driver_raw=yes
else
  libmikmod_driver_raw=no
fi])

AC_ARG_ENABLE([stdout], [AS_HELP_STRING([--enable-stdout],[include support for output to stdout [guessed]])],
[if test "$enableval" = "yes"
then
  libmikmod_driver_stdout=yes
else
  libmikmod_driver_stdout=no
fi])

AC_ARG_ENABLE([pipe], [AS_HELP_STRING([--enable-pipe],[include support for output via a pipe to another command [guessed]])],
[if test "$enableval" = "yes"
then
  libmikmod_driver_pipe=yes
else
  libmikmod_driver_pipe=no
fi])

AC_ARG_ENABLE([win], [AS_HELP_STRING([--enable-win],[Windows only: include Windows MCI driver [guessed]])],
[if test "$enableval" = "yes"
then
  libmikmod_driver_win=yes
else
  libmikmod_driver_win=no
fi])

AC_ARG_ENABLE([ds], [AS_HELP_STRING([--enable-ds],[Windows only: include Direct Sound driver [guessed]])],
[if test "$enableval" = "yes"
then
  libmikmod_driver_ds=yes
else
  libmikmod_driver_ds=no
fi])

AC_ARG_ENABLE([os2], [AS_HELP_STRING([--enable-os2],[OS/2 only: include the OS/2 MCI driver [guessed]])],
[if test "$enableval" = "yes"
then
  libmikmod_driver_os2=$libmikmod_os2
else
  libmikmod_driver_os2=no
fi])

AC_ARG_ENABLE([dart], [AS_HELP_STRING([--enable-dart],[OS/2 (Warp 4) only: include OS/2 direct audio (DART) [guessed]])],
[if test "$enableval" = "yes"
then
  libmikmod_driver_dart=$libmikmod_os2
else
  libmikmod_driver_dart=no
fi])

AC_ARG_ENABLE([sam9407], [AS_HELP_STRING([--enable-sam9407],[Linux only: include sam9407 driver [default=no]])],
[if test "$enableval" = "yes"
then
  libmikmod_driver_sam9407=$libmikmod_linux
else
  libmikmod_driver_sam9407=no
fi])

AC_ARG_ENABLE([ultra], [AS_HELP_STRING([--enable-ultra],[Linux only: include Ultra driver [default=no]])],
[if test "$enableval" = "yes"
then
  libmikmod_driver_ultra=$libmikmod_linux
else
  libmikmod_driver_ultra=no
fi])

AC_ARG_ENABLE([dl], [AS_HELP_STRING([--enable-dl],[load alsa, esound and ultra drivers at runtime [default=yes]])],
[if test "$enableval" = "yes"
then
  libmikmod_dynload=yes
else
  libmikmod_dynload=no
fi])

AC_ARG_ENABLE([debug], [AS_HELP_STRING([--enable-debug],[build a debug version of libmikmod [default=no]])],
[if test "$enableval" = "yes"
then
  libmikmod_debug=yes
else
  libmikmod_debug=no
fi])

AC_ARG_ENABLE([threads], [AS_HELP_STRING([--enable-threads],[build a thread-safe version of libmikmod [guessed]])],
[if test "$enableval" = "yes"
then
	libmikmod_threads=yes
else
	libmikmod_threads=no
fi])

AC_ARG_ENABLE([unix], [AS_HELP_STRING([--enable-unix],[override the default value of MIKMOD_UNIX in mikmod_internals.h (if you *really* need to.)])],
[if test "$enableval" = "yes"
then
  libmikmod_unix=1
elif test "$enableval" = "no"
 then
  libmikmod_unix=0
else
  libmikmod_unix=default
fi])

AC_ARG_ENABLE([doc], [AS_HELP_STRING([--enable-doc],[Build and install the documentation [default=yes]])],
[if test "$enableval" = "yes"
then
  libmikmod_docs=yes
else
  libmikmod_docs=no
fi])

# ===================
# Check for programs.
# ===================

AC_PROG_CC
AC_PROG_EGREP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
LT_INIT([win32-dll])

# =============================================================
# Check for typedefs, structures, and compiler characteristics.
# =============================================================

AC_PROG_GCC_TRADITIONAL
AC_C_CONST
AC_C_INLINE
AC_CACHE_CHECK([if compiler understands the signed keyword],[libmikmod_cv_gcc_signed],
	AC_LANG_PUSH([C])
	AC_TRY_COMPILE(,[signed int t = 0;],[libmikmod_cv_gcc_signed=yes],[libmikmod_cv_gcc_signed=no])
	AC_LANG_POP([C]))
if test $libmikmod_cv_gcc_signed = no
then
	AC_DEFINE(signed,,[Define to empty if compiler does not understand the `signed' keyword.])
fi

AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_C_BIGENDIAN

# =======================
# Check for header files.
# =======================

AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h limits.h malloc.h memory.h sys/ioctl.h unistd.h])

# ============================
# Check for library functions.
# ============================

AC_CHECK_FUNCS([setenv snprintf srandom])
AC_REPLACE_FUNCS([memcmp strstr])

# ====================
# Check for libraries.
# ====================

# dl functions
# ------------
if test $libmikmod_mingw = yes
then
	# libmikmod_dynload variable is for dlopen() api only
	libmikmod_dynload=no
fi

if test $libmikmod_dynload = yes
then
	libmikmod_dynload=no
	libmikmod_dllib=
	# SunOS-style shared libraries
	AC_CHECK_HEADERS([dlfcn.h])
	if test $ac_cv_header_dlfcn_h = yes
	then
		AC_CHECK_LIB([c],[dlopen],[libmikmod_dynload=sun],
		             [AC_CHECK_LIB([dl],[dlopen],[libmikmod_dynload=sun
		                                      libmikmod_dllib="-ldl"],[libmikmod_dynload=no])])
	else
		# HP-UX-style shared libraries
		AC_CHECK_HEADERS([dl.h])
		if test $ac_cv_header_dl_h = yes
		then
			AC_CHECK_LIB([dld],[shl_load],[libmikmod_dynload=hp
			                           libmikmod_dllib="-ldld"
			                           CFLAGS="${CFLAGS} -DMIKMOD_DLAPI_HP=1 -I${srcdir}/dlapi"],
			                          [libmikmod_dynload=no])
		fi
	fi
fi

if test $libmikmod_dynload = sun
then
	AC_CACHE_CHECK([wherever dynamically loaded symbols need underscores],[libmikmod_cv_dynload_underscores],
		libmikmod_oldlibs=$LIBS
		LIBS="$LIBS $libmikmod_dllib"
		AC_LANG_PUSH([C])
		AC_TRY_RUN(
[#include <dlfcn.h>
#ifndef RTLD_GLOBAL
#define RTLD_GLOBAL 0
#endif
int main(void) {
	void *libc, *printfptr;
	libc = dlopen("libc.so", RTLD_LAZY | RTLD_GLOBAL);
	if (!libc) exit(1);
	printfptr = dlsym(libc, "_printf");
	dlclose(libc);
	exit(!printfptr);
}],
			[libmikmod_cv_dynload_underscores=yes],
			[libmikmod_cv_dynload_underscores=no],
			[libmikmod_cv_dynload_underscores=no])
		AC_LANG_POP([C])
		LIBS=$libmikmod_oldlibs
	)
	if test $libmikmod_cv_dynload_underscores = yes
	then
		AC_DEFINE([DLSYM_NEEDS_UNDERSCORE], 1, [Define if your system needs leading underscore to function names in dlsym() calls])
	fi
fi

# POSIX.4 threads
# ---------------
if test $libmikmod_mingw = yes || test $libmikmod_amiga = yes
then
	dnl libmikmod_threads variable is for pthreads only
	libmikmod_threads=no
fi

if test $libmikmod_threads = yes
then
	libmikmod_threads=no
	AC_CACHE_CHECK([if compiler recognizes -pthread],[libmikmod_cv_gcc_pthread],
		ac_save_CFLAGS=$CFLAGS
		CFLAGS="$CFLAGS -pthread"
		AC_LANG_PUSH([C])
		AC_TRY_LINK([#include <pthread.h>],
			[int p = pthread_create(NULL,NULL,NULL,NULL);],
			[libmikmod_cv_gcc_pthread=yes],
			[libmikmod_cv_gcc_pthread=no])
		AC_LANG_POP([C])
		CFLAGS=$ac_save_CFLAGS
	)
	if test $libmikmod_cv_gcc_pthread = yes
	then
		dnl There is no need for -pthread on darwin or haiku, as gcc
		dnl complains about unrecognized option -pthread
		if test $libmikmod_darwin = no && test $libmikmod_haiku = no
		then
			libmikmod_threads=""
			CFLAGS="$CFLAGS -pthread"
		fi
	else
		dnl AC_CHECK_HEADERS(pthread.h) unreliable
		AC_CHECK_LIB([pthread],[pthread_create],[libmikmod_threads=-lpthread],
				[AC_CHECK_LIB([c_r],[pthread_create],[libmikmod_threads=-lc_r])])
	fi
fi

# ALSA - Linux only
# -----------------
if test $libmikmod_driver_alsa = yes
then
	libmikmod_driver_alsa=no
	AC_CHECK_HEADERS([alsa/asoundlib.h],[AC_CHECK_LIB([asound],[snd_pcm_open],[libmikmod_driver_alsa=yes])])
fi

# esd
# ---
if test $libmikmod_driver_esd = yes
then
	libmikmod_driver_esd=no
	AM_PATH_ESD("0.2.18",[libmikmod_driver_esd=yes])
fi

# Windows audio
# ---------------------
case $host_os in
 mingw*|msys*|cygwin*)
	# do a windows.h check, just in case..
	AC_CHECK_HEADERS([windows.h],,[AC_MSG_ERROR([Targeting windows, but windows.h not available])])
	if test $libmikmod_driver_ds = yes
	then
		libmikmod_driver_ds=no
		AC_CHECK_HEADERS([dsound.h],[libmikmod_driver_ds=yes],[],[#include <windows.h>])
	fi
	;;

 *)	libmikmod_driver_win=no
	libmikmod_driver_ds=no
	;;
esac

# OS/2 audio
# ---------------------
case $host_os in
 emx*)
	# do an os2.h check, just in case..
	AC_CHECK_HEADERS([os2.h],,[AC_MSG_ERROR([Targeting OS/2, but os2.h not available])])
	;;
esac

# sgi audio - IRIX only
# ---------------------
if test $libmikmod_driver_sgi = yes
then
	libmikmod_driver_sgi=no
	AC_CHECK_HEADERS([dmedia/audio.h],[AC_CHECK_LIB([audio],[ALseterrorhandler],[libmikmod_driver_sgi=yes])])
fi

# audiofile
# ---------
if test $libmikmod_driver_af = yes
then
	libmikmod_driver_af=no
	AC_CHECK_HEADERS([AF/AFlib.h],[AC_CHECK_LIB([AF],[AFOpenAudioConn],[libmikmod_driver_af=yes])])
fi

# libgus - Linux only
# -------------------
if test $libmikmod_driver_ultra = yes
then
	libmikmod_driver_ultra=no
	AC_CHECK_HEADERS([libgus.h])
	if test $ac_cv_header_libgus_h = yes
	then
		AC_CACHE_CHECK([if libgus is new enough],[libmikmod_cv_libgus_h_good],
		 [AC_TRY_COMPILE([#include <libgus.h>],
[gus_info_t *info;
int flg1=GUS_INSTR_SIMPLE;
int flg2=GUS_WAVE_BIDIR;],
				[libmikmod_cv_libgus_h_good=yes],[libmikmod_cv_libgus_h_good=no])])

		if test $libmikmod_cv_libgus_h_good = yes
		then
			AC_CHECK_LIB([gus],[gus_cards],[libmikmod_driver_ultra=yes])
		fi
	fi
fi

# sam9407 - Linux only
# --------------------
if test $libmikmod_driver_sam9407 = yes
then
	libmikmod_driver_sam9407=no
	AC_CHECK_HEADERS([sys/sam9407.h],[libmikmod_driver_sam9407=yes])
fi

# AIX audio
# ---------
if test $libmikmod_driver_aix = yes
then
	libmikmod_driver_aix=no
	AC_CHECK_HEADERS([sys/audio.h sys/acpa.h])
	if test $ac_cv_header_sys_audio_h = yes && test $ac_cv_header_sys_acpa_h = yes
	then
		libmikmod_driver_aix=yes
	fi
fi

# HP/UX audio
# -----------
if test $libmikmod_driver_hp = yes
then
	libmikmod_driver_hp=no
	AC_CHECK_HEADERS([sys/audio.h],[libmikmod_driver_hp=yes])
fi

# sndio - OpenBSD only
# --------------------
if test $libmikmod_driver_sndio = yes
then
	libmikmod_driver_sndio=no
	AC_CHECK_HEADERS([sndio.h],[AC_CHECK_LIB([sndio],[sio_open],[libmikmod_driver_sndio=yes])])
fi

# sunaudio - NetBSD/OpenBSD/SunOS/Solaris only
# --------------------------------------------
if test $libmikmod_driver_sun = yes
then
	libmikmod_driver_sun=no
	AC_CHECK_HEADERS([sun/audioio.h sys/audioio.h])
	if test $ac_cv_header_sun_audioio_h = yes || test $ac_cv_header_sys_audioio_h = yes
	then
		libmikmod_driver_sun=yes
	fi
fi

# ================
# Choose settings.
# ================

if test $libmikmod_driver_raw = yes
then
	libmikmod_driverlist="raw $libmikmod_driverlist"
	AC_DEFINE([DRV_RAW], 1, [Define if you want a raw pcm data file writer driver])
fi
if test $libmikmod_driver_wav = yes
then
	libmikmod_driverlist="wav $libmikmod_driverlist"
	AC_DEFINE([DRV_WAV], 1, [Define if you want a .wav file writer driver])
fi

if test $libmikmod_driver_pipe = yes
then
	libmikmod_driverlist="pipe $libmikmod_driverlist"
	AC_DEFINE([DRV_PIPE], 1, [Define if your system supports binary pipes (i.e. Unix)])
fi
if test $libmikmod_driver_stdout = yes
then
	libmikmod_driverlist="stdout $libmikmod_driverlist"
	AC_DEFINE([DRV_STDOUT], 1, [Define if you want support for output to stdout])
fi

if test $libmikmod_driver_af = yes
then
	libmikmod_driverlist="$libmikmod_driverlist audiofile"
	AC_DEFINE([DRV_AF], 1, [Define if the DEC AudioFile server driver is compiled])
	LIBRARY_LIB="-lAF $LIBRARY_LIB"
fi

if test $libmikmod_driver_win = yes
then
	libmikmod_driverlist="win $libmikmod_driverlist"
	AC_DEFINE([DRV_WIN], 1, [Define if the Windows MCI driver is compiled])
	LIBRARY_LIB="-lwinmm $LIBRARY_LIB"
fi

if test $libmikmod_driver_ds = yes
then
	libmikmod_driverlist="ds $libmikmod_driverlist"
	AC_DEFINE([DRV_DS], 1, [Define if the Windows DirectSound driver is compiled])
	LIBRARY_LIB="-ldsound $LIBRARY_LIB"
fi

if test $libmikmod_driver_os2 = yes
then
	libmikmod_driverlist="os2 $libmikmod_driverlist"
	AC_DEFINE([DRV_OS2], 1, [Define if the OS/2 MCI driver is compiled])
	# drv_os2 and drv_dart both link to mmpm2
	if test $libmikmod_driver_dart = no
	then
		LIBRARY_LIB="-lmmpm2 $LIBRARY_LIB"
	fi
fi

if test $libmikmod_driver_dart = yes
then
	libmikmod_driverlist="dart $libmikmod_driverlist"
	AC_DEFINE([DRV_DART], 1, [Define if the OS/2 direct audio (DART) driver is compiled])
	LIBRARY_LIB="-lmmpm2 $LIBRARY_LIB"
fi

if test $libmikmod_driver_aix = yes
then
	libmikmod_driverlist="aix $libmikmod_driverlist"
	AC_DEFINE([DRV_AIX], 1, [Define if the AIX audio driver is compiled])
fi

if test $libmikmod_driver_alsa = yes
then
	libmikmod_driverlist="alsa $libmikmod_driverlist"
	AC_DEFINE([DRV_ALSA], 1, [Define if the Linux ALSA driver is compiled])
	if test $libmikmod_dynload = no
	then
		LIBRARY_LIB="-lasound $LIBRARY_LIB"
	fi
fi

if test $libmikmod_driver_esd = yes
then
	libmikmod_driverlist="esd $libmikmod_driverlist"
	AC_DEFINE([DRV_ESD], 1, [Define if the Enlightened Sound Daemon driver is compiled])
	CFLAGS="$CFLAGS $ESD_CFLAGS"
	# Under Solaris and perhaps other systems, we have to forward esd
	# dependencies to libmikmod's dependency list.
	# However, no need to keep libraries we won't require (libossaudio on BSD
	# systems, libaudiofile, libasound on Linux)
	if test $libmikmod_dynload != no
	then
		ESD_LIBS="`echo $ESD_LIBS|sed -e 's/-lesd//'|sed -e 's/-lasound//'|sed -e 's/-laudiofile//'|sed -e 's/-lossaudio//'`"
	fi
	LIBRARY_LIB="$ESD_LIBS $LIBRARY_LIB"
fi

if test $libmikmod_driver_hp = yes
then
	libmikmod_driverlist="hp $libmikmod_driverlist"
	AC_DEFINE([DRV_HP], 1, [Define if the HP-UX audio driver is compiled])
fi

if test $libmikmod_driver_oss = yes && test $libmikmod_mingw = no && test $libmikmod_darwin = no
then
	AC_CHECK_HEADERS([sys/soundcard.h machine/soundcard.h soundcard.h])

	# NetBSD and OpenBSD use ossaudio emulation layer,
	# otherwise no link library is needed.
	oss_library=""
	case $host_os in
	netbsd*|openbsd*)
		AC_CHECK_LIB([ossaudio],[_oss_ioctl],[oss_library="-lossaudio"])
		;;
	esac

	AC_MSG_CHECKING([whether OSS code compiles])
	AC_TRY_COMPILE([#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif
#ifdef HAVE_SYS_SOUNDCARD_H
# include <sys/soundcard.h>
#elif defined(HAVE_MACHINE_SOUNDCARD_H)
# include <machine/soundcard.h>
#elif defined(HAVE_SOUNDCARD_H)
# include <soundcard.h>
#else
error No OSS header present.
#endif],
			[int i=SNDCTL_DSP_RESET;],
			[libmikmod_driver_oss=yes],
			[libmikmod_driver_oss=no])
	AC_MSG_RESULT([$libmikmod_driver_oss])
	if test $libmikmod_driver_oss = yes
	then
		libmikmod_driverlist="oss $libmikmod_driverlist"
		AC_DEFINE([DRV_OSS], 1, [Define if the Open Sound System driver is compiled])
		if test -n "$oss_library"
		then
			LIBRARY_LIB="$oss_library $LIBRARY_LIB"
		fi
	fi
fi

if test $libmikmod_driver_sam9407 = yes
then
	libmikmod_driverlist="sam9407 $libmikmod_driverlist"
	AC_DEFINE([DRV_SAM9407], 1, [Define if the Linux SAM9407 driver is compiled])
fi

if test $libmikmod_driver_sgi = yes
then
	libmikmod_driverlist="sgi $libmikmod_driverlist"
	AC_DEFINE([DRV_SGI], 1, [Define if the SGI audio driver is compiled])
	LIBRARY_LIB="-laudio $LIBRARY_LIB"
fi

if test $libmikmod_driver_sndio = yes
then
	libmikmod_driverlist="sndio $libmikmod_driverlist"
	AC_DEFINE([DRV_SNDIO], 1, [Define if the OpenBSD sndio driver is compiled])
	LIBRARY_LIB="-lsndio $LIBRARY_LIB"
fi

if test $libmikmod_driver_sun = yes
then
	libmikmod_driverlist="sunaudio $libmikmod_driverlist"
	AC_DEFINE([DRV_SUN], 1, [Define if the Sun audio driver or compatible (NetBSD, OpenBSD) is compiled])
fi

if test $libmikmod_driver_ultra = yes
then
	libmikmod_driverlist="ultra $libmikmod_driverlist"
	AC_DEFINE([DRV_ULTRA], 1, [Define if the Linux Ultra driver is compiled])
	if test $libmikmod_dynload = no
	then
		LIBRARY_LIB="-lgus $LIBRARY_LIB"
	fi
fi

if test $libmikmod_debug = yes
then
	AC_DEFINE([MIKMOD_DEBUG], 1, [Define if you want a debug version of the library])
	CC="$CC -g"
fi

if test $libmikmod_dynload != no
then
	AC_DEFINE([MIKMOD_DYNAMIC], 1, [Define if you want runtime dynamic linking of ALSA and EsounD drivers])

	LIBRARY_LIB="$libmikmod_dllib $LIBRARY_LIB"
	if test $libmikmod_dynload = sun
	then
		# RTLD_GLOBAL is not defined under every system
		AC_CACHE_CHECK([if RTLD_GLOBAL is defined],libmikmod_cv_decl_rtld_global,
		 [AC_TRY_COMPILE([#include <dlfcn.h>],[int flag=RTLD_GLOBAL;],
			[libmikmod_cv_decl_rtld_global=yes],
			[libmikmod_cv_decl_rtld_global=no])])
		if test $libmikmod_cv_decl_rtld_global = yes
		then
			AC_DEFINE([HAVE_RTLD_GLOBAL], 1, [Define if your system has RTLD_GLOBAL defined in <dlfcn.h>])
		fi
	fi
fi

# If compiling with gcc, use adequate optimization flags
if test $ac_cv_c_compiler_gnu = yes
then
	CFLAGS="$CFLAGS -Wall"
	if test $libmikmod_debug = yes
	then
		CFLAGS="$CFLAGS -Werror"
	fi
	CFLAGS="$CFLAGS -ffast-math"
	if test $libmikmod_darwin = yes
	then
		CFLAGS="$CFLAGS -fno-common"
	fi

	# On at least x86 platforms, gcc 2.7.2.1 and earlier won't work if -O3 (or
	# -finline-functions) are used. Versions 2.7.2.3, 2.8.x and egcs are ok
	# (didn't test 2.7.2.2).
	# Until there's an easy way to catch broken inlining, we choose flags known
	# to work correctly depending of the compiler version.
	AC_CACHE_CHECK([if inlining functions is safe],[libmikmod_cv_gcc_inline_safe],
	 [AC_TRY_COMPILE(,[
#if !(defined __GNUC__ && (__GNUC__ > 2 || __GNUC__ == 2 && __GNUC_MINOR__ > 7))
#error inlining functions not safe
#endif
], [libmikmod_cv_gcc_inline_safe=yes], [libmikmod_cv_gcc_inline_safe=no])])
	if test $libmikmod_cv_gcc_inline_safe = no
	then
		CFLAGS="$CFLAGS -fno-strength-reduce"
	fi
fi

if test x$libmikmod_threads != xno
then
	AC_DEFINE([HAVE_PTHREAD], 1, [Define if your system provides POSIX.4 threads])
	CFLAGS="$CFLAGS -D_REENTRANT"
	LIBRARY_LIB="$libmikmod_threads $LIBRARY_LIB"
	REENTRANT="-D_REENTRANT"
	if test $libmikmod_cv_gcc_pthread = yes
	then
		dnl There is no need for -pthread on darwin or haiku, as gcc
		dnl complains about unrecognized option -pthread
		if test $libmikmod_darwin = no && test $libmikmod_haiku = no
		then
			REENTRANT="-pthread $REENTRANT"
			LIB_LDADD="-pthread"
		fi
	fi
fi

# MIKMOD_UNIX override
if test "$libmikmod_unix" = "0" || test "$libmikmod_unix" = "1"
then
   AC_DEFINE_UNQUOTED([MIKMOD_UNIX], $libmikmod_unix, [Define to 0 or 1 to override MIKMOD_UNIX in mikmod_internals.h.])
fi

if test $libmikmod_hqmixer = no
then
	AC_DEFINE([NO_HQMIXER], 1, [disable the high quality mixer (build only with the standart mixer)])
fi

# symbol visibility
ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -fvisibility=hidden -Werror"
AC_CACHE_CHECK([if compiler supports visibility attributes],[libmikmod_cv_gcc_visibility],
	AC_LANG_PUSH([C])
	AC_TRY_COMPILE([
__attribute__((visibility("default"))) int foo(void);
__attribute__((visibility("hidden")))  int bar(void);
int foo(void) { return 0; }
int bar(void) { return 1; }],
		[],
		[libmikmod_cv_gcc_visibility=yes],
		[libmikmod_cv_gcc_visibility=no])
	AC_LANG_POP([C]))
# we want symbol -fvisibility for elf targets, however it works
# with darwin/macho too.  other than that, windows, dos, os/2, amiga
# do not need it:  for any such targets, the -Werror switch is
# supposed to fail the above check. (I'm adding the manual test
# below nonetheless, just in case.)
case $host_os in
   mingw*|cygwin*|emx*|*djgpp|amigaos*|aros*|morphos*)
	libmikmod_cv_gcc_visibility=no
	;;
esac
CFLAGS="$ac_save_CFLAGS"
if test $libmikmod_cv_gcc_visibility = yes
then
   CFLAGS="$CFLAGS -DSYM_VISIBILITY -fvisibility=hidden"
fi

# =================
# Create Makefiles.
# =================

AM_CONDITIONAL([BUILD_DOCS], [test "$libmikmod_docs" = yes])

AC_SUBST(REENTRANT)

AC_SUBST(LIBRARY_LIB)

AC_SUBST(LIBMIKMOD_MAJOR_VERSION)
AC_SUBST(LIBMIKMOD_MINOR_VERSION)
AC_SUBST(LIBMIKMOD_MICRO_VERSION)
AC_SUBST(LIBMIKMOD_VERSION)

AC_SUBST(LIB_LDADD)

AC_CONFIG_FILES([
libmikmod-config
libmikmod.pc
libmikmod.spec
Makefile
docs/Makefile
docs/libmikmod-config.1
dlapi/Makefile
drivers/Makefile
include/Makefile
loaders/Makefile
mmio/Makefile
playercode/Makefile
posix/Makefile])
AC_CONFIG_HEADERS([config.h])
AC_OUTPUT

chmod +x libmikmod-config

# ====================
# Print configuration.
# ====================

if test $libmikmod_dynload != no
then
	libmikmod_dynload="yes, ${libmikmod_dynload} style"
fi

echo "
Library configuration:

  Source code location: $srcdir
  Compiler: $CC
  Compiler flags: $CFLAGS
  Debug version: $libmikmod_debug
  Dynamically loaded drivers whenever possible: $libmikmod_dynload
  Drivers to be compiled: $libmikmod_driverlist
  HQ Mixer: $libmikmod_hqmixer
  Install path: $prefix/lib, $prefix/include
"
