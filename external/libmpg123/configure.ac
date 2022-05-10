dnl configure.ac: source for the configure script

dnl copyright by the mpg123 project - free software under the terms of the LGPL 2.1
dnl see COPYING and AUTHORS files in distribution or http://mpg123.org
dnl initially written by Nicholas J. Humfrey

dnl Require autoconf version >= 2.57
AC_PREREQ(2.57)

dnl ############# Initialisation
AC_INIT([mpg123], [1.22.4], [mpg123-devel@lists.sourceforge.net])
dnl Increment API_VERSION when the API gets changes (new functions).
API_VERSION=42
LIB_PATCHLEVEL=0
dnl Since we want to be backwards compatible, both sides get set to API_VERSION.
LIBMPG123_VERSION=$API_VERSION:$LIB_PATCHLEVEL:$API_VERSION
AC_SUBST(LIBMPG123_VERSION)
AC_SUBST(API_VERSION)

AC_CONFIG_SRCDIR(src/libmpg123.c)
AC_CONFIG_AUX_DIR(build)
AC_CONFIG_MACRO_DIR([m4])
AM_MAINTAINER_MODE
AC_CANONICAL_HOST

dnl Version 1.7 of automake is recommended
AM_INIT_AUTOMAKE(1.7)
AC_CONFIG_HEADERS([src/config.h])


# You get strange symptoms like jack module build failing because the AC_C_CONST failed to detect the working const support.
# In that case, the test failed because -Werror, not because no const there...
# After looking again, there are possibly more tests being obscured by false failures.
AC_MSG_CHECKING([for -Werror in CFLAGS (It breaks tests)])
if echo "$CFLAGS" | grep Werror; then
	AC_MSG_RESULT([yes])
	AC_MSG_WARN([You have -Werror in CFLAGS. That may break some tests and make this configure bogus.
If you want paranoid compilation, use --enable-nagging option, which adds -Werror for gcc.
Also note that you shall not run make distcheck after configuring with --enable-nagging.
distcheck uses the generated CFLAGS...
Anyhow, continuing at your own risk.])
else
	AC_MSG_RESULT([no])
fi

dnl ############# Compiler and tools Checks

LT_LDFLAGS=-export-dynamic
EXEC_LT_LDFLAGS=
be_static=no
all_static=no
AC_MSG_CHECKING([if you are up to something totally static with LDFLAGS/CFLAGS])
for f in $LDFLAGS $CFLAGS
do
	case "$f" in
		-all-static)
			be_static=yes
			all_static=yes
		;;
		-static)
			be_static=yes
		;;
	esac
done
if test "x$be_static" = xyes; then
	AC_MSG_RESULT([yes])
	LT_LDFLAGS=-all-static
	EXEC_LT_LDFLAGS="$LT_LDFLAGS"
else
	AC_MSG_RESULT([no])
fi
if test "x$all_static" = xyes; then
	AC_MSG_WARN( Use -static in LDFLAGS for all-static linking! Your compiler may blow up on that -all-static. )
fi

AM_PROG_AS
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CPP
AC_PROG_INSTALL
dnl "Checking for egrep is broken after removal of libltdl stuff... checks use $EGREP, so searching it here."
AC_PROG_EGREP
AC_C_CONST
AC_INLINE
AC_C_BIGENDIAN

dnl We only want shared libraries by default
AC_DISABLE_STATIC
AC_ENABLE_SHARED

if test x"$enable_shared" = xno; then
	LT_LDFLAGS=
else
	AC_DEFINE(DYNAMIC_BUILD, 1, [ Define if building with dynamcally linked libmpg123])
fi

dnl Configure libtool

LT_INIT([win32-dll])

AC_SUBST(LT_LDFLAGS)
AC_SUBST(EXEC_LT_LDFLAGS)

dnl ############## Configurable Options

AC_ARG_ENABLE(debug,
              [  --enable-debug=[no/yes] turn on debugging],
              [
                if test "x$enableval" = xyes
                then
                  debugging="enabled"
                else
                  debugging="disabled"
                fi
              ],
              [ debugging="disabled" ]
)

AC_ARG_ENABLE(nagging,
              [  --enable-nagging=[no/yes] turn on GCC's pedantic nagging with error on warnings, also enables debugging ],
              [
                if test "x$enableval" = xyes
                then
                  nagging="enabled"
                  debugging="enabled"
               else
                  nagging="disabled"
                fi
              ],
              [ nagging="disabled" ]
)

if test x"$debugging" = xenabled; then
	AC_DEFINE(DEBUG, 1, [ Define if debugging is enabled. ])
fi

AC_ARG_ENABLE(gapless,
              [  --enable-gapless=[no/yes] turn on gapless (enabled per default)],
              [
                if test "x$enableval" = xyes
                then
                  gapless="enabled"
                  AC_DEFINE(GAPLESS, 1, [ Define if gapless is enabled. ])
                else
                  gapless="disabled"
                fi
              ],
              [
                gapless="enabled"
                AC_DEFINE(GAPLESS, 1, [ Define if gapless is enabled. ])
              ]
)

dnl Optional objects list, depends on decoder choice and core feature selection.
dnl Not just for specific decoders anymore...
s_fpu=
DECODER_OBJ=
DECODER_LOBJ=

dnl Core features that can be disabled to reduce binary size.

id3v2=enabled
AC_ARG_ENABLE(id3v2,
              [  --disable-id3v2=[no/yes] no ID3v2 parsing ],
              [
                if test "x$enableval" = xno; then
                  id3v2="disabled"
                fi
              ], [])

# id3v2 depends on strings... so check that in between.
string=enabled
AC_ARG_ENABLE(string,
              [  --disable-string=[no/yes] no string API (this will disable ID3v2; main mpg123 won't build anymore) ],
              [
                if test "x$enableval" = xno; then
                  string="disabled"
                fi
              ], [])

if test "x$string" = "xdisabled"; then
  AC_DEFINE(NO_STRING, 1, [ Define to disable string functions. ])
  id3v2=disabled
 	AC_MSG_WARN([ID3v2 support disabled because of string API being disabled.])
else
  DECODER_OBJ="$DECODER_OBJ stringbuf.\$(OBJEXT)"
  DECODER_LOBJ="$DECODER_LOBJ stringbuf.lo"
fi

if test "x$id3v2" = "xdisabled"; then
  AC_DEFINE(NO_ID3V2, 1, [ Define to disable ID3v2 parsing. ])
fi

icy=enabled
AC_ARG_ENABLE(icy,
              [  --disable-icy=[no/yes] no ICY metainfo parsing/conversion (main mpg123 won't build!) ],
              [
                if test "x$enableval" = xno; then
                  icy="disabled"
                fi
              ], [])

if test "x$icy" = "xdisabled"; then
  AC_DEFINE(NO_ICY, 1, [ Define to disable ICY handling. ])
else
  DECODER_OBJ="$DECODER_OBJ icy.\$(OBJEXT) icy2utf8.\$(OBJEXT)"
  DECODER_LOBJ="$DECODER_LOBJ icy.lo icy2utf8.lo"
fi

ntom=enabled
AC_ARG_ENABLE(ntom,
              [  --disable-ntom=[no/yes] no flexible resampling ],
              [
                if test "x$enableval" = xno; then
                  ntom="disabled"
                fi
              ], [])

if test "x$ntom" = "xdisabled"; then
  AC_DEFINE(NO_NTOM, 1, [ Define to disable ntom resampling. ])
else
  DECODER_OBJ="$DECODER_OBJ ntom.\$(OBJEXT)"
  DECODER_LOBJ="$DECODER_LOBJ ntom.lo"
fi

downsample=enabled
AC_ARG_ENABLE(downsample,
              [  --disable-downsample=[no/yes] no downsampled decoding ],
              [
                if test "x$enableval" = xno; then
                  downsample="disabled"
                fi
              ], [])

if test "x$downsample" = "xdisabled"; then
  AC_DEFINE(NO_DOWNSAMPLE, 1, [ Define to disable downsampled decoding. ])
fi

feeder=enabled
AC_ARG_ENABLE(feeder,
              [  --disable-feeder=[no/yes] no feeder decoding, no buffered readers ],
              [
                if test "x$enableval" = xno; then
                  feeder="disabled"
                fi
              ], [])

if test "x$feeder" = "xdisabled"; then
  AC_DEFINE(NO_FEEDER, 1, [ Define to disable feeder and buffered readers. ])
fi

messages=enabled
AC_ARG_ENABLE(messages,
              [  --disable-messages=[no/yes] no error/warning messages on the console ],
              [
                if test "x$enableval" = xno; then
                  messages="disabled"
                fi
              ], [])

allquiet=disabled
AC_ARG_ENABLE(allquiet,
              [  --enable-allquiet=[no/yes] no messages on the console from library (implies --disable-messages) ],
              [
                if test "x$enableval" = xyes; then
                  allquiet="enabled"
                fi
              ], [])

if test "x$messages" = "xdisabled" -o "x$allquiet" = "xenabled"; then
  AC_DEFINE(NO_WARNING, 1, [ Define to disable warning messages. ])
  AC_DEFINE(NO_ERRORMSG,   1, [ Define to disable error messages. ])
  AC_DEFINE(NO_ERETURN, 1, [ Define to disable error messages in combination with a return value (the return is left intact). ])
fi
if test "x$allquiet" = "xenabled"; then
  AC_DEFINE(LIBMPG123_QUIET, 1, [ Define to disable all messages from library. ])
fi

newhuff=enabled
AC_ARG_ENABLE(new-huffman,
[  --enable-new-huffman=[yes/no] use new huffman decoding scheme by Taihei (faster on modern CPUs at least, so on by default) ],
[
  if test "x$enableval" = xno; then
    newhuff=disabled
  fi
]
, [])

if test "x$newhuff" = "xenabled"; then
  AC_DEFINE(USE_NEW_HUFFTABLE, 1, [ Define for new Huffman decoding scheme. ])
fi

integers=fast
AC_ARG_ENABLE(int-quality,
[  --enable-int-quality=[yes/no] use rounding instead of fast truncation for integer output, where possible ],
[
  if test "x$enableval" = xyes; then
    integers=quality
    AC_DEFINE(ACCURATE_ROUNDING,   1, [ Define to use proper rounding. ])
  fi
], [])

int16=enabled
AC_ARG_ENABLE(16bit,
              [  --disable-16bit=[no/yes] no 16 bit integer output ],
              [
                if test "x$enableval" = xno; then
                  int16="disabled"
                fi
              ], [])

int8=enabled
AC_ARG_ENABLE(8bit,
              [  --disable-8bit=[no/yes] no 8 bit integer output ],
              [
                if test "x$enableval" = xno; then
                  int8="disabled"
                fi
              ], [])

int32=enabled
AC_ARG_ENABLE(32bit,
              [  --disable-32bit=[no/yes] no 32 bit integer output (also 24 bit) ],
              [
                if test "x$enableval" = xno; then
                  int32="disabled"
                fi
              ], [])

real=enabled
AC_ARG_ENABLE(real,
              [  --disable-real=[no/yes] no real (floating point) output ],
              [
                if test "x$enableval" = xno; then
                  real="disabled"
                fi
              ], [])

equalizer=enabled
AC_ARG_ENABLE(equalizer,
              [  --disable-equalizer=[no/yes] no equalizer support ],
              [
                if test "x$enableval" = xno; then
                  equalizer="disabled"
                fi
              ], [])


AC_ARG_WITH([cpu], [
  --with-cpu=generic[[_fpu]]      Use generic processor code with floating point arithmetic
  --with-cpu=generic_float      Plain alias to generic_fpu now... float output is a normal runtime option!
  --with-cpu=generic_nofpu      Use generic processor code with fixed point arithmetic (p.ex. ARM)
  --with-cpu=generic_dither     Use generic processor code with floating point arithmetic and dithering for 1to1 16bit decoding.
  --with-cpu=i386[[_fpu]]         Use code optimized for i386 processors with floating point arithmetic
  --with-cpu=i386_nofpu         Use code optimized for i386 processors with fixed point arithmetic
  --with-cpu=i486         Use code optimized for i486 processors (only usable alone!)
  --with-cpu=i586         Use code optimized for i586 processors
  --with-cpu=i586_dither  Use code optimized for i586 processors with dithering (noise shaping), adds 256K to binary size
  --with-cpu=3dnow         Use code optimized for 3DNow processors
  --with-cpu=3dnow_vintage Use code optimized for older 3DNow processors (K6 family)
  --with-cpu=3dnowext      Use code optimized for 3DNowExt processors (K6-3+, Athlon)
  --with-cpu=3dnowext_alone     Really only 3DNowExt decoder, without 3DNow fallback for flexible rate
  --with-cpu=3dnow_vintage       Use code optimized for older extended 3DNow processors (like K6-III+)
  --with-cpu=mmx          Use code optimized for MMX processors
  --with-cpu=mmx_alone          Really only MMX decoder, without i586 fallback for flexible rate
  --with-cpu=sse          Use code optimized for SSE processors
  --with-cpu=sse_vintage  Use code optimized for older SSE processors (plain C DCT36)
  --with-cpu=sse_alone          Really only SSE decoder, without i586 fallback for flexible rate
  --with-cpu=avx          Use code optimized for x86-64 with AVX processors
  --with-cpu=x86          Pack all x86 opts into one binary (excluding i486, including dither)
  --with-cpu=x86-64       Use code optimized for x86-64 processors (AMD64 and Intel64, including AVX and dithered generic)
  --with-cpu=altivec      Use code optimized for Altivec processors (PowerPC G4 and G5)
  --with-cpu=ppc_nofpu    Use code optimized for PowerPC processors with fixed point arithmetic
  --with-cpu=neon         Use code optimized for ARM NEON SIMD engine (Cortex-A series)
  --with-cpu=arm_fpu      Pack neon and generic[[_dither]] decoders, for ARM processors with FPU and/or NEON
  --with-cpu=arm_nofpu    Use code optimized for ARM processors with fixed point arithmetic
  --with-cpu=neon64       Use code optimized for AArch64 NEON SIMD engine
  --with-cpu=aarch64      Pack neon64 and generic[[_dither]] decoders, for 64bit ARM processors
])


# opt-in for IEEE754 floating point action
# We want to make sure not to apply hacks relying on IEEE754 storage format on platforms where we did not make sure that this actually is valid.
# It _should_ be fine for about all computers still out there, but we play safe.
ieee=no
case $host in
  i?86-*|x86_64-*|powerpc*|sparc*)
    ieee=yes
  ;;
esac
if test "x$ieee" = xyes; then
  echo "We assume IEEE754 floating point format."
  AC_DEFINE(IEEE_FLOAT,   1, [ Define to indicate that float storage follows IEEE754. ])
fi

newoldwritesample=disabled
case $host in
  aarch64-*-linux*|arm64-*-linux*)
    cpu_type="aarch64"
  ;;
  aarch64-apple-darwin*|arm64-apple-darwin*)
    cpu_type="aarch64"
  ;;
  arm*-*-linux*-*eabihf|armv7hl*-*-linux*)
    cpu_type="arm_fpu"
  ;;
  arm*-*-linux*)
    # check that... perhaps we are better off on arm with kernel math emulation
    cpu_type="arm_nofpu"
  ;;
  armv7*-apple-darwin*)
    cpu_type="arm_fpu"
  ;;
  i386-*-linux*|i386-*-kfreebsd*-gnu)
    cpu_type="i386_fpu"
    newoldwritesample=enabled
  ;;
  i486-*-linux*|i486-*-kfreebsd*-gnu)
    cpu_type="i486"
    newoldwritesample=enabled
  ;;
  i586-*-linux*|i586-*-kfreebsd*-gnu)
    cpu_type="x86"
    newoldwritesample=enabled
  ;;
  i686-*-linux*|i686-*-kfreebsd*-gnu)
    cpu_type="x86"
    newoldwritesample=enabled
  ;;
  x86_64-*-linux*|x86_64-*-kfreebsd*-gnu)
    cpu_type="x86-64"
  ;;
  *-*-linux*|*-*-kfreebsd*-gnu)
    cpu_type="generic_fpu"
  ;;
  i?86-apple-darwin10*)
    AC_MSG_CHECKING([if CPU type supports x86-64])
    case `sysctl -n hw.optional.x86_64` in
      1)
        AC_MSG_RESULT([yes])
        cpu_type="x86-64"
      ;;
      *)
        AC_MSG_RESULT([no])
        cpu_type="x86"
        newoldwritesample=enabled
      ;;
    esac
  ;;
  i?86-apple-darwin*)
    cpu_type="x86"
    newoldwritesample=enabled
  ;;
  x86_64-apple-darwin*)
    cpu_type="x86-64"
  ;;
  *-apple-darwin*)
    AC_MSG_CHECKING([if CPU type supports AltiVec])
    case `machine` in
      ppc7400 | ppc7450 | ppc970)
        AC_MSG_RESULT([yes])
        cpu_type="altivec"
      ;;
      *)
        AC_MSG_RESULT([no])
        cpu_type="generic_fpu"
      ;;
    esac
  ;;
  i?86-*-dragonfly* | i?86-*-freebsd* | i?86-*-midnightbsd* | i?86-*-mirbsd* | i?86-*-netbsd* | i?86-*-openbsd*)
    cpu_type="x86"
    newoldwritesample=enabled
  ;;
  x86_64-*-dragonfly* | x86_64-*-freebsd* | x86_64-*-midnightbsd* | x86_64-*-mirbsd* | x86_64-*-netbsd* | x86_64-*-openbsd*)
    cpu_type="x86-64"
  ;;
  *-*-dragonfly* | *-*-freebsd* | *-*-midnightbsd* | *-*-mirbsd* | *-*-netbsd* | *-*-openbsd*)
    cpu_type="generic_fpu"
  ;;
  i386-*-solaris*)
    cpu_type=x86
    newoldwritesample=enabled
  ;;
  x86_64-*-solaris*)
    cpu_type=x86-64
  ;;
  *-*-solaris*)
    cpu_type="generic_fpu"
  ;;
  # os2-emx = OS/2 with some Unix fun; so p.ex. buffer works.
  # Till we sorted out the assembler troubles, generic CPU is default.
  i386-pc-os2-emx)
    cpu_type=generic_fpu
    newoldwritesample=enabled
  ;;
  x86_64-pc-os2-emx)
    # We are optimistic hat the future knows OS/2 on x86-64;-)
    cpu_type=generic_fpu
  ;;
  *-pc-os2-emx)
    cpu_type="generic_fpu"
  ;;
  *-dec-osf*)
    cpu_type="generic_fpu"
  ;;
  x86_64-*-cygwin*)
    cpu_type="x86-64"
  ;;
  i686-*-cygwin*)
    cpu_type="x86"
    newoldwritesample=enabled
  ;;
  i586-*-cygwin*)
    cpu_type="x86"
    newoldwritesample=enabled
  ;;
  i486-*-cygwin*)
    cpu_type="i486"
    newoldwritesample=enabled
  ;;
  i386-*-cygwin*)
    cpu_type="i386"
    newoldwritesample=enabled
  ;;
  *-cygwin*)
    cpu_type="generic_fpu"
  ;;
  i@<:@3-7@:>@86-*-mingw32*)
    LIBS="$LIBS"
    cpu_type="x86"
    newoldwritesample=enabled
  ;;
  x86_64-*-mingw32*)
    LIBS="$LIBS"
    cpu_type="x86-64"
  ;;
  i386-*-nto-qnx*)
    cpu_type="x86"
    newoldwritesample=enabled
  ;;
	i386-*)
		AC_MSG_WARN([Unknown host operating system])
		cpu_type="i386"
		ADD_CPPFLAGS="-DGENERIC $ADD_CPPFLAGS"
    newoldwritesample=enabled
	;;
	i486-*)
		AC_MSG_WARN([Unknown host operating system])
		cpu_type="i486"
		ADD_CPPFLAGS="-DGENERIC $ADD_CPPFLAGS"
    newoldwritesample=enabled
	;;
	i586-*)
		AC_MSG_WARN([Unknown host operating system])
		cpu_type="x86"
		ADD_CPPFLAGS="-DGENERIC $ADD_CPPFLAGS"
    newoldwritesample=enabled
	;;
	i686-*)
		AC_MSG_WARN([Unknown host operating system])
		cpu_type="x86"
		ADD_CPPFLAGS="-DGENERIC $ADD_CPPFLAGS"
    newoldwritesample=enabled
	;;
	x86_64-*)
		AC_MSG_WARN([Unknown host operating system])
		cpu_type="x86-64"
		ADD_CPPFLAGS="-DGENERIC $ADD_CPPFLAGS"
	;;
  *)
  	AC_MSG_WARN([Unknown host operating system])
    cpu_type="generic_fpu"
    ADD_CPPFLAGS="-DGENERIC $ADD_CPPFLAGS"
  ;;
esac

AC_ARG_ENABLE(newoldwritesample,
[  --enable-newoldwritesample=[no/yes] enable new/old WRITE_SAMPLE macro for non-accurate 16 bit output, faster on certain CPUs (default on on x86-32)],
[
	if test "x$enableval" = xyes
	then
		newoldwritesample=enabled
	else
		newoldwritesample=disabled
	fi
])

dnl Did user choose other CPU type ?
if test "x$with_cpu" != "x"; then
	cpu_type=$with_cpu
fi

# Flag for 32 bit synth output or post-processing.
case "$cpu_type" in
*_nofpu)
  synth32=false
  AC_DEFINE(NO_SYNTH32, 1, [ Define for post-processed 32 bit formats. ])
;;
*)
  synth32=true
;;
esac

if test "x$int16" = "xdisabled"; then
  AC_DEFINE(NO_16BIT, 1, [ Define to disable 16 bit integer output. ])
else
  DECODER_OBJ="$DECODER_OBJ synth.\$(OBJEXT)"
  DECODER_LOBJ="$DECODER_LOBJ synth.lo"
fi

# 8bit works only through 16bit
if test "x$int16" = "xdisabled"; then
  int8=disabled
fi
if test "x$int8" = "xdisabled"; then
  AC_DEFINE(NO_8BIT, 1, [ Define to disable 8 bit integer output. ])
else
  DECODER_OBJ="$DECODER_OBJ synth_8bit.\$(OBJEXT)"
  DECODER_LOBJ="$DECODER_LOBJ synth_8bit.lo"
fi

if test "x$int32" = "xdisabled"; then
  AC_DEFINE(NO_32BIT, 1, [ Define to disable 32 bit and 24 bit integer output. ])
else
  if $synth32; then
    s_fpu="$s_fpu synth_s32"
  fi
fi

if test "x$real" = "xdisabled"; then
  AC_DEFINE(NO_REAL, 1, [ Define to disable real output. ])
else
  if $synth32; then
    s_fpu="$s_fpu synth_real"
  fi
fi

if test "x$equalizer" = "xdisabled"; then
  AC_DEFINE(NO_EQUALIZER, 1, [ Define to disable equalizer. ])
else
  DECODER_OBJ="$DECODER_OBJ equalizer.\$(OBJEXT)"
  DECODER_LOBJ="$DECODER_LOBJ equalizer.lo"
fi

layer1=enabled
AC_ARG_ENABLE(layer1,
              [  --disable-layer1=[no/yes] no layer I decoding ],
              [
                if test "x$enableval" = xno; then
                  layer1="disabled"
                fi
              ], [])

if test "x$layer1" = "xdisabled"; then
  AC_DEFINE(NO_LAYER1, 1, [ Define to disable layer I. ])
else
  # layer1 needs code in layer2
  DECODER_OBJ="$DECODER_OBJ layer1.\$(OBJEXT) layer2.\$(OBJEXT)"
  DECODER_LOBJ="$DECODER_LOBJ layer1.lo layer2.lo"
fi

layer2=enabled
AC_ARG_ENABLE(layer2,
              [  --disable-layer2=[no/yes] no layer II decoding ],
              [
                if test "x$enableval" = xno; then
                  layer2="disabled"
                fi
              ], [])

if test "x$layer2" = "xdisabled"; then
  AC_DEFINE(NO_LAYER2, 1, [ Define to disable layer II. ])
else
  # layer1 may have added the objects already
  if test "x$layer1" = "xdisabled"; then
    DECODER_OBJ="$DECODER_OBJ layer2.\$(OBJEXT)"
    DECODER_LOBJ="$DECODER_LOBJ layer2.lo"
  fi
fi

layer3=enabled
AC_ARG_ENABLE(layer3,
              [  --disable-layer3=[no/yes] no layer III decoding ],
              [
                if test "x$enableval" = xno; then
                  layer3="disabled"
                fi
              ], [])

if test "x$layer3" = "xdisabled"; then
  AC_DEFINE(NO_LAYER3, 1, [ Define to disable layer III. ])
else
  DECODER_OBJ="$DECODER_OBJ layer3.\$(OBJEXT)"
  DECODER_LOBJ="$DECODER_LOBJ layer3.lo"
fi

AC_ARG_WITH([optimization], [
  --with-optimization=0   No Optimization
  --with-optimization=1   Limited Optimization (-O) (for gcc)
  --with-optimization=2   Default Optimization (-O2 ...) (for gcc)
  --with-optimization=3   More Optimize than default (-O3 ...) (for gcc)
  --with-optimization=4   Optimize yet more (-O4 ...) (for gcc)
])

AC_ARG_WITH([seektable], [
  --with-seektable=<size> choose size of seek index table (0 disables it), default 1000
])


dnl ############## Assembler, compiler properties

# based on posting from John Dalgliesh <johnd@defyne.org> on ffmpeg (LGPL) mailing list
# extended to use balign if present
AC_MSG_CHECKING([if .balign is present])
echo '.balign 4' > conftest.s
if $CCAS -c -o conftest.o conftest.s 1>/dev/null 2>&1; then

  AC_MSG_RESULT([yes])
  AC_DEFINE(ASMALIGN_BALIGN, 1, [ Define if .balign is present. ])

else

AC_MSG_RESULT([no])

# find if .align arg is power-of-two or not
asmalign_exp="unknown"
if test x"$asmalign_exp" = xunknown; then
	AC_MSG_CHECKING([if .align takes 2-exponent])
	asmalign_exp="no"
	echo '.align 3' > conftest.s
	if $CCAS -c -o conftest.o conftest.s 1>/dev/null 2>&1; then
		asmalign_exp="yes"
		AC_MSG_RESULT([yes])
	else
		AC_MSG_RESULT([no])
	fi
	rm -f conftest.o conftest.s
fi
if test x"$asmalign_exp" = xyes; then
	AC_DEFINE(ASMALIGN_EXP, 1, [ Define if .align takes 3 for alignment of 2^3=8 bytes instead of 8. ])
else
	AC_DEFINE(ASMALIGN_BYTE, 1, [ Define if .align just takes byte count. ])
fi

fi

ccalign="unknown"
if test x"$ccalign" = xunknown; then
	AC_MSG_CHECKING([__attribute__((aligned(16)))])
	ccalign="no"
	echo '__attribute__((aligned(16))) float var;' > conftest.c
	if $CC -c -o conftest.o conftest.c >/dev/null 2>&1; then
		ccalign="yes"
		AC_MSG_RESULT([yes])
	else
		AC_MSG_RESULT([no])
	fi
	rm -f conftest.o conftest.c
fi

dnl We apply alignment hints only to cpus that need it.
dnl See further below for the definition of CCALIGN

avx_support="unknown"
if test x"$avx_support" = xunknown; then
	AC_MSG_CHECKING([if assembler supports AVX instructions])
	avx_support="no"
	echo '.text' > conftest.s
	echo 'vaddps %ymm0,%ymm0,%ymm0' >> conftest.s
	if $CCAS -c -o conftest.o conftest.s 1>/dev/null 2>&1; then
		avx_support="yes"
		AC_MSG_RESULT([yes])
	else
		AC_MSG_RESULT([no])
	fi
	rm -f conftest.o conftest.s
fi

if test x"$avx_support" = xno; then
	AC_CHECK_PROGS(YASM,[yasm],no)
	if test x"$YASM" != xno; then
		AC_MSG_CHECKING([if $YASM supports GAS syntax and AVX instructions])
		echo '.text' > conftest.s
		echo 'vaddps %ymm0,%ymm0,%ymm0' >> conftest.s
		if $YASM -pgas -rcpp -o conftest.o conftest.s 1>/dev/null 2>&1; then
			avx_support="yes"
			YASMFLAGS="-pgas -rgas -mamd64"
			AC_MSG_RESULT([yes])
		else
			AC_MSG_RESULT([no])
		fi
		rm -f conftest.o conftest.s
	fi
else
	YASM="no"
fi

if test "x$cpu_type" = "xavx"; then
	if test "x$avx_support" != "xyes"; then
		AC_MSG_ERROR([Assembler doesn't understand AVX instructions.])
	fi
fi

AC_SUBST(YASM)
AC_SUBST(YASMFLAGS)

dnl ############## Really basic headers, needed for other checks.

AC_HEADER_STDC
dnl Is it too paranoid to specifically check for stdint.h and limits.h?
AC_CHECK_HEADERS([stdio.h stdlib.h string.h strings.h unistd.h sched.h sys/ioctl.h sys/types.h stdint.h limits.h inttypes.h sys/time.h sys/wait.h sys/resource.h sys/signal.h signal.h])

dnl ############## Types

dnl Large file support stuff needs cleanup. There are superfluous variables.

dnl Detect large file support, enable switches if needed.
AC_SYS_LARGEFILE
dnl If we do have a switch for large files, rename off_t-aware API calls.
dnl Using the file_offset_bits variable here is fine for linux (possibly Solaris),
dnl Others... we'll have to see.
dnl Note: I started writing this with with multiline replacements.
dnl Does not work. Automake insists on putting these into Makefiles where they break things.
dnl It is also assumed that a system that does not set file offset bits is not
dnl sensitive to largefile changes, i.e. FreeBSD always using 64 bit off_t.
if test "x$ac_cv_sys_file_offset_bits" = x || echo "$ac_cv_sys_file_offset_bits" | $GREP  '@<:@^0-9@:>@' > /dev/null; then
	dnl if it has non-numeric chars or is empty... ignore...
	LFS_LOBJ=
	largefile_sensitive=no
else
	# Add dual-mode wrapper code.
	LFS_LOBJ=lfs_wrap.lo
	largefile_sensitive=yes
fi

# Using the lower level macros instead of AC_TYPE_* for compatibility with not freshest autoconf.
AC_CHECK_TYPE(size_t, unsigned long)
AC_CHECK_TYPE(uintptr_t, unsigned long)
AC_CHECK_TYPE(ssize_t, long)
AC_CHECK_TYPE(off_t,  long int)
AC_CHECK_TYPE(int32_t, int)
AC_CHECK_TYPE(int64_t, long long)
AC_CHECK_TYPE(uint32_t, unsigned int)
AC_CHECK_TYPE(int16_t, short)
AC_CHECK_TYPE(uint16_t, unsigned short)
AC_CHECK_SIZEOF(size_t,4)
AC_CHECK_SIZEOF(ssize_t,4)
AC_CHECK_SIZEOF(off_t,4)
AC_CHECK_SIZEOF(int32_t)
AC_CHECK_SIZEOF(long,4)

dnl The native type used for aliases is what off_t maps to without any largefile-
dnl enabling switches. So, it's long int if the system is largefile-senstive,
dnl but it is actual plain off_t if the system does not have such switches.
if test "x$largefile_sensitive" = xyes; then
  lfs_alias_type=long
  lfs_alias_size=$ac_cv_sizeof_long
else
  lfs_alias_type=off_t
  lfs_alias_size=$ac_cv_sizeof_off_t
fi

if test "x$lfs_alias_size" = "x"; then 
  AC_MSG_ERROR([Cannot determine sizeof(lfs_alias_t)?])
else
  LFS_ALIAS_BITS=`expr "$lfs_alias_size" "*" "8"`
  AC_DEFINE_UNQUOTED([lfs_alias_t], $lfs_alias_type,
    [Define to the native offset type (long or actually off_t).])
  AC_DEFINE_UNQUOTED([LFS_ALIAS_BITS], $LFS_ALIAS_BITS,
    [Define this to the size of native offset type in bits, used for LFS alias functions.])
fi

lfs_alias=enabled
AC_ARG_ENABLE(lfs-alias,
              [  --disable-lfs-alias disable alias wrappers for largefile bitness (mpg123_seek_32 or mpg123_seek_64 in addition to mpg123_seek, or the other way around; It is a mess, do not play with this!) ],
              [
                if test "x$enableval" = xno; then
                  lfs_alias="disabled"
                fi
              ],
              [
                case $host in
                *-cygwin*)
                  lfs_alias="disabled"
                  AC_MSG_NOTICE([lfs-alias disabled for Cygwin, use --enable-lfs-alias explicitly to enable, only if you know what you are doing])
                ;;
                esac

              ])

AC_MSG_CHECKING([if we want to enable alias wrappers for largefile])
if test "x$lfs_alias" = "xenabled"; then
  AC_MSG_RESULT([yes])
  LFS_LOBJ="$LFS_LOBJ lfs_alias.lo"
else
  AC_MSG_RESULT([no])
fi

AC_SUBST(LFS_LOBJ)


dnl ############## Function Checks

AC_CHECK_FUNCS( strerror )

AC_CHECK_FUNCS( strdup )

dnl ############## Header and Library Checks

# Substitutions for the installable mpg123.h header
if test "x$ac_cv_header_stdio_h" = "xyes"; then
	INCLUDE_STDIO_H="#include <stdio.h>"
else
	INCLUDE_STDIO_H="/* #include <stdio.h> is not available on this system */"
fi
AC_SUBST(INCLUDE_STDIO_H)

if test "x$ac_cv_header_stdlib_h" = "xyes"; then
	INCLUDE_STDLIB_H="#include <stdlib.h>"
else
	INCLUDE_STDLIB_H="/* #include <stdlib.h> is not available on this system */"
fi
AC_SUBST(INCLUDE_STDLIB_H)

if test "x$ac_cv_header_sys_types_h" = "xyes"; then
	INCLUDE_SYS_TYPE_H="#include <sys/types.h>"
else
	INCLUDE_SYS_TYPE_H="/* #include <sys/types.h> is not available on this system */"
fi
AC_SUBST(INCLUDE_SYS_TYPE_H)


# Checks for maths libraries.
AC_CHECK_LIB([m], [sqrt])

# attempt to make the signal stuff work... also with GENERIC - later
#if test x"$ac_cv_header_sys_signal_h" = xyes; then
#	AC_CHECK_FUNCS( sigemptyset sigaddset sigprocmask sigaction )
#	if test x"$ac_cv_func_sigemptyset" = xyes &&
#	   test x"$ac_cv_func_sigaddset" = xyes &&
#	   test x"$ac_cv_func_sigprocmask" = xyes &&
#	   test x"$ac_cv_func_sigaction" = xyes; then
#	AC_DEFINE( 
#fi

dnl ############## Choose compiler flags and CPU

# do not assume gcc here, so no flags by default
ADD_CFLAGS=""
ADD_CPPFLAGS=""
ADD_LDFLAGS=""
LIBS="$LIBS"

# Consider moving that stuff.
AC_CHECK_HEADER([os2.h], [ADD_CPPFLAGS="$ADD_CPPFLAGS -DOS2"])
# On OS/2, we need to link to os2term to make terminal control actually work.
AC_CHECK_LIB([os2term], [tcsetattr], [ADD_LDFLAGS="$ADD_LDFLAGS -los2term"])

# If debugging is enabled then make warnings errors
if test x"$debugging" = xenabled; then
	ADD_CFLAGS="-g"
	# gcc specific...
	if test x"$GCC" = xyes; then
		ADD_CFLAGS="$ADD_CFLAGS -Wall -fno-strict-aliasing"
		if test x"$nagging" = xenabled; then
			ADD_CFLAGS="$ADD_CFLAGS -Werror -pedantic -DPLAIN_C89"
		fi
	fi
fi

dnl Only try the attribute_align_arg mumbo-jumbo on x86, x86-64 warns/errors out on that attribute.
dnl ...dunno even what about other architectures.
case $host in
  i?86-*)
    AC_DEFINE(ABI_ALIGN_FUN, 1, [ Define if your architecture wants/needs/can use attribute_align_arg and alignment checks. It is for 32bit x86... ])
 ;;
esac

s_altivec="synth_altivec dct64_altivec"
s_i386="dct64_i386"
s_i486="$s_i386 synth_i486 dct64_i486"
s_i586="$s_i386 synth_i586"
s_i586d="$s_i386 synth_i586_dither"
s_3dnow="$s_i386 synth_3dnow dct64_3dnow"
if test "x$equalizer" = "xenabled"; then
  s_3dnow="$s_3dnow equalizer_3dnow"
fi
s_3dnowext="$s_i386 dct64_3dnowext tabinit_mmx synth_3dnowext"
s_3dnow_vintage=$s_3dnow
s_3dnowext_vintage=$s_3dnowext
if test "x$layer3" = "xenabled"; then
  s_3dnow_vintage="$s_3dnow_vintage dct36_3dnow"
  s_3dnowext_vintage="$s_3dnowext_vintage dct36_3dnowext"
fi
s_mmx="$s_i386 dct64_mmx tabinit_mmx synth_mmx"
s_sse_vintage="$s_i386 tabinit_mmx dct64_sse_float synth_sse_float synth_stereo_sse_float synth_sse_s32 synth_stereo_sse_s32 "
s_sse="$s_sse_vintage dct36_sse"
s_x86_64="dct36_x86_64 dct64_x86_64_float synth_x86_64_float synth_x86_64_s32 synth_stereo_x86_64_float synth_stereo_x86_64_s32"
s_x86_64_mono_synths="synth_x86_64_float synth_x86_64_s32"
s_x86_64_avx="dct36_avx dct64_avx_float synth_stereo_avx_float synth_stereo_avx_s32"
s_x86multi="getcpuflags"
s_x86_64_multi="getcpuflags_x86_64"
s_dither="dither"
s_neon="dct36_neon dct64_neon_float synth_neon_float synth_neon_s32 synth_stereo_neon_float synth_stereo_neon_s32"
s_neon64="dct36_neon64 dct64_neon64_float synth_neon64_float synth_neon64_s32 synth_stereo_neon64_float synth_stereo_neon64_s32"
s_arm_multi="getcpuflags_arm check_neon"

# choose optimized 16bit decoder for SSE, quality or fast
# note: supporting deactivation of output formats for these decoders would need more logic here
if test "x$integers" = "xquality"; then
  s_sse="$s_sse synth_sse_accurate synth_stereo_sse_accurate"
  s_sse_vintage="$s_sse_vintage synth_sse_accurate synth_stereo_sse_accurate"
  s_x86_64="$s_x86_64 synth_x86_64_accurate synth_stereo_x86_64_accurate"
  s_x86_64_mono_synths="$s_x86_64_mono_synths synth_x86_64_accurate"
  s_x86_64_avx="$s_x86_64_avx synth_stereo_avx_accurate"
  s_arm="synth_arm_accurate"
  s_neon="$s_neon synth_neon_accurate synth_stereo_neon_accurate"
  s_neon64="$s_neon64 synth_neon64_accurate synth_stereo_neon64_accurate"
else
  s_sse="$s_sse dct64_sse synth_sse" # no stereo
  s_sse_vintage="$s_sse_vintage dct64_sse synth_sse" # no stereo
  s_x86_64="$s_x86_64 synth_x86_64 dct64_x86_64 synth_stereo_x86_64"
  s_x86_64_mono_synths="$s_x86_64_mono_synths synth_x86_64"
  s_x86_64_avx="$s_x86_64_avx dct64_avx synth_stereo_avx"
  s_arm="synth_arm"
  s_neon="$s_neon dct64_neon synth_neon synth_stereo_neon"
  s_neon64="$s_neon64 dct64_neon64 synth_neon64 synth_stereo_neon64"
fi

dnl CPU specific compiler flags and sources
case $cpu_type in
  generic)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_GENERIC -DREAL_IS_FLOAT"
    more_sources="$s_fpu"
    ccalign=no
  ;;
  generic_dither)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_GENERIC_DITHER -DREAL_IS_FLOAT"
    more_sources="$s_fpu $s_dither"
    ccalign=no
  ;;
dnl Not disabling buffer for float mode... check that this is OK now!
  generic_fpu | generic_float)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_GENERIC -DREAL_IS_FLOAT"
    more_sources="$s_fpu"
    ccalign=no
  ;;
  generic_nofpu)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_GENERIC -DREAL_IS_FIXED"
    more_sources=
    ccalign=no
  ;;
  ppc_nofpu)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_GENERIC -DOPT_PPC -DREAL_IS_FIXED"
    more_sources=
    ccalign=no
  ;;
  arm_nofpu)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_ARM -DREAL_IS_FIXED"
    more_sources="$s_arm"
    ccalign=no
  ;;
  altivec)
    ADD_CFLAGS="$ADD_CFLAGS -maltivec"
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DREAL_IS_FLOAT -DOPT_ALTIVEC"
    more_sources="$s_altivec $s_fpu"
    AC_MSG_CHECKING([if $CC accepts -faltivec])
    touch conftest.c
    if $CC -faltivec -c -o conftest.o conftest.c >/dev/null 2>&1; then
        ADD_CFLAGS="$ADD_CFLAGS -faltivec"
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi
    rm -f conftest.o conftest.c
  ;;
  neon)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_NEON -DREAL_IS_FLOAT"
    more_sources="$s_neon $s_fpu"
  ;;
  arm_fpu)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_MULTI -DOPT_GENERIC -DOPT_GENERIC_DITHER -DOPT_NEON -DREAL_IS_FLOAT"
    more_sources="$s_neon $s_fpu $s_dither $s_arm_multi"
  ;;
  neon64)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_NEON64 -DREAL_IS_FLOAT"
    more_sources="$s_neon64 $s_fpu"
  ;;
  aarch64)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_MULTI -DOPT_GENERIC -DOPT_GENERIC_DITHER -DOPT_NEON64 -DREAL_IS_FLOAT"
    more_sources="$s_neon64 $s_fpu $s_dither $s_arm_multi"
  ;;
  i386) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_I386 -DREAL_IS_FLOAT"
    more_sources="$s_fpu $s_i386"
    ccalign=no
  ;;
  i386_fpu) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_I386 -DREAL_IS_FLOAT"
    more_sources="$s_fpu $s_i386"
    ccalign=no
  ;;
  i386_nofpu) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_I386 -DREAL_IS_FIXED"
    more_sources="$s_i386"
    ccalign=no
  ;;
  i486) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_I486 -DREAL_IS_FLOAT"
    more_sources="$s_fpu $s_i486"
    ccalign=no
  ;;
  i586) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_I586 -DREAL_IS_FLOAT"
    more_sources="$s_fpu $s_i586"
  ;;
  i586_dither) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_I586_DITHER -DREAL_IS_FLOAT"
    more_sources="$s_fpu $s_i586d $s_dither"
  ;;
  3dnow)
    # legacy 3dnow had the 3dnow paired with i586...
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_MULTI -DOPT_I586 -DOPT_3DNOW -DREAL_IS_FLOAT"
    more_sources="$s_fpu $s_i586 $s_3dnow $s_x86multi "
  ;;
  3dnow_vintage)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_MULTI -DOPT_I586 -DOPT_3DNOW_VINTAGE -DREAL_IS_FLOAT"
    more_sources="$s_fpu $s_i586 $s_3dnow_vintage $s_x86multi "
  ;;
  3dnow_alone)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_3DNOW -DREAL_IS_FLOAT"
    more_sources="$s_fpu $s_3dnow"
  ;;
  3dnowext_alone) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_3DNOWEXT -DREAL_IS_FLOAT"
    more_sources="$s_fpu $s_3dnowext"
  ;;
  3dnowext_vintage) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_MULTI -DOPT_3DNOW -DOPT_3DNOWEXT_VINTAGE -DREAL_IS_FLOAT"
    more_sources="$s_fpu $s_3dnowext_vintage $s_3dnow $s_x86multi"
  ;;
  3dnowext) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_MULTI -DOPT_3DNOW -DOPT_3DNOWEXT -DREAL_IS_FLOAT"
    more_sources="$s_fpu $s_3dnowext $s_3dnow $s_x86multi"
  ;;
  mmx_alone|mmx) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_MMX -DREAL_IS_FLOAT"
    more_sources="$s_fpu $s_mmx"
  ;;
  sse_alone|sse) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_SSE -DREAL_IS_FLOAT"
    more_sources="$s_fpu $s_sse"
  ;;
  sse_vintage) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_SSE_VINTAGE -DREAL_IS_FLOAT"
    more_sources="$s_fpu $s_sse_vintage"
  ;;
  avx) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_AVX -DREAL_IS_FLOAT"
    more_sources="$s_fpu $s_x86_64_avx $s_x86_64_mono_synths"
	if test "x$YASM" != "xno"; then
		use_yasm_for_avx="yes"
	fi
  ;;
  x86|x86_dither)
    # plain C dct36 always there, for vintage/non-vintage
    # Selection of non-/vintage sources implies that
    # $s_3dnow_vintage contains $s_3dnow,
    # $s_sse contains $s_sse_vintage.
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_MULTI -DOPT_GENERIC -DOPT_GENERIC_DITHER -DOPT_I386 -DOPT_I586 -DOPT_I586_DITHER -DOPT_MMX -DOPT_3DNOW -DOPT_3DNOW_VINTAGE -DOPT_3DNOWEXT -DOPT_3DNOWEXT_VINTAGE -DOPT_SSE -DOPT_SSE_VINTAGE -DREAL_IS_FLOAT"
    more_sources="$s_fpu $s_i386 $s_i586 $s_i586d $s_mmx $s_3dnow_vintage $s_3dnowext_vintage $s_sse $s_x86multi $s_dither"
  ;;
  x86-64_alone) 
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_X86_64 -DREAL_IS_FLOAT"
    more_sources="$s_fpu $s_x86_64"
  ;;
  x86-64|x86-64_all|x86-64_dither)
    ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_MULTI -DOPT_X86_64 -DOPT_GENERIC -DOPT_GENERIC_DITHER -DREAL_IS_FLOAT"
    more_sources="$s_fpu $s_x86_64 $s_dither $s_x86_64_multi"
	if test "x$avx_support" = "xyes"; then
		ADD_CPPFLAGS="$ADD_CPPFLAGS -DOPT_AVX"
		more_sources="$more_sources $s_x86_64_avx"
		if test "x$YASM" != "xno"; then
			use_yasm_for_avx="yes"
		fi
	fi
  ;;
  *)
  	AC_MSG_ERROR([Unknown CPU type '$cpu_type'])
  ;;
esac

# Use yasm instead of the default assembler for AVX sources
if test "x$use_yasm_for_avx" = "xyes"; then
	case $host_os in
		*cygwin*|*mingw*)
			YASM_FORMAT="-f win64"
		;;
		*darwin*)
			YASM_FORMAT="-f macho"
		;;
		*)
			YASM_FORMAT="-f elf"
		;;
	esac
	AC_DEFINE(USE_YASM_FOR_AVX, 1, [Define to use yasm for assemble AVX sources.])
fi
AM_CONDITIONAL( [USE_YASM_FOR_AVX], [test "x$use_yasm_for_avx" = xyes] )
AC_SUBST(YASM_FORMAT)

# Check if we want feature report function.
AC_MSG_CHECKING([if we want feature report function])
feature_report=enabled
AC_ARG_ENABLE(feature_report,
	[  --disable-feature_report Disable feature report function ],
	[
		if test "x$enableval" = xno; then
		AC_MSG_RESULT([no])
		feature_report=disabled
		else
		AC_MSG_RESULT([yes])
		fi
	],
	[AC_MSG_RESULT([yes])])

if test "x$feature_report" = xenabled; then
	more_sources="$more_sources feature"
fi

# Return 0 if first arg appears in list of following arguments.
# Return 1 otherwise.
word_in_list() {
  word=$1
  shift
  # Avoid naming conflict with the outside!
  # Is local not POSIX sh?
  for given_word in "$@"
  do
    if test "x$word" = "x$given_word"; then
      return 0
    fi
  done
  return 1
}

for i in $more_sources
do
  # Make sure every file is only once in the list.
  word_in_list "$i.\$(OBJEXT)" $DECODER_OBJ || DECODER_OBJ="$DECODER_OBJ $i.\$(OBJEXT)"
  word_in_list "$i.lo" $DECODER_LOBJ || DECODER_LOBJ="$DECODER_LOBJ $i.lo"
done

AC_SUBST(DECODER_OBJ)
AC_SUBST(DECODER_LOBJ)

dnl Finally insert the code switch for alignment, above cpu selection having possibly changed the setting.
if test x"$ccalign" = xyes; then
	AC_DEFINE(CCALIGN, 1, [ Define if __attribute__((aligned(16))) shall be used ])
fi


dnl ############## Compiler Optimizations

CFLAGS="$ADD_CFLAGS $CFLAGS"
if test x"$newoldwritesample" = xenabled; then
	ADD_CPPFLAGS="$ADD_CPPFLAGS -DNEWOLD_WRITE_SAMPLE"
fi
CPPFLAGS="$ADD_CPPFLAGS $CPPFLAGS"
LDFLAGS="$ADD_LDFLAGS $LDFLAGS"

# None chosen?
if test "x$with_optimization" = "x"; then
	if test x"$debugging" = xenabled; then
		with_optimization="0"
	else
		# enable (gcc specific) default opts only with gcc
		if test "x$GCC" = xyes; then
			with_optimization="2"
		else
			with_optimization="0"
		fi
	fi
fi

case $with_optimization in
  0)
    # No Optimizations
    CFLAGS="-g -fno-common $CFLAGS"
  ;;
  1)
    CFLAGS="-O -fno-common $CFLAGS"
  ;;
  2)
    CFLAGS="-O2 -fomit-frame-pointer -ffast-math -fno-common $CFLAGS"
  ;;
  3)
    CFLAGS="-O3 -fomit-frame-pointer -funroll-all-loops -finline-functions -ffast-math -fno-common $CFLAGS"
  ;;
  4)
    CFLAGS="-O4 -fomit-frame-pointer -funroll-all-loops -finline-functions -ffast-math -fno-common $CFLAGS"
  ;;
  *)
  	AC_MSG_ERROR([Unknown optimizations level '$with_optimization'])
  ;;
esac

dnl If using gcc, prevent a bad alignment option from breaking things.
dnl Let's default to safe 16-byte alignment for any special x86 or altivec stuff.
if test "x$GCC" = xyes && echo "$CFLAGS" | $GREP 'mpreferred-stack-boundary=' > /dev/null; then
	case $cpu_type in
		x86 | i586* | mmx | sse | 3dnow* | altivec)
			AC_MSG_WARN( [ You are trying to mess with stack alignment. I know better. ] )
			CFLAGS="$CFLAGS -mpreferred-stack-boundary=4"
		;;
	esac
fi

dnl ############## Seektable size

if test "x$with_seektable" = "x"; then
	seektable=1000
else
	seektable=$with_seektable
fi

if test "$seektable" -gt 0; then
	AC_DEFINE(FRAME_INDEX, 1, [ Define if frame index should be used. ])
fi
# Define that always... also without frame index enabled.
AC_DEFINE_UNQUOTED(INDEX_SIZE, $seektable, [size of the frame index seek table])

dnl ############## Win32 function checks
# Check if we want Unicode for Win32. Cygwin does not need _wopen
win32_specific_codes=disabled
win32_unicode=unneeded
win32_wide_working=no
win32_winver_bump=no
AC_CHECK_HEADERS([windows.h], [win32_specific_codes=enabled])

if test "x$win32_specific_codes" = xenabled && test "x$host_os" != "xcygwin"; then
#### Check for Wide functions
  AC_CHECK_FUNC([_wopen], [win32_unicode=enabled],[win32_unicode=disabled])
  AC_MSG_CHECKING([if we want Unicode File Open for Win32])
  if test "x$win32_unicode" = xenabled; then
    AC_MSG_RESULT([yes])
    AC_MSG_CHECKING([if Unicode functions working])
    AC_LINK_IFELSE([AC_LANG_SOURCE([
      #include <windows.h>
      #include <stdlib.h>
      int main()
      {
        MultiByteToWideChar (0, 0, NULL, 0, NULL, 0);
        WideCharToMultiByte (0, 0, NULL, 0, NULL, 0, NULL, NULL);
        return 0;
      }
    ])], [win32_wide_working=yes], [win32_winver_bump=yes])
    if test "x$win32_wide_working" = xno; then
      AC_LINK_IFELSE([AC_LANG_SOURCE([
        #define WINVER 0x501
        #define _WIN32_WINNT 0x501
        #include <windows.h>
        #include <stdlib.h>
        int main()
        {
          MultiByteToWideChar (0, 0, NULL, 0, NULL, 0);
          WideCharToMultiByte (0, 0, NULL, 0, NULL, 0, NULL, NULL);
          return 0;
        }
      ])], [win32_wide_working=yes], [AC_MSG_RESULT([no])])
    fi
    if test "x$win32_wide_working" = xyes; then
      AC_MSG_RESULT([yes])
      AC_DEFINE([WANT_WIN32_UNICODE], [1], [ Define to use Unicode for Windows ])
    fi
  else
    AC_MSG_WARN([Unicode File Open for Win32 disabled])
  fi

#### WINVER Bump
if test x$win32_specific_codes = xenabled; then
  AC_MSG_CHECKING([if WINVER and _WIN32_WINNT needs version bumps])
  if test "x$win32_winver_bump" = "xyes"; then
    AC_MSG_RESULT([yes])
  # MultiByteToWideChar and WideCharToMultiByte are available for Win2K and above
    AC_DEFINE([WINVER], [0x0501], [ WinXP and above for ipv6 ])
    AC_DEFINE([_WIN32_WINNT], [0x0501], [ WinXP and above for ipv6 ])
  else
    AC_MSG_RESULT([no])
  fi
fi

fi #END OF WIN32 CHECKS

#### Check mingw.org for EOVERFLOW
AC_MSG_CHECKING([if we have EOVERFLOW macro])
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#include <errno.h>
int i = (EOVERFLOW) + 0;
])],[eoverflow_present=yes],[eoverflow_present=no])
AC_MSG_RESULT([$eoverflow_present])
AS_IF([test "x$eoverflow_present" = "xyes"],[],[AC_DEFINE([EOVERFLOW],[EFBIG],[Use EFBIG as substitude for EOVERFLOW, mingw.org may lack the latter])])

#### Use Win32 support codes
AM_CONDITIONAL([WIN32_CODES], [ test "x$win32_specific_codes" = xenabled ])

dnl ############## Final Output

LIBMPG123_LIBS=$LIBS
AC_SUBST(LIBMPG123_LIBS)

AC_CONFIG_FILES([
	Makefile
	libmpg123.pc
	src/Makefile
	src/mpg123.h
])

AC_OUTPUT

dnl ############## Display Message

echo "
  $PACKAGE_NAME $PACKAGE_VERSION

  Install path ............ $prefix
  CPU Optimization ........ $cpu_type
  Compiler Optimization ... $with_optimization
  Gapless Support ......... $gapless
  Debugging ............... $debugging
  Seek table size ......... $seektable
"
if test x"$LARGEFILE_BITS" = x; then
  echo "  File offsets ............ default"
else
  echo "  File offsets ............ $LARGEFILE_BITS"
  echo "    The lib will (try to) support default offset size, too."
fi
echo "  LFS alias symbols ....... $lfs_alias ($LFS_ALIAS_BITS)"
echo "  LFS alias type .......... $lfs_alias_type"

echo -n "
  Core libmpg123 features:
  Layer I ................. $layer1
  Layer II ................ $layer2
  Layer III ............... $layer3
  NtoM resampling ......... $ntom
  downsampled decoding .... $downsample
  Feeder/buffered input ... $feeder
  ID3v2 parsing ........... $id3v2
  String API .............. $string
  ICY parsing/conversion .. $icy
  Error/warning messages .. $messages
  Win32 Unicode File Open.. $win32_unicode
  Feature Report Function.. $feature_report
  Output formats (nofpu will disable all but 16 or 8 bit!):
  8 bit integer ........... $int8
  16 bit integer .......... $int16
  32/24 bit integer ....... $int32
  real (32 bit float) ..... $real
  Equalizer ............... $equalizer
  Optimization detail:
  Integer conversion ...... $integers
  New/old WRITE_SAMPLE .... $newoldwritesample
  new Huffman scheme ...... $newhuff
"
# just an empty line
echo

if test x"$with_optimization" = x0; then
	echo "No optimization flags chosen, make sure you have something basic in your CFLAGS at least...
"
fi
if test x"$cpu_type" = xi486; then
  echo "WARNING: You selected the i486 decoder. This is not recommened for regular use."
  echo "It was designed for actual i486 CPUs a long time ago. Generic C code is likely"
  echo "to perform better nowadays. Also, the decoder misses features like volume"
  echo "scaling and has clipping issues. Continue if you are into software archeology"
  echo "only."
fi

if test x"$debugging" = xenabled; then
	echo "CFLAGS='$CFLAGS'"
	echo "LIBS='$LIBS'"
else
    echo "Next type 'make' and then 'make install'."
fi
