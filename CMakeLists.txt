cmake_minimum_required(VERSION 3.16...4.0)

if(NOT DEFINED CMAKE_BUILD_TYPE)
  set(cmake_build_type_undefined 1)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# See docs/release_checklist.md
set(MAJOR_VERSION 3)
set(MINOR_VERSION 1)
set(MICRO_VERSION 0)
set(SDL_REQUIRED_VERSION 3.3.3)

project(SDL3_mixer
    LANGUAGES C
    VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${MICRO_VERSION}"
)

include("${SDL3_mixer_SOURCE_DIR}/cmake/GetGitRevisionDescription.cmake")
include("${SDL3_mixer_SOURCE_DIR}/cmake/PrivateSdlFunctions.cmake")
include("${SDL3_mixer_SOURCE_DIR}/cmake/sdlcpu.cmake")
include("${SDL3_mixer_SOURCE_DIR}/cmake/sdlmanpages.cmake")
include("${SDL3_mixer_SOURCE_DIR}/cmake/sdlplatform.cmake")
sdl_calculate_derived_version_variables(${MAJOR_VERSION} ${MINOR_VERSION} ${MICRO_VERSION})

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
cmake_dependent_option(BUILD_SHARED_LIBS "Build the library as a shared library" ON PLATFORM_SUPPORTS_SHARED OFF)

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

option(SDLMIXER_TESTS "Build unit tests?" OFF)
option(SDLMIXER_TESTS_INSTALL "Install unit tests?" OFF)

# These are implemented in SDL_mixer itself, not using a third-party library.
option(SDLMIXER_AIFF "Enable AIFF audio" ON)
option(SDLMIXER_WAVE "Enable WAVE audio" ON)
option(SDLMIXER_VOC "Enable VOC audio" ON)
option(SDLMIXER_AU "Enable AU audio" ON)

# Decoders that use third-party dependencies, even if they are compiled in, vendored, etc.
option(SDLMIXER_FLAC_LIBFLAC "Enable FLAC audio using libFLAC" ON)
cmake_dependent_option(SDLMIXER_FLAC_LIBFLAC_SHARED "Dynamically load LIBFLAC" "${SDLMIXER_DEPS_SHARED}" SDLMIXER_FLAC_LIBFLAC OFF)

option(SDLMIXER_FLAC_DRFLAC "Enable FLAC audio using drflac" ON)

option(SDLMIXER_GME "Support loading GME audio via game-music-emu" ON)
option(SDLMIXER_GME_SHARED "Dynamically load libgme" "${SDLMIXER_DEPS_SHARED}")

option(SDLMIXER_MOD_XMP "Support loading MOD audio via libxmp" ON)
cmake_dependent_option(SDLMIXER_MOD_XMP_LITE "Use libxmp-lite instead of libxmp" OFF "SDLMIXER_MOD_XMP;NOT SDLMIXER_VENDORED" OFF)
cmake_dependent_option(SDLMIXER_MOD_XMP_SHARED "Dynamically load libxmp(-lite)" "${SDLMIXER_DEPS_SHARED}" SDLMIXER_MOD_XMP OFF)

option(SDLMIXER_MP3_DRMP3 "Support loading MP3 audio via dr_mp3" ON)
option(SDLMIXER_MP3_MPG123 "Support loading MP3 audio via libmpg123" ON)
cmake_dependent_option(SDLMIXER_MP3_MPG123_SHARED "Dynamically load mpg123" "${SDLMIXER_DEPS_SHARED}" SDLMIXER_MP3_MPG123 OFF)

cmake_dependent_option(SDLMIXER_MIDI_FLUIDSYNTH "Support FluidSynth MIDI output" ON "NOT SDLMIXER_VENDORED" OFF)
cmake_dependent_option(SDLMIXER_MIDI_FLUIDSYNTH_SHARED "Dynamically load libfluidsynth" "${SDLMIXER_DEPS_SHARED}" SDLMIXER_MIDI_FLUIDSYNTH OFF)

option(SDLMIXER_MIDI_TIMIDITY "Support timidity MIDI output" ON)

option(SDLMIXER_OPUS "Enable Opus audio via libopus" ON)
cmake_dependent_option(SDLMIXER_OPUS_SHARED "Dynamically load libopus" "${SDLMIXER_DEPS_SHARED}" SDLMIXER_OPUS OFF)

option(SDLMIXER_VORBIS_STB "Enable Ogg Vorbis audio via stb_vorbis" ON)
option(SDLMIXER_VORBIS_VORBISFILE "Enable Ogg Vorbis audio via libvorbisfile" ON)
option(SDLMIXER_VORBIS_TREMOR "Enable Ogg Vorbis audio via libvorbisidec ('tremor')" OFF)

if (SDLMIXER_VORBIS_VORBISFILE AND SDLMIXER_VORBIS_TREMOR)
    message(FATAL_ERROR "Both SDLMIXER_VORBIS_VORBISFILE and SDLMIXER_VORBIS_TREMOR are both enabled, but only one can be used")
endif()

cmake_dependent_option(SDLMIXER_VORBIS_VORBISFILE_SHARED "Dynamically load vorbisfile library" "${SDLMIXER_DEPS_SHARED}" SDLMIXER_VORBIS_VORBISFILE OFF)
cmake_dependent_option(SDLMIXER_VORBIS_TREMOR_SHARED "Dynamically load vorbisidec ('tremor') library" "${SDLMIXER_DEPS_SHARED}" SDLMIXER_VORBIS_TREMOR OFF)

option(SDLMIXER_WAVPACK "Enable WavPack audio" ON)
cmake_dependent_option(SDLMIXER_WAVPACK_DSD "Enable WavPack DSD audio support" OFF SDLMIXER_WAVPACK OFF)
cmake_dependent_option(SDLMIXER_WAVPACK_SHARED "Dynamically load WavPack library" "${SDLMIXER_DEPS_SHARED}" SDLMIXER_WAVPACK OFF)

if(SDLMIXER_VORBIS_TREMOR OR SDLMIXER_VORBIS_VORBISFILE OR SDLMIXER_FLAC_LIBFLAC OR SDLMIXER_OPUS)
    set(SDLMIXER_OGG TRUE)
    set(SDLMIXER_OGG_install FALSE)
    if(SDLMIXER_VORBIS_VORBISFILE_SHARED OR SDLMIXER_FLAC_SHARED OR SDLMIXER_OPUS_SHARED)
        set(SDLMIXER_OGG_SHARED TRUE)
        set(SDLMIXER_OGG_install TRUE)
    else()
        set(SDLMIXER_OGG_SHARED FALSE)
        if(NOT SDLMIXER_BUILD_SHARED_LIBS)
            set(SDLMIXER_OGG_install TRUE)
        endif()
    endif()
else()
    set(SDLMIXER_OGG FALSE)
endif()

# Save BUILD_SHARED_LIBS variable
set(SDLMIXER_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

set(sdl_required_components Headers)

if(SDLMIXER_BUILD_SHARED_LIBS)
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

SDL_DetectTargetCPUArchitectures(SDL_CPU_NAMES)
SDL_DetectCMakePlatform()

set(BUILD_SHARED_LIBS ${SDLMIXER_BUILD_SHARED_LIBS})
add_library(${sdl3_mixer_target_name}
    src/SDL_mixer.c
    src/SDL_mixer_metadata_tags.c
    src/SDL_mixer_spatialization.c
    src/decoder_aiff.c
    src/decoder_au.c
    src/decoder_drflac.c
    src/decoder_drmp3.c
    src/decoder_flac.c
    src/decoder_fluidsynth.c
    src/decoder_gme.c
    src/decoder_mpg123.c
    src/decoder_opus.c
    src/decoder_raw.c
    src/decoder_sinewave.c
    src/decoder_stb_vorbis.c
    src/decoder_timidity.c
    src/decoder_voc.c
    src/decoder_vorbis.c
    src/decoder_wav.c
    src/decoder_wavpack.c
    src/decoder_xmp.c
)
add_library(SDL3_mixer::${sdl3_mixer_target_name} ALIAS ${sdl3_mixer_target_name})
if(NOT TARGET SDL3_mixer::SDL3_mixer)
    add_library(SDL3_mixer::SDL3_mixer ALIAS ${sdl3_mixer_target_name})
endif()
if("c_std_99" IN_LIST CMAKE_C_COMPILE_FEATURES)
    target_compile_features(${sdl3_mixer_target_name} PRIVATE c_std_99)
else()
    message(WARNING "target_compile_features does not know c_std_99 for C compiler")
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
# Make sure SDL3's include directory comes first
target_include_directories(${sdl3_mixer_target_name} PRIVATE $<TARGET_PROPERTY:SDL3::Headers,INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(${sdl3_mixer_target_name} PUBLIC SDL3::Headers)
if(SDLMIXER_BUILD_SHARED_LIBS)
    target_link_libraries(${sdl3_mixer_target_name} PRIVATE SDL3::SDL3-shared)
endif()
sdl_add_warning_options(${sdl3_mixer_target_name} WARNING_AS_ERROR ${SDLMIXER_WERROR})
if(WIN32 AND BUILD_SHARED_LIBS)
    target_sources(${sdl3_mixer_target_name} PRIVATE
        src/version.rc
    )
    if(MINGW)
        target_link_options(${sdl3_mixer_target_name} PRIVATE -static-libgcc)
    endif()
endif()
set_target_properties(${sdl3_mixer_target_name} PROPERTIES
    OUTPUT_NAME "SDL3_mixer"
    DEFINE_SYMBOL DLL_EXPORT
    EXPORT_NAME ${sdl3_mixer_target_name}
    C_VISIBILITY_PRESET "hidden"
)

sdl_target_link_option_version_file(${sdl3_mixer_target_name} "${CMAKE_CURRENT_SOURCE_DIR}/src/SDL_mixer.sym")

if(NOT ANDROID)
    set_target_properties(${sdl3_mixer_target_name} PROPERTIES
        SOVERSION "${SO_VERSION_MAJOR}"
        VERSION "${SO_VERSION}"
    )
    if(APPLE)
        set_target_properties(${sdl3_mixer_target_name} PROPERTIES
            MACHO_COMPATIBILITY_VERSION "${DYLIB_COMPAT_VERSION}"
            MACHO_CURRENT_VERSION "${DYLIB_CURRENT_VERSION}"
        )
        set_property(TARGET ${sdl3_mixer_target_name} APPEND PROPERTY LINK_DEPENDS
                "${PROJECT_SOURCE_DIR}/src/SDL_mixer.exports")
        target_link_options(${sdl3_mixer_target_name} PRIVATE
                "SHELL:-Wl,-exported_symbols_list,${PROJECT_SOURCE_DIR}/src/SDL_mixer.exports")
    endif()
endif()
if(SDLMIXER_BUILD_SHARED_LIBS)
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
if(SDLMIXER_BUILD_SHARED_LIBS)
    set_property(TARGET ${sdl3_mixer_target_name} PROPERTY INTERFACE_SDL3_SHARED ${SDLMIXER_BUILD_SHARED_LIBS})
    set_property(TARGET ${sdl3_mixer_target_name} APPEND PROPERTY COMPATIBLE_INTERFACE_BOOL SDL3_SHARED)
endif()

if(SDLMIXER_BUILD_SHARED_LIBS)
    sdl_target_link_options_no_undefined(${sdl3_mixer_target_name})
endif()

if(SDLMIXER_BUILD_SHARED_LIBS)
    # Make sure static library dependencies are built with -fPIC when building a shared SDL3_mixer
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

set(INSTALL_EXTRA_TARGETS)
set(PC_LIBS)
set(PC_REQUIRES)
set(SDLMIXER_BACKENDS)

if(SDLMIXER_OGG)
    # libogg is a requirement of libflac, libvorbisidec and libvorbisfile, so only need this library when vendoring
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
        target_include_directories(${sdl3_mixer_target_name} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

        if(NOT TARGET OpusFile::opusfile)
            add_library(OpusFile::opusfile ALIAS opusfile)
        endif()
        if(SDLMIXER_OPUS_SHARED OR NOT SDLMIXER_BUILD_SHARED_LIBS)
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
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE DECODER_OPUS)
        if(SDLMIXER_OPUS_SHARED)
            target_include_directories(${sdl3_mixer_target_name} PRIVATE
                $<TARGET_PROPERTY:OpusFile::opusfile,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:OpusFile::opusfile,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:OpusFile::opusfile,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_opusfile OpusFile::opusfile)
            message(STATUS "Dynamic opus (opusfile): ${dynamic_opusfile}")
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE "OPUS_DYNAMIC=\"${dynamic_opusfile}\"")
            if(SDLMIXER_VENDORED)
                add_dependencies(${sdl3_mixer_target_name} OpusFile::opusfile)
            endif()
        else()
            target_link_libraries(${sdl3_mixer_target_name} PRIVATE OpusFile::opusfile)
        endif()
    endif()
endif()

list(APPEND SDLMIXER_BACKENDS VORBIS_STB)
set(SDLMIXER_VORBIS_STB_ENABLED FALSE)
if(SDLMIXER_VORBIS_STB)
    set(SDLMIXER_VORBIS_STB_ENABLED TRUE)
    target_compile_definitions(${sdl3_mixer_target_name} PRIVATE DECODER_OGGVORBIS_STB)
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
        target_include_directories(${sdl3_mixer_target_name} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
        if(SDLMIXER_VORBIS_TREMOR_SHARED OR NOT SDLMIXER_BUILD_SHARED_LIBS)
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
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE DECODER_OGGVORBIS_VORBISFILE VORBIS_USE_TREMOR)
        if(SDLMIXER_VORBIS_TREMOR_SHARED)
            target_include_directories(${sdl3_mixer_target_name} PRIVATE
                $<TARGET_PROPERTY:tremor::tremor,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:tremor::tremor,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:tremor::tremor,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_tremor tremor::tremor)
            message(STATUS "Dynamic vorbis (tremor): ${dynamic_tremor}")
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE "VORBIS_DYNAMIC=\"${dynamic_tremor}\"")
            if(SDLMIXER_VENDORED)
                add_dependencies(${sdl3_mixer_target_name} tremor::tremor)
            endif()
        else()
            target_link_libraries(${sdl3_mixer_target_name} PRIVATE tremor::tremor)
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
        if(SDLMIXER_VORBIS_VORBISFILE_SHARED OR NOT SDLMIXER_BUILD_SHARED_LIBS)
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
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE DECODER_OGGVORBIS_VORBISFILE)
        if(SDLMIXER_VORBIS_VORBISFILE_SHARED)
            target_include_directories(${sdl3_mixer_target_name} PRIVATE
                $<TARGET_PROPERTY:Vorbis::vorbisfile,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:Vorbis::vorbisfile,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:Vorbis::vorbisfile,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_vorbisfile Vorbis::vorbisfile)
            message(STATUS "Dynamic vorbisfile: ${dynamic_vorbisfile}")
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE "VORBIS_DYNAMIC=\"${dynamic_vorbisfile}\"")
            if(SDLMIXER_VENDORED)
                add_dependencies(${sdl3_mixer_target_name} Vorbis::vorbisfile)
            endif()
        else()
            target_link_libraries(${sdl3_mixer_target_name} PRIVATE Vorbis::vorbisfile)
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
        if(SDLMIXER_FLAC_LIBFLAC_SHARED OR NOT SDLMIXER_BUILD_SHARED_LIBS)
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
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE DECODER_FLAC_LIBFLAC)
        if(SDLMIXER_FLAC_LIBFLAC_SHARED)
            target_include_directories(${sdl3_mixer_target_name} PRIVATE
                $<TARGET_PROPERTY:FLAC::FLAC,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:FLAC::FLAC,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:FLAC::FLAC,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_flac FLAC::FLAC)
            message(STATUS "Dynamic libflac: ${dynamic_flac}")
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE "FLAC_DYNAMIC=\"${dynamic_flac}\"")
            if(SDLMIXER_VENDORED)
                add_dependencies(${sdl3_mixer_target_name} FLAC)
            endif()
        else()
            target_link_libraries(${sdl3_mixer_target_name} PRIVATE FLAC::FLAC)
        endif()
    endif()
endif()

list(APPEND SDLMIXER_BACKENDS FLAC_DRFLAC)
set(SDLMIXER_FLAC_DRFLAC_ENABLED FALSE)
if(SDLMIXER_FLAC_DRFLAC)
    set(SDLMIXER_FLAC_DRFLAC_ENABLED TRUE)
    target_compile_definitions(${sdl3_mixer_target_name} PRIVATE DECODER_FLAC_DRFLAC)
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
            set(targets_gme gme_shared)
        else()
            set(GME_BUILD_STATIC ON)
            set(targets_gme gme_static gme_deps)
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
        if(SDLMIXER_GME_SHARED OR NOT SDLMIXER_BUILD_SHARED_LIBS)
            list(APPEND INSTALL_EXTRA_TARGETS ${targets_gme})
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
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE DECODER_GME)
        if(SDLMIXER_GME_SHARED)
            target_include_directories(${sdl3_mixer_target_name} PRIVATE
                $<TARGET_PROPERTY:gme::gme,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:gme::gme,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:gme::gme,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_gme gme::gme)
            message(STATUS "Dynamic libgme: ${dynamic_gme}")
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE "GME_DYNAMIC=\"${dynamic_gme}\"")
            if(SDLMIXER_VENDORED)
                add_dependencies(${sdl3_mixer_target_name} gme::gme)
            endif()
        else()
            target_link_libraries(${sdl3_mixer_target_name} PRIVATE gme::gme)
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
        if(SDLMIXER_MOD_XMP_SHARED OR NOT SDLMIXER_BUILD_SHARED_LIBS)
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
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE DECODER_MOD_XMP)
        if(SDLMIXER_MOD_XMP_SHARED)
            target_include_directories(${sdl3_mixer_target_name} PRIVATE
                $<TARGET_PROPERTY:${tgt_xmp},INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:${tgt_xmp},INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:${tgt_xmp},INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_xmp ${tgt_xmp})
            message(STATUS "Dynamic ${xmp_name}: ${dynamic_xmp}")
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE "XMP_DYNAMIC=\"${dynamic_xmp}\"")
            if(SDLMIXER_VENDORED)
                add_dependencies(${sdl3_mixer_target_name} ${tgt_xmp})
            endif()
        else()
            target_link_libraries(${sdl3_mixer_target_name} PRIVATE ${tgt_xmp})
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
    target_compile_definitions(${sdl3_mixer_target_name} PRIVATE DECODER_MP3_DRMP3)
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
        if(SDLMIXER_MP3_MPG123_SHARED OR NOT SDLMIXER_BUILD_SHARED_LIBS)
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
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE DECODER_MP3_MPG123)
        if(SDLMIXER_MP3_MPG123_SHARED)
            target_include_directories(${sdl3_mixer_target_name} PRIVATE
                $<TARGET_PROPERTY:MPG123::libmpg123,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:MPG123::libmpg123,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:MPG123::libmpg123,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_mpg123 MPG123::libmpg123)
            message(STATUS "Dynamic mpg123}: ${dynamic_mpg123}")
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE "MPG123_DYNAMIC=\"${dynamic_mpg123}\"")
            if(SDLMIXER_VENDORED)
                add_dependencies(${sdl3_mixer_target_name} MPG123::libmpg123)
            endif()
        else()
            target_link_libraries(${sdl3_mixer_target_name} PRIVATE MPG123::libmpg123)
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
        find_package(FluidSynth "2.2.0" ${required})
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
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE DECODER_MIDI_FLUIDSYNTH)
        if(SDLMIXER_MIDI_FLUIDSYNTH_SHARED)
            target_include_directories(${sdl3_mixer_target_name} PRIVATE
                $<TARGET_PROPERTY:FluidSynth::libfluidsynth,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:FluidSynth::libfluidsynth,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:FluidSynth::libfluidsynth,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_fluidsynth FluidSynth::libfluidsynth)
            message(STATUS "Dynamic fluidsynth: ${dynamic_fluidsynth}")
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE "FLUIDSYNTH_DYNAMIC=\"${dynamic_fluidsynth}\"")
            if(SDLMIXER_VENDORED)
                add_dependencies(${sdl3_mixer_target_name} FluidSynth::libfluidsynth)
            endif()
        else()
            target_link_libraries(${sdl3_mixer_target_name} PRIVATE FluidSynth::libfluidsynth)
        endif()
    endif()
endif()

list(APPEND SDLMIXER_BACKENDS MIDI_TIMIDITY)
set(SDLMIXER_MIDI_TIMIDITY_ENABLED FALSE)
if(SDLMIXER_MIDI_TIMIDITY)
    set(SDLMIXER_MIDI_TIMIDITY_ENABLED TRUE)
    target_compile_definitions(${sdl3_mixer_target_name} PRIVATE DECODER_MIDI_TIMIDITY)
    target_sources(${sdl3_mixer_target_name} PRIVATE
        src/timidity/common.c
        src/timidity/instrum.c
        src/timidity/mix.c
        src/timidity/output.c
        src/timidity/playmidi.c
        src/timidity/readmidi.c
        src/timidity/resample.c
        src/timidity/tables.c
        src/timidity/timidity.c
    )
endif()

list(APPEND SDLMIXER_BACKENDS WAVE)
set(SDLMIXER_WAVE_ENABLED FALSE)
if(SDLMIXER_WAVE)
    set(SDLMIXER_WAVE_ENABLED TRUE)
    target_compile_definitions(${sdl3_mixer_target_name} PRIVATE DECODER_WAV)
endif()

list(APPEND SDLMIXER_BACKENDS AIFF)
set(SDLMIXER_AIFF_ENABLED FALSE)
if(SDLMIXER_AIFF)
    set(SDLMIXER_AIFF_ENABLED TRUE)
    target_compile_definitions(${sdl3_mixer_target_name} PRIVATE DECODER_AIFF)
endif()

list(APPEND SDLMIXER_BACKENDS VOC)
set(SDLMIXER_VOC_ENABLED FALSE)
if(SDLMIXER_VOC)
    set(SDLMIXER_VOC_ENABLED TRUE)
    target_compile_definitions(${sdl3_mixer_target_name} PRIVATE DECODER_VOC)
endif()

list(APPEND SDLMIXER_BACKENDS AU)
set(SDLMIXER_AU_ENABLED FALSE)
if(SDLMIXER_AU)
    set(SDLMIXER_AU_ENABLED TRUE)
    target_compile_definitions(${sdl3_mixer_target_name} PRIVATE DECODER_AU)
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
        if(SDLMIXER_WAVPACK_SHARED OR NOT SDLMIXER_BUILD_SHARED_LIBS)
            list(APPEND INSTALL_EXTRA_TARGETS wavpack)
        endif()
        if(NOT SDLMIXER_WAVPACK_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:WavPack::WavPack>)
        endif()
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE HAVE_WAVPACK_H)
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
        target_compile_definitions(${sdl3_mixer_target_name} PRIVATE DECODER_WAVPACK)
        if(SDLMIXER_WAVPACK_DSD)
          target_compile_definitions(${sdl3_mixer_target_name} PRIVATE DECODER_WAVPACK_DSD)
        endif()
        if(SDLMIXER_WAVPACK_SHARED)
            target_include_directories(${sdl3_mixer_target_name} PRIVATE
                $<TARGET_PROPERTY:WavPack::WavPack,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:WavPack::WavPack,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:WavPack::WavPack,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_wavpack WavPack::WavPack)
            message(STATUS "Dynamic WavPack: ${dynamic_wavpack}")
            target_compile_definitions(${sdl3_mixer_target_name} PRIVATE "WAVPACK_DYNAMIC=\"${dynamic_wavpack}\"")
            if(SDLMIXER_VENDORED)
                add_dependencies(${sdl3_mixer_target_name} WavPack::WavPack)
            endif()
        else()
            target_link_libraries(${sdl3_mixer_target_name} PRIVATE WavPack::WavPack)
        endif()
    endif()
endif()

# Restore BUILD_SHARED_LIBS
set(BUILD_SHARED_LIBS ${SDLMIXER_BUILD_SHARED_LIBS})

if(SDLMIXER_INSTALL)
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
    if(BUILD_SHARED_LIBS)
        set(pdbdir "${CMAKE_INSTALL_BINDIR}")
    else()
        set(pdbdir "${CMAKE_INSTALL_LIBDIR}")
    endif()
    if(MSVC)
        SDL_install_pdb("${sdl3_mixer_target_name}" "${pdbdir}")
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
            DESTINATION "${SDLMIXER_INSTALL_CMAKEDIR}"
            COMPONENT devel
        )
    endif()
    install(EXPORT SDL3MixerTargets
        FILE ${sdl3_mixer_target_name}-targets.cmake
        NAMESPACE SDL3_mixer::
        DESTINATION "${SDLMIXER_INSTALL_CMAKEDIR}"
        COMPONENT devel
    )

    export(TARGETS ${sdl3_mixer_target_name} ${INSTALL_EXTRA_TARGETS} NAMESPACE "SDL3_mixer::" FILE "${sdl3_mixer_target_name}-targets.cmake")

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

if(SDLMIXER_TESTS)
  add_subdirectory(test)
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
