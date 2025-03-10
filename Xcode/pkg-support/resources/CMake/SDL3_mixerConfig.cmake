# SDL3_mixer CMake configuration file:
# This file is meant to be placed in Resources/CMake of a SDL3_mixer framework

# INTERFACE_LINK_OPTIONS needs CMake 3.12
cmake_minimum_required(VERSION 3.12...3.28)

include(FeatureSummary)
set_package_properties(SDL3_mixer PROPERTIES
    URL "https://www.libsdl.org/projects/SDL_mixer/"
    DESCRIPTION "SDL_mixer is a sample multi-channel audio mixer library"
)

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

set(SDLMIXER_GME                   TRUE)

set(SDLMIXER_MOD                   FALSE)
set(SDLMIXER_MOD_XMP               TRUE)
set(SDLMIXER_MOD_XMP_LITE          TRUE)

set(SDLMIXER_MP3                   TRUE)
set(SDLMIXER_MP3_DRMP3             TRUE)
set(SDLMIXER_MP3_MPG123            FALSE)

set(SDLMIXER_MIDI                  TRUE)
set(SDLMIXER_MIDI_FLUIDSYNTH       FALSE)
set(SDLMIXER_MIDI_NATIVE           TRUE)
set(SDLMIXER_MIDI_TIMIDITY         FALSE)

set(SDLMIXER_OPUS                  TRUE)

set(SDLMIXER_VORBIS                STB)
set(SDLMIXER_VORBIS_STB            TRUE)
set(SDLMIXER_VORBIS_TREMOR         FALSE)
set(SDLMIXER_VORBIS_VORBISFILE     FALSE)

set(SDLMIXER_WAVE                  TRUE)


# Compute the installation prefix relative to this file.
set(_sdl3_mixer_framework_path "${CMAKE_CURRENT_LIST_DIR}")                                     # > /SDL3_mixer.framework/Resources/CMake/
get_filename_component(_sdl3_mixer_framework_path "${_sdl3_mixer_framework_path}" REALPATH)     # > /SDL3_mixer.framework/Versions/Current/Resources/CMake
get_filename_component(_sdl3_mixer_framework_path "${_sdl3_mixer_framework_path}" REALPATH)     # > /SDL3_mixer.framework/Versions/A/Resources/CMake/
get_filename_component(_sdl3_mixer_framework_path "${_sdl3_mixer_framework_path}" PATH)         # > /SDL3_mixer.framework/Versions/A/Resources/
get_filename_component(_sdl3_mixer_framework_path "${_sdl3_mixer_framework_path}" PATH)         # > /SDL3_mixer.framework/Versions/A/
get_filename_component(_sdl3_mixer_framework_path "${_sdl3_mixer_framework_path}" PATH)         # > /SDL3_mixer.framework/Versions/
get_filename_component(_sdl3_mixer_framework_path "${_sdl3_mixer_framework_path}" PATH)         # > /SDL3_mixer.framework/
get_filename_component(_sdl3_mixer_framework_parent_path "${_sdl3_mixer_framework_path}" PATH)  # > /

# All targets are created, even when some might not be requested though COMPONENTS.
# This is done for compatibility with CMake generated SDL3_mixer-target.cmake files.

if(NOT TARGET SDL3_mixer::SDL3_mixer-shared)
    add_library(SDL3_mixer::SDL3_mixer-shared SHARED IMPORTED)
    set_target_properties(SDL3_mixer::SDL3_mixer-shared
        PROPERTIES
            FRAMEWORK "TRUE"
            IMPORTED_LOCATION "${_sdl3_mixer_framework_path}/SDL3_mixer"
            COMPATIBLE_INTERFACE_BOOL "SDL3_SHARED"
            INTERFACE_SDL3_SHARED "ON"
            COMPATIBLE_INTERFACE_STRING "SDL_VERSION"
            INTERFACE_SDL_VERSION "SDL3"
    )
endif()
set(SDL3_mixer_SDL3_mixer-shared_FOUND TRUE)

set(SDL3_mixer_SDL3_mixer-static FALSE)

unset(_sdl3_mixer_framework_path)
unset(_sdl3_mixer_framework_parent_path)

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
