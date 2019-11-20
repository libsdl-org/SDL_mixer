cmake_minimum_required(VERSION 3.0)
project(SDL_mixer C)


if (ANDROID)

    # FIXME: missing CMakeLists.txt for MPG123
    set(SUPPORT_MP3_MPG123 OFF CACHE BOOL "" FORCE)

    option(SUPPORT_FLAC "Support loading FLAC music with libFLAC" OFF)
    option(SUPPORT_OGG "Support loading OGG Vorbis music via Tremor" OFF)
    option(SUPPORT_MP3_MPG123 "Support loading MP3 music via MPG123" OFF)
    option(SUPPORT_MOD_MODPLUG "Support loading MOD music via modplug" OFF)
    option(SUPPORT_MID_TIMIDITY "Support TiMidity" OFF)

    include_directories(include src src/codecs)

    add_library(SDL2_mixer SHARED)

    target_sources(SDL2_mixer PRIVATE
            src/effect_position.c src/effects_internal.c src/effect_stereoreverse.c
            src/mixer.c src/music.c src/compat.c
            src/codecs/load_aiff.c src/codecs/load_voc.c
            src/codecs/music_cmd.c src/codecs/music_flac.c
            src/codecs/music_fluidsynth.c src/codecs/music_mad.c
            src/codecs/music_mikmod.c src/codecs/music_modplug.c
            src/codecs/music_mpg123.c src/codecs/music_nativemidi.c
            src/codecs/music_ogg.c src/codecs/music_opus.c
            src/codecs/music_timidity.c src/codecs/music_wav.c)

    if (SUPPORT_FLAC)
        add_definitions(-DMUSIC_FLAC)
        add_subdirectory(external/flac-1.3.3)
        include_directories(external/flac-1.3.3/include)
        target_link_libraries(SDL2_mixer PRIVATE FLAC)
    endif()

    if (SUPPORT_OGG)
        add_definitions(-DMUSIC_OGG -DOGG_USE_TREMOR -DOGG_HEADER=<ivorbisfile.h>)
        add_subdirectory(external/libogg-1.3.2)
        add_subdirectory(external/libvorbisidec-1.2.1)
        include_directories(external/libvorbisidec-1.2.1)
        target_link_libraries(SDL2_mixer PRIVATE vorbisfile vorbisidec ogg)
    endif()

    if (SUPPORT_MP3_MPG123)
        add_definitions(-DMUSIC_MP3_MPG123)
        add_subdirectory(external/mpg123-1.25.13)
        target_link_libraries(SDL2_mixer PRIVATE mpg123)
    endif()

    if (SUPPORT_MOD_MODPLUG)
        add_definitions(-DMUSIC_MOD_MODPLUG -DMODPLUG_HEADER=<modplug.h>)
        add_subdirectory(external/libmodplug-0.8.9.0)
        include_directories(external/libmodplug-0.8.9.0/src)
        target_link_libraries(SDL2_mixer PRIVATE modplug)
    endif()

    if (SUPPORT_MID_TIMIDITY)
        add_definitions(-DMUSIC_MID_TIMIDITY)
        add_subdirectory(timidity)
        target_link_libraries(SDL2_mixer PRIVATE timidity)
    endif()

    target_include_directories(SDL2_mixer PUBLIC .)
    target_link_libraries(SDL2_mixer PRIVATE SDL2)
else()

endif()
