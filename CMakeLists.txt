cmake_minimum_required(VERSION 3.16)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# See docs/release_checklist.md
set(MAJOR_VERSION 3)
set(MINOR_VERSION 0)
set(MICRO_VERSION 0)
set(SDL_REQUIRED_VERSION 3.0.0)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the SDL_mixer source code and call cmake from there")
endif()

project(SDL3_mixer
    LANGUAGES C
    VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${MICRO_VERSION}"
)

include("${SDL3_mixer_SOURCE_DIR}/cmake/GetGitRevisionDescription.cmake")
include("${SDL3_mixer_SOURCE_DIR}/cmake/PrivateSdlFunctions.cmake")
include("${SDL3_mixer_SOURCE_DIR}/cmake/sdlmanpages.cmake")
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

option(CMAKE_POSITION_INDEPENDENT_CODE "Build static libraries with -fPIC" ON)
option(BUILD_SHARED_LIBS "Build the library as a shared library" ON)

cmake_dependent_option(SDL3MIXER_INSTALL "Enable SDL3mixer install target" ${SDL3MIXER_ROOTPROJECT} "${sdl3mixer_install_enableable}" OFF)
cmake_dependent_option(SDL3MIXER_INSTALL_MAN "Install man pages for SDL3_mixer" ${SDL3MIXER_ROOTPROJECT} "SDL3MIXER_INSTALL" OFF)
option(SDL3MIXER_DEPS_SHARED "Default value for loading dependencies dynamically" ON)
option(SDL3MIXER_VENDORED "Use vendored third-party libraries" ${vendored_default})
option(SDL3MIXER_WERROR "Treat warnings as errors" OFF)

option(SDL3MIXER_STRICT "Fail when a dependency could not be found" OFF)
set(required "")
set(fatal_error "STATUS")
if(SDL3MIXER_STRICT)
  set(required "REQUIRED")
  set(fatal_error "FATAL_ERROR")
endif()
set(SDL3MIXER_DEPS_FOUND )
set(SDL3MIXER_DEPS_NOT_FOUND )

option(SDL3MIXER_SAMPLES "Build the SDL3_mixer sample program(s)" ${SDL3MIXER_SAMPLES_DEFAULT})
cmake_dependent_option(SDL3MIXER_SAMPLES_INSTALL "Install the SDL3_mixer sample program(s)" OFF "SDL3MIXER_SAMPLES;SDL3MIXER_INSTALL" OFF)

if(UNIX AND NOT APPLE)
    set(sdl3mixer_cmd_default ON)
else()
    set(sdl3mixer_cmd_default OFF)
endif()
option(SDL3MIXER_CMD "Support an external music player" ${sdl3mixer_cmd_default})

option(SDL3MIXER_SNDFILE "Support loading sounds via libsndfile" OFF)
option(SDL3MIXER_SNDFILE_SHARED "Dynamically load libsndfile" "${SDL3MIXER_DEPS_SHARED}")

option(SDL3MIXER_FLAC "Enable FLAC music" ON)

cmake_dependent_option(SDL3MIXER_FLAC_LIBFLAC "Enable FLAC music using libFLAC" OFF SDL3MIXER_FLAC OFF)
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
    message(FATAL_ERROR "MOD support was enabled (SDL3MIXER_MOD) but neither modplug (SDL3MIXER_MOD_MODPLUG) or xmp (SDL3MIXER_MOD_XMP) was enabled.")
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
cmake_dependent_option(SDL3MIXER_WAVPACK_DSD "Enable WavPack DSD music support" OFF SDL3MIXER_WAVPACK OFF)
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

set(sdl_required_components Headers)

if(SDL3MIXER_BUILD_SHARED_LIBS)
    set(sdl3_mixer_target_name SDL3_mixer-shared)
    set(sdl3_target_name SDL3::SDL3-shared)

    list(APPEND sdl_required_components SDL3-shared)
else()
    set(sdl3_mixer_target_name SDL3_mixer-static)
    set(sdl3_target_name SDL3::SDL3)
endif()

if(NOT TARGET SDL3::Headers OR NOT TARGET ${sdl3_target_name})
    find_package(SDL3 ${SDL_REQUIRED_VERSION} REQUIRED COMPONENTS ${sdl_required_components})
endif()

set(BUILD_SHARED_LIBS ${SDL3MIXER_BUILD_SHARED_LIBS})
add_library(${sdl3_mixer_target_name}
    src/codecs/load_aiff.c
    src/codecs/load_voc.c
    src/codecs/load_sndfile.c
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
add_library(SDL3_mixer::${sdl3_mixer_target_name} ALIAS ${sdl3_mixer_target_name})
if(NOT TARGET SDL3_mixer::SDL3_mixer)
    add_library(SDL3_mixer::SDL3_mixer ALIAS ${sdl3_mixer_target_name})
endif()
target_include_directories(${sdl3_mixer_target_name}
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
        src
        src/codecs
)
target_compile_definitions(${sdl3_mixer_target_name} PRIVATE
    BUILD_SDL
    SDL_BUILD_MAJOR_VERSION=${MAJOR_VERSION}
    SDL_BUILD_MINOR_VERSION=${MINOR_VERSION}
    SDL_BUILD_MICRO_VERSION=${MICRO_VERSION}
)
target_link_libraries(${sdl3_mixer_target_name} PUBLIC SDL3::Headers)
if(SDL3MIXER_BUILD_SHARED_LIBS)
    target_link_libraries(${sdl3_mixer_target_name} PRIVATE SDL3::SDL3-shared)
endif()
sdl_add_warning_options(${sdl3_mixer_target_name} WARNING_AS_ERROR ${SDL3MIXER_WERROR})
if(WIN32 AND BUILD_SHARED_LIBS)
    target_sources(${sdl3_mixer_target_name} PRIVATE
        src/version.rc
    )
endif()
set_target_properties(${sdl3_mixer_target_name} PROPERTIES
    OUTPUT_NAME "SDL3_mixer"
    DEFINE_SYMBOL DLL_EXPORT
    EXPORT_NAME ${sdl3_mixer_target_name}
    C_VISIBILITY_PRESET "hidden"
)
if(NOT ANDROID)
    if(APPLE)
        # the SOVERSION property corresponds to the compatibility version and VERSION corresponds to the current version
        # https://cmake.org/cmake/help/latest/prop_tgt/SOVERSION.html#mach-o-versions
        set_target_properties(${sdl3_mixer_target_name} PROPERTIES
            SOVERSION "${DYLIB_COMPAT_VERSION}"
            VERSION "${DYLIB_CURRENT_VERSION}"
        )
    else()
        set_target_properties(${sdl3_mixer_target_name} PROPERTIES
            SOVERSION "${SO_VERSION_MAJOR}"
            VERSION "${SO_VERSION}"
        )
    endif()
endif()
if(SDL3MIXER_BUILD_SHARED_LIBS)
    if(WIN32)
        set_target_properties(${sdl3_mixer_target_name} PROPERTIES
            PREFIX ""
            DLL_NAME_WITH_SOVERSION FALSE
        )
    endif()
else()
    if(MSVC)
        set_target_properties(${sdl3_mixer_target_name} PROPERTIES
            OUTPUT_NAME "SDL3_mixer-static"
        )
    endif()
endif()

# Use `Compatible Interface Properties` to ensure a shared SDL3_mixer is linked to a shared SDL3 library
if(SDL3MIXER_BUILD_SHARED_LIBS)
    set_property(TARGET ${sdl3_mixer_target_name} PROPERTY INTERFACE_SDL3_SHARED ${SDL3MIXER_BUILD_SHARED_LIBS})
    set_property(TARGET ${sdl3_mixer_target_name} APPEND PROPERTY COMPATIBLE_INTERFACE_BOOL SDL3_SHARED)
endif()

if(SDL3MIXER_BUILD_SHARED_LIBS)
    sdl_target_link_options_no_undefined(${sdl3_mixer_target_name})
endif()

sdl_target_link_option_version_file(${sdl3_mixer_target_name} "${CMAKE_CURRENT_SOURCE_DIR}/src/SDL_mixer.sym")

if(SDL3MIXER_BUILD_SHARED_LIBS)
    # Make sure static library dependencies are built with -fPIC when building a shared SDL3_mixer
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

set(INSTALL_EXTRA_TARGETS)
set(PC_LIBS)
set(PC_REQUIRES)
set(SDL3MIXER_BACKENDS)

list(APPEND SDL3MIXER_BACKENDS CMD)
set(SDL3MIXER_CMD_ENABLED FALSE)
if(SDL3MIXER_CMD)
    target_compile_definitions(${sdl3_mixer_target_name} PRIVATE MUSIC_CMD)
    set(fork_found OFF)
    if(NOT fork_found)
        check_symbol_exists(fork unistd.h HAVE_FORK)
        if(HAVE_FORK)
            set(SDL3MIXER_CMD_ENABLED TRUE)
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE HAVE_FORK)
        endif()
    endif()
    if(NOT fork_found)
        check_symbol_exists(vfork unistd.h HAVE_VFORK)
        if(HAVE_VFORK)
            set(SDL3MIXER_CMD_ENABLED TRUE)
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE HAVE_VFORK)
        endif()
    endif()
    if(NOT SDL3MIXER_CMD_ENABLED)
        message(${fatal_error} "Neither fork() nor vfork() or available on this platform. Reconfigure with -DSDL3MIXER_CMD=OFF.")
    endif()
endif()

list(APPEND SDL3MIXER_BACKENDS SNDFILE)
set(SDL3MIXER_SNDFILE_ENABLED FALSE)
if(SDL3MIXER_SNDFILE)
    if(SDL3MIXER_VENDORED)
        message(STATUS "Using vendored libsndfile")
        message(${fatal_error} "libsndfile is not vendored.")
    else()
        find_package(SndFile ${required})
        if(SndFile_FOUND)
            set(SDL3MIXER_SNDFILE_ENABLED TRUE)
            message(STATUS "Using system libsndfile")
            if(NOT SDL3MIXER_SNDFILE_SHARED)
                list(APPEND PC_REQUIRES sndfile)
            endif()
        else()
            message(${fatal_error} "libsndfile NOT found")
        endif()
    endif()
    if(SDL3MIXER_SNDFILE_ENABLED)
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE LOAD_SNDFILE)
        if(SDL3MIXER_SNDFILE_SHARED)
            target_include_directories(${sdl3_mixer_target_name} PRIVATE
                $<TARGET_PROPERTY:SndFile::sndfile,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:SndFile::sndfile,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:SndFile::sndfile,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_sndfile SndFile::sndfile)
            message(STATUS "Dynamic libsndfile: ${dynamic_sndfile}")
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE "SNDFILE_DYNAMIC=\"${dynamic_sndfile}\"")
            if(SDL3MIXER_VENDORED)
                add_dependencies(${sdl3_mixer_target_name} SndFile::sndfile)
            endif()
        else()
            target_link_libraries(${sdl3_mixer_target_name} PRIVATE SndFile::sndfile)
        endif()
    endif()
endif()

if(SDL3MIXER_OGG)
    # libogg is a requirement of libflac, libtremor and libvorbisfile, so only need this library when vendoring
    if(SDL3MIXER_VENDORED)
        message(STATUS "Using vendored libogg")
        set(BUILD_SHARED_LIBS ${SDL3MIXER_OGG_SHARED})
        set(INSTALL_CMAKE_PACKAGE_MODULE FALSE)
        set(BUILD_TESTING OFF)
        sdl_check_project_in_subfolder(external/ogg ogg SDL3MIXER_VENDORED)
        add_subdirectory(external/ogg EXCLUDE_FROM_ALL)
        if(SDL3MIXER_OGG_install)
            list(APPEND INSTALL_EXTRA_TARGETS ogg)
        endif()
    endif()
endif()

list(APPEND SDL3MIXER_BACKENDS OPUS)
set(SDL3MIXER_OPUS_ENABLED FALSE)
if(SDL3MIXER_OPUS)
    if(SDL3MIXER_VENDORED)
        set(SDL3MIXER_OPUS_ENABLED TRUE)
        # vendored libogg already handled
        if(NOT TARGET ogg)
            message(FATAL_ERROR "ogg target not present")
        endif()
        message(STATUS "Using vendored opus")
        set(BUILD_SHARED_LIBS ${SDL3MIXER_OPUS_SHARED})
        set(BUILD_PROGRAMS OFF)
        sdl_check_project_in_subfolder(external/opus opus SDL3MIXER_VENDORED)
        add_subdirectory(external/opus EXCLUDE_FROM_ALL)

        set(OP_DISABLE_DOCS TRUE)
        set(OP_DISABLE_EXAMPLES TRUE)
        set(OP_DISABLE_HTTP TRUE)
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
        target_include_directories(${sdl3_mixer_target_name} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

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
        find_package(OpusFile ${required})
        if(OpusFile_FOUND)
            set(SDL3MIXER_OPUS_ENABLED TRUE)
            message(STATUS "Using system opusfile")
            if(NOT SDL3MIXER_OPUS_SHARED)
                list(APPEND PC_REQUIRES opusfile)
            endif()
        else()
            message(${fatal_error} "opusfile NOT found")
        endif()
    endif()
    if(SDL3MIXER_OPUS_ENABLED)
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE MUSIC_OPUS)
        if(SDL3MIXER_OPUS_SHARED)
            target_include_directories(${sdl3_mixer_target_name} PRIVATE
                $<TARGET_PROPERTY:OpusFile::opusfile,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:OpusFile::opusfile,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:OpusFile::opusfile,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_opusfile OpusFile::opusfile)
            message(STATUS "Dynamic opus (opusfile): ${dynamic_opusfile}")
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE "OPUS_DYNAMIC=\"${dynamic_opusfile}\"")
            if(SDL3MIXER_VENDORED)
                add_dependencies(${sdl3_mixer_target_name} OpusFile::opusfile)
            endif()
        else()
            target_link_libraries(${sdl3_mixer_target_name} PRIVATE OpusFile::opusfile)
        endif()
    endif()
endif()

list(APPEND SDL3MIXER_BACKENDS VORBIS_STB)
set(SDL3MIXER_VORBIS_STB_ENABLED FALSE)
if(SDL3MIXER_VORBIS_STB)
    set(SDL3MIXER_VORBIS_STB_ENABLED TRUE)
    message(STATUS "Enabled ogg music: using stb_vorbis")
    target_compile_definitions(${sdl3_mixer_target_name} PRIVATE MUSIC_OGG)
    target_compile_definitions(${sdl3_mixer_target_name} PRIVATE OGG_USE_STB)
endif()

list(APPEND SDL3MIXER_BACKENDS VORBIS_TREMOR)
set(SDL3MIXER_VORBIS_TREMOR_ENABLED FALSE)
if(SDL3MIXER_VORBIS_TREMOR)
    if(SDL3MIXER_VENDORED)
        set(SDL3MIXER_VORBIS_TREMOR_ENABLED TRUE)
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
        target_include_directories(${sdl3_mixer_target_name} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
        if(SDL3MIXER_VORBIS_TREMOR_SHARED OR NOT SDL3MIXER_BUILD_SHARED_LIBS)
            list(APPEND INSTALL_EXTRA_TARGETS vorbisidec)
        endif()
        if(NOT SDL3MIXER_VORBIS_TREMOR_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:vorbisidec> -l$<TARGET_FILE_BASE_NAME:ogg>)
        endif()
    else()
        message(STATUS "Using system tremor")
        find_package(tremor ${required})
        if(tremor_FOUND)
            if(NOT SDL3MIXER_VORBIS_TREMOR_SHARED)
                list(APPEND PC_REQUIRES tremor)
            endif()
            set(SDL3MIXER_VORBIS_TREMOR_ENABLED TRUE)
        else()
            message(${fatal_error} "tremor NOT found")
        endif()
    endif()
    if(SDL3MIXER_VORBIS_TREMOR_ENABLED)
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE MUSIC_OGG OGG_USE_TREMOR)
        if(SDL3MIXER_VORBIS_TREMOR_SHARED)
            target_include_directories(${sdl3_mixer_target_name} PRIVATE
                $<TARGET_PROPERTY:tremor::tremor,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:tremor::tremor,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:tremor::tremor,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_tremor tremor::tremor)
            message(STATUS "Dynamic vorbis (tremor): ${dynamic_tremor}")
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE "OGG_DYNAMIC=\"${dynamic_tremor}\"")
            if(SDL3MIXER_VENDORED)
                add_dependencies(${sdl3_mixer_target_name} tremor::tremor)
            endif()
        else()
            target_link_libraries(${sdl3_mixer_target_name} PRIVATE tremor::tremor)
        endif()
    endif()
endif()

list(APPEND SDL3MIXER_BACKENDS VORBIS_VORBISFILE)
set(SDL3MIXER_VORBIS_VORBISFILE_ENABLED FALSE)
if(SDL3MIXER_VORBIS_VORBISFILE)
    if(SDL3MIXER_VENDORED)
        set(SDL3MIXER_VORBIS_VORBISFILE_ENABLED TRUE)
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
        find_package(Vorbis ${required})
        if(Vorbis_FOUND)
            set(SDL3MIXER_VORBIS_VORBISFILE_ENABLED TRUE)
            message(STATUS "Using system vorbisfile")
            if(NOT SDL3MIXER_VORBIS_VORBISFILE_SHARED)
                list(APPEND PC_REQUIRES vorbisfile)
            endif()
        else()
            message(${fatal_error} "vorbisfile NOT found")
        endif()
    endif()
    if(SDL3MIXER_VORBIS_VORBISFILE_ENABLED)
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE MUSIC_OGG)
        if(SDL3MIXER_VORBIS_VORBISFILE_SHARED)
            target_include_directories(${sdl3_mixer_target_name} PRIVATE
                $<TARGET_PROPERTY:Vorbis::vorbisfile,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:Vorbis::vorbisfile,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:Vorbis::vorbisfile,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_vorbisfile Vorbis::vorbisfile)
            message(STATUS "Dynamic vorbisfile: ${dynamic_vorbisfile}")
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE "OGG_DYNAMIC=\"${dynamic_vorbisfile}\"")
            if(SDL3MIXER_VENDORED)
                add_dependencies(${sdl3_mixer_target_name} Vorbis::vorbisfile)
            endif()
        else()
            target_link_libraries(${sdl3_mixer_target_name} PRIVATE Vorbis::vorbisfile)
        endif()
    endif()
endif()

set(SDL3MIXER_VORBIS_ENABLED FALSE)
if(SDL3MIXER_VORBIS_STB_ENABLED OR SDL3MIXER_VORBIS_TREMOR_ENABLED OR SDL3MIXER_VORBIS_VORBISFILE_ENABLED)
    set(SDL3MIXER_VORBIS_ENABLED TRUE)
endif()

list(APPEND SDL3MIXER_BACKENDS FLAC_LIBFLAC)
set(SDL3MIXER_FLAC_LIBFLAC_ENABLED FALSE)
if(SDL3MIXER_FLAC_LIBFLAC)
    if(SDL3MIXER_VENDORED)
        set(SDL3MIXER_FLAC_LIBFLAC_ENABLED TRUE)
        # vendored libogg already handled
        if(NOT TARGET ogg)
            message(FATAL_ERROR "ogg target not present")
        endif()
        set(BUILD_SHARED_LIBS "${SDL3MIXER_FLAC_LIBFLAC_SHARED}")
        set(INSTALL_CMAKE_CONFIG_MODULE OFF)
        set(WITH_OGG OFF)
        set(BUILD_CXXLIBS OFF)
        set(BUILD_EXAMPLES OFF)
        set(BUILD_PROGRAMS OFF)
        set(BUILD_TESTING OFF)
        set(INSTALL_MANPAGES OFF)
        message(STATUS "Using vendored libflac")
        sdl_check_project_in_subfolder(external/flac libflac SDL3MIXER_VENDORED)
        add_subdirectory(external/flac EXCLUDE_FROM_ALL)
        if(SDL3MIXER_FLAC_LIBFLAC_SHARED OR NOT SDL3MIXER_BUILD_SHARED_LIBS)
            list(APPEND INSTALL_EXTRA_TARGETS FLAC)
        endif()
        if(NOT SDL3MIXER_FLAC_LIBFLAC_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:FLAC::FLAC> -l$<TARGET_FILE_BASE_NAME:ogg>)
        endif()
    else()
        find_package(FLAC ${required})
        if(FLAC_FOUND)
            set(SDL3MIXER_FLAC_LIBFLAC_ENABLED TRUE)
            message(STATUS "Using system libflac")
            if(NOT SDL3MIXER_FLAC_LIBFLAC_SHARED)
                list(APPEND PC_REQUIRES flac)
            endif()
        else()
            message(${fatal_error} "libflac NOT found")
        endif()
    endif()
    if(SDL3MIXER_FLAC_LIBFLAC_ENABLED)
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE MUSIC_FLAC_LIBFLAC)
        if(SDL3MIXER_FLAC_LIBFLAC_SHARED)
            target_include_directories(${sdl3_mixer_target_name} PRIVATE
                $<TARGET_PROPERTY:FLAC::FLAC,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:FLAC::FLAC,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:FLAC::FLAC,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_flac FLAC::FLAC)
            message(STATUS "Dynamic libflac: ${dynamic_flac}")
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE "FLAC_DYNAMIC=\"${dynamic_flac}\"")
            if(SDL3MIXER_VENDORED)
                add_dependencies(${sdl3_mixer_target_name} FLAC)
            endif()
        else()
            target_link_libraries(${sdl3_mixer_target_name} PRIVATE FLAC::FLAC)
        endif()
    endif()
endif()

list(APPEND SDL3MIXER_BACKENDS FLAC_DRFLAC)
set(SDL3MIXER_FLAC_DRFLAC_ENABLED FALSE)
if(SDL3MIXER_FLAC_DRFLAC)
    set(SDL3MIXER_FLAC_DRFLAC_ENABLED TRUE)
    target_compile_definitions(${sdl3_mixer_target_name} PRIVATE MUSIC_FLAC_DRFLAC)
endif()

set(SDL3MIXER_FLAC_ENABLED FALSE)
if(SDL3MIXER_FLAC_DRFLAC_ENABLED OR SDL3MIXER_FLAC_LIBFLAC_ENABLED)
    set(SDL3MIXER_FLAC_ENABLED TRUE)
endif()

list(APPEND SDL3MIXER_BACKENDS GME)
set(SDL3MIXER_GME_ENABLED FALSE)
if(SDL3MIXER_GME)
    if(SDL3MIXER_VENDORED)
        set(SDL3MIXER_GME_ENABLED TRUE)
        set(BUILD_SHARED_LIBS "${SDL3MIXER_GME_SHARED}")
        set(ENABLE_UBSAN OFF)
        set(BUILD_FRAMEWORK OFF)
        set(GME_ZLIB OFF)
        set(GME_UNRAR OFF)
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
        find_package(gme ${required})
        if(gme_FOUND)
            set(SDL3MIXER_GME_ENABLED TRUE)
            message(STATUS "Using system libgme")
            if(NOT SDL3MIXER_GME_SHARED)
                list(APPEND PC_REQUIRES libgme)
            endif()
        else()
            message(${fatal_error} "libgme NOT found")
        endif()
    endif()
    if(SDL3MIXER_GME_ENABLED)
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE MUSIC_GME)
        if(SDL3MIXER_GME_SHARED)
            target_include_directories(${sdl3_mixer_target_name} PRIVATE
                $<TARGET_PROPERTY:gme::gme,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:gme::gme,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:gme::gme,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_gme gme::gme)
            message(STATUS "Dynamic libgme: ${dynamic_gme}")
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE "GME_DYNAMIC=\"${dynamic_gme}\"")
            if(SDL3MIXER_VENDORED)
                add_dependencies(${sdl3_mixer_target_name} gme::gme)
            endif()
        else()
            target_link_libraries(${sdl3_mixer_target_name} PRIVATE gme::gme)
        endif()
    endif()
endif()

list(APPEND SDL3MIXER_BACKENDS MOD_MODPLUG)
set(SDL3MIXER_MOD_MODPLUG_ENABLED FALSE)
if(SDL3MIXER_MOD_MODPLUG)
    if(SDL3MIXER_VENDORED)
        message(STATUS "Using vendored libmodplug")
        message(${fatal_error} "libmodplug is not vendored.")
    else()
        find_package(modplug ${required})
        if(modplug_FOUND)
            set(SDL3MIXER_MOD_MODPLUG_ENABLED TRUE)
            message(STATUS "Using system libmodplug")
            if(NOT SDL3MIXER_MOD_MODPLUG_SHARED)
                list(APPEND PC_REQUIRES libmodplug)
            endif()
        else()
            message(${fatal_error} "libmodplug NOT found")
        endif()
    endif()
    if(SDL3MIXER_MOD_MODPLUG_ENABLED)
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE MUSIC_MOD_MODPLUG)
        if(SDL3MIXER_MOD_MODPLUG_SHARED)
            target_include_directories(${sdl3_mixer_target_name} PRIVATE
                $<TARGET_PROPERTY:modplug::modplug,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:modplug::modplug,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:modplug::modplug,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_modplug modplug::modplug)
            message(STATUS "Dynamic modplug: ${dynamic_modplug}")
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE "MODPLUG_DYNAMIC=\"${dynamic_modplug}\"")
            if(SDL3MIXER_VENDORED)
                add_dependencies(${sdl3_mixer_target_name} modplug::modplug)
            endif()
        else()
            target_link_libraries(${sdl3_mixer_target_name} PRIVATE modplug::modplug)
        endif()
    endif()
endif()

list(APPEND SDL3MIXER_BACKENDS MOD_XMP)
set(SDL3MIXER_MOD_XMP_ENABLED FALSE)
if(SDL3MIXER_MOD_XMP)
    if(SDL3MIXER_VENDORED)
        set(SDL3MIXER_MOD_XMP_ENABLED TRUE)
        message(STATUS "Using vendored libxmp")
        sdl_check_project_in_subfolder(external/libxmp libxmp SDL3MIXER_VENDORED)
        set(LIBXMP_DISABLE_DEPACKERS ON CACHE BOOL "Disable archive depackers")
        set(LIBXMP_DISABLE_PROWIZARD ON CACHE BOOL "Disable ProWizard format loaders")
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
            find_package(libxmp-lite ${required})
            if(libxmp-lite_FOUND)
                set(SDL3MIXER_MOD_XMP_ENABLED TRUE)
                message(STATUS "Using system libxmp-lite")
                set(tgt_xmp libxmp-lite::libxmp-lite)
                set(xmp_name libxmp-lite)
                if(NOT SDL3MIXER_MOD_XMP_SHARED)
                    list(APPEND PC_REQUIRES libxmp-lite)
                endif()
            else()
                message(${fatal_error} "libxmp-lite NOT found")
            endif()
        else()
            find_package(libxmp ${required})
            if(libxmp_FOUND)
                set(SDL3MIXER_MOD_XMP_ENABLED TRUE)
                message(STATUS "Using system libxmp")
                if(TARGET libxmp::xmp_shared AND SDL3MIXER_MOD_XMP_SHARED)
                  set(tgt_xmp libxmp::xmp_shared)
                elseif(TARGET libxmp::xmp_static)
                  set(tgt_xmp libxmp::xmp_static)
                else()
                  set(tgt_xmp libxmp::libxmp)
                endif()
                set(xmp_name libxmp)
                if(NOT SDL3MIXER_MOD_XMP_SHARED)
                    list(APPEND PC_REQUIRES libxmp)
                endif()
            else()
                message(${fatal_error} "libxmp NOT found")
            endif()
        endif()
    endif()
    if(SDL3MIXER_MOD_XMP_ENABLED)
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE MUSIC_MOD_XMP)
        if(SDL3MIXER_MOD_XMP_SHARED)
            target_include_directories(${sdl3_mixer_target_name} PRIVATE
                $<TARGET_PROPERTY:${tgt_xmp},INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:${tgt_xmp},INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:${tgt_xmp},INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_xmp ${tgt_xmp})
            message(STATUS "Dynamic ${xmp_name}: ${dynamic_xmp}")
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE "XMP_DYNAMIC=\"${dynamic_xmp}\"")
            if(SDL3MIXER_VENDORED)
                add_dependencies(${sdl3_mixer_target_name} ${tgt_xmp})
            endif()
        else()
            target_link_libraries(${sdl3_mixer_target_name} PRIVATE ${tgt_xmp})
        endif()
    endif()
endif()

set(SDL3MIXER_MOD_ENABLED FALSE)
if(SDL3MIXER_MOD_MODPLUG_ENABLED OR SDL3MIXER_MOD_XMP_ENABLED OR SDL3MIXER_MOD_XMP_ENABLED)
    set(SDL3MIXER_MOD_ENABLED TRUE)
endif()

list(APPEND SDL3MIXER_BACKENDS MP3_DRMP3)
set(SDL3MIXER_MP3_DRMP3_ENABLED FALSE)
if(SDL3MIXER_MP3_DRMP3)
    set(SDL3MIXER_MP3_DRMP3_ENABLED TRUE)
    target_compile_definitions(${sdl3_mixer_target_name} PRIVATE MUSIC_MP3_DRMP3)
endif()

list(APPEND SDL3MIXER_BACKENDS MP3_MPG123)
set(SDL3MIXER_MP3_MPG123_ENABLED FALSE)
if(SDL3MIXER_MP3_MPG123)
    if(SDL3MIXER_VENDORED)
        set(SDL3MIXER_MP3_MPG123_ENABLED TRUE)
        message(STATUS "Using vendored mpg123")
        sdl_check_project_in_subfolder(external/mpg123/ports/cmake mpg123 SDL3MIXER_VENDORED)
        set(BUILD_LIBOUT123 FALSE)
        set(BUILD_PROGRAMS OFF)
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
        find_package(mpg123 ${required})
        if(mpg123_FOUND)
            set(SDL3MIXER_MP3_MPG123_ENABLED TRUE)
            message(STATUS "Using system mpg123")
            if(NOT SDL3MIXER_MP3_MPG123_SHARED)
                list(APPEND PC_REQUIRES libmpg123)
            endif()
        else()
            message(${fatal_error} "mpg123 NOT found")
        endif()
    endif()
    if(SDL3MIXER_MP3_MPG123_ENABLED)
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE MUSIC_MP3_MPG123)
        if(SDL3MIXER_MP3_MPG123_SHARED)
            target_include_directories(${sdl3_mixer_target_name} PRIVATE
                $<TARGET_PROPERTY:MPG123::libmpg123,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:MPG123::libmpg123,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:MPG123::libmpg123,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_mpg123 MPG123::libmpg123)
            message(STATUS "Dynamic mpg123}: ${dynamic_mpg123}")
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE "MPG123_DYNAMIC=\"${dynamic_mpg123}\"")
            if(SDL3MIXER_VENDORED)
                add_dependencies(${sdl3_mixer_target_name} MPG123::libmpg123)
            endif()
        else()
            target_link_libraries(${sdl3_mixer_target_name} PRIVATE MPG123::libmpg123)
        endif()
    endif()
endif()

set(SDL3MIXER_MP3_ENABLED FALSE)
if(SDL3MIXER_MP3_DRMP3_ENABLED OR SDL3MIXER_MP3_MPG123_ENABLED)
    set(SDL3MIXER_MP3_ENABLED TRUE)
endif()

list(APPEND SDL3MIXER_BACKENDS MIDI_FLUIDSYNTH)
set(SDL3MIXER_MIDI_FLUIDSYNTH_ENABLED FALSE)
if(SDL3MIXER_MIDI_FLUIDSYNTH)
    if(SDL3MIXER_VENDORED)
        message(STATUS "Using vendored FluidSynth")
        message(${fatal_error} "FluidSynth is not vendored.")
        set(SDL3MIXER_MIDI_FLUIDSYNTH_ENABLED FALSE)
    else()
        find_package(FluidSynth ${required})
        if(FluidSynth_FOUND)
            set(SDL3MIXER_MIDI_FLUIDSYNTH_ENABLED TRUE)
            message(STATUS "Using system FluidSynth")
            if(NOT SDL3MIXER_MIDI_FLUIDSYNTH_SHARED)
                list(APPEND PC_REQUIRES fluidsynth)
            endif()
        else()
            message(${fatal_error} "FluidSynth NOT found")
        endif()
    endif()
    if(SDL3MIXER_MIDI_FLUIDSYNTH_ENABLED)
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE MUSIC_MID_FLUIDSYNTH)
        if(SDL3MIXER_MIDI_FLUIDSYNTH_SHARED)
            target_include_directories(${sdl3_mixer_target_name} PRIVATE
                $<TARGET_PROPERTY:FluidSynth::libfluidsynth,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:FluidSynth::libfluidsynth,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:FluidSynth::libfluidsynth,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_fluidsynth FluidSynth::libfluidsynth)
            message(STATUS "Dynamic fluidsynth: ${dynamic_fluidsynth}")
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE "FLUIDSYNTH_DYNAMIC=\"${dynamic_fluidsynth}\"")
            if(SDL3MIXER_VENDORED)
                add_dependencies(${sdl3_mixer_target_name} FluidSynth::libfluidsynth)
            endif()
        else()
            target_link_libraries(${sdl3_mixer_target_name} PRIVATE FluidSynth::libfluidsynth)
        endif()
    endif()
endif()

list(APPEND SDL3MIXER_BACKENDS MIDI_NATIVE)
list(APPEND SDL3MIXER_MIDI_NATIVE_ENABLED FALSE)
if(SDL3MIXER_MIDI_NATIVE)
    set(midi_common_sources
        src/codecs/native_midi/native_midi_common.c
        src/codecs/native_midi/native_midi_common.h
    )
    if(WIN32)
        list(APPEND SDL3MIXER_MIDI_NATIVE_ENABLED TRUE)
        target_sources(${sdl3_mixer_target_name} PRIVATE src/codecs/native_midi/native_midi_win32.c ${midi_common_sources})
        target_link_libraries(${sdl3_mixer_target_name} PRIVATE winmm)
    elseif(APPLE)
        list(APPEND SDL3MIXER_MIDI_NATIVE_ENABLED TRUE)
        target_sources(${sdl3_mixer_target_name} PRIVATE src/codecs/native_midi/native_midi_macosx.c ${midi_common_sources})
        target_link_libraries(${sdl3_mixer_target_name} PRIVATE -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit -Wl,-framework,CoreServices)
    elseif(HAIKU)
        list(APPEND SDL3MIXER_MIDI_NATIVE_ENABLED TRUE)
        enable_language(CXX)
        target_sources(${sdl3_mixer_target_name} PRIVATE src/codecs/native_midi/native_midi_haiku.cpp ${midi_common_sources})
        target_link_libraries(${sdl3_mixer_target_name} PRIVATE midi)
    endif()
    if(SDL3MIXER_MIDI_NATIVE_ENABLED)
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE MUSIC_MID_NATIVE)
    else()
        message(${fatal_error} "native mididi NOT available for current platform")
    endif()
endif()

list(APPEND SDL3MIXER_BACKENDS MIDI_TIMIDITY)
set(SDL3MIXER_MIDI_TIMIDITY_ENABLED FALSE)
if(SDL3MIXER_MIDI_TIMIDITY)
    set(SDL3MIXER_MIDI_TIMIDITY_ENABLED TRUE)
    target_compile_definitions(${sdl3_mixer_target_name} PRIVATE MUSIC_MID_TIMIDITY)
    target_sources(${sdl3_mixer_target_name} PRIVATE
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

set(SDL3MIXER_MIDI_ENABLED FALSE)
if(SDL3MIXER_MIDI_FLUIDSYNTH_ENABLED OR SDL3MIXER_MIDI_NATIVE_ENABLED OR SDL3MIXER_MIDI_TIMIDITY_ENABLED)
    set(SDL3MIXER_MIDI_ENABLED TRUE)
endif()

list(APPEND SDL3MIXER_BACKENDS WAVE)
set(SDL3MIXER_WAVE_ENABLED FALSE)
if(SDL3MIXER_WAVE)
    set(SDL3MIXER_WAVE_ENABLED TRUE)
    target_compile_definitions(${sdl3_mixer_target_name} PRIVATE MUSIC_WAV)
endif()

list(APPEND SDL3MIXER_BACKENDS WAVPACK)
set(SDL3MIXER_WAVPACK_ENABLED FALSE)
if(SDL3MIXER_WAVPACK)
    if(SDL3MIXER_VENDORED)
        set(SDL3MIXER_WAVPACK_ENABLED TRUE)
        message(STATUS "Using vendored WavPack")
        sdl_check_project_in_subfolder(external/wavpack WavPack SDL3MIXER_VENDORED)
        set(WAVPACK_BUILD_PROGRAMS FALSE)
        set(WAVPACK_BUILD_COOLEDIT_PLUGIN OFF)
        set(WAVPACK_BUILD_WINAMP_PLUGIN OFF)
        set(WAVPACK_BUILD_DOCS OFF)
        set(BUILD_SHARED_LIBS "${SDL3MIXER_WAVPACK_SHARED}")
        add_subdirectory(external/wavpack EXCLUDE_FROM_ALL)
        if(SDL3MIXER_WAVPACK_SHARED OR NOT SDL3MIXER_BUILD_SHARED_LIBS)
            list(APPEND INSTALL_EXTRA_TARGETS wavpack)
        endif()
        if(NOT SDL3MIXER_WAVPACK_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:WavPack::WavPack>)
        endif()
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE HAVE_WAVPACK_H)
    else()
        find_package(wavpack ${required})
        if(wavpack_FOUND)
            set(SDL3MIXER_WAVPACK_ENABLED TRUE)
            message(STATUS "Using system WavPack")
            if(NOT SDL3MIXER_WAVPACK_SHARED)
                list(APPEND PC_REQUIRES wavpack)
            endif()
        else()
            message(${fatal_error} "wavpack NOT found")
        endif()
    endif()
    if(SDL3MIXER_WAVPACK_ENABLED)
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE MUSIC_WAVPACK)
        if(SDL3MIXER_WAVPACK_DSD)
          target_compile_definitions(${sdl3_mixer_target_name} PRIVATE MUSIC_WAVPACK_DSD)
        endif()
        if(SDL3MIXER_WAVPACK_SHARED)
            target_include_directories(${sdl3_mixer_target_name} PRIVATE
                $<TARGET_PROPERTY:WavPack::WavPack,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:WavPack::WavPack,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:WavPack::WavPack,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_wavpack WavPack::WavPack)
            message(STATUS "Dynamic WavPack: ${dynamic_wavpack}")
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE "WAVPACK_DYNAMIC=\"${dynamic_wavpack}\"")
            if(SDL3MIXER_VENDORED)
                add_dependencies(${sdl3_mixer_target_name} WavPack::WavPack)
            endif()
        else()
            target_link_libraries(${sdl3_mixer_target_name} PRIVATE WavPack::WavPack)
        endif()
    endif()
endif()

# Restore BUILD_SHARED_LIBS
set(BUILD_SHARED_LIBS ${SDL3MIXER_BUILD_SHARED_LIBS})

if(SDL3MIXER_INSTALL)
    install(
        TARGETS ${sdl3_mixer_target_name}
        EXPORT SDL3MixerTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT devel
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT library
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT library
    )
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/include/SDL3_mixer/SDL_mixer.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/SDL3_mixer" COMPONENT devel
    )

    if(INSTALL_EXTRA_TARGETS)
        install(TARGETS ${INSTALL_EXTRA_TARGETS}
            EXPORT SDL3MixerTargets
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT library
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT library
        )
    endif()

    if(WIN32 AND NOT MINGW)
        set(SDL3MIXER_INSTALL_CMAKEDIR_ROOT_DEFAULT "cmake")
    else()
        set(SDL3MIXER_INSTALL_CMAKEDIR_ROOT_DEFAULT "${CMAKE_INSTALL_LIBDIR}/cmake")
    endif()
    set(SDL3MIXER_INSTALL_CMAKEDIR_ROOT "${SDL3MIXER_INSTALL_CMAKEDIR_ROOT_DEFAULT}" CACHE STRING "Root folder where to install SDL3_mixerConfig.cmake related files (SDL3_mixer subfolder for non-MSVC projects)")
    set(SDLMIXER_PKGCONFIG_INSTALLDIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

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
            "${CMAKE_CURRENT_LIST_DIR}/cmake/PkgConfigHelper.cmake"
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
            cmake/FindSndFile.cmake
        DESTINATION "${SDL3MIXER_INSTALL_CMAKEDIR}"
        COMPONENT devel
    )
    install(EXPORT SDL3MixerTargets
        FILE ${sdl3_mixer_target_name}-targets.cmake
        NAMESPACE SDL3_mixer::
        DESTINATION "${SDL3MIXER_INSTALL_CMAKEDIR}"
        COMPONENT devel
    )

    file(RELATIVE_PATH SDL_PATH_PREFIX_RELATIVE_TO_PKGCONFIG "${CMAKE_INSTALL_PREFIX}/${SDLMIXER_PKGCONFIG_INSTALLDIR}" "${CMAKE_INSTALL_PREFIX}")
    string(REGEX REPLACE "[/]+$" "" SDL_PATH_PREFIX_RELATIVE_TO_PKGCONFIG "${SDL_PATH_PREFIX_RELATIVE_TO_PKGCONFIG}")
    set(SDL_PKGCONFIG_PREFIX "\${pcfiledir}/${SDL_PATH_PREFIX_RELATIVE_TO_PKGCONFIG}")

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

    if(SDL3MIXER_INSTALL_MAN)
        sdl_get_git_revision_hash(SDL3MIXER_REVISION)
        SDL_generate_manpages(
            HEADERS_DIR "${PROJECT_SOURCE_DIR}/include/SDL3_mixer"
            SYMBOL "Mix_Init"
            WIKIHEADERS_PL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build-scripts/wikiheaders.pl"
            REVISION "${SDL3MIXER_REVISION}"
        )
    endif()
endif()

if(SDL3MIXER_SAMPLES)
    check_include_file("signal.h" HAVE_SIGNAL_H)
    check_symbol_exists("setbuf" "stdio.h" HAVE_SETBUF)

    add_executable(playmus examples/playmus.c)
    add_executable(playwave examples/playwave.c)

    foreach(prog playmus playwave)
        sdl_add_warning_options(${prog} WARNING_AS_ERROR ${SDL3MIXER_WERROR})
        target_link_libraries(${prog} PRIVATE SDL3_mixer::${sdl3_mixer_target_name})
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

set(available_deps)
set(unavailable_deps)
foreach(dep IN LISTS SDL3MIXER_BACKENDS)
    set(var SDL3MIXER_${dep}_ENABLED)
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
