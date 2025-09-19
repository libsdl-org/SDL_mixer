cmake_minimum_required(VERSION 3.16...4.0)

if(NOT DEFINED CMAKE_BUILD_TYPE)
  set(cmake_build_type_undefined 1)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# See docs/release_checklist.md
set(MAJOR_VERSION 3)
set(MINOR_VERSION 0)
set(MICRO_VERSION 0)
set(SDL_REQUIRED_VERSION 3.0.0)

project(SDL3_mixer
    LANGUAGES C
    VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${MICRO_VERSION}"
)

include("${SDL3_mixer_SOURCE_DIR}/cmake/GetGitRevisionDescription.cmake")
include("${SDL3_mixer_SOURCE_DIR}/cmake/PrivateSdlFunctions.cmake")
include("${SDL3_mixer_SOURCE_DIR}/cmake/sdlcpu.cmake")
include("${SDL3_mixer_SOURCE_DIR}/cmake/sdlmanpages.cmake")
include("${SDL3_mixer_SOURCE_DIR}/cmake/sdlplatform.cmake")

message(STATUS "Configuring ${PROJECT_NAME} ${PROJECT_VERSION}")

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(SDLMIXER_ROOTPROJECT ON)
else()
    set(SDLMIXER_ROOTPROJECT OFF)
endif()

# By default, configure in RelWithDebInfo configuration
if(SDLMIXER_ROOTPROJECT)
  get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  if(is_multi_config)
    # The first item in CMAKE_CONFIGURATION_TYPES is the default configuration
    if(DEFINED CMAKE_CONFIGURATION_TYPES AND "RelWithDebInfo" IN_LIST CMAKE_CONFIGURATION_TYPES)
      list(REMOVE_ITEM CMAKE_CONFIGURATION_TYPES "RelWithDebInfo")
      list(INSERT CMAKE_CONFIGURATION_TYPES 0 "RelWithDebInfo")
      set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "CMake configuration types" FORCE)
    endif()
  else()
    if(cmake_build_type_undefined)
      set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "CMake build type" FORCE)
    endif()
  endif()
endif()

set(SDLMIXER_SAMPLES_DEFAULT ${SDLMIXER_ROOTPROJECT})
if(ANDROID)
    set(SDLMIXER_SAMPLES_DEFAULT OFF)
endif()

# option() honors normal variables.
cmake_policy(SET CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

if(POLICY CMP0112)
    # Target file component generator expressions do not add target dependencies.
    cmake_policy(SET CMP0112 NEW)
endif()

if(POLICY CMP0099)
    # Make `INTERFACE_LINK_DIRECTORIES` a transitive usage requirement.
    # This is needed for static dependencies which have transitive dependencies
    # outside of compiler default search paths.
    cmake_policy(SET CMP0099 NEW)
endif()

# Assume MSVC projects don't have a package manager and need vendored dependencies (by default).
# Most other platforms have some kind of package manager.
# FIXME: consider a package manager such as conan/vcpkg instead of vendoring
if(ANDROID OR MSVC)
    set(vendored_default ON)
else()
    set(vendored_default OFF)
endif()

set(sdl3mixer_install_enableable ON)
if((TARGET SDL3-shared OR TARGET SDL3-static) AND SDL_DISABLE_INSTALL)
    # Cannot install SDL3_mixer when SDL3 is built in same built, and is not installed.
    set(sdl3mixer_install_enableable OFF)
endif()

if(NOT DEFINED CMAKE_FIND_PACKAGE_PREFER_CONFIG)
    set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)
endif()

include(CheckIncludeFile)
include(CheckSymbolExists)
include(CMakeDependentOption)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
include(PkgConfigHelper)

set(PLATFORM_SUPPORTS_SHARED ON)
if(EMSCRIPTEN OR VITA OR PSP OR PS2 OR N3DS OR RISCOS)
  set(PLATFORM_SUPPORTS_SHARED OFF)
endif()

option(CMAKE_POSITION_INDEPENDENT_CODE "Build static libraries with -fPIC" ${PLATFORM_SUPPORTS_SHARED})

# SDL_mixer SDL2-api-on-3 can only be built as a static library. This is not negotiable :)
# See https://github.com/libsdl-org/SDL_mixer/issues/764
set(BUILD_SHARED_LIBS OFF)

cmake_dependent_option(SDLMIXER_INSTALL "Enable SDL3mixer install target" ${SDLMIXER_ROOTPROJECT} "${sdl3mixer_install_enableable}" OFF)
cmake_dependent_option(SDLMIXER_INSTALL_CPACK "Create binary SDL3_mixer archive using CPack" ${SDLMIXER_ROOTPROJECT} "SDLMIXER_INSTALL" OFF)
cmake_dependent_option(SDLMIXER_INSTALL_MAN "Install man pages for SDL3_mixer" OFF "SDLMIXER_INSTALL" OFF)
cmake_dependent_option(SDLMIXER_DEPS_SHARED "Load dependencies dynamically" ON PLATFORM_SUPPORTS_SHARED OFF)
cmake_dependent_option(SDLMIXER_RELOCATABLE "Create relocatable SDL_mixer package" "${MSVC}" SDLMIXER_INSTALL OFF)
option(SDLMIXER_VENDORED "Use vendored third-party libraries" ${vendored_default})
option(SDLMIXER_WERROR "Treat warnings as errors" OFF)

option(SDLMIXER_STRICT "Fail when a dependency could not be found" OFF)
set(required "")
set(fatal_error "STATUS")
if(SDLMIXER_STRICT)
  set(required "REQUIRED")
  set(fatal_error "FATAL_ERROR")
endif()

option(SDLMIXER_SAMPLES "Build the SDL3_mixer sample program(s)" ${SDLMIXER_SAMPLES_DEFAULT})
cmake_dependent_option(SDLMIXER_SAMPLES_INSTALL "Install the SDL3_mixer sample program(s)" OFF "SDLMIXER_SAMPLES;SDLMIXER_INSTALL" OFF)

option(SDLMIXER_SNDFILE "Support loading sounds via libsndfile" OFF)
option(SDLMIXER_SNDFILE_SHARED "Dynamically load libsndfile" "${SDLMIXER_DEPS_SHARED}")

option(SDLMIXER_FLAC "Enable FLAC music" ON)

cmake_dependent_option(SDLMIXER_FLAC_LIBFLAC "Enable FLAC music using libFLAC" OFF SDLMIXER_FLAC OFF)
cmake_dependent_option(SDLMIXER_FLAC_LIBFLAC_SHARED "Dynamically load LIBFLAC" "${SDLMIXER_DEPS_SHARED}" SDLMIXER_FLAC_LIBFLAC OFF)

cmake_dependent_option(SDLMIXER_FLAC_DRFLAC "Enable FLAC music using drflac" ON SDLMIXER_FLAC OFF)

option(SDLMIXER_GME "Support loading GME music via game-music-emu" ON)
option(SDLMIXER_GME_SHARED "Dynamically load libgme" "${SDLMIXER_DEPS_SHARED}")

option(SDLMIXER_MOD "Support loading MOD music" ON)

cmake_dependent_option(SDLMIXER_MOD_XMP "Support loading MOD music via libxmp" ON SDLMIXER_MOD OFF)
cmake_dependent_option(SDLMIXER_MOD_XMP_LITE "Use libxmp-lite instead of libxmp" OFF "SDLMIXER_MOD_XMP;NOT SDLMIXER_VENDORED" OFF)
cmake_dependent_option(SDLMIXER_MOD_XMP_SHARED "Dynamically load libxmp(-lite)" "${SDLMIXER_DEPS_SHARED}" SDLMIXER_MOD_XMP OFF)

if(SDLMIXER_MOD AND NOT SDLMIXER_MOD_XMP)
    message(FATAL_ERROR "MOD support was enabled (SDLMIXER_MOD) but xmp (SDLMIXER_MOD_XMP) was disabled.")
endif()

option(SDLMIXER_MP3 "Enable MP3 music" ON)

cmake_dependent_option(SDLMIXER_MP3_DRMP3 "Support loading MP3 music via dr_mp3" ON SDLMIXER_MP3 OFF)

cmake_dependent_option(SDLMIXER_MP3_MPG123 "Support loading MP3 music via MPG123" OFF SDLMIXER_MP3 OFF)
cmake_dependent_option(SDLMIXER_MP3_MPG123_SHARED "Dynamically load mpg123" "${SDLMIXER_DEPS_SHARED}" SDLMIXER_MP3_MPG123 OFF)

if(SDLMIXER_MP3 AND NOT (SDLMIXER_MP3_DRMP3 OR SDLMIXER_MP3_MPG123))
    message(FATAL_ERROR "MP3 support was enabled (SDLMIXER_MP3) but neither drmp3 (SDLMIXER_MP3_DRMP3) or mpg123 (SDLMIXER_MP3_MPG123) were enabled.")
endif()

option(SDLMIXER_MIDI "Enable MIDI music" ON)

cmake_dependent_option(SDLMIXER_MIDI_FLUIDSYNTH "Support FluidSynth MIDI output" ON "SDLMIXER_MIDI;NOT SDLMIXER_VENDORED" OFF)
cmake_dependent_option(SDLMIXER_MIDI_FLUIDSYNTH_SHARED "Dynamically load libfluidsynth" "${SDLMIXER_DEPS_SHARED}" SDLMIXER_MIDI_FLUIDSYNTH OFF)

if(WIN32 OR APPLE OR HAIKU OR "${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    cmake_dependent_option(SDLMIXER_MIDI_NATIVE "Support native MIDI output" ON SDLMIXER_MIDI OFF)
else()
    set(SDLMIXER_MIDI_NATIVE OFF)
endif()

cmake_dependent_option(SDLMIXER_MIDI_TIMIDITY "Support timidity MIDI output" ON SDLMIXER_MIDI OFF)

if(SDLMIXER_MIDI AND NOT (SDLMIXER_MIDI_TIMIDITY OR SDLMIXER_MIDI_NATIVE OR SDLMIXER_MIDI_FLUIDSYNTH))
    message(FATAL_ERROR "MIDI support was enabled (SDLMIXER_MIDI) but neither FluidSynth (SDLMIXER_MIDI_FLUIDSYNTH), native (SDLMIXER_MIDI_NATIVE) or timidity (SDLMIXER_MIDI_TIMIDITY) was enabled")
endif()

option(SDLMIXER_OPUS "Enable Opus music" ON)
cmake_dependent_option(SDLMIXER_OPUS_SHARED "Dynamically load libopus" "${SDLMIXER_DEPS_SHARED}" SDLMIXER_OPUS OFF)

set(sdl3mixer_vorbis_strings STB TREMOR VORBISFILE)
set(SDLMIXER_VORBIS "STB" CACHE STRING "Enable OGG Vorbis music")
set_property(CACHE SDLMIXER_VORBIS PROPERTY STRINGS "${sdl3mixer_vorbis_strings}")
if(SDLMIXER_VORBIS)
    if(NOT SDLMIXER_VORBIS IN_LIST sdl3mixer_vorbis_strings)
        message(FATAL_ERROR "SDLMIXER_VORBIS contains an invalid value (=${SDLMIXER_VORBIS}). It must be one of ${sdl3mixer_vorbis_strings}.")
    endif()
endif()
set(SDLMIXER_VORBIS_STB OFF)
set(SDLMIXER_VORBIS_TREMOR OFF)
set(SDLMIXER_VORBIS_VORBISFILE OFF)
if(SDLMIXER_VORBIS STREQUAL "STB")
    set(SDLMIXER_VORBIS_STB ON)
endif()
if(SDLMIXER_VORBIS STREQUAL "TREMOR")
    set(SDLMIXER_VORBIS_TREMOR ON)
endif()
if(SDLMIXER_VORBIS STREQUAL "VORBISFILE")
    set(SDLMIXER_VORBIS_VORBISFILE ON)
endif()
cmake_dependent_option(SDLMIXER_VORBIS_TREMOR_SHARED "Dynamically load tremor library" "${SDLMIXER_DEPS_SHARED}" SDLMIXER_VORBIS_TREMOR OFF)
cmake_dependent_option(SDLMIXER_VORBIS_VORBISFILE_SHARED "Dynamically load vorbisfile library" "${SDLMIXER_DEPS_SHARED}" SDLMIXER_VORBIS_VORBISFILE OFF)

option(SDLMIXER_WAVE "Enable streaming WAVE music" ON)

option(SDLMIXER_WAVPACK "Enable WavPack music" ON)
cmake_dependent_option(SDLMIXER_WAVPACK_DSD "Enable WavPack DSD music support" OFF SDLMIXER_WAVPACK OFF)
cmake_dependent_option(SDLMIXER_WAVPACK_SHARED "Dynamically load WavPack library" "${SDLMIXER_DEPS_SHARED}" SDLMIXER_WAVPACK OFF)

if(SDLMIXER_VORBIS_TREMOR OR SDLMIXER_VORBIS_VORBISFILE OR SDLMIXER_FLAC_LIBFLAC OR SDLMIXER_OPUS)
    set(SDLMIXER_OGG TRUE)
    set(SDLMIXER_OGG_install FALSE)
    if(SDLMIXER_VORBIS_VORBISFILE_SHARED OR SDLMIXER_FLAC_SHARED OR SDLMIXER_OPUS_SHARED)
        set(SDLMIXER_OGG_SHARED TRUE)
        set(SDLMIXER_OGG_install TRUE)
    else()
        set(SDLMIXER_OGG_SHARED FALSE)
        set(SDLMIXER_OGG_install TRUE)
    endif()
else()
    set(SDLMIXER_OGG FALSE)
endif()

set(sdl_required_components Headers)

if(NOT TARGET SDL3::Headers OR NOT TARGET SDL3::SDL3)
    find_package(SDL3 ${SDL_REQUIRED_VERSION} REQUIRED COMPONENTS Headers)
endif()

SDL_DetectTargetCPUArchitectures(SDL_CPU_NAMES)
SDL_DetectCMakePlatform()

add_library(SDL3_mixer-static STATIC
    src/codecs/load_aiff.c
    src/codecs/load_voc.c
    src/codecs/load_sndfile.c
    src/codecs/mp3utils.c
    src/codecs/music_drflac.c
    src/codecs/music_drmp3.c
    src/codecs/music_flac.c
    src/codecs/music_fluidsynth.c
    src/codecs/music_gme.c
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
add_library(SDL3_mixer::SDL3_mixer-static ALIAS SDL3_mixer-static)
if(NOT TARGET SDL3_mixer::SDL3_mixer)
    add_library(SDL3_mixer::SDL3_mixer ALIAS SDL3_mixer-static)
endif()
target_include_directories(SDL3_mixer-static
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
        src
        src/codecs
)
target_compile_definitions(SDL3_mixer-static PRIVATE
    BUILD_SDL
    SDL_BUILD_MAJOR_VERSION=${MAJOR_VERSION}
    SDL_BUILD_MINOR_VERSION=${MINOR_VERSION}
    SDL_BUILD_MICRO_VERSION=${MICRO_VERSION}
)
target_link_libraries(SDL3_mixer-static PUBLIC SDL3::Headers)
sdl_add_warning_options(SDL3_mixer-static WARNING_AS_ERROR ${SDLMIXER_WERROR})
if(WIN32 AND BUILD_SHARED_LIBS)
    target_sources(SDL3_mixer-static PRIVATE
        src/version.rc
    )
    if(MINGW)
        target_link_options(SDL3_mixer-static PRIVATE -static-libgcc)
    endif()
endif()
set_target_properties(SDL3_mixer-static PROPERTIES
    OUTPUT_NAME "SDL3_mixer"
    DEFINE_SYMBOL DLL_EXPORT
    EXPORT_NAME SDL3_mixer-static
    C_VISIBILITY_PRESET "hidden"
)

sdl_target_link_option_version_file(SDL3_mixer-static "${CMAKE_CURRENT_SOURCE_DIR}/src/SDL_mixer.sym")

if(MSVC)
    set_target_properties(SDL3_mixer-static PROPERTIES
        OUTPUT_NAME "SDL3_mixer-static"
    )
endif()

set(INSTALL_EXTRA_TARGETS)
set(PC_LIBS)
set(PC_REQUIRES)
set(SDLMIXER_BACKENDS)

list(APPEND SDLMIXER_BACKENDS SNDFILE)
set(SDLMIXER_SNDFILE_ENABLED FALSE)
if(SDLMIXER_SNDFILE)
    if(SDLMIXER_VENDORED)
        message(STATUS "Using vendored libsndfile")
        message(${fatal_error} "libsndfile is not vendored.")
    else()
        find_package(SndFile ${required})
        if(SndFile_FOUND)
            set(SDLMIXER_SNDFILE_ENABLED TRUE)
            message(STATUS "Using system libsndfile")
            if(NOT SDLMIXER_SNDFILE_SHARED)
                list(APPEND PC_REQUIRES sndfile)
            endif()
        else()
            message(${fatal_error} "libsndfile NOT found")
        endif()
    endif()
    if(SDLMIXER_SNDFILE_ENABLED)
        target_compile_definitions(SDL3_mixer-static PRIVATE LOAD_SNDFILE)
        if(SDLMIXER_SNDFILE_SHARED)
            target_include_directories(SDL3_mixer-static PRIVATE
                $<TARGET_PROPERTY:SndFile::sndfile,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:SndFile::sndfile,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:SndFile::sndfile,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_sndfile SndFile::sndfile)
            message(STATUS "Dynamic libsndfile: ${dynamic_sndfile}")
            target_compile_definitions(SDL3_mixer-static PRIVATE "SNDFILE_DYNAMIC=\"${dynamic_sndfile}\"")
            if(SDLMIXER_VENDORED)
                add_dependencies(SDL3_mixer-static SndFile::sndfile)
            endif()
        else()
            target_link_libraries(SDL3_mixer-static PRIVATE SndFile::sndfile)
        endif()
    endif()
endif()

if(SDLMIXER_OGG)
    # libogg is a requirement of libflac, libtremor and libvorbisfile, so only need this library when vendoring
    if(SDLMIXER_VENDORED)
        message(STATUS "Using vendored libogg")
        set(BUILD_SHARED_LIBS ${SDLMIXER_OGG_SHARED})
        set(INSTALL_CMAKE_PACKAGE_MODULE FALSE)
        set(BUILD_TESTING OFF)
        sdl_check_project_in_subfolder(external/ogg ogg SDLMIXER_VENDORED)
        add_subdirectory(external/ogg external/ogg-build EXCLUDE_FROM_ALL)
        if(SDLMIXER_OGG_install)
            list(APPEND INSTALL_EXTRA_TARGETS ogg)
        endif()
    endif()
endif()

list(APPEND SDLMIXER_BACKENDS OPUS)
set(SDLMIXER_OPUS_ENABLED FALSE)
if(SDLMIXER_OPUS)
    if(SDLMIXER_VENDORED)
        set(SDLMIXER_OPUS_ENABLED TRUE)
        # vendored libogg already handled
        if(NOT TARGET ogg)
            message(FATAL_ERROR "ogg target not present")
        endif()
        message(STATUS "Using vendored opus")
        set(BUILD_SHARED_LIBS ${SDLMIXER_OPUS_SHARED})
        set(BUILD_PROGRAMS OFF)
        sdl_check_project_in_subfolder(external/opus opus SDLMIXER_VENDORED)
        add_subdirectory(external/opus external/opus-build EXCLUDE_FROM_ALL)

        set(OP_DISABLE_DOCS TRUE)
        set(OP_DISABLE_EXAMPLES TRUE)
        set(OP_DISABLE_HTTP TRUE)
        message(STATUS "Using vendored opusfile")
        set(BUILD_SHARED_LIBS ${SDLMIXER_OPUS_SHARED})
        sdl_check_project_in_subfolder(external/opusfile opusfile SDLMIXER_VENDORED)
        add_subdirectory(external/opusfile external/opusfile-build EXCLUDE_FROM_ALL)
        if(MSVC)
            set_property(TARGET opusfile PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
        endif()

        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/opusfile.h" "#include \"${CMAKE_CURRENT_SOURCE_DIR}/external/opusfile/include/opusfile.h\"\n")
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/opus"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/opusfile.h" "${CMAKE_CURRENT_BINARY_DIR}/opus/opusfile.h"
        )
        target_include_directories(SDL3_mixer-static PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

        if(NOT TARGET OpusFile::opusfile)
            add_library(OpusFile::opusfile ALIAS opusfile)
        endif()
        if(SDLMIXER_OPUS_SHARED)
            list(APPEND INSTALL_EXTRA_TARGETS opus opusfile)
        endif()
        if(NOT SDLMIXER_OPUS_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:opusfile> -l$<TARGET_FILE_BASE_NAME:opus> -l$<TARGET_FILE_BASE_NAME:ogg>)
        endif()
    else()
        find_package(OpusFile ${required})
        if(OpusFile_FOUND AND Ogg_FOUND)
            set(SDLMIXER_OPUS_ENABLED TRUE)
            message(STATUS "Using system opusfile")
            if(NOT SDLMIXER_OPUS_SHARED)
                list(APPEND PC_REQUIRES opusfile)
            endif()
        else()
            message(${fatal_error} "opusfile NOT found")
        endif()
    endif()
    if(SDLMIXER_OPUS_ENABLED)
        target_compile_definitions(SDL3_mixer-static PRIVATE MUSIC_OPUS)
        if(SDLMIXER_OPUS_SHARED)
            target_include_directories(SDL3_mixer-static PRIVATE
                $<TARGET_PROPERTY:OpusFile::opusfile,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:OpusFile::opusfile,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:OpusFile::opusfile,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_opusfile OpusFile::opusfile)
            message(STATUS "Dynamic opus (opusfile): ${dynamic_opusfile}")
            target_compile_definitions(SDL3_mixer-static PRIVATE "OPUS_DYNAMIC=\"${dynamic_opusfile}\"")
            if(SDLMIXER_VENDORED)
                add_dependencies(SDL3_mixer-static OpusFile::opusfile)
            endif()
        else()
            target_link_libraries(SDL3_mixer-static PRIVATE OpusFile::opusfile)
        endif()
    endif()
endif()

list(APPEND SDLMIXER_BACKENDS VORBIS_STB)
set(SDLMIXER_VORBIS_STB_ENABLED FALSE)
if(SDLMIXER_VORBIS_STB)
    set(SDLMIXER_VORBIS_STB_ENABLED TRUE)
    message(STATUS "Enabled ogg music: using stb_vorbis")
    target_compile_definitions(SDL3_mixer-static PRIVATE MUSIC_OGG)
    target_compile_definitions(SDL3_mixer-static PRIVATE OGG_USE_STB)
endif()

list(APPEND SDLMIXER_BACKENDS VORBIS_TREMOR)
set(SDLMIXER_VORBIS_TREMOR_ENABLED FALSE)
if(SDLMIXER_VORBIS_TREMOR)
    if(SDLMIXER_VENDORED)
        set(SDLMIXER_VORBIS_TREMOR_ENABLED TRUE)
        # vendored libogg already handled
        if(NOT TARGET ogg)
            message(FATAL_ERROR "ogg target not present")
        endif()
        message(STATUS "Using vendored tremor")
        set(BUILD_SHARED_LIBS ${SDLMIXER_VORBIS_TREMOR_SHARED})
        sdl_check_project_in_subfolder(external/tremor tremor SDLMIXER_VENDORED)
        add_subdirectory(external/tremor external/tremor-build EXCLUDE_FROM_ALL)
        if(NOT TARGET tremor::tremor)
            add_library(tremor::tremor ALIAS vorbisidec)
        endif()

        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ivorbisfile.h" "#include \"${CMAKE_CURRENT_SOURCE_DIR}/external/tremor/ivorbisfile.h\"\n")
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/tremor"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/ivorbisfile.h" "${CMAKE_CURRENT_BINARY_DIR}/tremor/ivorbisfile.h"
        )
        target_include_directories(SDL3_mixer-static PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
        if(SDLMIXER_VORBIS_TREMOR_SHARED)
            list(APPEND INSTALL_EXTRA_TARGETS vorbisidec)
        endif()
        if(NOT SDLMIXER_VORBIS_TREMOR_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:vorbisidec> -l$<TARGET_FILE_BASE_NAME:ogg>)
        endif()
    else()
        message(STATUS "Using system tremor")
        find_package(tremor ${required})
        if(tremor_FOUND)
            if(NOT SDLMIXER_VORBIS_TREMOR_SHARED)
                list(APPEND PC_REQUIRES vorbisidec)
            endif()
            set(SDLMIXER_VORBIS_TREMOR_ENABLED TRUE)
        else()
            message(${fatal_error} "tremor NOT found")
        endif()
    endif()
    if(SDLMIXER_VORBIS_TREMOR_ENABLED)
        target_compile_definitions(SDL3_mixer-static PRIVATE MUSIC_OGG OGG_USE_TREMOR)
        if(SDLMIXER_VORBIS_TREMOR_SHARED)
            target_include_directories(SDL3_mixer-static PRIVATE
                $<TARGET_PROPERTY:tremor::tremor,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:tremor::tremor,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:tremor::tremor,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_tremor tremor::tremor)
            message(STATUS "Dynamic vorbis (tremor): ${dynamic_tremor}")
            target_compile_definitions(SDL3_mixer-static PRIVATE "OGG_DYNAMIC=\"${dynamic_tremor}\"")
            if(SDLMIXER_VENDORED)
                add_dependencies(SDL3_mixer-static tremor::tremor)
            endif()
        else()
            target_link_libraries(SDL3_mixer-static PRIVATE tremor::tremor)
        endif()
    endif()
endif()

list(APPEND SDLMIXER_BACKENDS VORBIS_VORBISFILE)
set(SDLMIXER_VORBIS_VORBISFILE_ENABLED FALSE)
if(SDLMIXER_VORBIS_VORBISFILE)
    if(SDLMIXER_VENDORED)
        set(SDLMIXER_VORBIS_VORBISFILE_ENABLED TRUE)
        # vendored libogg already handled
        if(NOT TARGET ogg)
            message(FATAL_ERROR "ogg target not present")
        endif()
        message(STATUS "Using vendored vorbis + vorbisfile")
        set(BUILD_SHARED_LIBS ${SDLMIXER_VORBIS_VORBISFILE_SHARED})
        sdl_check_project_in_subfolder(external/vorbis vorbisfile SDLMIXER_VENDORED)
        add_subdirectory(external/vorbis external/vorbis-build EXCLUDE_FROM_ALL)
        if(NOT TARGET Vorbis::vorbisfile)
            add_library(Vorbis::vorbisfile ALIAS vorbisfile)
        endif()
        if(SDLMIXER_VORBIS_VORBISFILE_SHARED)
            list(APPEND INSTALL_EXTRA_TARGETS vorbis vorbisfile)
        endif()
        if(NOT SDLMIXER_VORBIS_VORBISFILE_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:vorbisfile>)
        endif()
    else()
        find_package(Vorbis ${required})
        if(Vorbis_FOUND)
            set(SDLMIXER_VORBIS_VORBISFILE_ENABLED TRUE)
            message(STATUS "Using system vorbisfile")
            if(NOT SDLMIXER_VORBIS_VORBISFILE_SHARED)
                list(APPEND PC_REQUIRES vorbisfile)
            endif()
        else()
            message(${fatal_error} "vorbisfile NOT found")
        endif()
    endif()
    if(SDLMIXER_VORBIS_VORBISFILE_ENABLED)
        target_compile_definitions(SDL3_mixer-static PRIVATE MUSIC_OGG)
        if(SDLMIXER_VORBIS_VORBISFILE_SHARED)
            target_include_directories(SDL3_mixer-static PRIVATE
                $<TARGET_PROPERTY:Vorbis::vorbisfile,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:Vorbis::vorbisfile,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:Vorbis::vorbisfile,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_vorbisfile Vorbis::vorbisfile)
            message(STATUS "Dynamic vorbisfile: ${dynamic_vorbisfile}")
            target_compile_definitions(SDL3_mixer-static PRIVATE "OGG_DYNAMIC=\"${dynamic_vorbisfile}\"")
            if(SDLMIXER_VENDORED)
                add_dependencies(SDL3_mixer-static Vorbis::vorbisfile)
            endif()
        else()
            target_link_libraries(SDL3_mixer-static PRIVATE Vorbis::vorbisfile)
        endif()
    endif()
endif()

set(SDLMIXER_VORBIS_ENABLED FALSE)
if(SDLMIXER_VORBIS_STB_ENABLED OR SDLMIXER_VORBIS_TREMOR_ENABLED OR SDLMIXER_VORBIS_VORBISFILE_ENABLED)
    set(SDLMIXER_VORBIS_ENABLED TRUE)
endif()

list(APPEND SDLMIXER_BACKENDS FLAC_LIBFLAC)
set(SDLMIXER_FLAC_LIBFLAC_ENABLED FALSE)
if(SDLMIXER_FLAC_LIBFLAC)
    if(SDLMIXER_VENDORED)
        set(SDLMIXER_FLAC_LIBFLAC_ENABLED TRUE)
        # vendored libogg already handled
        if(NOT TARGET ogg)
            message(FATAL_ERROR "ogg target not present")
        endif()
        set(BUILD_SHARED_LIBS "${SDLMIXER_FLAC_LIBFLAC_SHARED}")
        set(INSTALL_CMAKE_CONFIG_MODULE OFF)
        set(WITH_OGG OFF)
        set(BUILD_CXXLIBS OFF)
        set(BUILD_EXAMPLES OFF)
        set(BUILD_PROGRAMS OFF)
        set(BUILD_TESTING OFF)
        set(INSTALL_MANPAGES OFF)
        message(STATUS "Using vendored libflac")
        sdl_check_project_in_subfolder(external/flac libflac SDLMIXER_VENDORED)
        add_subdirectory(external/flac external/flac-build EXCLUDE_FROM_ALL)
        if(SDLMIXER_FLAC_LIBFLAC_SHARED)
            list(APPEND INSTALL_EXTRA_TARGETS FLAC)
        endif()
        if(NOT SDLMIXER_FLAC_LIBFLAC_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:FLAC::FLAC> -l$<TARGET_FILE_BASE_NAME:ogg>)
        endif()
    else()
        find_package(FLAC ${required})
        if(FLAC_FOUND)
            set(SDLMIXER_FLAC_LIBFLAC_ENABLED TRUE)
            message(STATUS "Using system libflac")
            if(NOT SDLMIXER_FLAC_LIBFLAC_SHARED)
                list(APPEND PC_REQUIRES flac)
            endif()
            # Xiph's CMake script does not handle Threads::Threads correctly:
            # https://github.com/xiph/flac/issues/820
            get_property(flac_interface_link_libraries TARGET FLAC::FLAC PROPERTY INTERFACE_LINK_LIBRARIES)
            if(Threads::Threads IN_LIST flac_interface_link_libraries)
                find_package(Threads)
            endif()
        else()
            message(${fatal_error} "libflac NOT found")
        endif()
    endif()
    if(SDLMIXER_FLAC_LIBFLAC_ENABLED)
        target_compile_definitions(SDL3_mixer-static PRIVATE MUSIC_FLAC_LIBFLAC)
        if(SDLMIXER_FLAC_LIBFLAC_SHARED)
            target_include_directories(SDL3_mixer-static PRIVATE
                $<TARGET_PROPERTY:FLAC::FLAC,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:FLAC::FLAC,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:FLAC::FLAC,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_flac FLAC::FLAC)
            message(STATUS "Dynamic libflac: ${dynamic_flac}")
            target_compile_definitions(SDL3_mixer-static PRIVATE "FLAC_DYNAMIC=\"${dynamic_flac}\"")
            if(SDLMIXER_VENDORED)
                add_dependencies(SDL3_mixer-static FLAC)
            endif()
        else()
            target_link_libraries(SDL3_mixer-static PRIVATE FLAC::FLAC)
        endif()
    endif()
endif()

list(APPEND SDLMIXER_BACKENDS FLAC_DRFLAC)
set(SDLMIXER_FLAC_DRFLAC_ENABLED FALSE)
if(SDLMIXER_FLAC_DRFLAC)
    set(SDLMIXER_FLAC_DRFLAC_ENABLED TRUE)
    target_compile_definitions(SDL3_mixer-static PRIVATE MUSIC_FLAC_DRFLAC)
endif()

set(SDLMIXER_FLAC_ENABLED FALSE)
if(SDLMIXER_FLAC_DRFLAC_ENABLED OR SDLMIXER_FLAC_LIBFLAC_ENABLED)
    set(SDLMIXER_FLAC_ENABLED TRUE)
endif()

list(APPEND SDLMIXER_BACKENDS GME)
set(SDLMIXER_GME_ENABLED FALSE)
if(SDLMIXER_GME)
    if(SDLMIXER_VENDORED)
        set(SDLMIXER_GME_ENABLED TRUE)
        set(GME_BUILD_SHARED "${SDLMIXER_GME_SHARED}")
        if(SDLMIXER_GME_SHARED)
            set(GME_BUILD_STATIC OFF)
            set(tgt_gme gme_shared)
        else()
            set(GME_BUILD_STATIC ON)
            set(tgt_gme gme_static gme_deps)
        endif()
        set(GME_BUILD_FRAMEWORK OFF)
        set(GME_BUILD_TESTING OFF)
        set(GME_BUILD_EXAMPLES OFF)
        set(GME_ENABLE_UBSAN OFF)
        option(GME_ZLIB "Enable GME to support compressed sound formats" OFF)
        message(STATUS "Using vendored libgme")
        sdl_check_project_in_subfolder(external/libgme libgme SDLMIXER_VENDORED)
        enable_language(CXX)
        add_subdirectory(external/libgme external/libgme-build EXCLUDE_FROM_ALL)
        if(SDLMIXER_GME_SHARED)
            list(APPEND INSTALL_EXTRA_TARGETS ${tgt_gme})
        endif()
        if(NOT SDLMIXER_GME_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:gme>)
        endif()
    else()
        find_package(gme ${required})
        if(gme_FOUND)
            set(SDLMIXER_GME_ENABLED TRUE)
            message(STATUS "Using system libgme")
            if(NOT SDLMIXER_GME_SHARED)
                list(APPEND PC_REQUIRES libgme)
            endif()
        else()
            message(${fatal_error} "libgme NOT found")
        endif()
    endif()
    if(SDLMIXER_GME_ENABLED)
        target_compile_definitions(SDL3_mixer-static PRIVATE MUSIC_GME)
        if(SDLMIXER_GME_SHARED)
            target_include_directories(SDL3_mixer-static PRIVATE
                $<TARGET_PROPERTY:gme::gme,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:gme::gme,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:gme::gme,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_gme gme::gme)
            message(STATUS "Dynamic libgme: ${dynamic_gme}")
            target_compile_definitions(SDL3_mixer-static PRIVATE "GME_DYNAMIC=\"${dynamic_gme}\"")
            if(SDLMIXER_VENDORED)
                add_dependencies(SDL3_mixer-static gme::gme)
            endif()
        else()
            target_link_libraries(SDL3_mixer-static PRIVATE gme::gme)
        endif()
    endif()
endif()

list(APPEND SDLMIXER_BACKENDS MOD_XMP)
set(SDLMIXER_MOD_XMP_ENABLED FALSE)
if(SDLMIXER_MOD_XMP)
    if(SDLMIXER_VENDORED)
        set(SDLMIXER_MOD_XMP_ENABLED TRUE)
        message(STATUS "Using vendored libxmp")
        sdl_check_project_in_subfolder(external/libxmp libxmp SDLMIXER_VENDORED)
        set(LIBXMP_DISABLE_DEPACKERS ON CACHE BOOL "Disable archive depackers")
        set(LIBXMP_DISABLE_PROWIZARD ON CACHE BOOL "Disable ProWizard format loaders")
        if(SDLMIXER_MOD_XMP_SHARED)
            set(BUILD_STATIC OFF)
            set(BUILD_SHARED ON)
            set(tgt_xmp xmp_shared)
        else()
            set(BUILD_STATIC ON)
            set(BUILD_SHARED OFF)
            set(tgt_xmp xmp_static)
        endif()
        set(xmp_name libxmp)
        add_subdirectory(external/libxmp external/libxmp-build EXCLUDE_FROM_ALL)
        if(SDLMIXER_MOD_XMP_SHARED)
            list(APPEND INSTALL_EXTRA_TARGETS ${tgt_xmp})
        endif()
        if(NOT SDLMIXER_MOD_XMP_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:${tgt_xmp}>)
        endif()
    else()
        if(SDLMIXER_MOD_XMP_LITE)
            find_package(libxmp-lite ${required})
            if(libxmp-lite_FOUND)
                set(SDLMIXER_MOD_XMP_ENABLED TRUE)
                message(STATUS "Using system libxmp-lite")
                set(tgt_xmp libxmp-lite::libxmp-lite)
                set(xmp_name libxmp-lite)
                if(NOT SDLMIXER_MOD_XMP_SHARED)
                    list(APPEND PC_REQUIRES libxmp-lite)
                endif()
            else()
                message(${fatal_error} "libxmp-lite NOT found")
            endif()
        else()
            find_package(libxmp ${required})
            if(libxmp_FOUND)
                set(SDLMIXER_MOD_XMP_ENABLED TRUE)
                message(STATUS "Using system libxmp")
                if(TARGET libxmp::xmp_shared)
                  set(tgt_xmp libxmp::xmp_shared)
                elseif(TARGET libxmp::xmp_static)
                  set(tgt_xmp libxmp::xmp_static)
                else()
                  set(tgt_xmp libxmp::libxmp)
                endif()
                set(xmp_name libxmp)
                if(NOT SDLMIXER_MOD_XMP_SHARED)
                    list(APPEND PC_REQUIRES libxmp)
                endif()
            else()
                message(${fatal_error} "libxmp NOT found")
            endif()
        endif()
    endif()
    if(SDLMIXER_MOD_XMP_ENABLED)
        target_compile_definitions(SDL3_mixer-static PRIVATE MUSIC_MOD_XMP)
        if(SDLMIXER_MOD_XMP_SHARED)
            target_include_directories(SDL3_mixer-static PRIVATE
                $<TARGET_PROPERTY:${tgt_xmp},INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:${tgt_xmp},INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:${tgt_xmp},INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_xmp ${tgt_xmp})
            message(STATUS "Dynamic ${xmp_name}: ${dynamic_xmp}")
            target_compile_definitions(SDL3_mixer-static PRIVATE "XMP_DYNAMIC=\"${dynamic_xmp}\"")
            if(SDLMIXER_VENDORED)
                add_dependencies(SDL3_mixer-static ${tgt_xmp})
            endif()
        else()
            target_link_libraries(SDL3_mixer-static PRIVATE ${tgt_xmp})
        endif()
    endif()
endif()

set(SDLMIXER_MOD_ENABLED FALSE)
if(SDLMIXER_MOD_XMP_ENABLED)
    set(SDLMIXER_MOD_ENABLED TRUE)
endif()

list(APPEND SDLMIXER_BACKENDS MP3_DRMP3)
set(SDLMIXER_MP3_DRMP3_ENABLED FALSE)
if(SDLMIXER_MP3_DRMP3)
    set(SDLMIXER_MP3_DRMP3_ENABLED TRUE)
    target_compile_definitions(SDL3_mixer-static PRIVATE MUSIC_MP3_DRMP3)
endif()

list(APPEND SDLMIXER_BACKENDS MP3_MPG123)
set(SDLMIXER_MP3_MPG123_ENABLED FALSE)
if(SDLMIXER_MP3_MPG123)
    if(SDLMIXER_VENDORED)
        set(SDLMIXER_MP3_MPG123_ENABLED TRUE)
        message(STATUS "Using vendored mpg123")
        sdl_check_project_in_subfolder(external/mpg123/ports/cmake mpg123 SDLMIXER_VENDORED)
        set(BUILD_LIBOUT123 FALSE)
        set(BUILD_PROGRAMS OFF)
        set(BUILD_SHARED_LIBS "${SDLMIXER_MP3_MPG123_SHARED}")
        add_subdirectory(external/mpg123/ports/cmake external/libmpg123-build/ports/cmake EXCLUDE_FROM_ALL)
        if(NOT TARGET MPG123::libmpg123)
            add_library(MPG123::libmpg123 ALIAS libmpg123)
        endif()
        if(SDLMIXER_MP3_MPG123_SHARED)
            list(APPEND INSTALL_EXTRA_TARGETS libmpg123)
        endif()
        if(NOT SDLMIXER_MP3_MPG123_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:MPG123::mpg123>)
        endif()
    else()
        find_package(mpg123 ${required})
        if(mpg123_FOUND)
            set(SDLMIXER_MP3_MPG123_ENABLED TRUE)
            message(STATUS "Using system mpg123")
            if(NOT SDLMIXER_MP3_MPG123_SHARED)
                list(APPEND PC_REQUIRES libmpg123)
            endif()
        else()
            message(${fatal_error} "mpg123 NOT found")
        endif()
    endif()
    if(SDLMIXER_MP3_MPG123_ENABLED)
        target_compile_definitions(SDL3_mixer-static PRIVATE MUSIC_MP3_MPG123)
        if(SDLMIXER_MP3_MPG123_SHARED)
            target_include_directories(SDL3_mixer-static PRIVATE
                $<TARGET_PROPERTY:MPG123::libmpg123,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:MPG123::libmpg123,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:MPG123::libmpg123,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_mpg123 MPG123::libmpg123)
            message(STATUS "Dynamic mpg123}: ${dynamic_mpg123}")
            target_compile_definitions(SDL3_mixer-static PRIVATE "MPG123_DYNAMIC=\"${dynamic_mpg123}\"")
            if(SDLMIXER_VENDORED)
                add_dependencies(SDL3_mixer-static MPG123::libmpg123)
            endif()
        else()
            target_link_libraries(SDL3_mixer-static PRIVATE MPG123::libmpg123)
        endif()
    endif()
endif()

set(SDLMIXER_MP3_ENABLED FALSE)
if(SDLMIXER_MP3_DRMP3_ENABLED OR SDLMIXER_MP3_MPG123_ENABLED)
    set(SDLMIXER_MP3_ENABLED TRUE)
endif()

list(APPEND SDLMIXER_BACKENDS MIDI_FLUIDSYNTH)
set(SDLMIXER_MIDI_FLUIDSYNTH_ENABLED FALSE)
if(SDLMIXER_MIDI_FLUIDSYNTH)
    if(SDLMIXER_VENDORED)
        message(STATUS "Using vendored FluidSynth")
        message(${fatal_error} "FluidSynth is not vendored.")
        set(SDLMIXER_MIDI_FLUIDSYNTH_ENABLED FALSE)
    else()
        find_package(FluidSynth ${required})
        if(FluidSynth_FOUND)
            set(SDLMIXER_MIDI_FLUIDSYNTH_ENABLED TRUE)
            message(STATUS "Using system FluidSynth")
            if(NOT SDLMIXER_MIDI_FLUIDSYNTH_SHARED)
                list(APPEND PC_REQUIRES fluidsynth)
            endif()
        else()
            message(${fatal_error} "FluidSynth NOT found")
        endif()
    endif()
    if(SDLMIXER_MIDI_FLUIDSYNTH_ENABLED)
        target_compile_definitions(SDL3_mixer-static PRIVATE MUSIC_MID_FLUIDSYNTH)
        if(SDLMIXER_MIDI_FLUIDSYNTH_SHARED)
            target_include_directories(SDL3_mixer-static PRIVATE
                $<TARGET_PROPERTY:FluidSynth::libfluidsynth,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:FluidSynth::libfluidsynth,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:FluidSynth::libfluidsynth,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_fluidsynth FluidSynth::libfluidsynth)
            message(STATUS "Dynamic fluidsynth: ${dynamic_fluidsynth}")
            target_compile_definitions(SDL3_mixer-static PRIVATE "FLUIDSYNTH_DYNAMIC=\"${dynamic_fluidsynth}\"")
            if(SDLMIXER_VENDORED)
                add_dependencies(SDL3_mixer-static FluidSynth::libfluidsynth)
            endif()
        else()
            target_link_libraries(SDL3_mixer-static PRIVATE FluidSynth::libfluidsynth)
        endif()
    endif()
endif()

list(APPEND SDLMIXER_BACKENDS MIDI_NATIVE)
list(APPEND SDLMIXER_MIDI_NATIVE_ENABLED FALSE)
if(SDLMIXER_MIDI_NATIVE)
    set(midi_common_sources
        src/codecs/native_midi/native_midi_common.c
        src/codecs/native_midi/native_midi_common.h
    )
    if(WIN32)
        list(APPEND SDLMIXER_MIDI_NATIVE_ENABLED TRUE)
        target_sources(SDL3_mixer-static PRIVATE src/codecs/native_midi/native_midi_win32.c ${midi_common_sources})
        target_link_libraries(SDL3_mixer-static PRIVATE winmm)
    elseif(APPLE)
        list(APPEND SDLMIXER_MIDI_NATIVE_ENABLED TRUE)
        target_sources(SDL3_mixer-static PRIVATE src/codecs/native_midi/native_midi_macosx.c ${midi_common_sources})
        target_link_libraries(SDL3_mixer-static PRIVATE -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit -Wl,-framework,CoreServices)
    elseif(HAIKU)
        list(APPEND SDLMIXER_MIDI_NATIVE_ENABLED TRUE)
        enable_language(CXX)
        target_sources(SDL3_mixer-static PRIVATE src/codecs/native_midi/native_midi_haiku.cpp ${midi_common_sources})
        target_link_libraries(SDL3_mixer-static PRIVATE midi)
    elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
        list(APPEND SDLMIXER_MIDI_NATIVE_ENABLED TRUE)
        target_sources(SDL3_mixer-static PRIVATE src/codecs/native_midi/native_midi_linux_alsa.c ${midi_common_sources})
        target_link_libraries(SDL3_mixer-static PRIVATE asound)
    endif()
    if(SDLMIXER_MIDI_NATIVE_ENABLED)
        target_compile_definitions(SDL3_mixer-static PRIVATE MUSIC_MID_NATIVE)
    else()
        message(${fatal_error} "native midi NOT available for current platform")
    endif()
endif()

list(APPEND SDLMIXER_BACKENDS MIDI_TIMIDITY)
set(SDLMIXER_MIDI_TIMIDITY_ENABLED FALSE)
if(SDLMIXER_MIDI_TIMIDITY)
    set(SDLMIXER_MIDI_TIMIDITY_ENABLED TRUE)
    target_compile_definitions(SDL3_mixer-static PRIVATE MUSIC_MID_TIMIDITY)
    target_sources(SDL3_mixer-static PRIVATE
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

set(SDLMIXER_MIDI_ENABLED FALSE)
if(SDLMIXER_MIDI_FLUIDSYNTH_ENABLED OR SDLMIXER_MIDI_NATIVE_ENABLED OR SDLMIXER_MIDI_TIMIDITY_ENABLED)
    set(SDLMIXER_MIDI_ENABLED TRUE)
endif()

list(APPEND SDLMIXER_BACKENDS WAVE)
set(SDLMIXER_WAVE_ENABLED FALSE)
if(SDLMIXER_WAVE)
    set(SDLMIXER_WAVE_ENABLED TRUE)
    target_compile_definitions(SDL3_mixer-static PRIVATE MUSIC_WAV)
endif()

list(APPEND SDLMIXER_BACKENDS WAVPACK)
set(SDLMIXER_WAVPACK_ENABLED FALSE)
if(SDLMIXER_WAVPACK)
    if(SDLMIXER_VENDORED)
        set(SDLMIXER_WAVPACK_ENABLED TRUE)
        message(STATUS "Using vendored WavPack")
        sdl_check_project_in_subfolder(external/wavpack WavPack SDLMIXER_VENDORED)
        set(WAVPACK_ENABLE_THREADS FALSE)
        set(WAVPACK_BUILD_PROGRAMS FALSE)
        set(WAVPACK_BUILD_COOLEDIT_PLUGIN OFF)
        set(WAVPACK_BUILD_WINAMP_PLUGIN OFF)
        set(WAVPACK_BUILD_DOCS OFF)
        set(BUILD_SHARED_LIBS "${SDLMIXER_WAVPACK_SHARED}")
        add_subdirectory(external/wavpack external/wavpack-build EXCLUDE_FROM_ALL)
        if(SDLMIXER_WAVPACK_SHARED)
            list(APPEND INSTALL_EXTRA_TARGETS wavpack)
        endif()
        if(NOT SDLMIXER_WAVPACK_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:WavPack::WavPack>)
        endif()
        target_compile_definitions(SDL3_mixer-static PRIVATE HAVE_WAVPACK_H)
    else()
        find_package(wavpack ${required})
        if(wavpack_FOUND)
            set(SDLMIXER_WAVPACK_ENABLED TRUE)
            message(STATUS "Using system WavPack")
            if(NOT SDLMIXER_WAVPACK_SHARED)
                list(APPEND PC_REQUIRES wavpack)
            endif()
        else()
            message(${fatal_error} "wavpack NOT found")
        endif()
    endif()
    if(SDLMIXER_WAVPACK_ENABLED)
        target_compile_definitions(SDL3_mixer-static PRIVATE MUSIC_WAVPACK)
        if(SDLMIXER_WAVPACK_DSD)
          target_compile_definitions(SDL3_mixer-static PRIVATE MUSIC_WAVPACK_DSD)
        endif()
        if(SDLMIXER_WAVPACK_SHARED)
            target_include_directories(SDL3_mixer-static PRIVATE
                $<TARGET_PROPERTY:WavPack::WavPack,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:WavPack::WavPack,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:WavPack::WavPack,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_wavpack WavPack::WavPack)
            message(STATUS "Dynamic WavPack: ${dynamic_wavpack}")
            target_compile_definitions(SDL3_mixer-static PRIVATE "WAVPACK_DYNAMIC=\"${dynamic_wavpack}\"")
            if(SDLMIXER_VENDORED)
                add_dependencies(SDL3_mixer-static WavPack::WavPack)
            endif()
        else()
            target_link_libraries(SDL3_mixer-static PRIVATE WavPack::WavPack)
        endif()
    endif()
endif()

if(SDLMIXER_INSTALL)
    install(
        TARGETS SDL3_mixer-static
        EXPORT SDL3MixerTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT devel
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT library
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT library
    )
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/include/SDL3_mixer/SDL_mixer.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/SDL3_mixer" COMPONENT devel
    )
    if(BUILD_SHARED_LIBS)
        set(pdbdir "${CMAKE_INSTALL_BINDIR}")
    else()
        set(pdbdir "${CMAKE_INSTALL_LIBDIR}")
    endif()
    if(MSVC)
        SDL_install_pdb("SDL3_mixer-static" "${pdbdir}")
    endif()

    if(INSTALL_EXTRA_TARGETS)
        install(TARGETS ${INSTALL_EXTRA_TARGETS}
            EXPORT SDL3MixerTargets
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT library
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT library
        )
        if(MSVC)
            foreach(tgt IN LISTS INSTALL_EXTRA_TARGETS)
                SDL_install_pdb("${tgt}" "${pdbdir}")
            endforeach()
        endif()
    endif()

    if(WIN32 AND NOT MINGW)
        set(SDLMIXER_INSTALL_CMAKEDIR_ROOT_DEFAULT "cmake")
    else()
        set(SDLMIXER_INSTALL_CMAKEDIR_ROOT_DEFAULT "${CMAKE_INSTALL_LIBDIR}/cmake")
    endif()
    set(SDLMIXER_INSTALL_CMAKEDIR_ROOT "${SDLMIXER_INSTALL_CMAKEDIR_ROOT_DEFAULT}" CACHE STRING "Root folder where to install SDL3_mixerConfig.cmake related files (SDL3_mixer subfolder for non-MSVC projects)")
    set(SDLMIXER_PKGCONFIG_INSTALLDIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

    if(WIN32 AND NOT MINGW)
        set(SDLMIXER_INSTALL_CMAKEDIR "${SDLMIXER_INSTALL_CMAKEDIR_ROOT}")
        set(LICENSES_PREFIX "licenses/SDL3_mixer")
    else()
        set(SDLMIXER_INSTALL_CMAKEDIR "${SDLMIXER_INSTALL_CMAKEDIR_ROOT}/SDL3_mixer")
        set(LICENSES_PREFIX "${CMAKE_INSTALL_DATAROOTDIR}/licenses/SDL3_mixer")
    endif()

    configure_package_config_file(cmake/SDL3_mixerConfig.cmake.in SDL3_mixerConfig.cmake
        NO_SET_AND_CHECK_MACRO
        INSTALL_DESTINATION "${SDLMIXER_INSTALL_CMAKEDIR}"
    )
    write_basic_package_version_file("${PROJECT_BINARY_DIR}/SDL3_mixerConfigVersion.cmake"
        COMPATIBILITY AnyNewerVersion
    )
    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/SDL3_mixerConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/SDL3_mixerConfigVersion.cmake"
          DESTINATION "${SDLMIXER_INSTALL_CMAKEDIR}"
          COMPONENT devel
    )
    if(NOT SDLMIXER_VENDORED)
        install(
            FILES
                cmake/PkgConfigHelper.cmake
                cmake/FindFLAC.cmake
                cmake/FindFluidSynth.cmake
                cmake/Findgme.cmake
                cmake/Findlibxmp.cmake
                cmake/Findlibxmp-lite.cmake
                cmake/FindOgg.cmake
                cmake/FindOpus.cmake
                cmake/FindOpusFile.cmake
                cmake/Findmpg123.cmake
                cmake/FindVorbis.cmake
                cmake/Findtremor.cmake
                cmake/Findwavpack.cmake
                cmake/FindSndFile.cmake
            DESTINATION "${SDLMIXER_INSTALL_CMAKEDIR}"
            COMPONENT devel
        )
    endif()
    install(EXPORT SDL3MixerTargets
        FILE SDL3_mixer-static-targets.cmake
        NAMESPACE SDL3_mixer::
        DESTINATION "${SDLMIXER_INSTALL_CMAKEDIR}"
        COMPONENT devel
    )

    export(TARGETS SDL3_mixer-static ${INSTALL_EXTRA_TARGETS} NAMESPACE "SDL3_mixer::" FILE "SDL3_mixer-static-targets.cmake")

    if(SDLMIXER_RELOCATABLE)
      file(RELATIVE_PATH SDL_PATH_PREFIX_RELATIVE_TO_PKGCONFIG "${CMAKE_INSTALL_PREFIX}/${SDLMIXER_PKGCONFIG_INSTALLDIR}" "${CMAKE_INSTALL_PREFIX}")
      string(REGEX REPLACE "[/]+$" "" SDL_PATH_PREFIX_RELATIVE_TO_PKGCONFIG "${SDL_PATH_PREFIX_RELATIVE_TO_PKGCONFIG}")
      set(SDL_PKGCONFIG_PREFIX "\${pcfiledir}/${SDL_PATH_PREFIX_RELATIVE_TO_PKGCONFIG}")
    else()
      set(SDL_PKGCONFIG_PREFIX "${CMAKE_INSTALL_PREFIX}")
    endif()

    if(IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}")
      set(INCLUDEDIR_FOR_PKG_CONFIG "${CMAKE_INSTALL_INCLUDEDIR}")
    else()
      set(INCLUDEDIR_FOR_PKG_CONFIG "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
    endif()
    if(IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
      set(LIBDIR_FOR_PKG_CONFIG "${CMAKE_INSTALL_LIBDIR}")
    else()
      set(LIBDIR_FOR_PKG_CONFIG "\${prefix}/${CMAKE_INSTALL_LIBDIR}")
    endif()

    string(JOIN " " PC_REQUIRES ${PC_REQUIRES})
    string(JOIN " " PC_LIBS ${PC_LIBS})
    configure_file(cmake/sdl3-mixer.pc.in sdl3-mixer.pc @ONLY)

    # Always install sdl3-mixer.pc file: libraries might be different between config modes
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sdl3-mixer.pc"
        DESTINATION "${SDLMIXER_PKGCONFIG_INSTALLDIR}" COMPONENT devel)

    install(FILES "LICENSE.txt"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/licenses/${PROJECT_NAME}"
        COMPONENT library
    )

    if(SDLMIXER_INSTALL_CPACK)
      if(MSVC)
        set(CPACK_GENERATOR "ZIP")
      else()
        set(CPACK_GENERATOR "TGZ")
      endif()
      configure_file(cmake/CPackProjectConfig.cmake.in CPackProjectConfig.cmake @ONLY)
      set(CPACK_PROJECT_CONFIG_FILE "${PROJECT_BINARY_DIR}/CPackProjectConfig.cmake")
      # CPACK_SOURCE_PACKAGE_FILE_NAME must end with "-src" (so we can block creating a source archive)
      set(CPACK_SOURCE_PACKAGE_FILE_NAME "SDL${PROJECT_VERSION_MAJOR}-${PROJECT_VERSION}-src")
      set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/dist")
      include(CPack)
    endif()

    if(SDLMIXER_INSTALL_MAN)
        sdl_get_git_revision_hash(SDLMIXER_REVISION)
        SDL_generate_manpages(
            HEADERS_DIR "${PROJECT_SOURCE_DIR}/include/SDL3_mixer"
            SYMBOL "Mix_Init"
            WIKIHEADERS_PL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build-scripts/wikiheaders.pl"
            REVISION "${SDLMIXER_REVISION}"
        )
    endif()
endif()

if(SDLMIXER_SAMPLES)
    find_package(SDL3 REQUIRED COMPONENTS SDL3 SDL3_test)

    check_include_file("signal.h" HAVE_SIGNAL_H)
    check_symbol_exists("setbuf" "stdio.h" HAVE_SETBUF)

    function(add_sdl_mixer_example_executable TARGET)
        if(ANDROID)
            add_library(${TARGET} SHARED ${ARGN})
        else()
            add_executable(${TARGET} ${ARGN})
        endif()
        sdl_add_warning_options(${TARGET} WARNING_AS_ERROR ${SDLMIXER_WERROR})
        sdl_target_link_options_no_undefined(${TARGET})
        target_link_libraries(${TARGET} PRIVATE SDL3::SDL3_test)
        target_link_libraries(${TARGET} PRIVATE SDL3_mixer::SDL3_mixer-static)
        target_link_libraries(${TARGET} PRIVATE SDL3::SDL3)
        if(HAVE_SIGNAL_H)
          target_compile_definitions(${TARGET} PRIVATE HAVE_SIGNAL_H)
        endif()
        if(HAVE_SETBUF)
          target_compile_definitions(${TARGET} PRIVATE HAVE_SETBUF)
        endif()

        if(SDLMIXER_SAMPLES_INSTALL)
            install(TARGETS ${TARGET}
                RUNTIME DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}/installed-tests/SDL3_mixer"
            )
        endif()
    endfunction()

    add_sdl_mixer_example_executable(playmus examples/playmus.c)
    add_sdl_mixer_example_executable(playwave examples/playwave.c)
endif()

set(available_deps)
set(unavailable_deps)
foreach(dep IN LISTS SDLMIXER_BACKENDS)
    set(var SDLMIXER_${dep}_ENABLED)
    if(NOT DEFINED ${var})
        message(AUTHOR_WARNING "${var} not defined")
    endif()
    if(${var})
        list(APPEND available_deps ${dep})
    else()
        list(APPEND unavailable_deps ${dep})
    endif()
endforeach()
string(JOIN " " avail_str ${available_deps})
string(TOLOWER "${avail_str}" avail_str)
string(JOIN " " unavail_str ${unavailable_deps})
string(TOLOWER "${unavail_str}" unavail_str)
message(STATUS "SDL3_mixer backends:")
message(STATUS "- enabled:  ${avail_str}")
message(STATUS "- disabled: ${unavail_str}")

set(msg "SDL_mixer sdl2-api-on-3 is provided as an aid in porting SDL2 apps to SDL3 and is deprecated by definition. When you read this message, it means you still need to port to SDL3_mixer.")

message(DEPRECATION "${msg}")
message(WARNING "${msg}")
if(COMMAND cmake_language AND NOT PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  # Make sure the warning is also displayed at the end of configuration.
  cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL message WARNING "${msg}")
endif()

if(TARGET SDL3_mixer-shared)
  message(SEND_ERROR "SDL_mixer sdl2-api-on-3 can only built as a static library!!!")
endif()
