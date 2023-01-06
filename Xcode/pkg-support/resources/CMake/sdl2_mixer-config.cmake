# SDL2_mixer CMake configuration file:
# This file is meant to be placed in Resources/CMake of a SDL2_mixer framework

# INTERFACE_LINK_OPTIONS needs CMake 3.12
cmake_minimum_required(VERSION 3.12)

include(FeatureSummary)
set_package_properties(SDL2_mixer PROPERTIES
    URL "https://www.libsdl.org/projects/SDL_mixer/"
    DESCRIPTION "SDL_mixer is a sample multi-channel audio mixer library"
)

set(SDL2_mixer_FOUND                TRUE)

set(SDL2MIXER_VENDORED              TRUE)

set(SDL2MIXER_CMD                   FALSE)

set(SDL2MIXER_FLAC_LIBFLAC          FALSE)
set(SDL2MIXER_FLAC_DRFLAC           TRUE)

set(SDL2MIXER_GME                   FALSE)

set(SDL2MIXER_MOD                   FALSE)
set(SDL2MIXER_MOD_MODPLUG           FALSE)
set(SDL2MIXER_MOD_XMP               FALSE)
set(SDL2MIXER_MOD_XMP_LITE          FALSE)

set(SDL2MIXER_MP3                   TRUE)
set(SDL2MIXER_MP3_DRMP3             TRUE)
set(SDL2MIXER_MP3_MPG123            FALSE)

set(SDL2MIXER_MIDI                  FALSE)
set(SDL2MIXER_MIDI_FLUIDSYNTH       FALSE)
set(SDL2MIXER_MIDI_NATIVE           FALSE)
set(SDL2MIXER_MIDI_TIMIDITY         FALSE)

set(SDL2MIXER_OPUS                  FALSE)

set(SDL2MIXER_VORBIS                STB)
set(SDL2MIXER_VORBIS_STB            TRUE)
set(SDL2MIXER_VORBIS_TREMOR         FALSE)
set(SDL2MIXER_VORBIS_VORBISFILE     FALSE)

set(SDL2MIXER_WAVE                  TRUE)

string(REGEX REPLACE "SDL2_mixer\\.framework.*" "SDL2_mixer.framework" _sdl2mixer_framework_path "${CMAKE_CURRENT_LIST_DIR}")
string(REGEX REPLACE "SDL2_mixer\\.framework.*" "" _sdl2mixer_framework_parent_path "${CMAKE_CURRENT_LIST_DIR}")

# All targets are created, even when some might not be requested though COMPONENTS.
# This is done for compatibility with CMake generated SDL2_mixer-target.cmake files.

if(NOT TARGET SDL2_mixer::SDL2_mixer)
    add_library(SDL2_mixer::SDL2_mixer INTERFACE IMPORTED)
    set_target_properties(SDL2_mixer::SDL2_mixer
        PROPERTIES
            INTERFACE_COMPILE_OPTIONS "SHELL:-F \"${_sdl2mixer_framework_parent_path}\""
            INTERFACE_INCLUDE_DIRECTORIES "${_sdl2mixer_framework_path}/Headers"
            INTERFACE_LINK_OPTIONS "SHELL:-F \"${_sdl2mixer_framework_parent_path}\";SHELL:-framework SDL2_mixer"
            COMPATIBLE_INTERFACE_BOOL "SDL2_SHARED"
            INTERFACE_SDL2_SHARED "ON"
    )
endif()

unset(_sdl2mixer_framework_path)
unset(_sdl2mixer_framework_parent_path)
