cmake_minimum_required(VERSION 3.1.0)
project(SDL2_mixer C)

# FIXME: CMAKE SUPPORT IN SDL2_mixer IS VERY INCOMPLETE YET !!!
#
# FIXME: make it able build against system codec libraries, too.
# FIXME: handle library versioning.
# FIXME: test accross different target platforms.
#
# FIXME: missing CMakeLists.txt for MPG123
set(SUPPORT_MP3_MPG123 OFF CACHE BOOL "" FORCE)

option(SUPPORT_WAV "Support loading WAVE music" ON)
option(SUPPORT_FLAC "Support loading FLAC music with libFLAC" OFF)
option(SUPPORT_OGG "Support loading OGG Vorbis music via Tremor" OFF)
option(SUPPORT_MP3_MPG123 "Support loading MP3 music via MPG123" OFF)
option(SUPPORT_MOD_MODPLUG "Support loading MOD music via modplug" OFF)
option(SUPPORT_MID_TIMIDITY "Support loading MIDI music via TiMidity" OFF)

option(BUILD_SHARED_LIBS "Enable shared library" ON)

if (NOT ANDROID AND NOT (TARGET SDL2 OR TARGET SDL2-static))
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

include_directories(include src src/codecs)

add_library(SDL2_mixer)
add_library(SDL2::mixer ALIAS SDL2_mixer)

target_sources(SDL2_mixer PRIVATE
        src/effect_position.c src/effects_internal.c src/effect_stereoreverse.c
        src/mixer.c src/music.c src/utils.c
        src/codecs/load_aiff.c src/codecs/load_voc.c
        src/codecs/music_cmd.c
        src/codecs/music_wav.c src/codecs/music_flac.c
        src/codecs/music_mad.c src/codecs/music_mpg123.c
        src/codecs/music_ogg.c src/codecs/music_opus.c
        src/codecs/music_mikmod.c src/codecs/music_modplug.c
        src/codecs/music_xmp.c
        src/codecs/music_fluidsynth.c src/codecs/music_timidity.c
        src/codecs/music_nativemidi.c)

if (SUPPORT_WAV)
    target_compile_definitions(SDL2_mixer PRIVATE -DMUSIC_WAV)
endif()

if (SUPPORT_FLAC)
    target_compile_definitions(SDL2_mixer PRIVATE -DMUSIC_FLAC)
    add_subdirectory(external/flac-1.3.3)
    target_include_directories(SDL2_mixer PRIVATE external/flac-1.3.3/include)
    target_link_libraries(SDL2_mixer PRIVATE FLAC)
endif()

if (SUPPORT_OGG)
    target_compile_definitions(SDL2_mixer PRIVATE -DMUSIC_OGG -DOGG_USE_TREMOR -DOGG_HEADER=<ivorbisfile.h>)
    add_subdirectory(external/libogg-1.3.2)
    add_subdirectory(external/libvorbisidec-1.2.1)
    target_include_directories(SDL2_mixer PRIVATE external/libvorbisidec-1.2.1)
    target_link_libraries(SDL2_mixer PRIVATE vorbisidec ogg)
endif()

if (SUPPORT_MP3_MPG123)
    target_compile_definitions(SDL2_mixer PRIVATE -DMUSIC_MP3_MPG123)
    add_subdirectory(external/mpg123-1.25.13)
    target_link_libraries(SDL2_mixer PRIVATE mpg123)
endif()

if (SUPPORT_MOD_MODPLUG)
    target_compile_definitions(SDL2_mixer PRIVATE -DMUSIC_MOD_MODPLUG -DMODPLUG_HEADER=<modplug.h>)
    add_subdirectory(external/libmodplug-0.8.9.0)
    target_include_directories(SDL2_mixer PRIVATE external/libmodplug-0.8.9.0/src)
    target_link_libraries(SDL2_mixer PRIVATE modplug)
endif()

if (SUPPORT_MID_TIMIDITY)
    target_compile_definitions(SDL2_mixer PRIVATE -DMUSIC_MID_TIMIDITY)
    add_subdirectory(src/codecs/timidity)
    target_link_libraries(SDL2_mixer PRIVATE timidity)
endif()

if(WIN32 AND BUILD_SHARED_LIBS)
  target_compile_definitions(SDL2_mixer PRIVATE -DDLL_EXPORT)
endif()

target_include_directories(SDL2_mixer PUBLIC include)
target_link_libraries(SDL2_mixer PRIVATE SDL2::SDL2)
