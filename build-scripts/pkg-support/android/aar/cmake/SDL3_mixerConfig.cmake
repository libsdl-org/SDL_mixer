# SDL CMake configuration file:
# This file is meant to be placed in lib/cmake/SDL3_mixer subfolder of a reconstructed Android SDL3_mixer SDK

cmake_minimum_required(VERSION 3.0...3.28)

include(FeatureSummary)
set_package_properties(SDL3_mixer PROPERTIES
    URL "https://www.libsdl.org/projects/SDL_mixer/"
    DESCRIPTION "SDL_mixer is a sample multi-channel audio mixer library"
)

# Copied from `configure_package_config_file`
macro(set_and_check _var _file)
    set(${_var} "${_file}")
    if(NOT EXISTS "${_file}")
        message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
    endif()
endmacro()

# Copied from `configure_package_config_file`
macro(check_required_components _NAME)
    foreach(comp ${${_NAME}_FIND_COMPONENTS})
        if(NOT ${_NAME}_${comp}_FOUND)
            if(${_NAME}_FIND_REQUIRED_${comp})
                set(${_NAME}_FOUND FALSE)
            endif()
        endif()
    endforeach()
endmacro()

set(SDL3_mixer_FOUND                TRUE)

set(SDLMIXER_VENDORED              TRUE)

set(SDLMIXER_FLAC_LIBFLAC          FALSE)
set(SDLMIXER_FLAC_DRFLAC           TRUE)

set(SDLMIXER_GME                   FALSE)

set(SDLMIXER_MOD                   FALSE)
set(SDLMIXER_MOD_XMP               FALSE)
set(SDLMIXER_MOD_XMP_LITE          FALSE)

set(SDLMIXER_MP3                   TRUE)
set(SDLMIXER_MP3_DRMP3             TRUE)
set(SDLMIXER_MP3_MPG123            FALSE)

set(SDLMIXER_MIDI                  FALSE)
set(SDLMIXER_MIDI_FLUIDSYNTH       FALSE)
set(SDLMIXER_MIDI_NATIVE           FALSE)
set(SDLMIXER_MIDI_TIMIDITY         TRUE)

set(SDLMIXER_OPUS                  FALSE)

set(SDLMIXER_VORBIS                STB)
set(SDLMIXER_VORBIS_STB            TRUE)
set(SDLMIXER_VORBIS_TREMOR         FALSE)
set(SDLMIXER_VORBIS_VORBISFILE     FALSE)

set(SDLMIXER_WAVE                  TRUE)

set(SDL3_mixer_FOUND TRUE)

if(SDL_CPU_X86)
    set(_sdl_arch_subdir "x86")
elseif(SDL_CPU_X64)
    set(_sdl_arch_subdir "x86_64")
elseif(SDL_CPU_ARM32)
    set(_sdl_arch_subdir "armeabi-v7a")
elseif(SDL_CPU_ARM64)
    set(_sdl_arch_subdir "arm64-v8a")
else()
    set(SDL3_mixer_FOUND FALSE)
    return()
endif()

get_filename_component(_sdl3mixer_prefix "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
get_filename_component(_sdl3mixer_prefix "${_sdl3mixer_prefix}/.." ABSOLUTE)
get_filename_component(_sdl3mixer_prefix "${_sdl3mixer_prefix}/.." ABSOLUTE)
set_and_check(_sdl3mixer_prefix          "${_sdl3mixer_prefix}")
set_and_check(_sdl3mixer_include_dirs    "${_sdl3mixer_prefix}/include")

set_and_check(_sdl3mixer_lib             "${_sdl3mixer_prefix}/lib/${_sdl_arch_subdir}/libSDL3_mixer.so")

unset(_sdl_arch_subdir)
unset(_sdl3mixer_prefix)

# All targets are created, even when some might not be requested though COMPONENTS.
# This is done for compatibility with CMake generated SDL3_mixer-target.cmake files.

set(SDL3_mixer_SDL3_mixer-shared_FOUND FALSE)
if(EXISTS "${_sdl3mixer_lib}")
    if(NOT TARGET SDL3_mixer::SDL3_mixer-shared)
        add_library(SDL3_mixer::SDL3_mixer-shared SHARED IMPORTED)
        set_target_properties(SDL3_mixer::SDL3_mixer-shared
            PROPERTIES
                IMPORTED_LOCATION "${_sdl3mixer_lib}"
                INTERFACE_INCLUDE_DIRECTORIES "${_sdl3mixer_include_dirs}"
                COMPATIBLE_INTERFACE_BOOL "SDL3_SHARED"
                INTERFACE_SDL3_SHARED "ON"
                COMPATIBLE_INTERFACE_STRING "SDL_VERSION"
                INTERFACE_SDL_VERSION "SDL3"
        )
    endif()
    set(SDL3_mixer_SDL3_mixer-shared_FOUND TRUE)
endif()
unset(_sdl3mixer_include_dirs)
unset(_sdl3mixer_lib)

set(SDL3_mixer_SDL3_mixer-static_FOUND FALSE)

if(SDL3_mixer_SDL3_mixer-shared_FOUND)
    set(SDL3_mixer_SDL3_mixer_FOUND TRUE)
endif()

function(_sdl_create_target_alias_compat NEW_TARGET TARGET)
    if(CMAKE_VERSION VERSION_LESS "3.18")
        # Aliasing local targets is not supported on CMake < 3.18, so make it global.
        add_library(${NEW_TARGET} INTERFACE IMPORTED)
        set_target_properties(${NEW_TARGET} PROPERTIES INTERFACE_LINK_LIBRARIES "${TARGET}")
    else()
        add_library(${NEW_TARGET} ALIAS ${TARGET})
    endif()
endfunction()

# Make sure SDL3_mixer::SDL3_mixer always exists
if(NOT TARGET SDL3_mixer::SDL3_mixer)
    if(TARGET SDL3_mixer::SDL3_mixer-shared)
        _sdl_create_target_alias_compat(SDL3_mixer::SDL3_mixer SDL3_mixer::SDL3_mixer-shared)
    endif()
endif()

check_required_components(SDL3_mixer)
