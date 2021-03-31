dnl Process this file with autoconf to produce a configure script

dnl ------------------------------------------------
dnl Initialization and Versioning
dnl ------------------------------------------------

AC_INIT
AC_CONFIG_SRCDIR([mdct.c])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE(libvorbisidec,1.2.1)

dnl AM_MAINTAINER_MODE only provides the option to configure to enable it
AM_MAINTAINER_MODE

dnl Library versioning

V_LIB_CURRENT=1
V_LIB_REVISION=3
V_LIB_AGE=0
AC_SUBST(V_LIB_CURRENT)
AC_SUBST(V_LIB_REVISION)
AC_SUBST(V_LIB_AGE)

dnl --------------------------------------------------
dnl Check for programs
dnl --------------------------------------------------

dnl save $CFLAGS since AC_PROG_CC likes to insert "-g -O2"
dnl if $CFLAGS is blank
cflags_save="$CFLAGS"
AC_PROG_CC
AC_PROG_CPP
CFLAGS="$cflags_save"

dnl AC_LIBTOOL_WIN32_DLL
dnl AC_PROG_LIBTOOL
LT_INIT([win32-dll])

# Test whenever ld supports -version-script
dnl AC_PROG_LD
dnl AC_PROG_LD_GNU
LT_PATH_LD

dnl check endianism
AC_C_BIGENDIAN


dnl --------------------------------------------------
dnl Set build flags based on environment
dnl --------------------------------------------------

dnl Set some target options

cflags_save="$CFLAGS"
ldflags_save="$LDFLAGS"
if test -z "$GCC"; then
        case $host in
        arm-*-*)
                DEBUG="-g -D_ARM_ASSEM_"
                CFLAGS="-O -D_ARM_ASSEM_"
                PROFILE="-p -g -O -D_ARM_ASSEM_" ;;
        *)
                DEBUG="-g"
                CFLAGS="-O"
                PROFILE="-g -p" ;;
        esac
else

        case $host in
        arm-*-*)
                DEBUG="-g -Wall -D__NO_MATH_INLINES -fsigned-char -D_ARM_ASSEM_"
                CFLAGS="-O2 -D_ARM_ASSEM_ -fsigned-char"
                PROFILE="-W -pg -g -O2 -D_ARM_ASSEM_ -fsigned-char -fno-inline-functions";;

        *)
                DEBUG="-g -Wall -D__NO_MATH_INLINES -fsigned-char"
                CFLAGS="-O2 -Wall -fsigned-char"
                PROFILE="-Wall -pg -g -O2 -fsigned-char -fno-inline-functions";;
        esac
fi
CFLAGS="$CFLAGS $cflags_save -D_REENTRANT"
LDFLAGS="$LDFLAGS $ldflags_save"


dnl --------------------------------------------------
dnl Options
dnl --------------------------------------------------

AC_ARG_ENABLE(low-accuracy,
   [AS_HELP_STRING([--enable-low-accuracy], [enable 32 bit only multiply operations])],
   CFLAGS="$CFLAGS -D_LOW_ACCURACY_"
)

AC_ARG_ENABLE(alloca,
   [AS_HELP_STRING([--disable-alloca], [disable alloca and only use variable-length arrays])],,
   [enable_alloca=yes]
)

dnl --------------------------------------------------
dnl Check for headers
dnl --------------------------------------------------

AC_CHECK_HEADERS(sys/types.h)
AC_CHECK_HEADER(memory.h,CFLAGS="$CFLAGS -DUSE_MEMORY_H",:)

dnl --------------------------------------------------
dnl Check for typedefs, structures, etc
dnl --------------------------------------------------

dnl none

dnl --------------------------------------------------
dnl Check for libraries
dnl --------------------------------------------------

PKG_PROG_PKG_CONFIG

HAVE_OGG=no
if test "x$PKG_CONFIG" != "x"
then
  PKG_CHECK_MODULES(OGG, ogg >= 1.0, HAVE_OGG=yes, HAVE_OGG=no)
fi
if test "x$HAVE_OGG" = "xno"
then
  dnl fall back to the old school test
  XIPH_PATH_OGG(, AC_MSG_ERROR(must have Ogg installed!))
  libs_save=$LIBS
  LIBS="$OGG_LIBS"
  AC_CHECK_FUNC(oggpack_writealign, , AC_MSG_ERROR(Ogg >= 1.0 required !))
  LIBS=$libs_save
fi

dnl --------------------------------------------------
dnl Stack allocation mode
dnl --------------------------------------------------

if test "x$enable_alloca" = "xyes"; then
  AC_FUNC_ALLOCA
fi

AC_MSG_CHECKING(for C99 variable-size arrays)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]],
  [[static int x; char a[++x]; a[sizeof a - 1] = 0; int N; return a[0];]] )],
  [has_var_arrays=yes;AC_DEFINE([VAR_ARRAYS],[],[Use C99 variable-size arrays])],
  [has_var_arrays=no]
)
AC_MSG_RESULT($has_var_arrays)


dnl --------------------------------------------------
dnl Version script (exported symbols control.)
dnl --------------------------------------------------

SHLIB_VERSION_ARG=""
if test "x$lt_cv_prog_gnu_ld" = "xyes"; then
   SHLIB_VERSION_ARG="-Wl,--version-script=Version_script"
fi
AC_SUBST(SHLIB_VERSION_ARG)


dnl --------------------------------------------------
dnl Do substitutions
dnl --------------------------------------------------

LIBS="$LIBS"

AC_SUBST(LIBS)
AC_SUBST(DEBUG)
AC_SUBST(PROFILE)

AC_CONFIG_FILES([Makefile Version_script vorbisidec.pc])
AC_OUTPUT
