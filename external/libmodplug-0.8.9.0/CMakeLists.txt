cmake_minimum_required(VERSION 3.2)
project(libModPlug VERSION 0.8.9.0 LANGUAGES CXX)

include(CheckFunctionExists)
include(CheckIncludeFile)
include(${CMAKE_CURRENT_SOURCE_DIR}/../audio_codec_common.cmake)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -std=gnu99")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-pedantic -std=gnu++98 -Wno-sequence-point")
    if(NOT APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-but-set-variable")
    endif()
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4100 /wd4244 /wd4267 /wd4996 /wd4702 /wd4706 /wd4456")
endif()

add_definitions(
    #-DMODPLUG_NO_FILESAVE
    #-DNO_MIDIFORMATS
    #-DNO_WAVFORMAT
    -DHAVE_CONFIG_H
    -DHAVE_CMAKE_CONFIG_H
    -DMODPLUG_STATIC
    -DMODPLUG_BUILD=1
    -D_REENTRANT
    -D_USE_MATH_DEFINES
)

foreach(_FN
        setenv putenv
        )
    string(TOUPPER ${_FN} _UPPER)
    set(_HAVEVAR "HAVE_${_UPPER}")
    CHECK_FUNCTION_EXISTS("${_FN}" ${_HAVEVAR})
endforeach()

CHECK_INCLUDE_FILE("malloc.h" HAVE_MALLOC_H)
CHECK_INCLUDE_FILE("memory.h" HAVE_MEMORY_H)
CHECK_INCLUDE_FILE("inttypes.h" HAVE_INTTYPES_H)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/config/)

add_library(modplug STATIC
    src/fastmix.cpp
    src/load_669.cpp
    src/load_abc.cpp
    src/load_amf.cpp
    src/load_ams.cpp
    src/load_dbm.cpp
    src/load_dmf.cpp
    src/load_dsm.cpp
    src/load_far.cpp
    src/load_it.cpp
    # src/load_j2b.cpp
    src/load_mdl.cpp
    src/load_med.cpp
    #src/load_mid.cpp
    src/load_mod.cpp
    src/load_mt2.cpp
    src/load_mtm.cpp
    src/load_okt.cpp
    src/load_pat.cpp
    src/load_psm.cpp
    src/load_ptm.cpp
    src/load_s3m.cpp
    src/load_stm.cpp
    src/load_ult.cpp
    src/load_umx.cpp
    #src/load_wav.cpp
    src/load_xm.cpp
    src/mmcmp.cpp
    src/modplug.cpp
    src/snd_dsp.cpp
    src/snd_flt.cpp
    src/snd_fx.cpp
    src/sndfile.cpp
    src/sndmix.cpp
)

# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include_p)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libogg/include)

target_include_directories(modplug PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(modplug PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libmodplug
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/libmodplug
)

install(TARGETS modplug
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
        INCLUDES DESTINATION "include")

install(FILES
        include/libmodplug/modplug.h
        DESTINATION include/libmodplug/)


