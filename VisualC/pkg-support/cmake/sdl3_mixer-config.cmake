# SDL3_mixer CMake configuration file:
# This file is meant to be placed in a cmake subfolder of SDL3_mixer-devel-3.x.y-VC

include(FeatureSummary)
set_package_properties(SDL3_mixer PROPERTIES
    URL "https://www.libsdl.org/projects/SDL_mixer/"
    DESCRIPTION "SDL_mixer is a sample multi-channel audio mixer library"
)

cmake_minimum_required(VERSION 3.0)

set(SDL3_mixer_FOUND                TRUE)

set(SDL3MIXER_VENDORED              TRUE)

set(SDL3MIXER_CMD                   FALSE)

set(SDL3MIXER_FLAC_LIBFLAC          FALSE)
set(SDL3MIXER_FLAC_DRFLAC           TRUE)

set(SDL3MIXER_GME                   FALSE)

set(SDL3MIXER_MOD                   TRUE)
set(SDL3MIXER_MOD_MODPLUG           TRUE)
set(SDL3MIXER_MOD_XMP               FALSE)
set(SDL3MIXER_MOD_XMP_LITE          FALSE)

set(SDL3MIXER_MP3                   TRUE)
set(SDL3MIXER_MP3_DRMP3             TRUE)
set(SDL3MIXER_MP3_MPG123            FALSE)

set(SDL3MIXER_MIDI                  TRUE)
set(SDL3MIXER_MIDI_FLUIDSYNTH       FALSE)
set(SDL3MIXER_MIDI_NATIVE           TRUE)
set(SDL3MIXER_MIDI_TIMIDITY         TRUE)

set(SDL3MIXER_OPUS                  TRUE)

set(SDL3MIXER_VORBIS                STB)
set(SDL3MIXER_VORBIS_STB            TRUE)
set(SDL3MIXER_VORBIS_TREMOR         FALSE)
set(SDL3MIXER_VORBIS_VORBISFILE     FALSE)

set(SDL3MIXER_WAVE                  TRUE)

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
