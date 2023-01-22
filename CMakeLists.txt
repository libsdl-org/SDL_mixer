cmake_minimum_required(VERSION 3.16)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# See docs/release_checklist.md
set(MAJOR_VERSION 3)
set(MINOR_VERSION 0)
set(MICRO_VERSION 0)
set(SDL_REQUIRED_VERSION 3.0.0)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the SDL_mixer source code and call cmake from there")
endif()

project(SDL3_mixer
    LANGUAGES C
    VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${MICRO_VERSION}"
)

include(PrivateSdlFunctions)
sdl_calculate_derived_version_variables(${MAJOR_VERSION} ${MINOR_VERSION} ${MICRO_VERSION})

message(STATUS "Configuring ${PROJECT_NAME} ${PROJECT_VERSION}")

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(SDL3MIXER_ROOTPROJECT ON)
else()
    set(SDL3MIXER_ROOTPROJECT OFF)
endif()

set(SDL3MIXER_SAMPLES_DEFAULT ${SDL3MIXER_ROOTPROJECT})
if(ANDROID)
    set(SDL3MIXER_SAMPLES_DEFAULT OFF)
endif()

if(POLICY CMP0112)
    # Target file component generator expressions do not add target dependencies.
    cmake_policy(SET CMP0112 NEW)
endif()

# Assume MSVC projects don't have a package manager and need vendored dependencies (by default).
# Most other platforms have some kind of package manager.
# FIXME: consider a package manager such as conan/vcpkg instead of vendoring
if(ANDROID OR MSVC)
    set(vendored_default ON)
else()
    set(vendored_default OFF)
endif()

include(CheckIncludeFile)
include(CheckSymbolExists)
include(CMakeDependentOption)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

option(CMAKE_POSITION_INDEPENDENT_CODE "Build static libraries with -fPIC" ON)
option(BUILD_SHARED_LIBS "Build the library as a shared library" ON)

option(SDL3MIXER_INSTALL "Enable SDL3mixer install target" ${SDL3MIXER_ROOTPROJECT})
option(SDL3MIXER_DEPS_SHARED "Default value for loading dependencies dynamically" ON)
option(SDL3MIXER_VENDORED "Use vendored third-party libraries" ${vendored_default})
option(SDL3MIXER_WERROR "Treat warnings as errors" OFF)

option(SDL3MIXER_SAMPLES "Build the SDL3_mixer sample program(s)" ${SDL3MIXER_SAMPLES_DEFAULT})
cmake_dependent_option(SDL3MIXER_SAMPLES_INSTALL "Install the SDL3_mixer sample program(s)" OFF "SDL3MIXER_SAMPLES;SDL3MIXER_INSTALL" OFF)

if(UNIX AND NOT APPLE)
    set(sdl3mixer_cmd_default ON)
else()
    set(sdl3mixer_cmd_default OFF)
endif()
option(SDL3MIXER_CMD "Support an external music player" ${sdl3mixer_cmd_default})

option(SDL3MIXER_FLAC "Enable FLAC music" ON)

cmake_dependent_option(SDL3MIXER_FLAC_LIBFLAC "Enable FLAC music using libFLAC" ON SDL3MIXER_FLAC OFF)
cmake_dependent_option(SDL3MIXER_FLAC_LIBFLAC_SHARED "Dynamically load LIBFLAC" "${SDL3MIXER_DEPS_SHARED}" SDL3MIXER_FLAC_LIBFLAC OFF)

cmake_dependent_option(SDL3MIXER_FLAC_DRFLAC "Enable FLAC music using drflac" ON SDL3MIXER_FLAC OFF)

option(SDL3MIXER_GME "Support loading GME music via game-music-emu" OFF)
option(SDL3MIXER_GME_SHARED "Dynamically load libgme" "${SDL3MIXER_DEPS_SHARED}")

option(SDL3MIXER_MOD "Support loading MOD music" ON)

cmake_dependent_option(SDL3MIXER_MOD_MODPLUG "Support loading MOD music via modplug" OFF SDL3MIXER_MOD OFF)
cmake_dependent_option(SDL3MIXER_MOD_MODPLUG_SHARED "Dynamically load modplug" "${SDL3MIXER_DEPS_SHARED}" SDL3MIXER_MOD_MODPLUG OFF)

cmake_dependent_option(SDL3MIXER_MOD_XMP "Support loading MOD music via libxmp" ON SDL3MIXER_MOD OFF)
cmake_dependent_option(SDL3MIXER_MOD_XMP_LITE "Use libxmp-lite instead of libxmp" OFF "SDL3MIXER_MOD_XMP;NOT SDL3MIXER_VENDORED" OFF)
cmake_dependent_option(SDL3MIXER_MOD_XMP_SHARED "Dynamically load libxmp(-lite)" "${SDL3MIXER_DEPS_SHARED}" SDL3MIXER_MOD_XMP OFF)

if(SDL3MIXER_MOD AND NOT (SDL3MIXER_MOD_MODPLUG OR SDL3MIXER_MOD_XMP))
    message(FATAL_EROR "MOD support was enabled (SDL3MIXER_MOD) but neither mpdplug (SDL3MIXER_MOD_MODPLUG) or xmp (SDL3MIXER_MOD_XMP) was enabled.")
endif()

option(SDL3MIXER_MP3 "Enable MP3 music" ON)

cmake_dependent_option(SDL3MIXER_MP3_DRMP3 "Support loading MP3 music via dr_mp3" ON SDL3MIXER_MP3 OFF)

cmake_dependent_option(SDL3MIXER_MP3_MPG123 "Support loading MP3 music via MPG123" OFF SDL3MIXER_MP3 OFF)
cmake_dependent_option(SDL3MIXER_MP3_MPG123_SHARED "Dynamically load mpg123" "${SDL3MIXER_DEPS_SHARED}" SDL3MIXER_MP3_MPG123 OFF)

if(SDL3MIXER_MP3 AND NOT (SDL3MIXER_MP3_DRMP3 OR SDL3MIXER_MP3_MPG123))
    message(FATAL_ERROR "MP3 support was enabled (SDL3MIXER_MP3) but neither drmp3 (SDL3MIXER_MP3_DRMP3) or mpg123 (SDL3MIXER_MP3_MPG123) were enabled.")
endif()

option(SDL3MIXER_MIDI "Enable MIDI music" ON)

cmake_dependent_option(SDL3MIXER_MIDI_FLUIDSYNTH "Support FluidSynth MIDI output" ON "SDL3MIXER_MIDI;NOT SDL3MIXER_VENDORED" OFF)
cmake_dependent_option(SDL3MIXER_MIDI_FLUIDSYNTH_SHARED "Dynamically load libfluidsynth" "${SDL3MIXER_DEPS_SHARED}" SDL3MIXER_MIDI_FLUIDSYNTH OFF)

if(WIN32 OR APPLE OR HAIKU)
    cmake_dependent_option(SDL3MIXER_MIDI_NATIVE "Support native MIDI output" ON SDL3MIXER_MIDI OFF)
else()
    set(SDL3MIXER_MIDI_NATIVE OFF)
endif()

cmake_dependent_option(SDL3MIXER_MIDI_TIMIDITY "Support timidity MIDI output" ON SDL3MIXER_MIDI OFF)

if(SDL3MIXER_MIDI AND NOT (SDL3MIXER_MIDI_TIMIDITY OR SDL3MIXER_MIDI_NATIVE OR SDL3MIXER_MIDI_FLUIDSYNTH))
    message(FATAL_ERROR "MIDI support was enabled (SDL3MIXER_MIDI) but neither FluidSynth (SDL3MIXER_MIDI_FLUIDSYNTH), native (SDL3MIXER_MIDI_NATIVE) or timidity (SDL3MIXER_MIDI_TIMIDITY) was enabled")
endif()

option(SDL3MIXER_OPUS "Enable Opus music" ON)
cmake_dependent_option(SDL3MIXER_OPUS_SHARED "Dynamically load libopus" "${SDL3MIXER_DEPS_SHARED}" SDL3MIXER_OPUS OFF)

set(sdl3mixer_vorbis_strings STB TREMOR VORBISFILE)
set(SDL3MIXER_VORBIS "STB" CACHE STRING "Enable OGG Vorbis music")
set_property(CACHE SDL3MIXER_VORBIS PROPERTY STRINGS "${sdl3mixer_vorbis_strings}")
if(SDL3MIXER_VORBIS)
    if(NOT SDL3MIXER_VORBIS IN_LIST sdl3mixer_vorbis_strings)
        message(FATAL_ERROR "SDL3MIXER_VORBIS contains an invalid value (=${SDL3MIXER_VORBIS}). It must be one of ${sdl3mixer_vorbis_strings}.")
    endif()
endif()
set(SDL3MIXER_VORBIS_STB OFF)
set(SDL3MIXER_VORBIS_TREMOR OFF)
set(SDL3MIXER_VORBIS_VORBISFILE OFF)
if(SDL3MIXER_VORBIS STREQUAL "STB")
    set(SDL3MIXER_VORBIS_STB ON)
endif()
if(SDL3MIXER_VORBIS STREQUAL "TREMOR")
    set(SDL3MIXER_VORBIS_TREMOR ON)
endif()
if(SDL3MIXER_VORBIS STREQUAL "VORBISFILE")
    set(SDL3MIXER_VORBIS_VORBISFILE ON)
endif()
cmake_dependent_option(SDL3MIXER_VORBIS_TREMOR_SHARED "Dynamically load tremor library" "${SDL3MIXER_DEPS_SHARED}" SDL3MIXER_VORBIS_TREMOR OFF)
cmake_dependent_option(SDL3MIXER_VORBIS_VORBISFILE_SHARED "Dynamically load vorbisfile library" "${SDL3MIXER_DEPS_SHARED}" SDL3MIXER_VORBIS_VORBISFILE OFF)

option(SDL3MIXER_WAVE "Enable streaming WAVE music" ON)

option(SDL3MIXER_WAVPACK "Enable WavPack music" ON)
cmake_dependent_option(SDL3MIXER_WAVPACK_SHARED "Dynamically load WavPack library" "${SDL3MIXER_DEPS_SHARED}" SDL3MIXER_WAVPACK OFF)

if(SDL3MIXER_VORBIS_TREMOR OR SDL3MIXER_VORBIS_VORBISFILE OR SDL3MIXER_FLAC_LIBFLAC OR SDL3MIXER_OPUS)
    set(SDL3MIXER_OGG TRUE)
    set(SDL3MIXER_OGG_install FALSE)
    if(SDL3MIXER_VORBIS_VORBISFILE_SHARED OR SDL3MIXER_FLAC_SHARED OR SDL3MIXER_OPUS_SHARED)
        set(SDL3MIXER_OGG_SHARED TRUE)
        set(SDL3MIXER_OGG_install TRUE)
    else()
        set(SDL3MIXER_OGG_SHARED FALSE)
        if(NOT SDL3MIXER_BUILD_SHARED_LIBS)
            set(SDL3MIXER_OGG_install TRUE)
        endif()
    endif()
else()
    set(SDL3MIXER_OGG FALSE)
endif()

# Save BUILD_SHARED_LIBS variable
set(SDL3MIXER_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

if(SDL3MIXER_BUILD_SHARED_LIBS)
    set(sdl3_mixer_export_name SDL3_mixer)
    set(sdl3_mixer_install_name_infix shared)
    set(sdl3_target_name SDL3::SDL3)
else()
    set(sdl3_mixer_export_name SDL3_mixer-static)
    set(sdl3_mixer_install_name_infix static)
    set(sdl3_target_name SDL3::SDL3-static)
endif()

if(NOT TARGET ${sdl3_target_name})
    find_package(SDL3 ${SDL_REQUIRED_VERSION} REQUIRED)
endif()

add_library(sdl3mixer_build_options INTERFACE)
if(MSVC)
    target_compile_options(sdl3mixer_build_options INTERFACE /W2)
else()
    target_compile_options(sdl3mixer_build_options INTERFACE -Wall -Wextra)
endif()
if(SDL3MIXER_WERROR)
    if(MSVC)
        target_compile_options(sdl3mixer_build_options INTERFACE /WX)
    else()
        target_compile_options(sdl3mixer_build_options INTERFACE -Werror)
    endif()
endif()

set(BUILD_SHARED_LIBS ${SDL3MIXER_BUILD_SHARED_LIBS})
add_library(SDL3_mixer
    src/codecs/load_aiff.c
    src/codecs/load_voc.c
    src/codecs/mp3utils.c
    src/codecs/music_cmd.c
    src/codecs/music_drflac.c
    src/codecs/music_drmp3.c
    src/codecs/music_flac.c
    src/codecs/music_fluidsynth.c
    src/codecs/music_gme.c
    src/codecs/music_modplug.c
    src/codecs/music_mpg123.c
    src/codecs/music_nativemidi.c
    src/codecs/music_ogg.c
    src/codecs/music_ogg_stb.c
    src/codecs/music_opus.c
    src/codecs/music_timidity.c
    src/codecs/music_wav.c
    src/codecs/music_wavpack.c
    src/codecs/music_xmp.c
    src/effect_position.c
    src/effect_stereoreverse.c
    src/effects_internal.c
    src/mixer.c
    src/music.c
    src/utils.c
)
add_library(SDL3_mixer::${sdl3_mixer_export_name} ALIAS SDL3_mixer)
target_include_directories(SDL3_mixer
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/SDL3>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/SDL3>"
    PRIVATE
        src
        src/codecs
)
target_compile_definitions(SDL3_mixer PRIVATE
    BUILD_SDL
    SDL_BUILD_MAJOR_VERSION=${MAJOR_VERSION}
    SDL_BUILD_MINOR_VERSION=${MINOR_VERSION}
    SDL_BUILD_MICRO_VERSION=${MICRO_VERSION}
)
target_link_libraries(SDL3_mixer PRIVATE $<BUILD_INTERFACE:${sdl3_target_name}>)
target_link_libraries(SDL3_mixer PRIVATE $<BUILD_INTERFACE:sdl3mixer_build_options>)
if(WIN32 AND BUILD_SHARED_LIBS)
    target_sources(SDL3_mixer PRIVATE
        version.rc
    )
endif()
set_target_properties(SDL3_mixer PROPERTIES
    DEFINE_SYMBOL DLL_EXPORT
    EXPORT_NAME ${sdl3_mixer_export_name}
    C_VISIBILITY_PRESET "hidden"
)
if(NOT ANDROID)
    if(APPLE)
        # the SOVERSION property corresponds to the compatibility version and VERSION corresponds to the current version
        # https://cmake.org/cmake/help/latest/prop_tgt/SOVERSION.html#mach-o-versions
        set_target_properties(SDL3_mixer PROPERTIES
            SOVERSION "${DYLIB_COMPAT_VERSION}"
            VERSION "${DYLIB_CURRENT_VERSION}"
        )
    else()
        set_target_properties(SDL3_mixer PROPERTIES
            SOVERSION "${SO_VERSION_MAJOR}"
            VERSION "${SO_VERSION}"
        )
    endif()
endif()
if(SDL3MIXER_BUILD_SHARED_LIBS)
    if(WIN32)
        set_target_properties(SDL3_mixer PROPERTIES
            PREFIX ""
        )
    endif()
else()
    if(MSVC)
        set_target_properties(SDL3_mixer PROPERTIES
            OUTPUT_NAME "SDL3_mixer-static"
        )
    endif()
endif()

# Use `Compatible Interface Properties` to ensure a shared SDL3_mixer is linked to a shared SDL3 library
if(SDL3MIXER_BUILD_SHARED_LIBS)
    set_property(TARGET SDL3_mixer PROPERTY INTERFACE_SDL3_SHARED ${SDL3MIXER_BUILD_SHARED_LIBS})
    set_property(TARGET SDL3_mixer APPEND PROPERTY COMPATIBLE_INTERFACE_BOOL SDL3_SHARED)
endif()

if(SDL3MIXER_BUILD_SHARED_LIBS)
    # Make sure static library dependencies are built with -fPIC when building a shared SDL3_mixer
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

set(INSTALL_EXTRA_TARGETS)
set(PC_LIBS)
set(PC_REQUIRES)

if(SDL3MIXER_CMD)
    target_compile_definitions(SDL3_mixer PRIVATE MUSIC_CMD)
    set(fork_found OFF)
    if(NOT fork_found)
        check_symbol_exists(fork unistd.h HAVE_FORK)
        if(HAVE_FORK)
            set(fork_found ON)
            target_compile_definitions(SDL3_mixer PRIVATE HAVE_FORK)
        endif()
    endif()
    if(NOT fork_found)
        check_symbol_exists(vfork unistd.h HAVE_VFORK)
        if(HAVE_VFORK)
            set(fork_found ON)
            target_compile_definitions(SDL3_mixer PRIVATE HAVE_VFORK)
        endif()
    endif()
    if(NOT fork_found)
        message(FATAL_ERROR "Neither fork() nor vfork() or available on this platform. Reconfigure with -DSDL3MIXER_CMD=OFF.")
    endif()
endif()

if(SDL3MIXER_OGG)
    # libogg is a requirement of libflac, libtremor and libvorbisfile, so only need this library when vendoring
    if(SDL3MIXER_VENDORED)
        message(STATUS "Using vendored libogg")
        set(BUILD_SHARED_LIBS ${SDL3MIXER_OGG_SHARED})
        set(INSTALL_CMAKE_PACKAGE_MODULE FALSE CACHE BOOL "Don't install cmake module for libogg")
        set(BUILD_TESTING OFF CACHE BOOL "Build tests")
        sdl_check_project_in_subfolder(external/ogg ogg SDL3MIXER_VENDORED)
        add_subdirectory(external/ogg EXCLUDE_FROM_ALL)
        if(SDL3MIXER_OGG_install)
            list(APPEND INSTALL_EXTRA_TARGETS ogg)
        endif()
    endif()
endif()

if(SDL3MIXER_OPUS)
    target_compile_definitions(SDL3_mixer PRIVATE MUSIC_OPUS)
    if(SDL3MIXER_VENDORED)
        # vendored libogg already handled
        if(NOT TARGET ogg)
            message(FATAL_ERROR "ogg target not present")
        endif()
        message(STATUS "Using vendored opus")
        set(BUILD_SHARED_LIBS ${SDL3MIXER_OPUS_SHARED})
        set(BUILD_PROGRAMS OFF CACHE BOOL "Build programs")
        sdl_check_project_in_subfolder(external/opus opus SDL3MIXER_VENDORED)
        add_subdirectory(external/opus EXCLUDE_FROM_ALL)

        set(OP_DISABLE_DOCS TRUE CACHE BOOL "Disable opusfile documentation")
        set(OP_DISABLE_EXAMPLES TRUE CACHE BOOL "Disable opusfile examples")
        set(OP_DISABLE_HTTP TRUE CACHE BOOL "Disable opusfile HTTP SUPPORT")
        message(STATUS "Using vendored opusfile")
        set(BUILD_SHARED_LIBS ${SDL3MIXER_OPUS_SHARED})
        sdl_check_project_in_subfolder(external/opusfile opusfile SDL3MIXER_VENDORED)
        add_subdirectory(external/opusfile EXCLUDE_FROM_ALL)
        if(MSVC)
            set_property(TARGET opusfile PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
        endif()

        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/opusfile.h" "#include \"${CMAKE_CURRENT_SOURCE_DIR}/external/opusfile/include/opusfile.h\"\n")
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/opus"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/opusfile.h" "${CMAKE_CURRENT_BINARY_DIR}/opus/opusfile.h"
        )
        target_include_directories(SDL3_mixer PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

        if(NOT TARGET OpusFile::opusfile)
            add_library(OpusFile::opusfile ALIAS opusfile)
        endif()
        if(SDL3MIXER_OPUS_SHARED OR NOT SDL3MIXER_BUILD_SHARED_LIBS)
            list(APPEND INSTALL_EXTRA_TARGETS opus opusfile)
        endif()
        if(NOT SDL3MIXER_OPUS_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:opusfile> -l$<TARGET_FILE_BASE_NAME:opus> -l$<TARGET_FILE_BASE_NAME:ogg>)
        endif()
    else()
        message(STATUS "Using system opusfile")
        find_package(OpusFile REQUIRED)
        if(NOT SDL3MIXER_OPUS_SHARED)
            list(APPEND PC_REQUIRES opusfile)
        endif()
    endif()
    if(SDL3MIXER_OPUS_SHARED)
        target_include_directories(SDL3_mixer PRIVATE
            $<TARGET_PROPERTY:OpusFile::opusfile,INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:OpusFile::opusfile,INTERFACE_INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:OpusFile::opusfile,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
        )
        target_get_dynamic_library(dynamic_opusfile OpusFile::opusfile)
        message(STATUS "Dynamic opus (opusfile): ${dynamic_opusfile}")
        target_compile_definitions(SDL3_mixer PRIVATE "OPUS_DYNAMIC=\"${dynamic_opusfile}\"")
        if(SDL3MIXER_VENDORED)
            add_dependencies(SDL3_mixer OpusFile::opusfile)
        endif()
    else()
        target_link_libraries(SDL3_mixer PRIVATE OpusFile::opusfile)
    endif()
endif()

if(SDL3MIXER_VORBIS_STB)
    message(STATUS "Enabled ogg music: using stb_vorbis")
    target_compile_definitions(SDL3_mixer PRIVATE MUSIC_OGG)
    target_compile_definitions(SDL3_mixer PRIVATE OGG_USE_STB)
endif()

if(SDL3MIXER_VORBIS_TREMOR)
    target_compile_definitions(SDL3_mixer PRIVATE MUSIC_OGG OGG_USE_TREMOR)
    if(SDL3MIXER_VENDORED)
        # vendored libogg already handled
        if(NOT TARGET ogg)
            message(FATAL_ERROR "ogg target not present")
        endif()
        message(STATUS "Using vendored tremor")
        set(BUILD_SHARED_LIBS ${SDL3MIXER_VORBIS_TREMOR_SHARED})
        sdl_check_project_in_subfolder(external/tremor tremor SDL3MIXER_VENDORED)
        add_subdirectory(external/tremor EXCLUDE_FROM_ALL)
        if(NOT TARGET tremor::tremor)
            add_library(tremor::tremor ALIAS vorbisidec)
        endif()

        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ivorbisfile.h" "#include \"${CMAKE_CURRENT_SOURCE_DIR}/external/tremor/ivorbisfile.h\"\n")
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/tremor"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/ivorbisfile.h" "${CMAKE_CURRENT_BINARY_DIR}/tremor/ivorbisfile.h"
        )
        target_include_directories(SDL3_mixer PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
        if(SDL3MIXER_VORBIS_TREMOR_SHARED OR NOT SDL3MIXER_BUILD_SHARED_LIBS)
            list(APPEND INSTALL_EXTRA_TARGETS vorbisidec)
        endif()
        if(NOT SDL3MIXER_VORBIS_TREMOR_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:vorbisidec> -l$<TARGET_FILE_BASE_NAME:ogg>)
        endif()
    else()
        message(STATUS "Using system tremor")
        find_package(tremor REQUIRED)
        if(NOT SDL3MIXER_VORBIS_TREMOR_SHARED)
            list(APPEND PC_REQUIRES tremor)
        endif()
    endif()
    if(SDL3MIXER_VORBIS_TREMOR_SHARED)
        target_include_directories(SDL3_mixer PRIVATE
            $<TARGET_PROPERTY:tremor::tremor,INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:tremor::tremor,INTERFACE_INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:tremor::tremor,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
        )
        target_get_dynamic_library(dynamic_tremor tremor::tremor)
        message(STATUS "Dynamic vorbis (tremor): ${dynamic_tremor}")
        target_compile_definitions(SDL3_mixer PRIVATE "OGG_DYNAMIC=\"${dynamic_tremor}\"")
        if(SDL3MIXER_VENDORED)
            add_dependencies(SDL3_mixer tremor::tremor)
        endif()
    else()
        target_link_libraries(SDL3_mixer PRIVATE tremor::tremor)
    endif()
endif()

if(SDL3MIXER_VORBIS_VORBISFILE)
    target_compile_definitions(SDL3_mixer PRIVATE MUSIC_OGG)
    if(SDL3MIXER_VENDORED)
        # vendored libogg already handled
        if(NOT TARGET ogg)
            message(FATAL_ERROR "ogg target not present")
        endif()
        message(STATUS "Using vendored vorbis + vorbisfile")
        set(BUILD_SHARED_LIBS ${SDL3MIXER_VORBIS_VORBISFILE_SHARED})
        sdl_check_project_in_subfolder(external/vorbis vorbisfile SDL3MIXER_VENDORED)
        add_subdirectory(external/vorbis EXCLUDE_FROM_ALL)
        if(NOT TARGET Vorbis::vorbisfile)
            add_library(Vorbis::vorbisfile ALIAS vorbisfile)
        endif()
        if(SDL3MIXER_VORBIS_VORBISFILE_SHARED OR NOT SDL3MIXER_BUILD_SHARED_LIBS)
            list(APPEND INSTALL_EXTRA_TARGETS vorbis vorbisfile)
        endif()
        if(NOT SDL3MIXER_VORBIS_VORBISFILE_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:vorbisfile>)
        endif()
    else()
        message(STATUS "Using system vorbisfile")
        find_package(Vorbis REQUIRED)
        if(NOT SDL3MIXER_VORBIS_VORBISFILE_SHARED)
            list(APPEND PC_REQUIRES vorbisfile)
        endif()
    endif()
    if(SDL3MIXER_VORBIS_VORBISFILE_SHARED)
        target_include_directories(SDL3_mixer PRIVATE
            $<TARGET_PROPERTY:Vorbis::vorbisfile,INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:Vorbis::vorbisfile,INTERFACE_INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:Vorbis::vorbisfile,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
        )
        target_get_dynamic_library(dynamic_vorbisfile Vorbis::vorbisfile)
        message(STATUS "Dynamic vorbisfile: ${dynamic_vorbisfile}")
        target_compile_definitions(SDL3_mixer PRIVATE "OGG_DYNAMIC=\"${dynamic_vorbisfile}\"")
        if(SDL3MIXER_VENDORED)
            add_dependencies(SDL3_mixer Vorbis::vorbisfile)
        endif()
    else()
        target_link_libraries(SDL3_mixer PRIVATE Vorbis::vorbisfile)
    endif()
endif()

if(SDL3MIXER_FLAC_LIBFLAC)
    target_compile_definitions(SDL3_mixer PRIVATE MUSIC_FLAC_LIBFLAC)
    if(SDL3MIXER_VENDORED)
        # vendored libogg already handled
        if(NOT TARGET ogg)
            message(FATAL_ERROR "ogg target not present")
        endif()
        set(BUILD_SHARED_LIBS "${SDL3MIXER_FLAC_LIBFLAC_SHARED}")
        set(INSTALL_CMAKE_CONFIG_MODULE OFF CACHE BOOL "Disable libflac installation")
        set(WITH_OGG OFF CACHE BOOL "Disable finding ogg using `find_package` (we have vendored it)")
        set(BUILD_CXXLIBS OFF CACHE BOOL "Build FLAC++")
        set(BUILD_EXAMPLES OFF CACHE BOOL "Build FLAC examples")
        set(BUILD_PROGRAMS OFF CACHE BOOL "Build programs")
        set(BUILD_TESTING OFF CACHE BOOL "Build tests")
        set(INSTALL_MANPAGES OFF CACHE BOOL "Install FLAC manpages")
        message(STATUS "Using vendored libflac")
        sdl_check_project_in_subfolder(external/flac libflac SDL3MIXER_VENDORED)
        add_subdirectory(external/flac EXCLUDE_FROM_ALL)
        if(SDL3MIXER_FLAC_LIBFLAC_SHARED)
            list(APPEND INSTALL_EXTRA_TARGETS FLAC)
        endif()
        if(NOT SDL3MIXER_FLAC_LIBFLAC_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:FLAC::FLAC> -l$<TARGET_FILE_BASE_NAME:ogg>)
        endif()
    else()
        message(STATUS "Using system libflac")
        find_package(FLAC REQUIRED)
        if(NOT SDL3MIXER_FLAC_LIBFLAC_SHARED)
            list(APPEND PC_REQUIRES flac)
        endif()
    endif()
    if(SDL3MIXER_FLAC_LIBFLAC_SHARED)
        target_include_directories(SDL3_mixer PRIVATE
            $<TARGET_PROPERTY:FLAC::FLAC,INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:FLAC::FLAC,INTERFACE_INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:FLAC::FLAC,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
        )
        target_get_dynamic_library(dynamic_flac FLAC::FLAC)
        message(STATUS "Dynamic libflac: ${dynamic_flac}")
        target_compile_definitions(SDL3_mixer PRIVATE "FLAC_DYNAMIC=\"${dynamic_flac}\"")
        if(SDL3MIXER_VENDORED)
            add_dependencies(SDL3_mixer FLAC)
        endif()
    else()
        target_link_libraries(SDL3_mixer PRIVATE FLAC::FLAC)
    endif()
endif()

if(SDL3MIXER_FLAC_DRFLAC)
    target_compile_definitions(SDL3_mixer PRIVATE MUSIC_FLAC_DRFLAC)
endif()

if(SDL3MIXER_GME)
    target_compile_definitions(SDL3_mixer PRIVATE MUSIC_GME)
    if(SDL3MIXER_VENDORED)
        set(BUILD_SHARED_LIBS "${SDL3MIXER_GME_SHARED}")
        set(ENABLE_UBSAN OFF CACHE BOOL "UB sanitizer")
        set(BUILD_FRAMEWORK OFF CACHE BOOL "macos framework")
        set(GME_ZLIB OFF CACHE BOOL "GME supports compressed formats")
        message(STATUS "Using vendored libgme")
        sdl_check_project_in_subfolder(external/libgme libgme SDL3MIXER_VENDORED)
        add_subdirectory(external/libgme EXCLUDE_FROM_ALL)
        add_library(gme::gme ALIAS gme)
        if(SDL3MIXER_GME_SHARED)
            list(APPEND INSTALL_EXTRA_TARGETS gme)
        endif()
        if(NOT SDL3MIXER_GME_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:gme>)
        endif()
    else()
        message(STATUS "Using system libgme")
        find_package(gme REQUIRED)
        if(NOT SDL3MIXER_GME_SHARED)
            list(APPEND PC_REQUIRES libgme)
        endif()
    endif()
    if(SDL3MIXER_GME_SHARED)
        target_include_directories(SDL3_mixer PRIVATE
            $<TARGET_PROPERTY:gme::gme,INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:gme::gme,INTERFACE_INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:gme::gme,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
        )
        target_get_dynamic_library(dynamic_gme gme::gme)
        message(STATUS "Dynamic libgme: ${dynamic_gme}")
        target_compile_definitions(SDL3_mixer PRIVATE "GME_DYNAMIC=\"${dynamic_gme}\"")
        if(SDL3MIXER_VENDORED)
            add_dependencies(SDL3_mixer gme::gme)
        endif()
    else()
        target_link_libraries(SDL3_mixer PRIVATE gme::gme)
    endif()
endif()

if(SDL3MIXER_MOD_MODPLUG)
    target_compile_definitions(SDL3_mixer PRIVATE MUSIC_MOD_MODPLUG)
    if(SDL3MIXER_VENDORED)
        message(STATUS "Using vendored libmodplug")
        message(FATAL_ERROR "libmodplug is not vendored.")
    else()
        message(STATUS "Using system libmodplug")
        find_package(modplug REQUIRED)
        if(NOT SDL3MIXER_MOD_MODPLUG_SHARED)
            list(APPEND PC_REQUIRES libmodplug)
        endif()
    endif()
    if(SDL3MIXER_MOD_MODPLUG_SHARED)
        target_include_directories(SDL3_mixer PRIVATE
            $<TARGET_PROPERTY:modplug::modplug,INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:modplug::modplug,INTERFACE_INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:modplug::modplug,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
        )
        target_get_dynamic_library(dynamic_modplug modplug::modplug)
        message(STATUS "Dynamic modplug: ${dynamic_modplug}")
        target_compile_definitions(SDL3_mixer PRIVATE "MODPLUG_DYNAMIC=\"${dynamic_modplug}\"")
        if(SDL3MIXER_VENDORED)
            add_dependencies(SDL3_mixer modplug::modplug)
        endif()
    else()
        target_link_libraries(SDL3_mixer PRIVATE modplug::modplug)
    endif()
endif()

if(SDL3MIXER_MOD_XMP)
    target_compile_definitions(SDL3_mixer PRIVATE MUSIC_MOD_XMP)
    if(SDL3MIXER_VENDORED)
        message(STATUS "Using vendored libxmp")
        sdl_check_project_in_subfolder(external/libxmp libxmp SDL3MIXER_VENDORED)
        if(SDL3MIXER_MOD_XMP_SHARED)
            set(BUILD_STATIC OFF)
            set(BUILD_SHARED ON)
            set(tgt_xmp xmp_shared)
        else()
            set(BUILD_STATIC ON)
            set(BUILD_SHARED OFF)
            set(tgt_xmp xmp_static)
        endif()
        set(xmp_name libxmp)
        add_subdirectory(external/libxmp EXCLUDE_FROM_ALL)
        if(SDL3MIXER_MOD_XMP_SHARED OR NOT SDL3MIXER_BUILD_SHARED_LIBS)
            list(APPEND INSTALL_EXTRA_TARGETS ${tgt_xmp})
        endif()
        if(NOT SDL3MIXER_MOD_XMP_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:${tgt_xmp}>)
        endif()
    else()
        if(SDL3MIXER_MOD_XMP_LITE)
            message(STATUS "Using system libxmp-lite")
            find_package(libxmp-lite REQUIRED)
            set(tgt_xmp libxmp-lite::libxmp-lite)
            set(xmp_name libxmp-lite)
            if(NOT SDL3MIXER_MOD_XMP_SHARED)
                list(APPEND PC_REQUIRES libxmplite)
            endif()
        else()
            message(STATUS "Using system libxmp")
            find_package(libxmp REQUIRED)
            set(tgt_xmp libxmp::libxmp)
            set(xmp_name libxmp)
            if(NOT SDL3MIXER_MOD_XMP_SHARED)
                list(APPEND PC_REQUIRES libxmp)
            endif()
        endif()
    endif()
    if(SDL3MIXER_MOD_XMP_SHARED)
        target_include_directories(SDL3_mixer PRIVATE
            $<TARGET_PROPERTY:${tgt_xmp},INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:${tgt_xmp},INTERFACE_INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:${tgt_xmp},INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
        )
        target_get_dynamic_library(dynamic_xmp ${tgt_xmp})
        message(STATUS "Dynamic ${xmp_name}: ${dynamic_xmp}")
        target_compile_definitions(SDL3_mixer PRIVATE "XMP_DYNAMIC=\"${dynamic_xmp}\"")
        if(SDL3MIXER_VENDORED)
            add_dependencies(SDL3_mixer ${tgt_xmp})
        endif()
    else()
        target_link_libraries(SDL3_mixer PRIVATE ${tgt_xmp})
    endif()
endif()

if(SDL3MIXER_MP3_DRMP3)
    target_compile_definitions(SDL3_mixer PRIVATE MUSIC_MP3_DRMP3)
endif()

if(SDL3MIXER_MP3_MPG123)
    target_compile_definitions(SDL3_mixer PRIVATE MUSIC_MP3_MPG123)
    if(SDL3MIXER_VENDORED)
        message(STATUS "Using vendored mpg123")
        sdl_check_project_in_subfolder(external/mpg123/ports/cmake mpg123 SDL3MIXER_VENDORED)
        set(BUILD_LIBOUT123 FALSE CACHE BOOL "Don't configure libout123")
        set(BUILD_PROGRAMS OFF CACHE BOOL "Build programs")
        set(BUILD_SHARED_LIBS "${SDL3MIXER_MP3_MPG123_SHARED}")
        add_subdirectory(external/mpg123/ports/cmake EXCLUDE_FROM_ALL)
        if(NOT TARGET MPG123::libmpg123)
            add_library(MPG123::libmpg123 ALIAS libmpg123)
        endif()
        if(SDL3MIXER_MP3_MPG123_SHARED OR NOT SDL3MIXER_BUILD_SHARED_LIBS)
            list(APPEND INSTALL_EXTRA_TARGETS libmpg123)
        endif()
        if(NOT SDL3MIXER_MP3_MPG123_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:MPG123::mpg123>)
        endif()
    else()
        message(STATUS "Using system mpg123")
        find_package(mpg123 REQUIRED)
        if(NOT SDL3MIXER_MP3_MPG123_SHARED)
            list(APPEND PC_REQUIRES libmpg123)
        endif()
    endif()
    if(SDL3MIXER_MP3_MPG123_SHARED)
        target_include_directories(SDL3_mixer PRIVATE
            $<TARGET_PROPERTY:MPG123::libmpg123,INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:MPG123::libmpg123,INTERFACE_INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:MPG123::libmpg123,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
        )
        target_get_dynamic_library(dynamic_mpg123 MPG123::libmpg123)
        message(STATUS "Dynamic mpg123}: ${dynamic_mpg123}")
        target_compile_definitions(SDL3_mixer PRIVATE "MPG123_DYNAMIC=\"${dynamic_mpg123}\"")
        if(SDL3MIXER_VENDORED)
            add_dependencies(SDL3_mixer MPG123::libmpg123)
        endif()
    else()
        target_link_libraries(SDL3_mixer PRIVATE MPG123::libmpg123)
    endif()
endif()

if(SDL3MIXER_MIDI_FLUIDSYNTH)
    target_compile_definitions(SDL3_mixer PRIVATE MUSIC_MID_FLUIDSYNTH)
    if(SDL3MIXER_VENDORED)
        message(STATUS "Using vendored FluidSynth")
        message(FATAL_ERROR "FluidSynth is not vendored.")
    else()
        message(STATUS "Using system FluidSynth")
        find_package(FluidSynth REQUIRED)
        if(NOT SDL3MIXER_MIDI_FLUIDSYNTH_SHARED)
            list(APPEND PC_REQUIRES fluidsynth)
        endif()
    endif()
    if(SDL3MIXER_MIDI_FLUIDSYNTH_SHARED)
        target_include_directories(SDL3_mixer PRIVATE
            $<TARGET_PROPERTY:FluidSynth::FluidSynth,INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:FluidSynth::FluidSynth,INTERFACE_INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:FluidSynth::FluidSynth,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
        )
        target_get_dynamic_library(dynamic_fluidsynth FluidSynth::FluidSynth)
        message(STATUS "Dynamic fluidsynth: ${dynamic_fluidsynth}")
        target_compile_definitions(SDL3_mixer PRIVATE "FLUIDSYNTH_DYNAMIC=\"${dynamic_fluidsynth}\"")
        if(SDL3MIXER_VENDORED)
            add_dependencies(SDL3_mixer FluidSynth::FluidSynth)
        endif()
    else()
        target_link_libraries(SDL3_mixer PRIVATE FluidSynth::FluidSynth)
    endif()
endif()

if(SDL3MIXER_MIDI_NATIVE)
    target_compile_definitions(SDL3_mixer PRIVATE MUSIC_MID_NATIVE)
    target_sources(SDL3_mixer PRIVATE
        src/codecs/native_midi/native_midi_common.c
        src/codecs/native_midi/native_midi_common.h
    )
    if(WIN32)
        target_sources(SDL3_mixer PRIVATE src/codecs/native_midi/native_midi_win32.c)
        target_link_libraries(SDL3_mixer PRIVATE winmm)
    elseif(APPLE)
        target_sources(SDL3_mixer PRIVATE src/codecs/native_midi/native_midi_macosx.c)
        target_link_libraries(SDL3_mixer PRIVATE -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit -Wl,-framework,CoreServices)
    elseif(HAIKU)
        enable_language(CXX)
        target_sources(SDL3_mixer PRIVATE src/codecs/native_midi/native_midi_haiku.cpp)
        target_link_libraries(SDL3_mixer PRIVATE midi)
    endif()
endif()

if(SDL3MIXER_MIDI_TIMIDITY)
    target_compile_definitions(SDL3_mixer PRIVATE MUSIC_MID_TIMIDITY)
    target_sources(SDL3_mixer PRIVATE
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

if(SDL3MIXER_WAVE)
    target_compile_definitions(SDL3_mixer PRIVATE MUSIC_WAV)
endif()

if(SDL3MIXER_WAVPACK)
    target_compile_definitions(SDL3_mixer PRIVATE MUSIC_WAVPACK)
    if(SDL3MIXER_VENDORED)
        message(STATUS "Using vendored WevPack")
        sdl_check_project_in_subfolder(external/wavpack WavPack SDL3MIXER_VENDORED)
        set(WAVPACK_BUILD_PROGRAMS FALSE CACHE BOOL "Don't build WavPack programs")
        set(WAVPACK_BUILD_COOLEDIT_PLUGIN OFF CACHE BOOL "Don't build WavPack CoolEdit plugin")
        set(WAVPACK_BUILD_WINAMP_PLUGIN OFF CACHE BOOL "Don't build WavPack WinAMP plugin")
        set(WAVPACK_BUILD_DOCS OFF CACHE BOOL "Don't build WavPack documentation")
        set(BUILD_SHARED_LIBS "${SDL3MIXER_WAVPACK_SHARED}")
        add_subdirectory(external/wavpack EXCLUDE_FROM_ALL)
        if(SDL3MIXER_WAVPACK_SHARED OR NOT SDL3MIXER_BUILD_SHARED_LIBS)
            list(APPEND INSTALL_EXTRA_TARGETS wavpack)
        endif()
        if(NOT SDL3MIXER_WAVPACK_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:WavPack::WavPack>)
        endif()
        target_compile_definitions(SDL3_mixer PRIVATE HAVE_WAVPACK_H)
    else()
        message(STATUS "Using system WavPack")
        find_package(wavpack REQUIRED)
        if(NOT SDL3MIXER_WAVPACK_SHARED)
            list(APPEND PC_REQUIRES wavpack)
        endif()
    endif()
    if(SDL3MIXER_WAVPACK_SHARED)
        target_include_directories(SDL3_mixer PRIVATE
            $<TARGET_PROPERTY:WavPack::WavPack,INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:WavPack::WavPack,INTERFACE_INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:WavPack::WavPack,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
        )
        target_get_dynamic_library(dynamic_wavpack WavPack::WavPack)
        message(STATUS "Dynamic WavPack: ${dynamic_wavpack}")
        target_compile_definitions(SDL3_mixer PRIVATE "WAVPACK_DYNAMIC=\"${dynamic_wavpack}\"")
        if(SDL3MIXER_VENDORED)
            add_dependencies(SDL3_mixer WavPack::WavPack)
        endif()
    else()
        target_link_libraries(SDL3_mixer PRIVATE WavPack::WavPack)
    endif()
endif()

# Restore BUILD_SHARED_LIBS
set(BUILD_SHARED_LIBS ${SDL3MIXER_BUILD_SHARED_LIBS})

if(SDL3MIXER_INSTALL)
    install(
        TARGETS SDL3_mixer
        EXPORT SDL3MixerTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT devel
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT library
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT library
    )
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/include/SDL3/SDL_mixer.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/SDL3" COMPONENT devel
    )

    if(INSTALL_EXTRA_TARGETS)
        install(TARGETS ${INSTALL_EXTRA_TARGETS}
            EXPORT SDL3MixerTargets
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT devel
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT library
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT library
            PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" COMPONENT devel
        )
    endif()

    if(WIN32 AND NOT MINGW)
        set(SDL3MIXER_INSTALL_CMAKEDIR_ROOT_DEFAULT "cmake")
    else()
        set(SDL3MIXER_INSTALL_CMAKEDIR_ROOT_DEFAULT "${CMAKE_INSTALL_LIBDIR}/cmake")
    endif()
    set(SDL3MIXER_INSTALL_CMAKEDIR_ROOT "${SDL3MIXER_INSTALL_CMAKEDIR_ROOT_DEFAULT}" CACHE STRING "Root folder where to install SDL3Config.cmake related files (SDL3 subfolder for non-MSVC projects)")

    if(WIN32 AND NOT MINGW)
        set(SDL3MIXER_INSTALL_CMAKEDIR "${SDL3MIXER_INSTALL_CMAKEDIR_ROOT}")
        set(LICENSES_PREFIX "licenses/SDL3_mixer")
    else()
        set(SDL3MIXER_INSTALL_CMAKEDIR "${SDL3MIXER_INSTALL_CMAKEDIR_ROOT}/SDL3_mixer")
        set(LICENSES_PREFIX "${CMAKE_INSTALL_DATAROOTDIR}/licenses/SDL3_mixer")
    endif()

    configure_package_config_file(cmake/SDL3_mixerConfig.cmake.in SDL3_mixerConfig.cmake
        INSTALL_DESTINATION "${SDL3MIXER_INSTALL_CMAKEDIR}"
    )
    write_basic_package_version_file("${PROJECT_BINARY_DIR}/SDL3_mixerConfigVersion.cmake"
        COMPATIBILITY AnyNewerVersion
    )
    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/SDL3_mixerConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/SDL3_mixerConfigVersion.cmake"
            cmake/FindFLAC.cmake
            cmake/FindFluidSynth.cmake
            cmake/Findgme.cmake
            cmake/Findlibxmp.cmake
            cmake/Findlibxmp-lite.cmake
            cmake/Findmodplug.cmake
            cmake/FindOpusFile.cmake
            cmake/Findmpg123.cmake
            cmake/FindVorbis.cmake
            cmake/Findtremor.cmake
            cmake/Findwavpack.cmake
        DESTINATION "${SDL3MIXER_INSTALL_CMAKEDIR}"
        COMPONENT devel
    )
    install(EXPORT SDL3MixerTargets
        FILE SDL3_mixer-${sdl3_mixer_install_name_infix}-targets.cmake
        NAMESPACE SDL3_mixer::
        DESTINATION "${SDL3MIXER_INSTALL_CMAKEDIR}"
        COMPONENT devel
    )

    string(JOIN " " PC_REQUIRES ${PC_REQUIRES})
    string(JOIN " " PC_LIBS ${PC_LIBS})
    configure_file(cmake/sdl3-mixer.pc.in sdl3-mixer.pc @ONLY)

    if(CMAKE_SYSTEM_NAME MATCHES FreeBSD)
        # FreeBSD uses ${PREFIX}/libdata/pkgconfig
        set(PC_DESTDIR "libdata/pkgconfig")
    else()
        set(PC_DESTDIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
    endif()
    # Always install sdl3-mixer.pc file: libraries might be different between config modes
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sdl3-mixer.pc"
        DESTINATION "${PC_DESTDIR}" COMPONENT devel)

    install(FILES "LICENSE.txt"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/licenses/${PROJECT_NAME}"
        COMPONENT library
    )
endif()

if(SDL3MIXER_SAMPLES)
    check_include_file("signal.h" HAVE_SIGNAL_H)
    check_symbol_exists("setbuf" "stdio.h" HAVE_SETBUF)

    add_executable(playmus playmus.c)
    add_executable(playwave playwave.c)

    foreach(prog playmus playwave)
        target_link_libraries(${prog} PRIVATE SDL3_mixer::${sdl3_mixer_export_name})
        target_link_libraries(${prog} PRIVATE ${sdl3_target_name})
        if(HAVE_SIGNAL_H)
            target_compile_definitions(${prog} PRIVATE HAVE_SIGNAL_H)
        endif()
        if(HAVE_SETBUF)
            target_compile_definitions(${prog} PRIVATE HAVE_SETBUF)
        endif()

        if(SDL3MIXER_SAMPLES_INSTALL)
            install(TARGETS ${prog}
                RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            )
        endif()
    endforeach()
endif()
