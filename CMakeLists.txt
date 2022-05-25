cmake_minimum_required(VERSION 3.1.0)
project(SDL2_mixer C)

# FIXME: CMAKE SUPPORT IN SDL2_mixer IS VERY INCOMPLETE YET !!!
#
# FIXME: make it able build against system codec libraries, too.
# FIXME: handle library versioning.
# FIXME: test accross different target platforms.

# See docs/release_checklist.md
set(MAJOR_VERSION 2)
set(MINOR_VERSION 5)
set(MICRO_VERSION 1)
set(FULL_VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${MICRO_VERSION}")

option(SUPPORT_WAV "Support loading WAVE music" ON)
option(SUPPORT_FLAC "Support loading FLAC music with libFLAC" OFF)
option(SUPPORT_OGG "Support loading OGG Vorbis music via libvorbis" OFF)
option(SUPPORT_OPUS "Support loading OGG Opus music via libopusfile" OFF)
option(SUPPORT_MP3_MPG123 "Support loading MP3 music via MPG123" OFF)
option(SUPPORT_MOD_MODPLUG "Support loading MOD music via modplug" OFF)
option(SUPPORT_MID_TIMIDITY "Support loading MIDI music via TiMidity" ON)

option(BUILD_SHARED_LIBS "Enable shared library" ON)

if (NOT (TARGET SDL2::SDL2 OR TARGET SDL2::SDL2-static))
    find_package(SDL2 REQUIRED)
    if(NOT TARGET SDL2::SDL2)
        # SDL < 2.0.12
        add_library(SDL2::SDL2 INTERFACE IMPORTED)
        set_target_properties(SDL2::SDL2 PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${SDL2_INCLUDE_DIRS} ${SDL2_INCLUDE_DIR}
            INTERFACE_LINK_LIBRARIES ${SDL2_LIBRARIES} ${SDL2_LIBRARY}
        )
    endif()
endif()

# Calculate a libtool-like version number
math(EXPR BINARY_AGE "${MINOR_VERSION} * 100 + ${MICRO_VERSION}")
if(MINOR_VERSION MATCHES "[02468]$")
    # Stable branch, 2.6.1 -> libSDL2_mixer-2.0.so.0.600.1
    set(INTERFACE_AGE ${MICRO_VERSION})
else()
    # Development branch, 2.5.1 -> libSDL2_mixer-2.0.so.0.501.0
    set(INTERFACE_AGE 0)
endif()

# Increment this if there is an incompatible change - but if that happens,
# we should rename the library from SDL2 to SDL3, at which point this would
# reset to 0 anyway.
set(LT_MAJOR "0")

math(EXPR LT_AGE "${BINARY_AGE} - ${INTERFACE_AGE}")
math(EXPR LT_CURRENT "${LT_MAJOR} + ${LT_AGE}")
set(LT_REVISION "${INTERFACE_AGE}")
# For historical reasons, the library name redundantly includes the major
# version twice: libSDL2_mixer-2.0.so.0.
# TODO: in SDL 3, set the OUTPUT_NAME to plain SDL3_mixer, which will simplify
# it to libSDL3_mixer.so.0
set(LT_RELEASE "2.0")
set(LT_VERSION "${LT_MAJOR}.${LT_AGE}.${LT_REVISION}")

# The following should match the versions in the Xcode project file.
# Each version is 1 higher than you might expect, for compatibility
# with libtool: macOS ABI versioning is 1-based, unlike other platforms
# which are normally 0-based.
math(EXPR DYLIB_CURRENT_VERSION_MAJOR "${LT_MAJOR} + ${LT_AGE} + 1")
math(EXPR DYLIB_CURRENT_VERSION_MINOR "${LT_REVISION}")
math(EXPR DYLIB_COMPAT_VERSION_MAJOR "${LT_MAJOR} + 1")
set(DYLIB_CURRENT_VERSION "${DYLIB_CURRENT_VERSION_MAJOR}.${DYLIB_CURRENT_VERSION_MINOR}.0")
# For historical reasons this is 3.0.0 rather than the expected 1.0.0
set(DYLIB_COMPATIBILITY_VERSION "3.0.0")

# For the static assertions in mixer.c
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDL_BUILD_MAJOR_VERSION=${MAJOR_VERSION}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDL_BUILD_MINOR_VERSION=${MINOR_VERSION}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDL_BUILD_MICRO_VERSION=${MICRO_VERSION}")

include_directories(include src src/codecs)

add_library(SDL2_mixer)
add_library(SDL2::mixer ALIAS SDL2_mixer)

if(SUPPORT_MID_TIMIDITY)
    set(TIMIDITY_SRCS
        src/codecs/timidity/common.c
        src/codecs/timidity/instrum.c
        src/codecs/timidity/mix.c
        src/codecs/timidity/output.c
        src/codecs/timidity/playmidi.c
        src/codecs/timidity/readmidi.c
        src/codecs/timidity/resample.c
        src/codecs/timidity/tables.c
        src/codecs/timidity/timidity.c
    )
endif()

target_sources(SDL2_mixer PRIVATE
        src/effect_position.c
        src/effects_internal.c
        src/effect_stereoreverse.c
        src/mixer.c
        src/music.c
        src/utils.c
        src/codecs/load_aiff.c
        src/codecs/load_voc.c
        src/codecs/music_cmd.c
        src/codecs/music_wav.c
        src/codecs/music_drflac.c
        src/codecs/music_flac.c
        src/codecs/music_drmp3.c
        src/codecs/music_mad.c
        src/codecs/music_mpg123.c
        src/codecs/mp3utils.c
        src/codecs/music_ogg.c
        src/codecs/music_ogg_stb.c
        src/codecs/music_opus.c
        src/codecs/music_mikmod.c
        src/codecs/music_modplug.c
        src/codecs/music_xmp.c
        src/codecs/music_fluidsynth.c
        src/codecs/music_timidity.c
        src/codecs/music_nativemidi.c
        ${TIMIDITY_SRCS}
)

if (SUPPORT_WAV)
    target_compile_definitions(SDL2_mixer PRIVATE -DMUSIC_WAV)
endif()

if (SUPPORT_OGG OR SUPPORT_FLAC OR SUPPORT_OPUS)
    add_subdirectory(external/ogg)
endif()

if (SUPPORT_FLAC)
    target_compile_definitions(SDL2_mixer PRIVATE -DMUSIC_FLAC_LIBFLAC)
    add_subdirectory(external/flac)
    target_include_directories(SDL2_mixer PRIVATE external/flac/include)
    target_link_libraries(SDL2_mixer PRIVATE FLAC)
endif()

if (SUPPORT_OGG)
    target_compile_definitions(SDL2_mixer PRIVATE -DMUSIC_OGG)
    add_subdirectory(external/vorbis)
    target_include_directories(SDL2_mixer PRIVATE external/vorbis/include)
    target_link_libraries(SDL2_mixer PRIVATE vorbisfile vorbis ogg)
endif()

if (SUPPORT_OPUS)
    target_compile_definitions(SDL2_mixer PRIVATE -DMUSIC_OPUS -DOPUSFILE_HEADER=<opusfile.h>)
    set(OP_DISABLE_HTTP ON CACHE BOOL "Disable HTTP support")
    set(OP_DISABLE_EXAMPLES ON CACHE BOOL "Do not build example applications")
    set(OP_DISABLE_DOCS ON CACHE BOOL "Do not build API documentation")
    add_subdirectory(external/opus)
    add_subdirectory(external/opusfile)
    target_link_libraries(SDL2_mixer PRIVATE opusfile)
endif()

if (SUPPORT_MP3_MPG123)
    target_compile_definitions(SDL2_mixer PRIVATE -DMUSIC_MP3_MPG123)
    add_subdirectory(external/mpg123/ports/cmake)
    target_include_directories(SDL2_mixer PRIVATE external/mpg123/ports/cmake/src/libmpg123)
    target_link_libraries(SDL2_mixer PRIVATE libmpg123)
endif()

if (SUPPORT_MOD_MODPLUG)
    target_compile_definitions(SDL2_mixer PRIVATE -DMUSIC_MOD_MODPLUG -DMODPLUG_HEADER=<modplug.h>)
    add_subdirectory(external/libmodplug)
    target_include_directories(SDL2_mixer PRIVATE external/libmodplug/src)
    target_link_libraries(SDL2_mixer PRIVATE modplug)
endif()

if (SUPPORT_MID_TIMIDITY)
    target_compile_definitions(SDL2_mixer PRIVATE -DMUSIC_MID_TIMIDITY)
endif()

if(BUILD_SHARED_LIBS)
    if(WIN32 OR OS2)
        set_target_properties(SDL2_mixer PROPERTIES PREFIX "")
    endif()
    if(APPLE)
        # TODO: Use DYLIB_COMPATIBILITY_VERSION, DYLIB_CURRENT_VERSION here
    endif()
    if(WIN32)
        target_compile_definitions(SDL2_mixer PRIVATE -DDLL_EXPORT)
        target_sources(SDL2_mixer PRIVATE version.rc)
    elseif(OS2)
        # OS/2 doesn't support a DLL name longer than 8 characters.
        set_target_properties(SDL2_mixer PROPERTIES
               OUTPUT_NAME "SDL2mix"
        )
    elseif(UNIX AND NOT ANDROID)
        # This is compatible with the libtool build
        set_target_properties(SDL2_mixer PROPERTIES
               VERSION ${LT_VERSION}
               SOVERSION ${LT_MAJOR}
               OUTPUT_NAME "SDL2_mixer-${LT_RELEASE}"
        )
    endif()
endif()

target_include_directories(SDL2_mixer PUBLIC include)

if (BUILD_SHARED_LIBS)
  target_link_libraries(SDL2_mixer PRIVATE SDL2::SDL2)
else()
  target_link_libraries(SDL2_mixer PRIVATE SDL2::SDL2-static)
endif()
