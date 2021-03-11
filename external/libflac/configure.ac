#  FLAC - Free Lossless Audio Codec
#  Copyright (C) 2001,2002,2003,2004,2005,2006,2007,2008,2009  Josh Coalson
#
#  This file is part the FLAC project.  FLAC is comprised of several
#  components distributed under different licenses.  The codec libraries
#  are distributed under Xiph.Org's BSD-like license (see the file
#  COPYING.Xiph in this distribution).  All other programs, libraries, and
#  plugins are distributed under the GPL (see COPYING.GPL).  The documentation
#  is distributed under the Gnu FDL (see COPYING.FDL).  Each file in the
#  FLAC distribution contains at the top the terms under which it may be
#  distributed.
#
#  Since this particular file is relevant to all components of FLAC,
#  it may be distributed under the Xiph.Org license, which is the least
#  restrictive of those mentioned above.  See the file COPYING.Xiph in this
#  distribution.

# NOTE that for many of the AM_CONDITIONALs we use the prefix FLaC__
# instead of FLAC__ since autoconf triggers off 'AC_' in strings

AC_PREREQ(2.60)
AC_INIT([flac], [1.3.0], [flac-dev@xiph.org], [flac], [https://www.xiph.org/flac/])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/libFLAC/format.c])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign 1.11 -Wall tar-pax no-dist-gzip dist-xz subdir-objects])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AM_MAINTAINER_MODE

user_cflags=$CFLAGS

#Prefer whatever the current ISO standard is.
AC_PROG_CC_STDC
AC_USE_SYSTEM_EXTENSIONS
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT([win32-dll disable-static pic-only])
AM_PROG_AS
AC_PROG_MAKE_SET

AC_SYS_LARGEFILE
AC_FUNC_FSEEKO

AC_CHECK_SIZEOF(off_t,1)	# Fake default value.
AC_CHECK_SIZEOF([void*])
AC_SEARCH_LIBS([lround],[m], [AC_DEFINE(HAVE_LROUND,1,lround support)])

# c flavor
AC_HEADER_STDC
AM_PROG_CC_C_O
AC_C_INLINE
AC_C_VARARRAYS
AC_C_TYPEOF

AC_CHECK_HEADERS([stdint.h inttypes.h byteswap.h sys/param.h termios.h])

AC_HEADER_TIOCGWINSZ

XIPH_C_BSWAP32

ac_cv_c_big_endian=0
ac_cv_c_little_endian=0
AC_C_BIGENDIAN([ac_cv_c_big_endian=1], [ac_cv_c_little_endian=1], [
	AC_MSG_WARN([[*****************************************************************]])
	AC_MSG_WARN([[*** Not able to determine endian-ness of target processor.       ]])
	AC_MSG_WARN([[*** The constants CPU_IS_BIG_ENDIAN and CPU_IS_LITTLE_ENDIAN in  ]])
	AC_MSG_WARN([[*** config.h may need to be hand editied.                        ]])
	AC_MSG_WARN([[*****************************************************************]])
])
AC_DEFINE_UNQUOTED(CPU_IS_BIG_ENDIAN, ${ac_cv_c_big_endian},
					[Target processor is big endian.])
AC_DEFINE_UNQUOTED(CPU_IS_LITTLE_ENDIAN, ${ac_cv_c_little_endian},
					[Target processor is little endian.])
AC_DEFINE_UNQUOTED(WORDS_BIGENDIAN, ${ac_cv_c_big_endian},
					[Target processor is big endian.])

dnl check for getopt in standard library
dnl AC_CHECK_FUNCS(getopt_long , , [LIBOBJS="$LIBOBJS getopt.o getopt1.o"] )
AC_CHECK_FUNCS(getopt_long, [], [])

case "$host_cpu" in
	i*86)
		cpu_ia32=true
		AC_DEFINE(FLAC__CPU_IA32)
		AH_TEMPLATE(FLAC__CPU_IA32, [define if building for ia32/i386])
		;;
	powerpc|powerpc64)
		cpu_ppc=true
		AC_DEFINE(FLAC__CPU_PPC)
		AH_TEMPLATE(FLAC__CPU_PPC, [define if building for PowerPC])
		;;
	sparc)
		cpu_sparc=true
		AC_DEFINE(FLAC__CPU_SPARC)
		AH_TEMPLATE(FLAC__CPU_SPARC, [define if building for SPARC])
		;;
esac
AM_CONDITIONAL(FLaC__CPU_IA32, test "x$cpu_ia32" = xtrue)
AM_CONDITIONAL(FLaC__CPU_PPC, test "x$cpu_ppc" = xtrue)
AM_CONDITIONAL(FLaC__CPU_SPARC, test "x$cpu_sparc" = xtrue)

case "$host" in
	i386-*-openbsd3.[[0-3]]) OBJ_FORMAT=aoutb ;;
	*-*-cygwin|*mingw*) OBJ_FORMAT=win32 ;;
	*-*-darwin*) OBJ_FORMAT=macho ;;
	*emx*) OBJ_FORMAT=aout ;;
	*djgpp) OBJ_FORMAT=coff ;;
	*) OBJ_FORMAT=elf ;;
esac
AC_SUBST(OBJ_FORMAT)
case "$host" in
	*-gnuspe)
		abi_spe=true
		AC_DEFINE(FLAC__CPU_PPC_SPE)
		AH_TEMPLATE(FLAC__CPU_PPC_SPE, [define if building for PowerPC with SPE ABI])
		;;
esac
AM_CONDITIONAL(FLaC__CPU_PPC_SPE, test "x$abi_spe" = xtrue)

os_is_windows=no
case "$host" in
	*mingw*)
		CPPFLAGS="-D__MSVCRT_VERSION__=0x0601 -D__USE_MINGW_ANSI_STDIO=0 $CPPFLAGS"
		os_is_windows=yes
		;;
esac

AM_CONDITIONAL(OS_IS_WINDOWS, test "x$os_is_windows" = xyes)

case "$host" in
	*-pc-linux-gnu)
		sys_linux=true
		AC_DEFINE(FLAC__SYS_LINUX)
		AH_TEMPLATE(FLAC__SYS_LINUX, [define if building for Linux])
		;;
	*-*-darwin*)
		sys_darwin=true
		AC_DEFINE(FLAC__SYS_DARWIN)
		AH_TEMPLATE(FLAC__SYS_DARWIN, [define if building for Darwin / MacOS X])
		;;
esac
AM_CONDITIONAL(FLaC__SYS_DARWIN, test "x$sys_darwin" = xtrue)
AM_CONDITIONAL(FLaC__SYS_LINUX, test "x$sys_linux" = xtrue)

if test "x$cpu_ia32" = xtrue ; then
AC_DEFINE(FLAC__ALIGN_MALLOC_DATA)
AH_TEMPLATE(FLAC__ALIGN_MALLOC_DATA, [define to align allocated memory on 32-byte boundaries])
fi

AC_ARG_ENABLE(asm-optimizations, AC_HELP_STRING([--disable-asm-optimizations], [Don't use any assembly optimization routines]), asm_opt=no, asm_opt=yes)
dnl ' Terminate the damn single quote
AM_CONDITIONAL(FLaC__NO_ASM, test "x$asm_opt" = xno)
if test "x$asm_opt" = xno ; then
AC_DEFINE(FLAC__NO_ASM)
AH_TEMPLATE(FLAC__NO_ASM, [define to disable use of assembly code])
fi

AC_ARG_ENABLE(debug,
AC_HELP_STRING([--enable-debug], [Turn on debugging]),
[case "${enableval}" in
	yes) debug=true ;;
	no)  debug=false ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
esac],[debug=false])
AM_CONDITIONAL(DEBUG, test "x$debug" = xtrue)

AC_ARG_ENABLE(sse,
AC_HELP_STRING([--enable-sse], [Enable SSE support by asserting that the OS supports SSE instructions]),
[case "${enableval}" in
	yes) sse_os=true ;;
	no)  sse_os=false ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-sse) ;;
esac],[sse_os=false])
AM_CONDITIONAL(FLaC__SSE_OS, test "x$sse_os" = xtrue)
if test "x$sse_os" = xtrue ; then
AC_DEFINE(FLAC__SSE_OS)
AH_TEMPLATE(FLAC__SSE_OS, [define if your operating system supports SSE instructions])
fi

AC_ARG_ENABLE(3dnow,
AC_HELP_STRING([--disable-3dnow], [Disable 3DNOW! optimizations]),
[case "${enableval}" in
	yes) use_3dnow=true ;;
	no)  use_3dnow=false ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-3dnow) ;;
esac],[use_3dnow=true])
AM_CONDITIONAL(FLaC__USE_3DNOW, test "x$use_3dnow" = xtrue)
if test "x$use_3dnow" = xtrue ; then
AC_DEFINE(FLAC__USE_3DNOW)
AH_TEMPLATE(FLAC__USE_3DNOW, [define to enable use of 3Dnow! instructions])
fi

AC_ARG_ENABLE(altivec,
AC_HELP_STRING([--disable-altivec], [Disable Altivec optimizations]),
[case "${enableval}" in
	yes) use_altivec=true ;;
	no)  use_altivec=false ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-altivec) ;;
esac],[use_altivec=true])
AM_CONDITIONAL(FLaC__USE_ALTIVEC, test "x$use_altivec" = xtrue)
if test "x$use_altivec" = xtrue ; then
AC_DEFINE(FLAC__USE_ALTIVEC)
AH_TEMPLATE(FLAC__USE_ALTIVEC, [define to enable use of Altivec instructions])
fi

AC_ARG_ENABLE(encoder,
	AC_HELP_STRING([--disable-encoder], [Do not include the FLAC encoder]),
	[want_encoder=$enableval], [want_encoder=yes])
if test "x$want_encoder" = xyes ; then
	AC_DEFINE([FLAC_INCLUDE_ENCODER],1,[define if the FLAC encoder is compiled])
	NASM_ENCODER_FLAGS="-d FLAC_INCLUDE_ENCODER"
else
	NASM_ENCODER_FLAGS=
fi
AC_SUBST(NASM_ENCODER_FLAGS)
AC_ARG_ENABLE(metadata,
	AC_HELP_STRING([--disable-metadata], [Do not include metadata manipulation interfaces]),
	[want_metadata=$enableval], [want_metadata=yes])
if test "x$want_metadata" = xyes ; then
	AC_DEFINE([FLAC_METADATA_INTERFACES],1,[define if FLAC metadata manipulation interfaces are compiled])
fi

AC_ARG_ENABLE(gcc-werror,
	AC_HELP_STRING([--enable-gcc-werror], [Enable -Werror in all Makefiles]))

AC_ARG_ENABLE(stack-smash-protection,
	AC_HELP_STRING([--enable-stack-smash-protection], [Enable GNU GCC stack smash protection]))

dnl check for ogg library
AC_ARG_ENABLE([ogg],
	AC_HELP_STRING([--disable-ogg], [Disable ogg support (default: test for libogg)]),
	[ want_ogg=$enableval ], [ want_ogg=yes ] )

if test "x$want_ogg" != "xno"; then
	XIPH_PATH_OGG(have_ogg=yes, AC_MSG_WARN([*** Ogg development enviroment not installed - Ogg support will not be built]))
fi

AM_CONDITIONAL(FLaC__HAS_OGG, [test "x$have_ogg" = xyes])
if test "x$have_ogg" = xyes ; then
	AC_DEFINE_UNQUOTED([FLAC__HAS_OGG],1,[define if you have the ogg library])
	OGG_PACKAGE="ogg"
else
	AC_DEFINE_UNQUOTED([FLAC__HAS_OGG],0)
	have_ogg=no
fi
AC_SUBST(OGG_PACKAGE)

# only matters for x86
AC_CHECK_PROGS(NASM, nasm)
AM_CONDITIONAL(FLaC__HAS_NASM, test -n "$NASM")
if test -n "$NASM" ; then
AC_DEFINE(FLAC__HAS_NASM)
AH_TEMPLATE(FLAC__HAS_NASM, [define if you are compiling for x86 and have the NASM assembler])
fi

if test "x$debug" = xtrue; then
	CPPFLAGS="-DDEBUG $CPPFLAGS"
	CFLAGS="-g $CFLAGS"
else
	CPPFLAGS="-DNDEBUG $CPPFLAGS"
	if test "x$GCC" = xyes; then
		if test "x$user_cflags" = x; then
			CFLAGS="-O2 -Wall -W"
			# -O3 -funroll-loops -Winline
		fi
	fi
fi

XIPH_GCC_VERSION

if test x$ac_cv_c_compiler_gnu = xyes ; then
	CFLAGS="$CFLAGS -Wstrict-prototypes -Wmissing-prototypes -Waggregate-return -Wcast-align -Wnested-externs -Wshadow -Wundef -Wmissing-declarations"
	# -Wcast-qual -Wbad-function-cast -Wwrite-strings -Winline -Wconversion

	if test x$enable_gcc_werror = "xyes" ; then
		CFLAGS="$CFLAGS -Wall -Wextra -Werror "
		fi

	if test "$GCC_MAJOR_VERSION" -ge 4 && test "$OBJ_FORMAT" = elf; then
		CPPFLAGS="$CPPFLAGS -DFLAC__USE_VISIBILITY_ATTR"
		CFLAGS="$CFLAGS -fvisibility=hidden"
		fi

	if test "$GCC_MAJOR_VERSION" -ge 4 && test "$OBJ_FORMAT" = macho; then
		CPPFLAGS="$CPPFLAGS -DFLAC__USE_VISIBILITY_ATTR"
		CFLAGS="$CFLAGS -fvisibility=hidden"
		fi

	if test "x$GCC_MAJOR_VERSION$GCC_MINOR_VERSION" = "x42" ; then
		XIPH_ADD_CFLAGS([-fgnu89-inline])
		fi
	fi

XIPH_ADD_CFLAGS([-Wextra])
XIPH_ADD_CFLAGS([-Wdeclaration-after-statement])

if test x$enable_stack_smash_protection = "xyes" ; then
	XIPH_GCC_STACK_PROTECTOR
	fi

AH_VERBATIM([FLAC_API_EXPORTS],
[/* libtool defines DLL_EXPORT for windows dll builds,
   but flac code relies on FLAC_API_EXPORTS instead. */
#ifdef DLL_EXPORT
#ifdef __cplusplus
# define FLACPP_API_EXPORTS
#else
# define FLAC_API_EXPORTS
#endif
#endif])

if test x$enable_shared != "xyes" ; then
dnl for correct FLAC_API
	CPPFLAGS="-DFLAC__NO_DLL $CPPFLAGS"
	fi

AC_CONFIG_FILES([ \
	Makefile \
	src/Makefile \
	src/libFLAC/Makefile \
	src/libFLAC/flac.pc \
	src/libFLAC/ia32/Makefile \
	src/libFLAC/include/Makefile \
	src/libFLAC/include/private/Makefile \
	src/libFLAC/include/protected/Makefile \
	src/share/Makefile \
	include/Makefile \
	include/FLAC/Makefile \
	include/share/Makefile
])
AC_OUTPUT

AC_MSG_RESULT([
-=-=-=-=-=-=-=-=-=-= Configuration Complete =-=-=-=-=-=-=-=-=-=-

  Configuration summary :

    FLAC version : ........................ ${VERSION}

    Host CPU : ............................ ${host_cpu}
    Host Vendor : ......................... ${host_vendor}
    Host OS : ............................. ${host_os}
])

	echo "    Compiler is GCC : ..................... ${ac_cv_c_compiler_gnu}"
if test x$ac_cv_c_compiler_gnu = xyes ; then
	echo "    GCC version : ......................... ${GCC_VERSION}"
fi
echo
