# SDL3_mixer CMake configuration file:
# This file is meant to be placed in a cmake subfolder of SDL3_mixer-devel-3.x.y-VC

include(FeatureSummary)
set_package_properties(SDL3_mixer PROPERTIES
    URL "https://www.libsdl.org/projects/SDL_mixer/"
    DESCRIPTION "SDL_mixer is a sample multi-channel audio mixer library"
)

cmake_minimum_required(VERSION 3.0)

set(SDL3_mixer_FOUND                TRUE)

set(SDLMIXER_VENDORED              TRUE)

set(SDLMIXER_CMD                   FALSE)

set(SDLMIXER_FLAC_LIBFLAC          FALSE)
set(SDLMIXER_FLAC_DRFLAC           TRUE)

set(SDLMIXER_GME                   FALSE)

set(SDLMIXER_MOD                   TRUE)
set(SDLMIXER_MOD_MODPLUG           TRUE)
set(SDLMIXER_MOD_XMP               FALSE)
set(SDLMIXER_MOD_XMP_LITE          FALSE)

set(SDLMIXER_MP3                   TRUE)
set(SDLMIXER_MP3_DRMP3             TRUE)
set(SDLMIXER_MP3_MPG123            FALSE)

set(SDLMIXER_MIDI                  TRUE)
set(SDLMIXER_MIDI_FLUIDSYNTH       FALSE)
set(SDLMIXER_MIDI_NATIVE           TRUE)
set(SDLMIXER_MIDI_TIMIDITY         TRUE)

set(SDLMIXER_OPUS                  TRUE)

set(SDLMIXER_VORBIS                STB)
set(SDLMIXER_VORBIS_STB            TRUE)
set(SDLMIXER_VORBIS_TREMOR         FALSE)
set(SDLMIXER_VORBIS_VORBISFILE     FALSE)

set(SDLMIXER_WAVE                  TRUE)

if(CMAKE_SIZEOF_VOID_P STREQUAL "4")
    set(_sdl_arch_subdir "x86")
elseif(CMAKE_SIZEOF_VOID_P STREQUAL "8")
    set(_sdl_arch_subdir "x64")
else()
    unset(_sdl_arch_subdir)
    set(SDL3_mixer_FOUND FALSE)
    return()
endif()

set(_sdl3mixer_incdir       "${CMAKE_CURRENT_LIST_DIR}/../include")
set(_sdl3mixer_library      "${CMAKE_CURRENT_LIST_DIR}/../lib/${_sdl_arch_subdir}/SDL3_mixer.lib")
set(_sdl3mixer_dll          "${CMAKE_CURRENT_LIST_DIR}/../lib/${_sdl_arch_subdir}/SDL3_mixer.dll")

# All targets are created, even when some might not be requested though COMPONENTS.
# This is done for compatibility with CMake generated SDL3_mixer-target.cmake files.

if(NOT TARGET SDL3_mixer::SDL3_mixer)
    add_library(SDL3_mixer::SDL3_mixer SHARED IMPORTED)
    set_target_properties(SDL3_mixer::SDL3_mixer
        PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${_sdl3mixer_incdir}"
            IMPORTED_IMPLIB "${_sdl3mixer_library}"
            IMPORTED_LOCATION "${_sdl3mixer_dll}"
            COMPATIBLE_INTERFACE_BOOL "SDL3_SHARED"
            INTERFACE_SDL3_SHARED "ON"
    )
endif()

unset(_sdl_arch_subdir)
unset(_sdl3mixer_incdir)
unset(_sdl3mixer_library)
unset(_sdl3mixer_dll)
