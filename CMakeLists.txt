cmake_minimum_required(VERSION 3.16)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# See docs/release_checklist.md
set(MAJOR_VERSION 2)
set(MINOR_VERSION 6)
set(MICRO_VERSION 2)
set(SDL_REQUIRED_VERSION 2.0.9)

# For historical reasons this is 3.0.0 rather than the expected 1.0.0
set(DYLIB_COMPATIBILITY_VERSION "3.0.0")

include(PrivateSdlFunctions)
sdl_calculate_derived_version_variables()

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the SDL_mixer source code and call cmake from there")
endif()

project(SDL2_mixer
    LANGUAGES C
    VERSION "${FULL_VERSION}"
)

message(STATUS "Configuring ${PROJECT_NAME} ${PROJECT_VERSION}")

if(POLICY CMP0112)
    # Target file component generator expressions do not add target dependencies.
    cmake_policy(SET CMP0112 NEW)
endif()

# Set defaults preventing destination file conflicts
set(SDL2MIXER_DEBUG_POSTFIX "d"
    CACHE STRING "Name suffix for debug builds")
mark_as_advanced(SDL2MIXER_DEBUG_POSTFIX)

# Assume MSVC projects don't have a package manager and need vendored dependencies (by default).
# Most other platforms have some kind of package manager.
# FIXME: consider a package manager such as conan/vcpkg instead of vendoring
if(MSVC)
    set(vendored_default ON)
else()
    set(vendored_default OFF)
endif()

include(CheckSymbolExists)
include(CMakeDependentOption)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

option(CMAKE_POSITION_INDEPENDENT_CODE "Build static libraries with -fPIC" ON)
option(BUILD_SHARED_LIBS "Build the library as a shared library" ON)

# Save BUILD_SHARED_LIBS variable as soon as possible
set(SDL2MIXER_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

option(SDL2MIXER_INSTALL "Enable SDL2mixer install target" ON)
option(SDL2MIXER_DEPS_SHARED "Default value for loading dependencies dynamically" ON)
option(SDL2MIXER_VENDORED "Use vendored third-party libraries" ${vendored_default})

option(SDL2MIXER_SAMPLES "Build the SDL2_mixer sample program(s)" ON)
cmake_dependent_option(SDL2MIXER_SAMPLES_INSTALL "Install the SDL2_mixer sample program(s)" OFF "SDL2MIXER_SAMPLES;SDL2MIXER_INSTALL" OFF)

if(UNIX AND NOT APPLE)
    set(sdl2mixer_cmd_default ON)
else()
    set(sdl2mixer_cmd_default OFF)
endif()
option(SDL2MIXER_CMD "Support an external music player" ${sdl2mixer_cmd_default})

option(SDL2MIXER_FLAC "Enable FLAC music" ON)

cmake_dependent_option(SDL2MIXER_FLAC_LIBFLAC "Enable FLAC music using libFLAC" ON SDL2MIXER_FLAC OFF)
cmake_dependent_option(SDL2MIXER_FLAC_LIBFLAC_SHARED "Dynamically load LIBFLAC" "${SDL2MIXER_DEPS_SHARED}" SDL2MIXER_FLAC_LIBFLAC OFF)

cmake_dependent_option(SDL2MIXER_FLAC_DRFLAC "Enable FLAC music using drflac" ON SDL2MIXER_FLAC OFF)

option(SDL2MIXER_MOD "Support loading MOD music" ON)

cmake_dependent_option(SDL2MIXER_MOD_MODPLUG "Support loading MOD music via modplug" ON SDL2MIXER_MOD OFF)
cmake_dependent_option(SDL2MIXER_MOD_MODPLUG_SHARED "Dynamically load modplug" "${SDL2MIXER_DEPS_SHARED}" SDL2MIXER_MOD_MODPLUG OFF)

cmake_dependent_option(SDL2MIXER_MOD_XMP "Support loading MOD music via libxmp" OFF SDL2MIXER_MOD OFF)
cmake_dependent_option(SDL2MIXER_MOD_XMP_LITE "Use libxmp-lite instead of libxmp" OFF SDL2MIXER_MOD_XMP OFF)
cmake_dependent_option(SDL2MIXER_MOD_XMP_SHARED "Dynamically load libxmp(-lite)" "${SDL2MIXER_DEPS_SHARED}" SDL2MIXER_MOD_XMP OFF)

if(SDL2MIXER_MOD AND NOT (SDL2MIXER_MOD_MODPLUG OR SDL2MIXER_MOD_XMP))
    message(FATAL_EROR "MOD support was enabled (SDL2MIXER_MOD) but neither mpdplug (SDL2MIXER_MOD_MODPLUG) or xmp (SDL2MIXER_MOD_XMP) was enabled.")
endif()

option(SDL2MIXER_MP3 "Enable MP3 music" ON)

cmake_dependent_option(SDL2MIXER_MP3_DRMP3 "Support loading MP3 music via dr_mp3" ON SDL2MIXER_MP3 OFF)

cmake_dependent_option(SDL2MIXER_MP3_MPG123 "Support loading MP3 music via MPG123" OFF SDL2MIXER_MP3 OFF)
cmake_dependent_option(SDL2MIXER_MP3_MPG123_SHARED "Dynamically load mpg123" "${SDL2MIXER_DEPS_SHARED}" SDL2MIXER_MP3_MPG123 OFF)

if(SDL2MIXER_MP3 AND NOT (SDL2MIXER_MP3_DRMP3 OR SDL2MIXER_MP3_MPG123))
    message(FATAL_ERROR "MP3 support was enabled (SDL2MIXER_MP3) but neither drmp3 (SDL2MIXER_MP3_DRMP3) or mpg123 (SDL2MIXER_MP3_MPG123) were enabled.")
endif()

option(SDL2MIXER_MIDI "Enable MIDI music" ON)

cmake_dependent_option(SDL2MIXER_MIDI_FLUIDSYNTH "Support FluidSynth MIDI output" ON "SDL2MIXER_MIDI;NOT SDL2MIXER_VENDORED" OFF)
cmake_dependent_option(SDL2MIXER_MIDI_FLUIDSYNTH_SHARED "Dynamically load libfluidsynth" "${SDL2MIXER_DEPS_SHARED}" SDL2MIXER_MIDI_FLUIDSYNTH OFF)

if(WIN32 OR APPLE OR HAIKU)
    cmake_dependent_option(SDL2MIXER_MIDI_NATIVE "Support native MIDI output" ON SDL2MIXER_MIDI OFF)
else()
    set(SDL2MIXER_MIDI_NATIVE OFF)
endif()

cmake_dependent_option(SDL2MIXER_MIDI_TIMIDITY "Support timidity MIDI output" ON SDL2MIXER_MIDI OFF)

if(SDL2MIXER_MIDI AND NOT (SDL2MIXER_MIDI_TIMIDITY OR SDL2MIXER_MIDI_NATIVE OR SDL2MIXER_MIDI_FLUIDSYNTH))
    message(FATAL_ERROR "MIDI support was enabled (SDL2MIXER_MIDI) but neither FluidSynth (SDL2MIXER_MIDI_FLUIDSYNTH), native (SDL2MIXER_MIDI_NATIVE) or timidity (SDL2MIXER_MIDI_TIMIDITY) was enabled")
endif()

option(SDL2MIXER_OPUS "Enable Opus music" ON)
cmake_dependent_option(SDL2MIXER_OPUS_SHARED "Dynamically load libopus" ON SDL2MIXER_OPUS OFF)

set(sdl2mixer_vorbis_strings STB TREMOR VORBISFILE)
set(SDL2MIXER_VORBIS "STB" CACHE STRING "Enable OGG Vorbis music")
set_property(CACHE SDL2MIXER_VORBIS PROPERTY STRINGS "${sdl2mixer_vorbis_strings}")
if(SDL2MIXER_VORBIS)
    if(NOT SDL2MIXER_VORBIS IN_LIST sdl2mixer_vorbis_strings)
        message(FATAL_ERROR "SDL2MIXER_VORBIS contains an invalid value (=${SDL2MIXER_VORBIS}). It must be one of ${sdl2mixer_vorbis_strings}.")
    endif()
endif()
set(SDL2MIXER_VORBIS_STB OFF)
set(SDL2MIXER_VORBIS_TREMOR OFF)
set(SDL2MIXER_VORBIS_VORBISFILE OFF)
if(SDL2MIXER_VORBIS STREQUAL "STB")
    set(SDL2MIXER_VORBIS_STB ON)
endif()
if(SDL2MIXER_VORBIS STREQUAL "TREMOR")
    set(SDL2MIXER_VORBIS_TREMOR ON)
endif()
if(SDL2MIXER_VORBIS STREQUAL "VORBISFILE")
    set(SDL2MIXER_VORBIS_VORBISFILE ON)
endif()
cmake_dependent_option(SDL2MIXER_VORBIS_TREMOR_SHARED "Dynamically load tremor library" ON SDL2MIXER_VORBIS_TREMOR OFF)
cmake_dependent_option(SDL2MIXER_VORBIS_VORBISFILE_SHARED "Dynamically load vorbisfile library" ON SDL2MIXER_VORBIS_VORBISFILE OFF)

option(SDL2MIXER_WAVE "Enable streaming WAVE music" ON)

if(SDL2MIXER_VORBIS_TREMOR OR SDL2MIXER_VORBIS_VORBISFILE OR SDL2MIXER_FLAC_LIBFLAC OR SDL2MIXER_OPUS)
    set(SDL2MIXER_OGG TRUE)
    set(SDL2MIXER_OGG_install FALSE)
    if(SDL2MIXER_VORBIS_VORBISFILE_SHARED OR SDL2MIXER_FLAC_SHARED OR SDL2MIXER_OPUS_SHARED)
        set(SDL2MIXER_OGG_SHARED TRUE)
        set(SDL2MIXER_OGG_install TRUE)
    else()
        set(SDL2MIXER_OGG_SHARED FALSE)
        if(NOT SDL2MIXER_BUILD_SHARED_LIBS)
            set(SDL2MIXER_OGG_install TRUE)
        endif()
    endif()
else()
    set(SDL2MIXER_OGG FALSE)
endif()

if(SDL2MIXER_BUILD_SHARED_LIBS)
    set(sdl2_mixer_export_name SDL2_mixer)
    set(sdl2_mixer_install_name_infix shared)
    set(sdl2_target_name SDL2::SDL2)
else()
    set(sdl2_mixer_export_name SDL2_mixer-static)
    set(sdl2_mixer_install_name_infix static)
    set(sdl2_target_name SDL2::SDL2-static)
endif()

sdl_find_sdl2(${sdl2_target_name} ${SDL_REQUIRED_VERSION})

set(BUILD_SHARED_LIBS ${SDL2MIXER_BUILD_SHARED_LIBS})
add_library(SDL2_mixer
    src/codecs/load_aiff.c
    src/codecs/load_voc.c
    src/codecs/mp3utils.c
    src/codecs/music_cmd.c
    src/codecs/music_drflac.c
    src/codecs/music_drmp3.c
    src/codecs/music_flac.c
    src/codecs/music_fluidsynth.c
    src/codecs/music_modplug.c
    src/codecs/music_mpg123.c
    src/codecs/music_nativemidi.c
    src/codecs/music_ogg.c
    src/codecs/music_ogg_stb.c
    src/codecs/music_opus.c
    src/codecs/music_timidity.c
    src/codecs/music_wav.c
    src/codecs/music_xmp.c
    src/effect_position.c
    src/effect_stereoreverse.c
    src/effects_internal.c
    src/mixer.c
    src/music.c
    src/utils.c
)
add_library(SDL2_mixer::${sdl2_mixer_export_name} ALIAS SDL2_mixer)
target_include_directories(SDL2_mixer
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/SDL2>"
    PRIVATE
        src
        src/codecs
)
target_compile_definitions(SDL2_mixer PRIVATE
    BUILD_SDL
    SDL_BUILD_MAJOR_VERSION=${MAJOR_VERSION}
    SDL_BUILD_MINOR_VERSION=${MINOR_VERSION}
    SDL_BUILD_MICRO_VERSION=${MICRO_VERSION}
)
target_link_libraries(SDL2_mixer PRIVATE $<BUILD_INTERFACE:${sdl2_target_name}>)
if(WIN32 AND BUILD_SHARED_LIBS)
    target_sources(SDL2_mixer PRIVATE
        version.rc
    )
endif()
set_target_properties(SDL2_mixer PROPERTIES
    DEFINE_SYMBOL DLL_EXPORT
    PUBLIC_HEADER "include/SDL_mixer.h"
    EXPORT_NAME ${sdl2_mixer_export_name}
    C_VISIBILITY_PRESET "hidden"
)
if(NOT ANDROID)
    set_target_properties(SDL2_mixer PROPERTIES
        DEBUG_POSTFIX "${SDL2MIXER_DEBUG_POSTFIX}"
    )
    if(APPLE)
        # the SOVERSION property corresponds to the compatibility version and VERSION corresponds to the current version
        # https://cmake.org/cmake/help/latest/prop_tgt/SOVERSION.html#mach-o-versions
        set_target_properties(SDL2_mixer PROPERTIES
            SOVERSION "${DYLIB_COMPATIBILITY_VERSION}"
            VERSION "${DYLIB_CURRENT_VERSION}"
        )
    else()
        set_target_properties(SDL2_mixer PROPERTIES
            SOVERSION "${LT_MAJOR}"
            VERSION "${LT_VERSION}"
        )
    endif()
endif()
if(SDL2MIXER_BUILD_SHARED_LIBS AND (APPLE OR (UNIX AND NOT ANDROID)))
    add_custom_command(TARGET SDL2_mixer POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E create_symlink "$<TARGET_SONAME_FILE_NAME:SDL2_mixer>" "libSDL2_mixer$<$<CONFIG:Debug>:${SDL2MIXER_DEBUG_POSTFIX}>$<TARGET_FILE_SUFFIX:SDL2_mixer>"
        # BYPRODUCTS "libSDL2_mixer$<$<CONFIG:Debug>:${SDL2MIXER_DEBUG_POSTFIX}>$<TARGET_FILE_SUFFIX:SDL2_mixer>" # Needs CMake 3.20
        WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
    )
endif()
if(SDL2MIXER_BUILD_SHARED_LIBS)
    if(WIN32 OR OS2)
        set_target_properties(SDL2_mixer PROPERTIES
            PREFIX ""
        )
    endif()
    if(OS2)
        # OS/2 doesn't support a DLL name longer than 8 characters.
        set_target_properties(SDL2_mixer PROPERTIES
            OUTPUT_NAME "SDL2mix"
        )
    elseif(UNIX AND NOT ANDROID)
        set_target_properties(SDL2_mixer PROPERTIES
            OUTPUT_NAME "SDL2_mixer-${LT_RELEASE}"
        )
    endif()
else()
    if(MSVC OR (WATCOM AND (WIN32 OR OS2)))
        set_target_properties(SDL2_mixer PROPERTIES
            OUTPUT_NAME "SDL2_mixer-static"
        )
    endif()
endif()

if(SDL2MIXER_BUILD_SHARED_LIBS)
    # Use `Compatible Interface Properties` to ensure a shared SDL2_mixer is linked to a shared SDL2 library
    set_property(TARGET SDL2_mixer PROPERTY INTERFACE_SDL2_SHARED ${SDL2MIXER_BUILD_SHARED_LIBS})
    set_property(TARGET SDL2_mixer APPEND PROPERTY COMPATIBLE_INTERFACE_BOOL SDL2_SHARED)
endif()

if(SDL2MIXER_BUILD_SHARED_LIBS)
    # Make sure static library dependencies are built with -fPIC when building a shared SDL2_mixer
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

set(INSTALL_EXTRA_TARGETS)
set(PC_LIBS)
set(PC_REQUIRES)

if(SDL2MIXER_CMD)
    target_compile_definitions(SDL2_mixer PRIVATE MUSIC_CMD)
    set(fork_found OFF)
    if(NOT fork_found)
        check_symbol_exists(fork sys/unistd.h HAVE_FORK)
        if(HAVE_FORK)
            set(fork_found ON)
            target_compile_definitions(SDL2_mixer PRIVATE HAVE_FORK)
        endif()
    endif()
    if(NOT fork_found)
        check_symbol_exists(fork sys/unistd.h HAVE_VFORK)
        if(HAVE_VFORK)
            set(fork_found ON)
            target_compile_definitions(SDL2_mixer PRIVATE HAVE_VFORK)
        endif()
    endif()
    if(NOT fork_found)
        message(FATAL_ERROR "Neither fork() or vfork() or available on this platform. Reconfigure with -DSDL2MIXER_CMD=OFF.")
    endif()
endif()

if(SDL2MIXER_OGG)
    # libogg is a requirement of libflac, libtremor and libvorbisfile, so only need this library when vendoring
    if(SDL2MIXER_VENDORED)
        message(STATUS "Using vendored libogg")
        set(BUILD_SHARED_LIBS ${SDL2MIXER_OGG_SHARED})
        set(INSTALL_CMAKE_PACKAGE_MODULE FALSE CACHE BOOL "Don't install cmake module for libogg")
        sdl_check_project_in_subfolder(external/ogg ogg SDL2MIXER_VENDORED)
        add_subdirectory(external/ogg EXCLUDE_FROM_ALL)
        if(SDL2MIXER_OGG_install)
            list(APPEND INSTALL_EXTRA_TARGETS ogg)
        endif()
    endif()
endif()

if(SDL2MIXER_OPUS)
    target_compile_definitions(SDL2_mixer PRIVATE MUSIC_OPUS)
    if(SDL2MIXER_VENDORED)
        # vendored libogg alread handled
        if(NOT TARGET ogg)
            message(FATAL_ERROR "ogg target not present")
        endif()
        message(STATUS "Using vendored opus")
        set(BUILD_SHARED_LIBS ${SDL2MIXER_OPUS_SHARED})
        sdl_check_project_in_subfolder(external/opus opus SDL2MIXER_VENDORED)
        add_subdirectory(external/opus EXCLUDE_FROM_ALL)

        set(OP_DISABLE_DOCS TRUE CACHE BOOL "Disable opusfile documentation")
        set(OP_DISABLE_HTTP TRUE CACHE BOOL "Disable opusfile HTTP SUPPORT")
        message(STATUS "Using vendored opusfile")
        set(BUILD_SHARED_LIBS ${SDL2MIXER_OPUS_SHARED})
        sdl_check_project_in_subfolder(external/opusfile opusfile SDL2MIXER_VENDORED)
        add_subdirectory(external/opusfile EXCLUDE_FROM_ALL)
        if(MSVC)
            set_property(TARGET opusfile PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
        endif()

        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/opusfile.h" "#include \"${CMAKE_CURRENT_SOURCE_DIR}/external/opusfile/include/opusfile.h\"\n")
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/opus"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/opusfile.h" "${CMAKE_CURRENT_BINARY_DIR}/opus/opusfile.h"
        )
        target_include_directories(SDL2_mixer PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

        if(NOT TARGET opusfile::opusfile)
            add_library(opusfile::opusfile ALIAS opusfile)
        endif()
        if(SDL2MIXER_OPUS_SHARED OR NOT SDL2MIXER_BUILD_SHARED_LIBS)
            list(APPEND INSTALL_EXTRA_TARGETS opus opusfile)
        endif()
        if(NOT SDL2MIXER_OPUS_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:opusfile> -l$<TARGET_FILE_BASE_NAME:opus> -l$<TARGET_FILE_BASE_NAME:ogg>)
        endif()
    else()
        message(STATUS "Using system opusfile")
        find_package(opusfile REQUIRED)
        if(NOT SDL2MIXER_OPUS_SHARED)
            list(APPEND PC_REQUIRES opusfile)
        endif()
    endif()
    if(SDL2MIXER_OPUS_SHARED)
        target_include_directories(SDL2_mixer PRIVATE
            $<TARGET_PROPERTY:opusfile::opusfile,INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:opusfile::opusfile,INTERFACE_INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:opusfile::opusfile,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
        )
        target_get_dynamic_library(dynamic_opusfile opusfile::opusfile)
        message(STATUS "Dynamic opus (opusfile): ${dynamic_opusfile}")
        target_compile_definitions(SDL2_mixer PRIVATE "OPUS_DYNAMIC=\"${dynamic_opusfile}\"")
        if(SDL2MIXER_VENDORED)
            add_dependencies(SDL2_mixer opusfile::opusfile)
        endif()
    else()
        target_link_libraries(SDL2_mixer PRIVATE opusfile::opusfile)
    endif()
endif()

if(SDL2MIXER_VORBIS_STB)
    message(STATUS "Enabled ogg music: using stb_vorbis")
    target_compile_definitions(SDL2_mixer PRIVATE MUSIC_OGG)
    target_compile_definitions(SDL2_mixer PRIVATE OGG_USE_STB)
endif()

if(SDL2MIXER_VORBIS_TREMOR)
    target_compile_definitions(SDL2_mixer PRIVATE MUSIC_OGG OGG_USE_TREMOR)
    if(SDL2MIXER_VENDORED)
        # vendored libogg alread handled
        if(NOT TARGET ogg)
            message(FATAL_ERROR "ogg target not present")
        endif()
        message(STATUS "Using vendored tremor")
        set(BUILD_SHARED_LIBS ${SDL2MIXER_VORBIS_TREMOR_SHARED})
        sdl_check_project_in_subfolder(external/tremor tremor SDL2MIXER_VENDORED)
        add_subdirectory(external/tremor EXCLUDE_FROM_ALL)
        if(NOT TARGET tremor::tremor)
            add_library(tremor::tremor ALIAS vorbisidec)
        endif()

        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ivorbisfile.h" "#include \"${CMAKE_CURRENT_SOURCE_DIR}/external/tremor/ivorbisfile.h\"\n")
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/tremor"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/ivorbisfile.h" "${CMAKE_CURRENT_BINARY_DIR}/tremor/ivorbisfile.h"
        )
        target_include_directories(SDL2_mixer PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
        if(SDL2MIXER_VORBIS_TREMOR_SHARED OR NOT SDL2MIXER_BUILD_SHARED_LIBS)
            list(APPEND INSTALL_EXTRA_TARGETS vorbisidec)
        endif()
        if(NOT SDL2MIXER_VORBIS_TREMOR_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:vorbisidec> -l$<TARGET_FILE_BASE_NAME:ogg>)
        endif()
    else()
        message(STATUS "Using system tremor")
        find_package(tremor REQUIRED)
        if(NOT SDL2MIXER_VORBIS_TREMOR_SHARED)
            list(APPEND PC_REQUIRES tremor)
        endif()
    endif()
    if(SDL2MIXER_VORBIS_TREMOR_SHARED)
        target_include_directories(SDL2_mixer PRIVATE
            $<TARGET_PROPERTY:tremor::tremor,INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:tremor::tremor,INTERFACE_INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:tremor::tremor,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
        )
        target_get_dynamic_library(dynamic_tremor tremor::tremor)
        message(STATUS "Dynamic vorbis (tremor): ${dynamic_tremor}")
        target_compile_definitions(SDL2_mixer PRIVATE "OGG_DYNAMIC=\"${dynamic_tremor}\"")
        if(SDL2MIXER_VENDORED)
            add_dependencies(SDL2_mixer tremor::tremor)
        endif()
    else()
        target_link_libraries(SDL2_mixer PRIVATE tremor::tremor)
    endif()
endif()

if(SDL2MIXER_VORBIS_VORBISFILE)
    target_compile_definitions(SDL2_mixer PRIVATE MUSIC_OGG)
    if(SDL2MIXER_VENDORED)
        # vendored libogg alread handled
        if(NOT TARGET ogg)
            message(FATAL_ERROR "ogg target not present")
        endif()
        message(STATUS "Using vendored vorbis + vorbisfile")
        set(BUILD_SHARED_LIBS ${SDL2MIXER_VORBIS_VORBISFILE_SHARED})
        sdl_check_project_in_subfolder(external/vorbis vorbisfile SDL2MIXER_VENDORED)
        add_subdirectory(external/vorbis EXCLUDE_FROM_ALL)
        if(NOT TARGET vorbisfile::vorbisfile)
            add_library(vorbisfile::vorbisfile ALIAS vorbisfile)
        endif()
        if(SDL2MIXER_VORBIS_VORBISFILE_SHARED OR NOT SDL2MIXER_BUILD_SHARED_LIBS)
            list(APPEND INSTALL_EXTRA_TARGETS vorbis vorbisfile)
        endif()
        if(NOT SDL2MIXER_VORBIS_VORBISFILE_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:vorbisfile>)
        endif()
    else()
        message(STATUS "Using system vorbisfile")
        find_package(vorbisfile REQUIRED)
        if(NOT SDL2MIXER_VORBIS_VORBISFILE_SHARED)
            list(APPEND PC_REQUIRES vorbisfile)
        endif()
    endif()
    if(SDL2MIXER_VORBIS_VORBISFILE_SHARED)
        target_include_directories(SDL2_mixer PRIVATE
            $<TARGET_PROPERTY:vorbisfile::vorbisfile,INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:vorbisfile::vorbisfile,INTERFACE_INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:vorbisfile::vorbisfile,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
        )
        target_get_dynamic_library(dynamic_vorbisfile vorbisfile::vorbisfile)
        message(STATUS "Dynamic vorbisfile: ${dynamic_vorbisfile}")
        target_compile_definitions(SDL2_mixer PRIVATE "OGG_DYNAMIC=\"${dynamic_vorbisfile}\"")
        if(SDL2MIXER_VENDORED)
            add_dependencies(SDL2_mixer vorbisfile::vorbisfile)
        endif()
    else()
        target_link_libraries(SDL2_mixer PRIVATE vorbisfile::vorbisfile)
    endif()
endif()

if(SDL2MIXER_FLAC_LIBFLAC)
    target_compile_definitions(SDL2_mixer PRIVATE MUSIC_FLAC_LIBFLAC)
    if(SDL2MIXER_VENDORED)
        # vendored libogg alread handled
        if(NOT TARGET ogg)
            message(FATAL_ERROR "ogg target not present")
        endif()
        set(BUILD_SHARED_LIBS "${SDL2MIXER_FLAC_LIBFLAC_SHARED}")
        set(INSTALL_CMAKE_CONFIG_MODULE OFF CACHE BOOL "Disable libflac installation")
        set(WITH_OGG OFF CACHE BOOL "Disable finding ogg using `find_package` (we have vendored it)")
        message(STATUS "Using vendored libflac")
        sdl_check_project_in_subfolder(external/flac libflac SDL2MIXER_VENDORED)
        add_subdirectory(external/flac EXCLUDE_FROM_ALL)
        if(SDL2MIXER_FLAC_LIBFLAC_SHARED)
            list(APPEND INSTALL_EXTRA_TARGETS FLAC)
        endif()
        if(NOT SDL2MIXER_FLAC_LIBFLAC_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:FLAC::FLAC> -l$<TARGET_FILE_BASE_NAME:ogg>)
        endif()
    else()
        message(STATUS "Using system libflac")
        find_package(FLAC REQUIRED)
        if(NOT SDL2MIXER_FLAC_LIBFLAC_SHARED)
            list(APPEND PC_REQUIRES flac)
        endif()
    endif()
    if(SDL2MIXER_FLAC_LIBFLAC_SHARED)
        target_include_directories(SDL2_mixer PRIVATE
            $<TARGET_PROPERTY:FLAC::FLAC,INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:FLAC::FLAC,INTERFACE_INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:FLAC::FLAC,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
        )
        target_get_dynamic_library(dynamic_flac FLAC::FLAC)
        message(STATUS "Dynamic libflac: ${dynamic_flac}")
        target_compile_definitions(SDL2_mixer PRIVATE "FLAC_DYNAMIC=\"${dynamic_flac}\"")
        if(SDL2MIXER_VENDORED)
            add_dependencies(SDL2_mixer FLAC)
        endif()
    else()
        target_link_libraries(SDL2_mixer PRIVATE FLAC)
    endif()
endif()

if(SDL2MIXER_FLAC_DRFLAC)
    target_compile_definitions(SDL2_mixer PRIVATE MUSIC_FLAC_DRFLAC)
endif()

if(SDL2MIXER_MOD_MODPLUG)
    target_compile_definitions(SDL2_mixer PRIVATE MUSIC_MOD_MODPLUG)
    if(SDL2MIXER_VENDORED)
        message(STATUS "Using vendored libmodplug")
        sdl_check_project_in_subfolder(external/libmodplug libmodplug SDL2MIXER_VENDORED)
        set(BUILD_SHARED_LIBS "${SDL2MIXER_MOD_MODPLUG_SHARED}")
        enable_language(CXX)
        add_subdirectory(external/libmodplug EXCLUDE_FROM_ALL)
        target_include_directories(SDL2_mixer PRIVATE external/libmodplug/src)
        if(NOT TARGET modplug::modplug)
            add_library(modplug::modplug ALIAS modplug)
            # <modplug.h> is not escaped properly for windres when using MinGW + "Unix Makefiles" ==> limit it to C sources
            target_compile_definitions(SDL2_mixer PRIVATE $<$<COMPILE_LANGUAGE:C>:MODPLUG_HEADER=<modplug.h$<ANGLE-R>>)
        endif()
        if(SDL2MIXER_MOD_MODPLUG_SHARED OR NOT SDL2MIXER_BUILD_SHARED_LIBS)
            list(APPEND INSTALL_EXTRA_TARGETS modplug)
        endif()
        if(NOT SDL2MIXER_MOD_MODPLUG_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:modplug>)
        endif()
    else()
        message(STATUS "Using system libmodplug")
        find_package(modplug REQUIRED)
        if(NOT SDL2MIXER_MOD_MODPLUG_SHARED)
            list(APPEND PC_REQUIRES libmodplug)
        endif()
    endif()
    if(SDL2MIXER_MOD_MODPLUG_SHARED)
        target_include_directories(SDL2_mixer PRIVATE
            $<TARGET_PROPERTY:modplug::modplug,INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:modplug::modplug,INTERFACE_INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:modplug::modplug,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
        )
        target_get_dynamic_library(dynamic_modplug modplug::modplug)
        message(STATUS "Dynamic modplug: ${dynamic_modplug}")
        target_compile_definitions(SDL2_mixer PRIVATE "MODPLUG_DYNAMIC=\"${dynamic_modplug}\"")
        if(SDL2MIXER_VENDORED)
            add_dependencies(SDL2_mixer modplug::modplug)
        endif()
    else()
        target_link_libraries(SDL2_mixer PRIVATE modplug::modplug)
    endif()
endif()

if(SDL2MIXER_MOD_XMP)
    target_compile_definitions(SDL2_mixer PRIVATE MUSIC_MOD_XMP)
    if(SDL2MIXER_VENDORED)
        message(STATUS "Using vendored libxmp/libxmp-lite")
        message(WARNING "Neither libxmp or libxmp-lite are vendored. Using system libxmp instead.")
    endif()
    if(SDL2MIXER_MOD_XMP_LITE)
        message(STATUS "Using system libxmp-lite")
        find_package(libxmp-lite REQUIRED)
        set(tgt_xmp libxmp-lite::libxmp-lite)
        set(xmp_name libxmp-lite)
        if(NOT SDL2MIXER_MOD_XMP_SHARED)
            list(APPEND PC_REQUIRES libxmplite)
        endif()
    else()
        message(STATUS "Using system libxmp")
        find_package(libxmp REQUIRED)
        set(tgt_xmp libxmp::libxmp)
        set(xmp_name libxmp)
        if(NOT SDL2MIXER_MOD_XMP_SHARED)
            list(APPEND PC_REQUIRES libxmp)
        endif()
    endif()
    if(SDL2MIXER_MOD_XMP_SHARED)
        target_include_directories(SDL2_mixer PRIVATE
            $<TARGET_PROPERTY:${tgt_xmp},INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:${tgt_xmp},INTERFACE_INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:${tgt_xmp},INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
        )
        target_get_dynamic_library(dynamic_xmp ${tgt_xmp})
        message(STATUS "Dynamic ${xmp_name}: ${dynamic_xmp}")
        target_compile_definitions(SDL2_mixer PRIVATE "XMP_DYNAMIC=\"${dynamic_xmp}\"")
        if(SDL2MIXER_VENDORED)
            add_dependencies(SDL2_mixer ${tgt_xmp})
        endif()
    else()
        target_link_libraries(SDL2_mixer PRIVATE ${tgt_xmp})
    endif()
endif()

if(SDL2MIXER_MP3_DRMP3)
    target_compile_definitions(SDL2_mixer PRIVATE MUSIC_MP3_DRMP3)
endif()

if(SDL2MIXER_MP3_MPG123)
    target_compile_definitions(SDL2_mixer PRIVATE MUSIC_MP3_MPG123)
    if(SDL2MIXER_VENDORED)
        message(STATUS "Using vendored mpg123")
        sdl_check_project_in_subfolder(external/mpg123/ports/cmake mpg123 SDL2MIXER_VENDORED)
        set(BUILD_LIBOUT123 FALSE CACHE BOOL "Don't configure libout123")
        set(BUILD_SHARED_LIBS "${SDL2MIXER_MP3_MPG123_SHARED}")
        add_subdirectory(external/mpg123/ports/cmake EXCLUDE_FROM_ALL)
        if(NOT TARGET MPG123::mpg123)
            add_library(MPG123::mpg123 ALIAS libmpg123)
        endif()
        if(SDL2MIXER_MP3_MPG123_SHARED OR NOT SDL2MIXER_BUILD_SHARED_LIBS)
            list(APPEND INSTALL_EXTRA_TARGETS libmpg123)
        endif()
        if(NOT SDL2MIXER_MP3_MPG123_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:MPG123::mpg123>)
        endif()
    else()
        message(STATUS "Using system mpg123")
        find_package(MPG123 REQUIRED)
        if(NOT SDL2MIXER_MP3_MPG123_SHARED)
            list(APPEND PC_REQUIRES libmpg123)
        endif()
    endif()
    if(SDL2MIXER_MP3_MPG123_SHARED)
        target_include_directories(SDL2_mixer PRIVATE
            $<TARGET_PROPERTY:MPG123::mpg123,INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:MPG123::mpg123,INTERFACE_INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:MPG123::mpg123,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
        )
        target_get_dynamic_library(dynamic_mpg123 MPG123::mpg123)
        message(STATUS "Dynamic mpg123}: ${dynamic_mpg123}")
        target_compile_definitions(SDL2_mixer PRIVATE "MPG123_DYNAMIC=\"${dynamic_mpg123}\"")
        if(SDL2MIXER_VENDORED)
            add_dependencies(SDL2_mixer MPG123::mpg123)
        endif()
    else()
        target_link_libraries(SDL2_mixer PRIVATE MPG123::mpg123)
    endif()
endif()

if(SDL2MIXER_MIDI_FLUIDSYNTH)
    target_compile_definitions(SDL2_mixer PRIVATE MUSIC_MID_FLUIDSYNTH)
    if(SDL2MIXER_VENDORED)
        message(STATUS "Using vendored FluidSynth")
        message(FATAL_ERROR "FluidSynth is not vendored.")
    else()
        message(STATUS "Using system FluidSynth")
        find_package(FluidSynth REQUIRED)
        if(NOT SDL2MIXER_MIDI_FLUIDSYNTH_SHARED)
            list(APPEND PC_REQUIRES fluidsynth)
        endif()
    endif()
    if(SDL2MIXER_MIDI_FLUIDSYNTH_SHARED)
        target_include_directories(SDL2_mixer PRIVATE
            $<TARGET_PROPERTY:FluidSynth::FluidSynth,INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:FluidSynth::FluidSynth,INTERFACE_INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:FluidSynth::FluidSynth,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
        )
        target_get_dynamic_library(dynamic_fluidsynth FluidSynth::FluidSynth)
        message(STATUS "Dynamic fluidsynth: ${dynamic_fluidsynth}")
        target_compile_definitions(SDL2_mixer PRIVATE "FLUIDSYNTH_DYNAMIC=\"${dynamic_fluidsynth}\"")
        if(SDL2MIXER_VENDORED)
            add_dependencies(SDL2_mixer FluidSynth::FluidSynth)
        endif()
    else()
        target_link_libraries(SDL2_mixer PRIVATE FluidSynth::FluidSynth)
    endif()
endif()

if(SDL2MIXER_MIDI_NATIVE)
    target_compile_definitions(SDL2_mixer PRIVATE MUSIC_MID_NATIVE)
    target_sources(SDL2_mixer PRIVATE
        src/codecs/native_midi/native_midi_common.c
        src/codecs/native_midi/native_midi_common.h
    )
    if(WIN32)
        target_sources(SDL2_mixer PRIVATE src/codecs/native_midi/native_midi_win32.c)
        target_link_libraries(SDL2_mixer PRIVATE winmm)
    elseif(APPLE)
        target_sources(SDL2_mixer PRIVATE src/codecs/native_midi/native_midi_macosx.c)
        target_link_options(SDL2_mixer PRIVATE -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit -Wl,-framework,CoreServices)
    elseif(HAIKU)
        enable_language(CXX)
        target_sources(SDL2_mixer PRIVATE src/codecs/native_midi/native_midi_haiku.cpp)
        target_link_libraries(SDL2_mixer PRIVATE midi)
    endif()
endif()

if(SDL2MIXER_MIDI_TIMIDITY)
    target_compile_definitions(SDL2_mixer PRIVATE MUSIC_MID_TIMIDITY)
    target_sources(SDL2_mixer PRIVATE
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

if(SDL2MIXER_WAVE)
    target_compile_definitions(SDL2_mixer PRIVATE MUSIC_WAV)
endif()

if(SDL2MIXER_INSTALL)
    install(
        TARGETS SDL2_mixer
        EXPORT SDL2MixerTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT devel
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT library
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT library
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/SDL2" COMPONENT devel
    )

    if(INSTALL_EXTRA_TARGETS)
        install(TARGETS ${INSTALL_EXTRA_TARGETS}
            EXPORT SDL2MixerTargets
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT devel
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT library
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT library
            PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" COMPONENT devel
        )
    endif()

    ##### export files #####
    if(WIN32 AND NOT MINGW)
        set(PKG_PREFIX "cmake")
    else()
        set(PKG_PREFIX "${CMAKE_INSTALL_LIBDIR}/cmake/SDL2_mixer")
    endif()

    configure_package_config_file(SDL2_mixerConfig.cmake.in SDL2_mixerConfig.cmake
        INSTALL_DESTINATION "${PKG_PREFIX}"
    )
    write_basic_package_version_file("${PROJECT_BINARY_DIR}/SDL2_mixerConfigVersion.cmake"
        VERSION ${FULL_VERSION}
        COMPATIBILITY AnyNewerVersion
    )
    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/SDL2_mixerConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/SDL2_mixerConfigVersion.cmake"
        DESTINATION "${PKG_PREFIX}"
        COMPONENT devel
    )
    if(NOT SDL2MIXER_BUILD_SHARED_LIBS)
        install(
            FILES
                cmake/FindFLAC.cmake
                cmake/FindFluidSynth.cmake
                cmake/Findlibxmp.cmake
                cmake/Findlibxmp-lite.cmake
                cmake/Findmodplug.cmake
                cmake/Findopusfile.cmake
                cmake/FindMPG123.cmake
                cmake/Findvorbisfile.cmake
                cmake/Findtremor.cmake
            DESTINATION "${PKG_PREFIX}"
            COMPONENT devel
        )
    endif()
    install(EXPORT SDL2MixerTargets
        FILE SDL2_mixer-${sdl2_mixer_install_name_infix}-targets.cmake
        NAMESPACE SDL2_mixer::
        DESTINATION "${PKG_PREFIX}"
        COMPONENT devel
    )

    if(SDL2MIXER_BUILD_SHARED_LIBS)
        # Only create a .pc file for a shared SDL2_mixer
        set(VERSION ${FULL_VERSION})
        set(SDL_VERSION ${SDL_REQUIRED_VERSION})
        set(prefix "${CMAKE_INSTALL_PREFIX}")
        set(exec_prefix "\${prefix}")
        set(libdir "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
        set(bindir "\${exec_prefix}/bin")
        set(includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
        string(JOIN " " PC_REQUIRES ${PC_REQUIRES})
        string(JOIN " " PC_LIBS ${PC_LIBS})
        configure_file(SDL2_mixer.pc.in ${CMAKE_CURRENT_BINARY_DIR}/SDL2_mixer.pc.intermediate @ONLY)
        file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/SDL2_mixer-$<CONFIG>.pc" INPUT "${CMAKE_CURRENT_BINARY_DIR}/SDL2_mixer.pc.intermediate")

        set(PC_DESTDIR)
        if(CMAKE_SYSTEM_NAME MATCHES FreeBSD)
            # FreeBSD uses ${PREFIX}/libdata/pkgconfig
            set(PC_DESTDIR "libdata/pkgconfig")
        else()
            set(PC_DESTDIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
        endif()
        # Only install a SDL2_mixer.pc file in Release mode
        install(CODE "
            # FIXME: use file(COPY_FILE) if minimum CMake version >= 3.21
            execute_process(COMMAND \"\${CMAKE_COMMAND}\" -E copy_if_different
                \"${CMAKE_CURRENT_BINARY_DIR}/SDL2_mixer-$<CONFIG>.pc\"
                \"${CMAKE_CURRENT_BINARY_DIR}/SDL2_mixer.pc\")
            file(INSTALL DESTINATION \"\${CMAKE_INSTALL_PREFIX}/${PC_DESTDIR}\"
                TYPE FILE
                FILES \"${CMAKE_CURRENT_BINARY_DIR}/SDL2_mixer.pc\")" CONFIG Release COMPONENT devel)
    endif()

    if(SDL2MIXER_BUILD_SHARED_LIBS AND (APPLE OR (UNIX AND NOT ANDROID)))
        install(
            FILES
                "${PROJECT_BINARY_DIR}/libSDL2_mixer$<$<CONFIG:Debug>:${SDL2MIXER_DEBUG_POSTFIX}>$<TARGET_FILE_SUFFIX:SDL2_mixer>"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            COMPONENT devel
        )
    endif()

    install(FILES "LICENSE.txt"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/licenses/${PROJECT_NAME}"
        COMPONENT library
    )
endif()

if(SDL2MIXER_SAMPLES)
    add_executable(playmus playmus.c)
    add_executable(playwave playwave.c)

    find_package(SDL2main)

    foreach(prog playmus playwave)
        # FIXME: mingw should be handled by SDL2::SDL2(-static) target
        if(MINGW)
            target_link_libraries(${prog} PRIVATE mingw32)
            target_link_options(${prog} PRIVATE -mwindows)
        endif()
        target_link_libraries(${prog} PRIVATE SDL2_mixer::${sdl2_mixer_export_name})
        if(TARGET SDL2::SDL2main)
            target_link_libraries(${prog} PRIVATE SDL2::SDL2main)
        endif()
        target_link_libraries(${prog} PRIVATE ${sdl2_target_name})

        if(SDL2MIXER_SAMPLES_INSTALL)
            install(TARGETS ${prog}
                RUNTIME DESTINATION "{CMAKE_INSTALL_BINDIR}"
            )
        endif()
    endforeach()
endif()

add_library(SDL2::mixer INTERFACE IMPORTED GLOBAL)
set_target_properties(SDL2::mixer PROPERTIES
    INTERFACE_LINK_LIBRARIES "SDL2_mixer"
)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.17")
    set_target_properties(SDL2::mixer PROPERTIES
        DEPRECATION "Use SDL2_mixer::SDL2_mixer or SDL2_mixer::SDL2_mixer-static instead"
    )
endif()
