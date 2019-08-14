cmake_minimum_required(VERSION 3.2)
project(libVorbis VERSION 1.3.6 LANGUAGES C)

include(${CMAKE_CURRENT_SOURCE_DIR}/../audio_codec_common.cmake)

add_definitions(
    -DHAVE_CONFIG_H
)

add_definitions(-DFLOAT_LOOKUP -DINT_LOOKUP)

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4244 /wd4267 /wd4305")
endif()

add_library(vorbisidec STATIC
    block.c
    codebook.c
    floor0.c
    floor1.c
    info.c
    mapping0.c
    mdct.c
    registry.c
    res012.c
    sharedbook.c
    synthesis.c
    window.c
)

add_library(vorbisfile STATIC
    vorbisfile.c
)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libogg-1.3.2/include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libogg-1.3.2/android)
 


target_include_directories(vorbisidec PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(vorbisfile PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

install(TARGETS vorbisidec vorbisfile 
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
        INCLUDES DESTINATION "include")

install(FILES
        include/vorbis/codec.h
        include/vorbis/vorbisfile.h
        DESTINATION include/vorbis)


