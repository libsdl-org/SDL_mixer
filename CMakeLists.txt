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

    add_library(SDL2_mixer SHARED)

    target_sources(SDL2_mixer PRIVATE
            effect_position.c effects_internal.c
            effect_stereoreverse.c load_aiff.c load_voc.c mixer.c music.c
            music_cmd.c music_flac.c music_fluidsynth.c music_mad.c
            music_mikmod.c music_modplug.c music_mpg123.c music_nativemidi.c
            music_ogg.c music_opus.c music_timidity.c music_wav.c)

    if (SUPPORT_FLAC)
        add_definitions(-DMUSIC_FLAC)
        add_subdirectory(external/flac-1.3.2)
        include_directories(external/flac-1.3.2/include)
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
        add_subdirectory(external/mpg123-1.25.6)
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
