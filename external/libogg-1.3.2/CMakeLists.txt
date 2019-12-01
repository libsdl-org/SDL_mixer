cmake_minimum_required(VERSION 2.8.11)
project(libogg C)

include(${CMAKE_CURRENT_SOURCE_DIR}/../audio_codec_common.cmake)

add_library(ogg STATIC
    src/bitwise.c
    src/framing.c
)

target_include_directories(ogg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(ogg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/android)

install(TARGETS ogg
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
        INCLUDES DESTINATION "include")

install(FILES
        android/ogg/config_types.h
        include/ogg/ogg.h
        include/ogg/os_types.h
        DESTINATION include/ogg)
