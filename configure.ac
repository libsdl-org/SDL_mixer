dnl Process this file with autoconf to produce a configure script.
AC_INIT
AC_CONFIG_SRCDIR([mixer.c])
AC_CONFIG_AUX_DIR(build-scripts)

dnl Set various version strings - taken gratefully from the GTk sources

# Making releases:
#   MICRO_VERSION += 1;
#   INTERFACE_AGE += 1;
#   BINARY_AGE += 1;
# if any functions have been added, set INTERFACE_AGE to 0.
# if backwards compatibility has been broken,
# set BINARY_AGE and INTERFACE_AGE to 0.
#
MAJOR_VERSION=1
MINOR_VERSION=2
MICRO_VERSION=13
INTERFACE_AGE=1
BINARY_AGE=13
VERSION=$MAJOR_VERSION.$MINOR_VERSION.$MICRO_VERSION

AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(MICRO_VERSION)
AC_SUBST(INTERFACE_AGE)
AC_SUBST(BINARY_AGE)
AC_SUBST(VERSION)

# libtool versioning
LT_INIT([win32-dll])

LT_RELEASE=$MAJOR_VERSION.$MINOR_VERSION
LT_CURRENT=`expr $MICRO_VERSION - $INTERFACE_AGE`
LT_REVISION=$INTERFACE_AGE
LT_AGE=`expr $BINARY_AGE - $INTERFACE_AGE`
m4_pattern_allow([^LT_])

AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

dnl Detect the canonical build and host environments
dnl AC_CANONICAL_HOST

dnl Check for tools
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET
LT_PROG_RC
PKG_PROG_PKG_CONFIG

dnl Set up the compiler and linker flags
case "$host" in
    *-*-cygwin*)
        # We build SDL on cygwin without the UNIX emulation layer
        BASE_CFLAGS="-I/usr/include/mingw -mno-cygwin"
        BASE_LDFLAGS="-mno-cygwin"
        ;;
    *)
        BASE_CFLAGS="-D_GNU_SOURCE=1"
        BASE_LDFLAGS=""
        ;;
esac
BUILD_CFLAGS="$CFLAGS $CPPFLAGS"
EXTRA_CFLAGS="$INCLUDE $BASE_CFLAGS"
BUILD_LDFLAGS="$LDFLAGS"
EXTRA_LDFLAGS="$BASE_LDFLAGS"
## These are common directories to find software packages
#for path in /usr/freeware /usr/pkg /usr/local; do
#    if test -d $path/include; then
#        EXTRA_CFLAGS="$EXTRA_CFLAGS -I$path/include"
#    fi
#    if test -d $path/lib; then
#        EXTRA_LDFLAGS="$EXTRA_LDFLAGS -L$path/lib"
#    fi
#done
CPPFLAGS="$CPPFLAGS $EXTRA_CFLAGS"
CFLAGS="$CFLAGS $EXTRA_CFLAGS"
LDFLAGS="$LDFLAGS $EXTRA_LDFLAGS"

dnl Check for compiler characteristics
AC_C_CONST

dnl See whether we can use gcc style dependency tracking
AC_ARG_ENABLE(dependency-tracking,
[AS_HELP_STRING([--enable-dependency-tracking],
               [Use gcc -MMD -MT dependency tracking [default=yes]])],
              , enable_dependency_tracking=yes)
if test x$enable_dependency_tracking = xyes; then
    have_gcc_mmd_mt=no
    AC_MSG_CHECKING(for GCC -MMD -MT option)
    AC_TRY_COMPILE([
    #if !defined(__GNUC__) || __GNUC__ < 3
    #error Dependency tracking requires GCC 3.0 or newer
    #endif
    ],[], [have_gcc_mmd_mt=yes])
    AC_MSG_RESULT($have_gcc_mmd_mt)

    if test x$have_gcc_mmd_mt = xyes; then
        DEPENDENCY_TRACKING_OPTIONS="-MMD -MT \$@"
    fi
fi

case "$host" in
    *mingw32ce*)
        #VERSION_SOURCES="$srcdir/version.rc"
        EXE=".exe"
        ;;
    *-*-cygwin* | *-*-mingw*)
        VERSION_SOURCES="$srcdir/version.rc"
        EXE=".exe"
        if test "$build" != "$host"; then # cross-compiling
            # Default cross-compile location
            ac_default_prefix=/usr/local/cross-tools/i386-mingw32
        else
            # Look for the location of the tools and install there
            if test "$BUILD_PREFIX" != ""; then
                ac_default_prefix=$BUILD_PREFIX
            elif test "$MINGW_PREFIX" != ""; then
                ac_default_prefix=$MINGW_PREFIX
            fi
        fi
        ;;
    *)
        SOURCES=""
        EXE=""
        ;;
esac

# Standard C sources
SOURCES="$SOURCES \
$srcdir/effect_position.c \
$srcdir/effect_stereoreverse.c \
$srcdir/effects_internal.c \
$srcdir/load_aiff.c \
$srcdir/load_voc.c \
$srcdir/mixer.c \
$srcdir/music.c"

dnl set this to use on systems that use lib64 instead of lib
base_bindir=`echo \${bindir} | sed 's/.*\/\(.*\)/\1/; q'`
base_libdir=`echo \${libdir} | sed 's/.*\/\(.*\)/\1/; q'`

dnl See if GCC's -Wall is supported.
CheckWarnAll()
{
    AC_MSG_CHECKING(for GCC -Wall option)
    have_gcc_Wall=no

    save_CFLAGS="$CFLAGS"
    CFLAGS="$save_CFLAGS -Wall"
    AC_TRY_COMPILE([int x = 0;],, [have_gcc_Wall=yes])
    AC_MSG_RESULT($have_gcc_Wall)
    CFLAGS="$save_CFLAGS"

    if test x$have_gcc_Wall = xyes; then
        EXTRA_CFLAGS="$EXTRA_CFLAGS -Wall"

dnl Haiku headers use multicharacter constants all over the place. Ignore these warnings when using -Wall.
        AC_MSG_CHECKING(for necessary GCC -Wno-multichar option)
        need_gcc_Wno_multichar=no
        case "$host" in
            *-*-beos* | *-*-haiku*)
                need_gcc_Wno_multichar=yes
                ;;
        esac
        AC_MSG_RESULT($need_gcc_Wno_multichar)
        if test x$need_gcc_Wno_multichar = xyes; then
            EXTRA_CFLAGS="$EXTRA_CFLAGS -Wno-multichar"
        fi
    fi
}

dnl See if GCC's -fvisibility=hidden is supported (gcc4 and later, usually).
CheckVisibilityHidden()
{
    AC_MSG_CHECKING(for GCC -fvisibility=hidden option)
    have_gcc_fvisibility=no
    case "$host" in
    *-*-cygwin* | *-*-mingw* | *-*-os2*)
        AC_MSG_RESULT([ignored for $host_os])
        return
        ;;
    esac

    visibility_CFLAGS="-fvisibility=hidden"
    save_CFLAGS="$CFLAGS"
    CFLAGS="$save_CFLAGS $visibility_CFLAGS -Werror"
    AC_TRY_COMPILE([
    #if !defined(__GNUC__) || __GNUC__ < 4
    #error SDL only uses visibility attributes in GCC 4 or newer
    #endif
    ],, [have_gcc_fvisibility=yes])
    AC_MSG_RESULT($have_gcc_fvisibility)
    CFLAGS="$save_CFLAGS"

    if test x$have_gcc_fvisibility = xyes; then
        EXTRA_CFLAGS="$EXTRA_CFLAGS $visibility_CFLAGS"
    fi
}

dnl Function to find a library in the compiler search path
find_lib()
{
    gcc_bin_path=[`$CC -print-search-dirs 2>/dev/null | fgrep programs: | sed 's/[^=]*=\(.*\)/\1/' | sed 's/:/ /g'`]
    gcc_lib_path=[`$CC -print-search-dirs 2>/dev/null | fgrep libraries: | sed 's/[^=]*=\(.*\)/\1/' | sed 's/:/ /g'`]
    env_lib_path=[`echo $LIBS $LDFLAGS $* | sed 's/-L[ ]*//g'`]
    if test "$cross_compiling" = yes; then
        host_lib_path=""
    else
        host_lib_path="$ac_default_prefix/$base_libdir $ac_default_prefix/$base_bindir /usr/$base_libdir /usr/local/$base_libdir"
    fi
    for path in $env_lib_path $gcc_bin_path $gcc_lib_path $host_lib_path; do
        lib=[`ls -- $path/$1 2>/dev/null | sed -e '/\.so\..*\./d' -e 's,.*/,,' | sort | tail -1`]
        if test x$lib != x; then
            echo $lib
            return
        fi
    done
}

dnl Check for SDL
SDL_VERSION=1.2.10
AM_PATH_SDL($SDL_VERSION,
            :,
            AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])
)
EXTRA_CFLAGS="$EXTRA_CFLAGS $SDL_CFLAGS"
EXTRA_LDFLAGS="$EXTRA_LDFLAGS $SDL_LIBS"

dnl check for GCC warning options
CheckWarnAll

dnl check for GCC visibility attributes
CheckVisibilityHidden

dnl Check for math library
AC_CHECK_LIB(m, pow, [LIBM="-lm"])

AC_CHECK_HEADERS([signal.h], [EXTRA_CFLAGS="$EXTRA_CFLAGS -DHAVE_SIGNAL_H"])
AC_CHECK_FUNCS(setbuf, [EXTRA_CFLAGS="$EXTRA_CFLAGS -DHAVE_SETBUF"])

dnl Check command-line options

AC_ARG_ENABLE([music-cmd],
[AS_HELP_STRING([--enable-music-cmd], [support an external music player [default=yes]])],
              [], [enable_music_cmd=detect])
if test "x$enable_music_cmd" != xno; then
    AC_CHECK_FUNCS([fork vfork])
    if test "x$ac_cv_func_fork" = "xyes"; then
        EXTRA_CFLAGS="$EXTRA_CFLAGS -DHAVE_FORK"
    elif test "x$ac_cv_func_vfork" = "xyes"; then
        EXTRA_CFLAGS="$EXTRA_CFLAGS -DHAVE_VFORK"
    elif test "x$enable_music_cmd" = "xyes"; then
        AC_MSG_ERROR([external music player not available on your platform])
    else
        enable_music_cmd=no
    fi
    if test "x$enable_music_cmd" != xno; then
        SOURCES="$SOURCES $srcdir/music_cmd.c"
        EXTRA_CFLAGS="$EXTRA_CFLAGS -DCMD_MUSIC"
    fi
fi

AC_ARG_ENABLE([music-wave],
[AS_HELP_STRING([--enable-music-wave], [enable streaming WAVE music [default=yes]])],
              [], [enable_music_wave=yes])
if test x$enable_music_wave = xyes; then
    SOURCES="$SOURCES $srcdir/wavestream.c"
    EXTRA_CFLAGS="$EXTRA_CFLAGS -DWAV_MUSIC"
fi

AC_ARG_ENABLE([music-mod],
[AS_HELP_STRING([--enable-music-mod], [enable MOD music via mikmod [default=yes]])],
              [], [enable_music_mod=yes])
AC_ARG_ENABLE([music-mod-shared],
[AS_HELP_STRING([--enable-music-mod-shared], [dynamically load MOD support [default=yes]])],
              [], [enable_music_mod_shared=yes])
if test x$enable_music_mod = xyes; then
    AC_PATH_PROG(LIBMIKMOD_CONFIG, libmikmod-config, no, [$PATH])
    AC_SUBST(LIBMIKMOD_CFLAGS)
    AC_SUBST(LIBMIKMOD_LIBS)
    if test "$LIBMIKMOD_CONFIG" != "no" ; then
       test -z "$LIBMIKMOD_CFLAGS" && LIBMIKMOD_CFLAGS=`$LIBMIKMOD_CONFIG --cflags`
       test -z "$LIBMIKMOD_LIBS" && LIBMIKMOD_LIBS=`$LIBMIKMOD_CONFIG --libs`
    fi
    CFLAGS_SAVED="$CFLAGS"
    LIBS_SAVED="$LIBS"
    CFLAGS="$CFLAGS $LIBMIKMOD_CFLAGS"
    LIBS="$LIBS $LIBMIKMOD_LIBS"
    AC_MSG_CHECKING([for libmikmod >= 3.1.10])
    AC_TRY_LINK([#include <mikmod.h>], [
#if (LIBMIKMOD_VERSION < 0x03010a)
choke me
#endif
Player_LoadGeneric(NULL,0,0);
], [have_libmikmod=yes],[have_libmikmod=no])

    CFLAGS="$CFLAGS_SAVED"
    LIBS="$LIBS_SAVED"
    AC_MSG_RESULT($have_libmikmod)

    if test x$have_libmikmod = xyes; then
        case "$host" in
            *-*-darwin*)
                mikmod_lib=[`find_lib libmikmod.dylib`]
                ;;
            *-*-cygwin* | *-*-mingw*)
                mikmod_lib=[`find_lib "libmikmod*.dll"`]
                ;;
            *)
                mikmod_lib=[`find_lib "libmikmod[0-9]*.so.*"`]
                if test x$mikmod_lib = x; then
                    mikmod_lib=[`find_lib "libmikmod.so.*"`]
                fi
                ;;
        esac
        SOURCES="$SOURCES $srcdir/*_mod.c"
        EXTRA_CFLAGS="$EXTRA_CFLAGS -DMOD_MUSIC $LIBMIKMOD_CFLAGS"
        if test x$enable_music_mod_shared = xyes && test x$mikmod_lib != x; then
            echo "-- dynamic libmikmod -> $mikmod_lib"
            EXTRA_CFLAGS="$EXTRA_CFLAGS -DMOD_DYNAMIC=\\\"$mikmod_lib\\\""
        else
            EXTRA_LDFLAGS="$EXTRA_LDFLAGS $LIBMIKMOD_LIBS"
            PC_LIBS="$PC_LIBS $LIBMIKMOD_LIBS"
        fi
    else
        AC_MSG_WARN([*** Unable to find MikMod library (http://mikmod.sourceforge.net/)])
    fi
fi

AC_ARG_ENABLE([music-mod-modplug],
[AS_HELP_STRING([--enable-music-mod-modplug], [enable MOD music via modplug [default=no]])],
              [], [enable_music_mod_modplug=no])
if test x$enable_music_mod_modplug = xyes; then
    PKG_CHECK_MODULES([MODPLUG], [libmodplug >= 0.8.7], [dnl
            have_libmodplug_hdr=yes
            have_libmodplug_lib=yes
            have_libmodplug_pc=yes
        ], [dnl
            AC_CHECK_HEADER([libmodplug/modplug.h], [have_libmodplug_hdr=yes])
            AC_CHECK_LIB([modplug], [ModPlug_Load], [have_libmodplug_lib=yes;MODPLUG_LIBS="-lmodplug"])
        ])

    if test x$have_libmodplug_hdr = xyes -a x$have_libmodplug_lib = xyes; then
        have_libmodplug=yes
        SOURCES="$SOURCES $srcdir/music_modplug.c"
        EXTRA_CFLAGS="$EXTRA_CFLAGS -DMODPLUG_MUSIC $MODPLUG_CFLAGS"
        EXTRA_LDFLAGS="$EXTRA_LDFLAGS $MODPLUG_LIBS"
        if test x$have_libmodplug_pc = xyes; then
            PC_REQUIRES="$PC_REQUIRES libmodplug"
        else
            PC_LIBS="$PC_LIBS $MODPLUG_LIBS"
        fi
    else
        AC_MSG_WARN([*** Unable to find ModPlug library (http://modplug-xmms.sourceforge.net/)])
    fi
fi

if test x$have_libmikmod != xyes -a x$have_libmodplug != xyes ; then
    AC_MSG_WARN([MOD support disabled])
fi

AC_ARG_ENABLE([music-midi],
[AS_HELP_STRING([--enable-music-midi], [enable MIDI music via timidity [default=yes]])],
              [], [enable_music_midi=yes])
if test x$enable_music_midi = xyes; then
    EXTRA_CFLAGS="$EXTRA_CFLAGS -DMID_MUSIC"
    AC_ARG_ENABLE([music-timidity-midi],
[AS_HELP_STRING([--enable-music-timidity-midi], [enable timidity MIDI output [default=yes]])],
                  [], [enable_music_timidity_midi=yes])
    if test x$enable_music_timidity_midi = xyes; then
        EXTRA_CFLAGS="$EXTRA_CFLAGS -DUSE_TIMIDITY_MIDI -I\$(srcdir)/timidity"
        SOURCES="$SOURCES $srcdir/timidity/*.c"
    fi
    AC_ARG_ENABLE([music-native-midi],
[AS_HELP_STRING([--enable-music-native-midi], [enable native MIDI music output [default=yes]])],
                  [], [enable_music_native_midi=yes])
    if test x$enable_music_native_midi = xyes; then
        use_music_native_midi=no
        case "$host" in
            *mingw32ce*)
                use_music_native_midi=no
                ;;
            *-*-cygwin* | *-*-mingw*)
                use_music_native_midi=yes
                EXTRA_LDFLAGS="$EXTRA_LDFLAGS -lwinmm"
                PC_LIBS="$PC_LIBS -lwinmm"
                ;;
            *-*-darwin*)
                use_music_native_midi=yes
                EXTRA_LDFLAGS="$EXTRA_LDFLAGS -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit -Wl,-framework,CoreServices"
                PC_LIBS="$PC_LIBS -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit -Wl,-framework,CoreServices"
                ;;
            *-*-haiku*)
                use_music_native_midi=yes_cpp
                EXTRA_LDFLAGS="$EXTRA_LDFLAGS -lmidi" 
                PC_LIBS="$PC_LIBS -lmidi"
                ;;
        esac
        if test x$use_music_native_midi = xyes; then
            EXTRA_CFLAGS="$EXTRA_CFLAGS -DUSE_NATIVE_MIDI -I\$(srcdir)/native_midi"
            SOURCES="$SOURCES $srcdir/native_midi/*.c"
        elif test x$use_music_native_midi = xyes_cpp; then
            EXTRA_CFLAGS="$EXTRA_CFLAGS -DUSE_NATIVE_MIDI -I\$(srcdir)/native_midi"
            SOURCES="$SOURCES $srcdir/native_midi/*.c"
            SOURCES_CXX="$SOURCES_CXX $srcdir/native_midi/*.cpp"
        fi
    fi
    AC_ARG_ENABLE([music-fluidsynth-midi],
[AS_HELP_STRING([--enable-music-fluidsynth-midi], [enable FluidSynth MIDI output [default=yes]])],
                  [], [enable_music_fluidsynth_midi=yes])
    AC_ARG_ENABLE([music-fluidsynth-shared],
[AS_HELP_STRING([--enable-music-fluidsynth-shared], [dynamically load FluidSynth support [default=yes]])],
                  [], [enable_music_fluidsynth_shared=yes])
    if test x$enable_music_fluidsynth_midi = xyes; then
        PKG_CHECK_MODULES([FLUIDSYNTH], [fluidsynth], [dnl
                have_fluidsynth_hdr=yes
                have_fluidsynth_lib=yes
                have_fluidsynth_pc=yes
            ], [dnl
                AC_CHECK_HEADER([fluidsynth.h], [have_fluidsynth_hdr=yes])
                AC_CHECK_LIB([fluidsynth], [fluid_player_add_mem], [have_fluidsynth_lib=yes;FLUIDSYNTH_LIBS="-lfluidsynth"])
            ])
        if test x$have_fluidsynth_hdr = xyes -a x$have_fluidsynth_lib = xyes; then
            case "$host" in
                *-*-darwin*)
                    fluidsynth_lib=[`find_lib libfluidsynth.dylib`]
                    ;;
                *-*-cygwin* | *-*-mingw*)
                    fluidsynth_lib=[`find_lib "fluidsynth*.dll"`]
                    if test x$fluidsynth_lib = x; then
                        fluidsynth_lib=[`find_lib "libfluidsynth*.dll"`]
                    fi
                    ;;
                *)
                    fluidsynth_lib=[`find_lib "libfluidsynth[0-9]*.so.*"`]
                    if test x$fluidsynth_lib = x; then
                        fluidsynth_lib=[`find_lib "libfluidsynth.so.*"`]
                    fi
                    ;;
            esac
            SOURCES="$SOURCES $srcdir/dynamic_fluidsynth.c $srcdir/music_fluidsynth.c"
            EXTRA_CFLAGS="$EXTRA_CFLAGS -DUSE_FLUIDSYNTH_MIDI $FLUIDSYNTH_CFLAGS"
            if test x$enable_music_fluidsynth_shared = xyes && test x$fluidsynth_lib != x; then
                echo "-- dynamic libfluidsyth -> $fluidsynth_lib"
                EXTRA_CFLAGS="$EXTRA_CFLAGS -DFLUIDSYNTH_DYNAMIC=\\\"$fluidsynth_lib\\\""
            else
                EXTRA_LDFLAGS="$EXTRA_LDFLAGS $FLUIDSYNTH_LIBS"
                if test x$have_fluidsynth_pc = xyes; then
                    PC_REQUIRES="$PC_REQUIRES fluidsynth"
                else
                    PC_LIBS="$PC_LIBS $FLUIDSYNTH_LIBS"
                fi
            fi
        else
            AC_MSG_WARN([*** Unable to find FluidSynth library (http://www.fluidsynth.org/)])
            AC_MSG_WARN([FluidSynth support disabled])
        fi
    fi
fi

AC_ARG_ENABLE([music-ogg],
[AS_HELP_STRING([--enable-music-ogg], [enable Ogg Vorbis music [default=yes]])],
              [], [enable_music_ogg=yes])
AC_ARG_ENABLE(music-ogg-tremor,
[AS_HELP_STRING([--enable-music-ogg-tremor], [enable OGG Vorbis music via libtremor [default=no]])],
              [], enable_music_ogg_tremor=no)
AC_ARG_ENABLE([music-ogg-shared],
[AS_HELP_STRING([--enable-music-ogg-shared], [dynamically load Ogg Vorbis support [default=yes]])],
              [], [enable_music_ogg_shared=yes])
if test x$enable_music_ogg = xyes; then
    LIBS_SAVED="$LIBS"
    if test x$enable_music_ogg_tremor = xyes; then
        PKG_CHECK_MODULES([TREMOR], [vorbisidec], [dnl
                have_tremor_hdr=yes
                have_tremor_lib=yes
                have_tremor_pc=yes
            ], [dnl
                AC_CHECK_HEADER([tremor/ivorbisfile.h], [have_tremor_hdr=yes])
                AC_CHECK_LIB([vorbisidec], [ov_open_callbacks], [have_tremor_lib=yes;TREMOR_LIBS="-lvorbisidec -logg"], [], [-logg])
            ])
        if test x$have_tremor_hdr = xyes -a x$have_tremor_lib = xyes; then
            case "$host" in
                *-*-darwin*)
                    ogg_lib=[`find_lib libvorbisidec.dylib`]
                    ;;
                *-*-cygwin* | *-*-mingw*)
                    ogg_lib=[`find_lib "vorbisidec*.dll"`]
                    if test x$ogg_lib = x; then
                        ogg_lib=[`find_lib "libvorbisidec*.dll"`]
                    fi
                    ;;
                *)
                    ogg_lib=[`find_lib "libvorbisidec[0-9]*.so.*"`]
                    if test x$ogg_lib = x; then
                        ogg_lib=[`find_lib "libvorbisidec.so.*"`]
                    fi
                    ;;
            esac
            SOURCES="$SOURCES $srcdir/*_ogg.c"
            EXTRA_CFLAGS="$EXTRA_CFLAGS -DOGG_MUSIC -DOGG_USE_TREMOR $TREMOR_CFLAGS"
            if test x$enable_music_ogg_shared = xyes && test x$ogg_lib != x; then
                echo "-- dynamic libvorbisidec -> $ogg_lib"
                EXTRA_CFLAGS="$EXTRA_CFLAGS -DOGG_DYNAMIC=\\\"$ogg_lib\\\""
            else
                EXTRA_LDFLAGS="$EXTRA_LDFLAGS $TREMOR_LIBS"
                if test x$have_tremor_pc = xyes; then
                    PC_REQUIRES="$PC_REQUIRES vorbisidec"
                else
                    PC_LIBS="$PC_LIBS $TREMOR_LIBS"
                fi
            fi
        else
            AC_MSG_WARN([*** Unable to find Ogg Vorbis Tremor library (http://www.xiph.org/)])
            AC_MSG_WARN([Ogg Vorbis support disabled])
        fi
    else
        PKG_CHECK_MODULES([VORBIS], [vorbisfile], [dnl
                have_ogg_hdr=yes
                have_ogg_lib=yes
                have_ogg_pc=yes
            ], [dnl
                AC_CHECK_HEADER([vorbis/vorbisfile.h], [have_ogg_hdr=yes])
                AC_CHECK_LIB([vorbisfile], [ov_open_callbacks], [have_ogg_lib=yes;VORBIS_LIBS="-lvorbisfile -lvorbis -logg -lm"], [], [-lvorbis -logg -lm])
            ])
        if test x$have_ogg_hdr = xyes -a x$have_ogg_lib = xyes; then
            case "$host" in
                *-*-darwin*)
                    ogg_lib=[`find_lib libvorbisfile.dylib`]
                    ;;
                *-*-cygwin* | *-*-mingw*)
                    ogg_lib=[`find_lib "libvorbisfile*.dll"`]
                    ;;
                *)
                    ogg_lib=[`find_lib "libvorbisfile[0-9]*.so.*"`]
                    if test x$ogg_lib = x; then
                        ogg_lib=[`find_lib "libvorbisfile.so.*"`]
                    fi
                    ;;
            esac
            SOURCES="$SOURCES $srcdir/*_ogg.c"
            EXTRA_CFLAGS="$EXTRA_CFLAGS -DOGG_MUSIC $VORBIS_CFLAGS"
            if test x$enable_music_ogg_shared = xyes && test x$ogg_lib != x; then
                echo "-- dynamic libvorbisfile -> $ogg_lib"
                EXTRA_CFLAGS="$EXTRA_CFLAGS -DOGG_DYNAMIC=\\\"$ogg_lib\\\""
            else
                EXTRA_LDFLAGS="$EXTRA_LDFLAGS $VORBIS_LIBS"
                if test x$have_ogg_pc = xyes; then
                    PC_REQUIRES="$PC_REQUIRES vorbisfile"
                else
                    PC_LIBS="$PC_LIBS $VORBIS_LIBS"
                fi
            fi
        else
            AC_MSG_WARN([*** Unable to find Ogg Vorbis library (http://www.xiph.org/)])
            AC_MSG_WARN([Ogg Vorbis support disabled])
        fi
    fi
    LIBS="$LIBS_SAVED"
fi

libflac_ver=8
AC_ARG_ENABLE([music-flac],
[AS_HELP_STRING([--enable-music-flac], [enable FLAC music [default=yes]])],
              [], [enable_music_flac=yes])
AC_ARG_ENABLE([music-flac-shared],
[AS_HELP_STRING([--enable-music-flac-shared],
              [dynamically load FLAC support [default=yes]])],
              [], [enable_music_flac_shared=yes])
if test x$enable_music_flac = xyes; then
    PKG_CHECK_MODULES([FLAC], [flac], [dnl
            have_flac_hdr=yes
            have_flac_lib=yes
            have_flac_pc=yes
        ], [dnl
            AC_CHECK_HEADER([FLAC/stream_decoder.h], [have_flac_hdr=yes])
            AC_CHECK_LIB([FLAC], [FLAC__stream_decoder_new], [have_flac_lib=yes;FLAC_LIBS="-lFLAC"])
        ])

    if test x$have_flac_hdr = xyes -a x$have_flac_lib = xyes; then
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $FLAC_CFLAGS"
        AC_MSG_CHECKING([for libflac so-name version >= $libflac_ver])
        AC_TRY_COMPILE([
#include "FLAC/export.h"
#include "stdio.h"
],[
#if defined(FLAC_API_VERSION_CURRENT) && (FLAC_API_VERSION_CURRENT >= $libflac_ver)
#else
#error "old-flac"
#endif
], have_flac_ver=yes, have_flac_ver=no)
        CFLAGS="$save_CFLAGS"
        AC_MSG_RESULT($have_flac_ver)
    fi
    if test x$have_flac_ver = xyes; then
        case "$host" in
            *-*-darwin*)
                flac_lib=[`find_lib libFLAC.dylib`]
                ;;
            *-*-cygwin* | *-*-mingw*)
                flac_lib=[`find_lib "libFLAC*.dll"`]
                ;;
            *)
                flac_lib=[`find_lib "libFLAC[0-9]*.so.*"`]
                if test x$flac_lib = x; then
                    flac_lib=[`find_lib "libFLAC.so.*"`]
                fi
                ;;
        esac
        SOURCES="$SOURCES $srcdir/*_flac.c"
        EXTRA_CFLAGS="$EXTRA_CFLAGS -DFLAC_MUSIC $FLAC_CFLAGS"
        if test x$enable_music_flac_shared = xyes && test x$flac_lib != x; then
            echo "-- dynamic libFLAC -> $flac_lib"
            EXTRA_CFLAGS="$EXTRA_CFLAGS -DFLAC_DYNAMIC=\\\"$flac_lib\\\""
        else
            EXTRA_LDFLAGS="$EXTRA_LDFLAGS $FLAC_LIBS"
            if test x$have_flac_pc = xyes; then
                PC_REQUIRES="$PC_REQUIRES flac"
            else
                PC_LIBS="$PC_LIBS $FLAC_LIBS"
            fi
        fi
    else
        AC_MSG_WARN([*** Unable to find FLAC library (http://flac.sourceforge.net/)])
        AC_MSG_WARN([FLAC support disabled])
    fi
fi

AC_ARG_ENABLE(music-mp3,
[AS_HELP_STRING([--enable-music-mp3], [enable MP3 music via libmpg123 [default=yes]])],
              [], enable_music_mp3=yes)
AC_ARG_ENABLE([music-mp3-shared],
[AS_HELP_STRING([--enable-music-mp3-shared], [dynamically load MP3 support [default=yes]])],
              [], [enable_music_mp3_shared=yes])
have_mpg123=no
if test x$enable_music_mp3 = xyes; then
    PKG_CHECK_MODULES([MPG123], [libmpg123], [dnl
            have_mpg123_hdr=yes
            have_mpg123_lib=yes
            have_mpg123_pc=yes
        ], [dnl
            AC_CHECK_HEADER([mpg123.h], [have_mpg123_hdr=yes])
            AC_CHECK_LIB([mpg123], [mpg123_replace_reader_handle], [have_mpg123_lib=yes;MPG123_LIBS="-lmpg123"])
        ])
    if test x$have_mpg123_hdr = xyes -a x$have_mpg123_lib = xyes; then
        have_mpg123=yes
        case "$host" in
            *-*-darwin*)
                mpg123_lib=[`find_lib libmpg123.dylib`]
                ;;
            *-*-cygwin* | *-*-mingw*)
                mpg123_lib=[`find_lib "libmpg123*.dll"`]
                ;;
            *)
                mpg123_lib=[`find_lib "libmpg123.so.*"`]
                ;;
        esac
        SOURCES="$SOURCES $srcdir/*_mp3.c $srcdir/*_mpg.c"
        EXTRA_CFLAGS="$EXTRA_CFLAGS -DMP3_MUSIC $MPG123_CFLAGS"
        if test x$enable_music_mp3_shared = xyes && test x$mpg123_lib != x; then
            echo "-- dynamic libmpg123 -> $mpg123_lib"
            EXTRA_CFLAGS="$EXTRA_CFLAGS -DMPG123_DYNAMIC=\\\"$mpg123_lib\\\""
        else
            EXTRA_LDFLAGS="$EXTRA_LDFLAGS $MPG123_LIBS"
            if test x$have_mpg123_pc = xyes; then
                PC_REQUIRES="$PC_REQUIRES libmpg123"
            else
                PC_LIBS="$PC_LIBS $MPG123_LIBS"
            fi
        fi
    else
        AC_MSG_WARN([*** Unable to find mpg123 library (https://www.mpg123.de)])
    fi
fi

AC_ARG_ENABLE(music-mp3-mad-gpl,
[AS_HELP_STRING([--enable-music-mp3-mad-gpl], [enable MP3 music via libmad GPL code [default=no]])],
                  [], [enable_music_mp3_mad_gpl=no])
have_libmad=no
if test x$enable_music_mp3_mad_gpl = xyes; then
    AC_MSG_CHECKING(for libmad headers)
    AC_TRY_COMPILE([
     #include "mad.h"
    ],[
    ],[
    have_libmad=yes
    ])
    AC_MSG_RESULT($have_libmad)
    if test x$have_libmad = xyes; then
        SOURCES="$SOURCES $srcdir/music_mad.c"
        EXTRA_CFLAGS="$EXTRA_CFLAGS -DMP3_MAD_MUSIC"
        EXTRA_LDFLAGS="$EXTRA_LDFLAGS -lmad"
        PC_LIBS="$PC_LIBS -lmad"
    else
        AC_MSG_WARN([*** Unable to find MAD library (http://www.underbit.com/products/mad/)])
    fi
fi

if test x$have_mpg123 != xyes -a x$have_libmad != xyes; then
    AC_MSG_WARN([MP3 support disabled])
else
    SOURCES="$SOURCES $srcdir/mp3utils.c"
fi

EXTRA_LDFLAGS="$EXTRA_LDFLAGS $LIBM"

OBJECTS=`echo $SOURCES`
DEPENDS=`echo $SOURCES`
OBJECTS=`echo "$OBJECTS" | sed 's,[[^ ]]*/\([[^ ]]*\)\.c,$(objects)/\1.lo,g'`
DEPENDS=`echo "$DEPENDS" | sed 's,\([[^ ]]*\)/\([[^ ]]*\)\.c,\\
$(objects)/\2.lo: \1/\2.c \$(objects)/.created\\
	\$(LIBTOOL) --mode=compile \$(CC) \$(CFLAGS) \$(EXTRA_CFLAGS) '"$DEPENDENCY_TRACKING_OPTIONS"' -c \$< -o \$@,g'`

OBJECTS_CXX=`echo $SOURCES_CXX`
DEPENDS_CXX=`echo $SOURCES_CXX`
OBJECTS_CXX=`echo "$OBJECTS_CXX" | sed 's,[[^ ]]*/\([[^ ]]*\)\.cpp,$(objects)/\1.lo,g'`
DEPENDS_CXX=`echo "$DEPENDS_CXX" | sed 's,\([[^ ]]*\)/\([[^ ]]*\)\.cpp,\\
$(objects)/\2.lo: \1/\2.cpp \$(objects)/.created\\
	\$(LIBTOOL) --mode=compile \$(CXX) \$(CFLAGS) \$(EXTRA_CFLAGS) '"$DEPENDENCY_TRACKING_OPTIONS"' -c \$< -o \$@,g'`
OBJECTS="$OBJECTS $OBJECTS_CXX"
DEPENDS="$DEPENDS $DEPENDS_CXX"
DEPENDS=`echo "$DEPENDS" | sed 's,\\$,\\\\$,g'`

VERSION_OBJECTS=`echo $VERSION_SOURCES`
VERSION_DEPENDS=`echo $VERSION_SOURCES`
VERSION_OBJECTS=`echo "$VERSION_OBJECTS" | sed 's,[[^ ]]*/\([[^ ]]*\)\.rc,$(objects)/\1.lo,g'`
VERSION_DEPENDS=`echo "$VERSION_DEPENDS" | sed 's,\([[^ ]]*\)/\([[^ ]]*\)\.rc,\\
$(objects)/\2.lo: \1/\2.rc \$(objects)/.created\\
	\$(LIBTOOL) --tag=RC --mode=compile \$(RC) \$< -o \$@,g'`
VERSION_DEPENDS=`echo "$VERSION_DEPENDS" | sed 's,\\$,\\\\$,g'`

PLAYWAVE_SOURCES="$srcdir/playwave.c"
PLAYWAVE_OBJECTS=`echo $PLAYWAVE_SOURCES`
PLAYWAVE_DEPENDS=`echo $PLAYWAVE_SOURCES`
PLAYWAVE_OBJECTS=`echo "$PLAYWAVE_OBJECTS" | sed 's,[[^ ]]*/\([[^ ]]*\)\.c,$(objects)/\1.lo,g'`
PLAYWAVE_DEPENDS=`echo "$PLAYWAVE_DEPENDS" | sed 's,\([[^ ]]*\)/\([[^ ]]*\)\.c,\\
$(objects)/\2.lo: \1/\2.c \$(objects)/.created\\
	\$(LIBTOOL) --mode=compile \$(CC) \$(CFLAGS) \$(EXTRA_CFLAGS) '"$DEPENDENCY_TRACKING_OPTIONS"' -c \$< -o \$@,g'`
PLAYWAVE_DEPENDS=`echo "$PLAYWAVE_DEPENDS" | sed 's,\\$,\\\\$,g'`

PLAYMUS_SOURCES="$srcdir/playmus.c"
PLAYMUS_OBJECTS=`echo $PLAYMUS_SOURCES`
PLAYMUS_DEPENDS=`echo $PLAYMUS_SOURCES`
PLAYMUS_OBJECTS=`echo "$PLAYMUS_OBJECTS" | sed 's,[[^ ]]*/\([[^ ]]*\)\.c,$(objects)/\1.lo,g'`
PLAYMUS_DEPENDS=`echo "$PLAYMUS_DEPENDS" | sed 's,\([[^ ]]*\)/\([[^ ]]*\)\.c,\\
$(objects)/\2.lo: \1/\2.c \$(objects)/.created\\
	\$(LIBTOOL) --mode=compile \$(CC) \$(CFLAGS) \$(EXTRA_CFLAGS) '"$DEPENDENCY_TRACKING_OPTIONS"' -c \$< -o \$@,g'`
PLAYMUS_DEPENDS=`echo "$PLAYMUS_DEPENDS" | sed 's,\\$,\\\\$,g'`

dnl Expand the sources and objects needed to build the library
AC_SUBST(ac_aux_dir)
AC_SUBST(OBJECTS)
AC_SUBST(VERSION_OBJECTS)
AC_SUBST(PLAYWAVE_OBJECTS)
AC_SUBST(PLAYMUS_OBJECTS)
AC_SUBST(BUILD_CFLAGS)
AC_SUBST(EXTRA_CFLAGS)
AC_SUBST(BUILD_LDFLAGS)
AC_SUBST(EXTRA_LDFLAGS)
AC_SUBST(EXE)
AC_SUBST(SDL_VERSION)
AC_SUBST(SDL_CFLAGS)
AC_SUBST(SDL_LIBS)
AC_SUBST(PC_REQUIRES)
AC_SUBST(PC_LIBS)

AC_CONFIG_FILES([
    Makefile SDL_mixer.spec SDL_mixer.qpg SDL_mixer.pc
])
AC_CONFIG_COMMANDS([default],
                   [cat >>Makefile <<__EOF__

# Build rules for objects
-include \$(OBJECTS:.lo=.d)
$DEPENDS
$VERSION_DEPENDS

-include \$(PLAYWAVE_OBJECTS:.lo=.d)
$PLAYWAVE_DEPENDS

-include \$(PLAYMUS_OBJECTS:.lo=.d)
$PLAYMUS_DEPENDS
__EOF__
], [
DEPENDS="$DEPENDS"
VERSION_DEPENDS="$VERSION_DEPENDS"
PLAYWAVE_DEPENDS="$PLAYWAVE_DEPENDS"
PLAYMUS_DEPENDS="$PLAYMUS_DEPENDS"
])
AC_OUTPUT
