cmake_minimum_required(VERSION 2.8.11)
project(libvorbisidec C)

include(${CMAKE_CURRENT_SOURCE_DIR}/../audio_codec_common.cmake)

add_definitions(-DHAVE_ALLOCA -DFLOAT_LOOKUP -DINT_LOOKUP)

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
    vorbisfile.c
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libogg-1.3.2/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libogg-1.3.2/android)

target_include_directories(vorbisidec PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

install(TARGETS vorbisidec
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
        INCLUDES DESTINATION "include")

install(FILES
        config_types.h
        ivorbiscodec.h
        ivorbisfile.h
        DESTINATION include/tremor)
