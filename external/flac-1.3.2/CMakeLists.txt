cmake_minimum_required(VERSION 3.2)
project(libFLAC VERSION 1.3.2 LANGUAGES C)

include(CheckIncludeFiles)
include(CheckFunctionExists)

include(${CMAKE_CURRENT_SOURCE_DIR}/../audio_codec_common.cmake)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    # Turn on warnings and legacy C/C++ standards to support more compilers
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Wno-pedantic")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-shift-negative-value")
endif()

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4267 /wd4334 /wd4996 /wd4244")
endif()

add_definitions(
    -DHAVE_CONFIG_H
    -DFLAC__NO_DLL
)

set(FLAC_VERSION "${libFLAC_VERSION}")

CHECK_FUNCTION_EXISTS(fseeko HAVE_FSEEKO)
if(HAVE_FSEEKO)
    add_definitions(-DHAVE_FSEEKO)
endif()

if(WIN32)
    CHECK_FUNCTION_EXISTS(_wutime64 HAFE_WUTIME64)
    try_compile(HAFE_WUTIME64_BUILT
                ${CMAKE_BINARY_DIR}/compile_tests
                ${CMAKE_CURRENT_SOURCE_DIR}/config.tests/wutime64/wutime64_test.c
    )
    if(HAFE_WUTIME64 AND HAFE_WUTIME64_BUILT)
        message("TEST: _wutime64 found and compiling!")
        add_definitions(-DHAFE_WUTIME64)
    else()
        message("TEST: _wutime64 DISABLED: found: [${HAFE_WUTIME64}] compiling: [${HAFE_WUTIME64_BUILT}]")
    endif()
endif()

set(FLAC_SRC)

list(APPEND FLAC_SRC
    src/libFLAC/bitmath.c
    src/libFLAC/bitreader.c
    src/libFLAC/bitwriter.c
    src/libFLAC/cpu.c
    src/libFLAC/crc.c
    src/libFLAC/fixed.c
    src/libFLAC/format.c
    src/libFLAC/lpc.c
    src/libFLAC/md5.c
    src/libFLAC/memory.c
    src/libFLAC/metadata_iterators.c
    src/libFLAC/metadata_object.c
    src/libFLAC/ogg_decoder_aspect.c
    src/libFLAC/ogg_encoder_aspect.c
    src/libFLAC/ogg_helper.c
    src/libFLAC/ogg_mapping.c
    src/libFLAC/stream_decoder.c
    src/libFLAC/stream_encoder.c
    src/libFLAC/stream_encoder_framing.c
    src/libFLAC/window.c
)

if(WIN32)
    add_definitions(-DUNICODE -D_UNICODE)
    list(APPEND FLAC_SRC
        src/share/win_utf8_io/win_utf8_io.c
        src/libFLAC/windows_unicode_filenames.c
    )
endif()

add_library(FLAC STATIC
    ${FLAC_SRC}
)

target_include_directories(FLAC PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(FLAC PRIVATE
    -DVERSION="${FLAC_VERSION}"
    -DPACKAGE_VERSION="${FLAC_VERSION}"
)
target_include_directories(FLAC PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../libogg-1.3.2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../libogg-1.3.2/android
)
target_include_directories(FLAC PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include_p
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libFLAC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libFLAC/include
    ${CMAKE_CURRENT_SOURCE_DIR}/android
)

install(TARGETS FLAC
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
        INCLUDES DESTINATION "include")

install(FILES
        include/FLAC/all.h
        include/FLAC/assert.h
        include/FLAC/callback.h
        include/FLAC/export.h
        include/FLAC/format.h
        include/FLAC/metadata.h
        include/FLAC/ordinals.h
        include/FLAC/stream_decoder.h
        include/FLAC/stream_encoder.h
        DESTINATION include/FLAC/)

