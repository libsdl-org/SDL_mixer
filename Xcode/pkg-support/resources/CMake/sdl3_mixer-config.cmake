# SDL3_mixer CMake configuration file:
# This file is meant to be placed in Resources/CMake of a SDL3_mixer framework

# INTERFACE_LINK_OPTIONS needs CMake 3.12
cmake_minimum_required(VERSION 3.12)

include(FeatureSummary)
set_package_properties(SDL3_mixer PROPERTIES
    URL "https://www.libsdl.org/projects/SDL_mixer/"
    DESCRIPTION "SDL_mixer is a sample multi-channel audio mixer library"
)

set(SDL3_mixer_FOUND                TRUE)

set(SDL3MIXER_VENDORED              TRUE)

set(SDL3MIXER_CMD                   FALSE)

set(SDL3MIXER_FLAC_LIBFLAC          FALSE)
set(SDL3MIXER_FLAC_DRFLAC           TRUE)

set(SDL3MIXER_GME                   FALSE)

set(SDL3MIXER_MOD                   FALSE)
set(SDL3MIXER_MOD_MODPLUG           FALSE)
set(SDL3MIXER_MOD_XMP               FALSE)
set(SDL3MIXER_MOD_XMP_LITE          FALSE)

set(SDL3MIXER_MP3                   TRUE)
set(SDL3MIXER_MP3_DRMP3             TRUE)
set(SDL3MIXER_MP3_MPG123            FALSE)

set(SDL3MIXER_MIDI                  FALSE)
set(SDL3MIXER_MIDI_FLUIDSYNTH       FALSE)
set(SDL3MIXER_MIDI_NATIVE           FALSE)
set(SDL3MIXER_MIDI_TIMIDITY         FALSE)

set(SDL3MIXER_OPUS                  FALSE)

set(SDL3MIXER_VORBIS                STB)
set(SDL3MIXER_VORBIS_STB            TRUE)
set(SDL3MIXER_VORBIS_TREMOR         FALSE)
set(SDL3MIXER_VORBIS_VORBISFILE     FALSE)

set(SDL3MIXER_WAVE                  TRUE)

string(REGEX REPLACE "SDL3_mixer\\.framework.*" "SDL3_mixer.framework" _sdl3mixer_framework_path "${CMAKE_CURRENT_LIST_DIR}")
string(REGEX REPLACE "SDL3_mixer\\.framework.*" "" _sdl3mixer_framework_parent_path "${CMAKE_CURRENT_LIST_DIR}")

# All targets are created, even when some might not be requested though COMPONENTS.
# This is done for compatibility with CMake generated SDL3_mixer-target.cmake files.

if(NOT TARGET SDL3_mixer::SDL3_mixer)
    add_library(SDL3_mixer::SDL3_mixer INTERFACE IMPORTED)
    set_target_properties(SDL3_mixer::SDL3_mixer
        PROPERTIES
            INTERFACE_COMPILE_OPTIONS "SHELL:-F \"${_sdl3mixer_framework_parent_path}\""
            INTERFACE_INCLUDE_DIRECTORIES "${_sdl3mixer_framework_path}/Headers"
            INTERFACE_LINK_OPTIONS "SHELL:-F \"${_sdl3mixer_framework_parent_path}\";SHELL:-framework SDL3_mixer"
            COMPATIBLE_INTERFACE_BOOL "SDL3_SHARED"
            INTERFACE_SDL3_SHARED "ON"
    )
endif()

unset(_sdl3mixer_framework_path)
unset(_sdl3mixer_framework_parent_path)
