dnl Process this file with autoconf to produce a configure script.

dnl Set various version strings - taken gratefully from the GTk sources
# See release_checklist.md
m4_define([MAJOR_VERSION_MACRO], [2])
m4_define([MINOR_VERSION_MACRO], [6])
m4_define([MICRO_VERSION_MACRO], [2])

AC_INIT([SDL2_mixer],
        [MAJOR_VERSION_MACRO.MINOR_VERSION_MACRO.MICRO_VERSION_MACRO],
        [https://github.com/libsdl-org/SDL_mixer/issues],
        [SDL2_mixer])
AC_CONFIG_SRCDIR([src/mixer.c])
AC_CONFIG_AUX_DIR(build-scripts)

AC_SUBST([MAJOR_VERSION], MAJOR_VERSION_MACRO)
AC_SUBST([MINOR_VERSION], MINOR_VERSION_MACRO)
AC_SUBST([MICRO_VERSION], MICRO_VERSION_MACRO)

BINARY_AGE=`expr $MINOR_VERSION \* 100 + $MICRO_VERSION`
AS_CASE(["$MINOR_VERSION"],
  [*@<:@02468@:>@],
    dnl Stable branch, 2.6.1 -> libSDL2_mixer-2.0.so.0.600.1
    [INTERFACE_AGE="$MICRO_VERSION"],
  [*],
    dnl Development branch, 2.5.1 -> libSDL2_mixer-2.0.so.0.501.0
    [INTERFACE_AGE=0])

AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(MICRO_VERSION)
AC_SUBST(INTERFACE_AGE)
AC_SUBST(BINARY_AGE)
# Automake defines VERSION to be the same as PACKAGE_VERSION, but we're
# not using Automake in SDL_mixer, so we need to set up the @VERSION@
# substitution for SDL2_mixer.pc ourselves
AC_SUBST([VERSION], [$PACKAGE_VERSION])

dnl libtool versioning
LT_INIT([win32-dll])

# For historical reasons, the library name redundantly includes the major
# version twice: libSDL2_mixer-2.0.so.0.
# TODO: in SDL 3, stop using -release, which will simplify it to libSDL3_mixer.so.0
LT_RELEASE=2.0

# Increment this if there is an incompatible change - but if that happens,
# we should rename the library from SDL2 to SDL3, at which point this would
# reset to 0 anyway.
LT_MAJOR=0

LT_REVISION=$INTERFACE_AGE
LT_AGE=`expr $BINARY_AGE - $INTERFACE_AGE`
LT_CURRENT=`expr $LT_MAJOR + $LT_AGE`
LT_EXTRA=""

m4_pattern_allow([^LT_])

AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)
AC_SUBST(LT_EXTRA)

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
        BASE_CFLAGS="-I/usr/include/mingw"
        BASE_LDFLAGS=""
        have_no_cygwin=no
        AC_MSG_CHECKING(for GCC -mno-cygwin option)
        save_CFLAGS="$CFLAGS"
        CFLAGS="$save_CFLAGS -mno-cygwin"
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],[])], [have_no_cygwin=yes],[])
        AC_MSG_RESULT($have_no_cygwin)
        CFLAGS="$save_CFLAGS"
        if test x$have_no_cygwin = xyes; then
            BASE_CFLAGS="$BASE_CFLAGS -mno-cygwin"
            BASE_LDFLAGS="-mno-cygwin"
        fi
        ;;
    *-*-os2*)
        # disable static builds on os/2
        enable_static=no
        # -DBUILD_SDL is needed for DECLSPEC
        BASE_CFLAGS="-DBUILD_SDL"
        BASE_LDFLAGS=""
        # OS/2 does not support a DLL name longer than 8 characters.
        LT_EXTRA="-os2dllname SDL2mix"
        ;;
    *)
        BASE_CFLAGS="-D_GNU_SOURCE=1"
        BASE_LDFLAGS=""
        ;;
esac
BUILD_CFLAGS="$CFLAGS $CPPFLAGS -I$srcdir/include -I$srcdir/src -I$srcdir/src/codecs"
EXTRA_CFLAGS="$INCLUDE $BASE_CFLAGS"
BUILD_LDFLAGS="$LDFLAGS"
EXTRA_LDFLAGS="$BASE_LDFLAGS"
MIXER_LIBS=
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

dnl See whether we can use gcc style dependency tracking
AC_ARG_ENABLE(dependency-tracking,
[AS_HELP_STRING([--enable-dependency-tracking],
               [Use gcc -MMD -MT dependency tracking [default=yes]])],
              , enable_dependency_tracking=yes)
if test x$enable_dependency_tracking = xyes; then
    have_gcc_mmd_mt=no
    AC_MSG_CHECKING(for GCC -MMD -MT option)
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #if !defined(__GNUC__) || __GNUC__ < 3
    #error Dependency tracking requires GCC 3.0 or newer
    #endif
    ]],[])], [have_gcc_mmd_mt=yes],[])
    AC_MSG_RESULT($have_gcc_mmd_mt)

    if test x$have_gcc_mmd_mt = xyes; then
        DEPENDENCY_TRACKING_OPTIONS="-MMD -MT \$@"
    fi
fi

case "$host" in
    *-*-cygwin* | *-*-mingw*)
        VERSION_SOURCES="$srcdir/version.rc"
        EXE=".exe"
        if test "$build" != "$host"; then # cross-compiling
            # Default cross-compile location
            ac_default_prefix=/usr/local/cross-tools/$host
        else
            # Look for the location of the tools and install there
            if test "$BUILD_PREFIX" != ""; then
                ac_default_prefix=$BUILD_PREFIX
            elif test "$MINGW_PREFIX" != ""; then
                ac_default_prefix=$MINGW_PREFIX
            fi
        fi
        ;;
    *-*-os2*)
        EXE=".exe"
        ;;
    *)
        EXE=""
        ;;
esac

# Standard C sources
SOURCES=`ls $srcdir/src/*.c $srcdir/src/codecs/*.c | fgrep -v playwave.c | fgrep -v playmus.c`

dnl set this to use on systems that use lib64 instead of lib
base_bindir=`echo \${bindir} | sed 's/.*\/\(.*\)/\1/; q'`
base_libdir=`echo \${libdir} | sed 's/.*\/\(.*\)/\1/; q'`

CheckNoUndef()
{
AC_MSG_CHECKING(for linker option --no-undefined)
    have_no_undefined=no
    case "${host_os}" in
dnl Skip this on platforms where it is just simply busted
    openbsd*) ;;
    darwin*) have_no_undefined="-Wl,-undefined,error"
       BUILD_LDFLAGS="$BUILD_LDFLAGS -Wl,-undefined,error" ;;
    *) save_LDFLAGS="$LDFLAGS"
       LDFLAGS="$LDFLAGS -Wl,--no-undefined"
       AC_LINK_IFELSE([AC_LANG_PROGRAM],[have_no_undefined=yes
          BUILD_LDFLAGS="$BUILD_LDFLAGS -Wl,--no-undefined"])
       LDFLAGS="$save_LDFLAGS"
       ;;
    esac
    AC_MSG_RESULT($have_no_undefined)
}

dnl See if GCC's -Wall is supported.
CheckWarnAll()
{
    AC_MSG_CHECKING(for GCC -Wall option)
    have_gcc_Wall=no

    save_CFLAGS="$CFLAGS"
    CFLAGS="$save_CFLAGS -Wall"
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([int x = 0;])], [have_gcc_Wall=yes])
    AC_MSG_RESULT($have_gcc_Wall)
    CFLAGS="$save_CFLAGS"

    if test x$have_gcc_Wall = xyes; then
        EXTRA_CFLAGS="$EXTRA_CFLAGS -Wall"

dnl Haiku headers use multicharacter constants all over the place. Ignore these warnings when using -Wall.
        AC_MSG_CHECKING(for necessary GCC -Wno-multichar option)
        need_gcc_Wno_multichar=no
        case "$host" in
            *-*-haiku*)
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
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #if !defined(__GNUC__) || __GNUC__ < 4
    #error SDL only uses visibility attributes in GCC 4 or newer
    #endif
    ]],[])], [have_gcc_fvisibility=yes],[])
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
SDL_VERSION=2.0.9
AM_PATH_SDL2($SDL_VERSION,
            :,
            AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])
)
EXTRA_CFLAGS="$EXTRA_CFLAGS $SDL_CFLAGS"

dnl For use in static assertions
EXTRA_CFLAGS="$EXTRA_CFLAGS -DSDL_BUILD_MAJOR_VERSION=$MAJOR_VERSION -DSDL_BUILD_MINOR_VERSION=$MINOR_VERSION -DSDL_BUILD_MICRO_VERSION=$MICRO_VERSION"

dnl check for GCC warning options
CheckWarnAll

dnl check for GCC visibility attributes
CheckVisibilityHidden

dnl Check for math library
AC_CHECK_LIB(m, pow, [LIBM="-lm"])

AC_CHECK_HEADERS([signal.h], [EXTRA_CFLAGS="$EXTRA_CFLAGS -DHAVE_SIGNAL_H"])
AC_CHECK_FUNCS(setbuf, [EXTRA_CFLAGS="$EXTRA_CFLAGS -DHAVE_SETBUF"])

dnl Check command-line options

SDL2MIXER_CMD=0
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
        EXTRA_CFLAGS="$EXTRA_CFLAGS -DMUSIC_CMD"
        SDL2MIXER_CMD=1
    fi
fi

SDL2MIXER_WAVE=0
AC_ARG_ENABLE([music-wave],
[AS_HELP_STRING([--enable-music-wave], [enable streaming WAVE music [default=yes]])],
              [], [enable_music_wave=yes])
if test x$enable_music_wave = xyes; then
    EXTRA_CFLAGS="$EXTRA_CFLAGS -DMUSIC_WAV"
    SDL2MIXER_WAVE=1
fi

AC_ARG_ENABLE([music-mod],
[AS_HELP_STRING([--enable-music-mod], [enable MOD music [default=yes]])],
              [], [enable_music_mod=yes])

SDL2MIXER_MOD_MODPLUG=0
AC_ARG_ENABLE([music-mod-modplug],
[AS_HELP_STRING([--enable-music-mod-modplug], [enable MOD music via modplug [default=yes]])],
              [], [enable_music_mod_modplug=yes])
AC_ARG_ENABLE([music-mod-modplug-shared],
[AS_HELP_STRING([--enable-music-mod-modplug-shared], [dynamically load modplug library [default=yes]])],
              [], [enable_music_mod_modplug_shared=yes])
if test x$enable_music_mod = xyes -a x$enable_music_mod_modplug = xyes; then
    PKG_CHECK_MODULES([MODPLUG], [libmodplug >= 0.8.8], [dnl
            have_libmodplug_hdr=yes
            have_libmodplug_lib=yes
            have_libmodplug_pc=yes
        ], [dnl
            AC_CHECK_HEADER([libmodplug/modplug.h], [have_libmodplug_hdr=yes])
            AC_CHECK_LIB([modplug], [ModPlug_Load], [have_libmodplug_lib=yes;MODPLUG_LIBS="-lmodplug"])
        ])

    if test x$have_libmodplug_hdr = xyes -a x$have_libmodplug_lib = xyes; then
        have_libmodplug=yes
        case "$host" in
            *-*-darwin*)
                modplug_lib=[`find_lib libmodplug.dylib`]
                ;;
            *-*-cygwin* | *-*-mingw*)
                modplug_lib=[`find_lib "libmodplug*.dll"`]
                ;;
            *)
                modplug_lib=[`find_lib "libmodplug[0-9]*.so.*"`]
                if test x$modplug_lib = x; then
                    modplug_lib=[`find_lib "libmodplug.so.*"`]
                fi
                ;;
        esac
        EXTRA_CFLAGS="$EXTRA_CFLAGS -DMUSIC_MOD_MODPLUG $MODPLUG_CFLAGS"
        if test x$enable_music_mod_modplug_shared = xyes && test x$modplug_lib != x; then
            echo "-- dynamic libmodplug -> $modplug_lib"
            EXTRA_CFLAGS="$EXTRA_CFLAGS -DMODPLUG_DYNAMIC=\\\"$modplug_lib\\\""
        else
            MIXER_LIBS="$MIXER_LIBS $MODPLUG_LIBS"
            if test x$have_libmodplug_pc = xyes; then
                PC_REQUIRES="$PC_REQUIRES libmodplug"
            else
                PC_LIBS="$PC_LIBS $MODPLUG_LIBS"
            fi
        fi
        SDL2MIXER_MOD_MODPLUG=1
    else
        AC_MSG_WARN([*** Unable to find ModPlug library (http://modplug-xmms.sourceforge.net/)])
    fi
fi

SDL2MIXER_MOD_XMP=0
SDL2MIXER_MOD_XMP_LITE=0
AC_ARG_ENABLE([music-mod-xmp],
[AS_HELP_STRING([--enable-music-mod-xmp], [enable MOD music via libxmp [default=no]])],
              [], [enable_music_mod_xmp=no])
AC_ARG_ENABLE([music-mod-xmp-lite],
[AS_HELP_STRING([--enable-music-mod-xmp-lite], [use libxmp-lite instead of libxmp [default=no]])],
              [], [enable_music_mod_xmp_lite=no])
AC_ARG_ENABLE([music-mod-xmp-shared],
[AS_HELP_STRING([--enable-music-mod-xmp-shared], [dynamically load xmp library [default=yes]])],
              [], [enable_music_mod_xmp_shared=yes])
if test x$enable_music_mod = xyes -a x$enable_music_mod_xmp = xyes; then
    xmplib=xmp
    if test x$enable_music_mod_xmp_lite = xyes; then
      xmplib=xmp-lite
    fi
    PKG_CHECK_MODULES([XMP], [lib$xmplib >= 4.2], [dnl
            have_libxmp_hdr=yes
            have_libxmp_lib=yes
            have_libxmp_pc=yes
        ], [dnl
            AC_CHECK_HEADER([xmp.h], [have_libxmp_hdr=yes])
            AC_CHECK_LIB([xmp], [xmp_load_module_from_memory], [have_libxmp_lib=yes;XMP_LIBS="-l$xmplib"])
        ])

    if test x$have_libxmp_hdr = xyes -a x$have_libxmp_lib = xyes; then
        have_libxmp=yes
        case "$host" in
            *-*-darwin*)
                xmp_lib=[`find_lib lib$xmplib.dylib`]
                ;;
            *-*-cygwin* | *-*-mingw*)
                xmp_lib=[`find_lib "lib$xmplib*.dll"`]
                ;;
            *)
                xmp_lib=[`find_lib "lib$xmplib[0-9]*.so.*"`]
                if test x$xmp_lib = x; then
                    xmp_lib=[`find_lib "lib$xmplib.so.*"`]
                fi
                ;;
        esac
        EXTRA_CFLAGS="$EXTRA_CFLAGS -DMUSIC_MOD_XMP $XMP_CFLAGS"
        if test x$enable_music_mod_xmp_shared = xyes && test x$xmp_lib != x; then
            echo "-- dynamic libxmp -> $xmp_lib"
            EXTRA_CFLAGS="$EXTRA_CFLAGS -DXMP_DYNAMIC=\\\"$xmp_lib\\\""
        else
            MIXER_LIBS="$MIXER_LIBS $XMP_LIBS"
            if test x$have_libxmp_pc = xyes; then
                PC_REQUIRES="$PC_REQUIRES lib$xmplib"
            else
                PC_LIBS="$PC_LIBS $XMP_LIBS"
            fi
        fi
        SDL2MIXER_MOD_XMP=1
        if test x$enable_music_mod_xmp_lite = xyes; then
            SDL2MIXER_MOD_XMP_LITE=1
        fi
    else
        AC_MSG_WARN([*** Unable to find xmp library (http://xmp.sourceforge.net/)])
    fi
fi

if test x$have_libmodplug != xyes -a x$have_libxmp != xyes; then
    AC_MSG_WARN([MOD support disabled])
fi

SDL2MIXER_MIDI_FLUIDSYNTH=0
SDL2MIXER_MIDI_NATIVE=0
SDL2MIXER_MIDI_TIMIDITY=0
AC_ARG_ENABLE([music-midi],
[AS_HELP_STRING([--enable-music-midi], [enable MIDI music [default=yes]])],
              [], [enable_music_midi=yes])
if test x$enable_music_midi = xyes; then
    EXTRA_CFLAGS="$EXTRA_CFLAGS -DMUSIC_MID"
    AC_ARG_ENABLE([music-midi-timidity],
[AS_HELP_STRING([--enable-music-midi-timidity], [enable timidity MIDI output [default=yes]])],
                  [], [enable_music_midi_timidity=yes])
    if test x$enable_music_midi_timidity = xyes; then
        EXTRA_CFLAGS="$EXTRA_CFLAGS -DMUSIC_MID_TIMIDITY -I\$(srcdir)/src/codecs/timidity"
        SOURCES="$SOURCES $srcdir/src/codecs/timidity/*.c"
        timidity_cfg=no
        AC_ARG_WITH([timidity-cfg],
[AS_HELP_STRING([--with-timidity-cfg=FILE],[Specify full path to timidity.cfg])], timidity_cfg="$withval", [])
        if test x$timidity_cfg != xyes -a x$timidity_cfg != xno; then
                EXTRA_CFLAGS="$EXTRA_CFLAGS -DTIMIDITY_CFG=\\\"$timidity_cfg\\\""
        fi
        SDL2MIXER_MIDI_TIMIDITY=1
    fi
    AC_ARG_ENABLE([music-midi-native],
[AS_HELP_STRING([--enable-music-midi-native], [enable native MIDI music output [default=yes]])],
                  [], [enable_music_midi_native=yes])
    if test x$enable_music_midi_native = xyes; then
        use_music_midi_native=no
        case "$host" in
            *-*-cygwin* | *-*-mingw*)
                use_music_midi_native=yes
                MIXER_LIBS="$MIXER_LIBS -lwinmm"
                PC_LIBS="$PC_LIBS -lwinmm"
                ;;
            *-*-darwin*)
                use_music_midi_native=yes
                MIXER_LIBS="$MIXER_LIBS -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit -Wl,-framework,CoreServices"
                PC_LIBS="$PC_LIBS -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit -Wl,-framework,CoreServices"
                ;;
            *-*-haiku*)
                use_music_midi_native=yes_cpp
                MIXER_LIBS="$MIXER_LIBS -lmidi"
                PC_LIBS="$PC_LIBS -lmidi"
                ;;
        esac
        if test x$use_music_midi_native = xyes; then
            EXTRA_CFLAGS="$EXTRA_CFLAGS -DMUSIC_MID_NATIVE -I\$(srcdir)/src/codecs/native_midi"
            SOURCES="$SOURCES $srcdir/src/codecs/native_midi/*.c"
            SDL2MIXER_MIDI_NATIVE=1
        elif test x$use_music_midi_native = xyes_cpp; then
            EXTRA_CFLAGS="$EXTRA_CFLAGS -DMUSIC_MID_NATIVE -I\$(srcdir)/src/codecs/native_midi"
            SOURCES="$SOURCES $srcdir/src/codecs/native_midi/*.c"
            SOURCES_CXX="$SOURCES_CXX $srcdir/src/codecs/native_midi/*.cpp"
            SDL2MIXER_MIDI_NATIVE=1
        fi
    fi
    AC_ARG_ENABLE([music-midi-fluidsynth],
[AS_HELP_STRING([--enable-music-midi-fluidsynth], [enable FluidSynth MIDI output [default=yes]])],
                  [], [enable_music_midi_fluidsynth=yes])
    AC_ARG_ENABLE([music-midi-fluidsynth-shared],
[AS_HELP_STRING([--enable-music-midi-fluidsynth-shared], [dynamically load FluidSynth library [default=yes]])],
                  [], [enable_music_midi_fluidsynth_shared=yes])
    if test x$enable_music_midi_fluidsynth = xyes; then
        PKG_CHECK_MODULES([FLUIDSYNTH], [fluidsynth], [dnl
                have_fluidsynth_hdr=yes
                have_fluidsynth_lib=yes
                have_fluidsynth_pc=yes
            ], [dnl
                AC_CHECK_HEADER([fluidsynth.h], [have_fluidsynth_hdr=yes])
                AC_CHECK_LIB([fluidsynth], [fluid_player_add_mem], [have_fluidsynth_lib=yes;FLUIDSYNTH_LIBS="-lfluidsynth"])
            ])
        if test x$have_fluidsynth_hdr = xyes -a x$have_fluidsynth_lib = xyes; then
            have_fluidsynth=yes
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
            EXTRA_CFLAGS="$EXTRA_CFLAGS -DMUSIC_MID_FLUIDSYNTH $FLUIDSYNTH_CFLAGS"
            if test x$enable_music_midi_fluidsynth_shared = xyes && test x$fluidsynth_lib != x; then
                echo "-- dynamic libfluidsynth -> $fluidsynth_lib"
                EXTRA_CFLAGS="$EXTRA_CFLAGS -DFLUIDSYNTH_DYNAMIC=\\\"$fluidsynth_lib\\\""
            else
                MIXER_LIBS="$MIXER_LIBS $FLUIDSYNTH_LIBS"
                if test x$have_fluidsynth_pc = xyes; then
                    PC_REQUIRES="$PC_REQUIRES fluidsynth"
                else
                    PC_LIBS="$PC_LIBS $FLUIDSYNTH_LIBS"
                fi
            fi
            SDL2MIXER_MIDI_FLUIDSYNTH=1
        else
            AC_MSG_WARN([*** Unable to find FluidSynth library (http://www.fluidsynth.org/)])
            AC_MSG_WARN([FluidSynth support disabled])
        fi
    fi
fi

if test x$enable_music_midi_timidity != xyes -a \
        x$use_music_midi_native != xyes -a x$use_music_midi_native != xyes_cpp -a \
        x$have_fluidsynth != xyes; then
    AC_MSG_WARN([MIDI support disabled])
fi

AC_ARG_ENABLE([music-ogg],
[AS_HELP_STRING([--enable-music-ogg], [enable Ogg Vorbis music [default=yes]])],
              [], [enable_music_ogg=yes])

SDL2MIXER_VORBIS_STB=0
AC_ARG_ENABLE(music-ogg-stb,
[AS_HELP_STRING([--enable-music-ogg-stb], [enable OGG Vorbis music via stb_vorbis [default=yes]])],
              [], enable_music_ogg_stb=yes)
if test x$enable_music_ogg = xyes -a x$enable_music_ogg_stb = xyes; then
    have_stb_vorbis=yes
    EXTRA_CFLAGS="$EXTRA_CFLAGS -DMUSIC_OGG -DOGG_USE_STB"
    SDL2MIXER_VORBIS_STB=1
fi

SDL2MIXER_VORBIS_VORBISFILE=0
AC_ARG_ENABLE(music-ogg-vorbis,
[AS_HELP_STRING([--enable-music-ogg-vorbis], [enable OGG Vorbis music via libvorbis [default=no]])],
              [], enable_music_ogg_vorbis=no)
AC_ARG_ENABLE([music-ogg-vorbis-shared],
[AS_HELP_STRING([--enable-music-ogg-vorbis-shared], [dynamically load libvorbis library [default=yes]])],
              [], [enable_music_ogg_vorbis_shared=yes])
if test x$enable_music_ogg = xyes -a x$enable_music_ogg_vorbis = xyes; then
    LIBS_SAVED="$LIBS"
    PKG_CHECK_MODULES([VORBIS], [vorbisfile], [dnl
            have_ogg_hdr=yes
            have_ogg_lib=yes
            have_ogg_pc=yes
        ], [dnl
            AC_CHECK_HEADER([vorbis/vorbisfile.h], [have_ogg_hdr=yes])
            AC_CHECK_LIB([vorbisfile], [ov_open_callbacks], [have_ogg_lib=yes;VORBIS_LIBS="-lvorbisfile -lvorbis -logg -lm"], [], [-lvorbis -logg -lm])
        ])
    LIBS="$LIBS_SAVED"

    if test x$have_ogg_hdr = xyes -a x$have_ogg_lib = xyes; then
        have_vorbis=yes
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
        EXTRA_CFLAGS="$EXTRA_CFLAGS -DMUSIC_OGG $VORBIS_CFLAGS"
        if test x$enable_music_ogg_vorbis_shared = xyes && test x$ogg_lib != x; then
            echo "-- dynamic libvorbisfile -> $ogg_lib"
            EXTRA_CFLAGS="$EXTRA_CFLAGS -DOGG_DYNAMIC=\\\"$ogg_lib\\\""
        else
            MIXER_LIBS="$MIXER_LIBS $VORBIS_LIBS"
            if test x$have_ogg_pc = xyes; then
                PC_REQUIRES="$PC_REQUIRES vorbisfile"
            else
                PC_LIBS="$PC_LIBS $VORBIS_LIBS"
            fi
        fi
        SDL2MIXER_VORBIS_VORBISFILE=1
    else
        AC_MSG_WARN([*** Unable to find Ogg Vorbis library (http://www.xiph.org/)])
    fi
fi

SDL2MIXER_VORBIS_TREMOR=0
AC_ARG_ENABLE(music-ogg-tremor,
[AS_HELP_STRING([--enable-music-ogg-tremor], [enable OGG Vorbis music via Tremor [default=no]])],
              [], enable_music_ogg_tremor=no)
AC_ARG_ENABLE([music-ogg-tremor-shared],
[AS_HELP_STRING([--enable-music-ogg-tremor-shared], [dynamically load Tremor library [default=yes]])],
              [], [enable_music_ogg_tremor_shared=yes])
if test x$enable_music_ogg = xyes -a x$enable_music_ogg_tremor = xyes; then
    LIBS_SAVED="$LIBS"
    PKG_CHECK_MODULES([TREMOR], [vorbisidec], [dnl
            have_tremor_hdr=yes
            have_tremor_lib=yes
            have_tremor_pc=yes
        ], [dnl
            AC_CHECK_HEADER([tremor/ivorbisfile.h], [have_tremor_hdr=yes])
            AC_CHECK_LIB([vorbisidec], [ov_open_callbacks], [have_tremor_lib=yes;TREMOR_LIBS="-lvorbisidec -logg"], [], [-logg])
        ])
    LIBS="$LIBS_SAVED"

    if test x$have_tremor_hdr = xyes -a x$have_tremor_lib = xyes; then
        have_tremor=yes
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
        EXTRA_CFLAGS="$EXTRA_CFLAGS -DMUSIC_OGG -DOGG_USE_TREMOR $TREMOR_CFLAGS"
        if test x$enable_music_ogg_tremor_shared = xyes && test x$ogg_lib != x; then
            echo "-- dynamic libvorbisidec -> $ogg_lib"
            EXTRA_CFLAGS="$EXTRA_CFLAGS -DOGG_DYNAMIC=\\\"$ogg_lib\\\""
        else
            MIXER_LIBS="$MIXER_LIBS $TREMOR_LIBS"
            if test x$have_tremor_pc = xyes; then
                PC_REQUIRES="$PC_REQUIRES vorbisidec"
            else
                PC_LIBS="$PC_LIBS $TREMOR_LIBS"
            fi
        fi
        SDL2MIXER_VORBIS_TREMOR=1
    else
        AC_MSG_WARN([*** Unable to find Ogg Vorbis Tremor library (http://www.xiph.org/)])
    fi
fi

if test x$enable_music_ogg = xyes -a \
        x$have_stb_vorbis != xyes -a x$have_vorbis != xyes -a x$have_tremor != xyes; then
    AC_MSG_WARN([Ogg Vorbis support disabled])
fi

SDL2MIXER_FLAC_DRFLAC=0
AC_ARG_ENABLE([music-flac],
[AS_HELP_STRING([--enable-music-flac], [enable FLAC music [default=yes]])],
              [], [enable_music_flac=yes])

AC_ARG_ENABLE(music-flac-drflac,
[AS_HELP_STRING([--enable-music-flac-drflac], [enable FLAC music via dr_flac [default=yes]])],
              [], [enable_music_flac_drflac=yes])
if test x$enable_music_flac = xyes -a x$enable_music_flac_drflac = xyes; then
    have_drflac=yes
    EXTRA_CFLAGS="$EXTRA_CFLAGS -DMUSIC_FLAC_DRFLAC"
    SDL2MIXER_FLAC_DRFLAC=1
fi

SDL2MIXER_FLAC_LIBFLAC=0
AC_ARG_ENABLE(music-flac-libflac,
[AS_HELP_STRING([--enable-music-flac-libflac], [enable FLAC music via libFLAC [default=no]])],
                  [], [enable_music_flac_libflac=no])
AC_ARG_ENABLE([music-flac-libflac-shared],
[AS_HELP_STRING([--enable-music-flac-libflac-shared], [dynamically load FLAC library [default=yes]])],
              [], [enable_music_flac_libflac_shared=yes])
if test x$enable_music_flac = xyes -a x$enable_music_flac_libflac = xyes; then
    libflac_ver=8
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
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include "FLAC/export.h"
#include "stdio.h"
]], [[
#if defined(FLAC_API_VERSION_CURRENT) && (FLAC_API_VERSION_CURRENT >= $libflac_ver)
#else
#error "old-flac"
#endif
]])], [have_flac_ver=yes],[have_flac_ver=no])
        CFLAGS="$save_CFLAGS"
        AC_MSG_RESULT($have_flac_ver)
    fi
    if test x$have_flac_ver = xyes; then
        have_libflac=yes
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
        EXTRA_CFLAGS="$EXTRA_CFLAGS -DMUSIC_FLAC_LIBFLAC $FLAC_CFLAGS"
        if test x$enable_music_flac_libflac_shared = xyes && test x$flac_lib != x; then
            echo "-- dynamic libFLAC -> $flac_lib"
            EXTRA_CFLAGS="$EXTRA_CFLAGS -DFLAC_DYNAMIC=\\\"$flac_lib\\\""
        else
            MIXER_LIBS="$MIXER_LIBS $FLAC_LIBS"
            if test x$have_flac_pc = xyes; then
                PC_REQUIRES="$PC_REQUIRES flac"
            else
                PC_LIBS="$PC_LIBS $FLAC_LIBS"
            fi
        fi
        SDL2MIXER_FLAC_LIBFLAC=1
    else
        AC_MSG_WARN([*** Unable to find FLAC library (https://xiph.org/flac/)])
    fi
fi

if test x$enable_music_flac = xyes -a \
        x$have_drflac != xyes -a x$have_libflac != xyes; then
    AC_MSG_WARN([FLAC support disabled])
fi

AC_ARG_ENABLE(music-mp3,
[AS_HELP_STRING([--enable-music-mp3], [enable MP3 music [default=yes]])],
              [], enable_music_mp3=yes)

SDL2MIXER_MP3_DRMP3=0
AC_ARG_ENABLE(music-mp3-drmp3,
[AS_HELP_STRING([--enable-music-mp3-drmp3], [enable MP3 music via dr_mp3 [default=yes]])],
              [], [enable_music_mp3_drmp3=yes])
if test x$enable_music_mp3 = xyes -a x$enable_music_mp3_drmp3 = xyes; then
    have_drmp3=yes
    EXTRA_CFLAGS="$EXTRA_CFLAGS -DMUSIC_MP3_DRMP3"
    SDL2MIXER_MP3_DRMP3=1
fi

SDL2MIXER_MP3_MPG123=0
AC_ARG_ENABLE(music-mp3-mpg123,
[AS_HELP_STRING([--enable-music-mp3-mpg123], [enable MP3 music via mpg123 [default=no]])],
                  [], [enable_music_mp3_mpg123=no])
AC_ARG_ENABLE([music-mp3-mpg123-shared],
[AS_HELP_STRING([--enable-music-mp3-mpg123-shared], [dynamically load mpg123 library [default=yes]])],
              [], [enable_music_mp3_mpg123_shared=yes])
if test x$enable_music_mp3 = xyes -a x$enable_music_mp3_mpg123 = xyes; then
    PKG_CHECK_MODULES([MPG123], [libmpg123], [dnl
            have_mpg123_hdr=yes
            have_mpg123_lib=yes
            have_mpg123_pc=yes
        ], [dnl
            AC_CHECK_HEADER([mpg123.h], [have_mpg123_hdr=yes])
            AC_CHECK_LIB([mpg123], [mpg123_replace_reader_handle], [have_mpg123_lib=yes;MPG123_LIBS="-lmpg123"])
        ])
    if test x$have_mpg123_hdr = xyes -a x$have_mpg123_lib = xyes; then
        have_libmpg123=yes
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
        EXTRA_CFLAGS="$EXTRA_CFLAGS -DMUSIC_MP3_MPG123 $MPG123_CFLAGS"
        if test x$enable_music_mp3_mpg123_shared = xyes && test x$mpg123_lib != x; then
            echo "-- dynamic libmpg123 -> $mpg123_lib"
            EXTRA_CFLAGS="$EXTRA_CFLAGS -DMPG123_DYNAMIC=\\\"$mpg123_lib\\\""
        else
            MIXER_LIBS="$MIXER_LIBS $MPG123_LIBS"
            if test x$have_mpg123_pc = xyes; then
                PC_REQUIRES="$PC_REQUIRES libmpg123"
            else
                PC_LIBS="$PC_LIBS $MPG123_LIBS"
            fi
        fi
        SDL2MIXER_MP3_MPG123=1
    else
        AC_MSG_WARN([*** Unable to find mpg123 library (https://www.mpg123.de)])
    fi
fi

if test x$enable_music_mp3 = xyes -a x$have_drmp3 != xyes -a x$have_libmpg123 != xyes; then
    AC_MSG_WARN([MP3 support disabled])
fi

SDL2MIXER_OPUS=0
AC_ARG_ENABLE([music-opus],
[AS_HELP_STRING([--enable-music-opus], [enable Opus music [default=yes]])],
              [], [enable_music_opus=yes])
AC_ARG_ENABLE([music-opus-shared],
[AS_HELP_STRING([--enable-music-opus-shared], [dynamically load opusfile library [default=yes]])],
              [], [enable_music_opus_shared=yes])
if test x$enable_music_opus = xyes; then
    LIBS_SAVED="$LIBS"
    PKG_CHECK_MODULES([OPUSFILE], [opusfile >= 0.2], [dnl
            have_opusfile_hdr=yes
            have_opusfile_lib=yes
            have_opusfile_pc=yes
        ], [dnl
            AC_CHECK_HEADER([opus/opusfile.h], [have_opusfile_hdr=yes])
            AC_CHECK_LIB([opusfile], [op_open_callbacks], [have_opusfile_lib=yes;OPUSFILE_LIBS="-lopusfile -lopus"], [], [-lopus -logg -lm])
        ])
    LIBS="$LIBS_SAVED"

    if test x$have_opusfile_hdr = xyes -a x$have_opusfile_lib = xyes; then
        have_opusfile=yes
        case "$host" in
            *-*-darwin*)
                opusfile_lib=[`find_lib libopusfile.dylib`]
                ;;
            *-*-cygwin* | *-*-mingw*)
                opusfile_lib=[`find_lib "libopusfile*.dll"`]
                ;;
            *)
                opusfile_lib=[`find_lib "libopusfile[0-9]*.so.*"`]
                if test x$opusfile_lib = x; then
                    opusfile_lib=[`find_lib "libopusfile.so.*"`]
                fi
                ;;
        esac
        EXTRA_CFLAGS="$EXTRA_CFLAGS -DMUSIC_OPUS $OPUSFILE_CFLAGS"
        if test x$enable_music_opus_shared = xyes && test x$opusfile_lib != x; then
            echo "-- dynamic opusfile -> $opusfile_lib"
            EXTRA_CFLAGS="$EXTRA_CFLAGS -DOPUS_DYNAMIC=\\\"$opusfile_lib\\\""
        else
            MIXER_LIBS="$MIXER_LIBS $OPUSFILE_LIBS"
            if test x$have_opusfile_pc = xyes; then
                PC_REQUIRES="$PC_REQUIRES opusfile"
            else
                PC_LIBS="$PC_LIBS $OPUSFILE_LIBS"
            fi
        fi
        SDL2MIXER_OPUS=1
    else
        AC_MSG_WARN([*** Unable to find opusfile library (http://opus-codec.org/)])
        AC_MSG_WARN([Opus support disabled])
    fi
fi

dnl check for LD --no-undefined option
CheckNoUndef

MIXER_LIBS="$MIXER_LIBS $LIBM"

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

dnl Calculate the location of the prefix, relative to the cmake folder
pkg_cmakedir='$libdir/cmake/SDL2_mixer'
AX_COMPUTE_RELATIVE_PATHS([pkg_cmakedir:prefix:cmake_prefix_relpath])
AC_SUBST([cmake_prefix_relpath])
AC_SUBST([cmake_prefix_relpath])

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
AC_SUBST(MIXER_LIBS)
AC_SUBST(EXE)
AC_SUBST(SDL_VERSION)
AC_SUBST(SDL_CFLAGS)
AC_SUBST(SDL_LIBS)
AC_SUBST(PC_REQUIRES)
AC_SUBST(PC_LIBS)

AC_SUBST(SDL2MIXER_CMD)
AC_SUBST(SDL2MIXER_FLAC_DRFLAC)
AC_SUBST(SDL2MIXER_FLAC_LIBFLAC)
AC_SUBST(SDL2MIXER_MOD_MODPLUG)
AC_SUBST(SDL2MIXER_MOD_XMP)
AC_SUBST(SDL2MIXER_MOD_XMP_LITE)
AC_SUBST(SDL2MIXER_MP3_DRMP3)
AC_SUBST(SDL2MIXER_MP3_MPG123)
AC_SUBST(SDL2MIXER_MIDI_FLUIDSYNTH)
AC_SUBST(SDL2MIXER_MIDI_NATIVE)
AC_SUBST(SDL2MIXER_MIDI_TIMIDITY)
AC_SUBST(SDL2MIXER_OPUS)
AC_SUBST(SDL2MIXER_VORBIS_STB)
AC_SUBST(SDL2MIXER_VORBIS_TREMOR)
AC_SUBST(SDL2MIXER_VORBIS_VORBISFILE)
AC_SUBST(SDL2MIXER_WAVE)

AC_CONFIG_FILES([
Makefile
SDL2_mixer.spec
SDL2_mixer.pc
sdl2_mixer-config.cmake
sdl2_mixer-config-version.cmake
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
