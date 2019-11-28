cmake_minimum_required(VERSION 2.8.0)

project(libmodplug)

set(VERSION "0.8.9.0")

option(BUILD_SHARED_LIBS "Build Shared Library (DLL)" OFF)
option(EXPORT_CXX "Export C++ interface from library" ON)
option(MIDIFMT_SUPPORT "Enable midi formats support" OFF)
option(WAV_SUPPORT "Enable wav formats support" OFF)
option(MMCMP_SUPPORT "Enable mmcmp and pp20 decompression support" OFF)

add_definitions(-DMODPLUG_BUILD)
if(MIDIFMT_SUPPORT)
  add_definitions(-DMIDIFMT_SUPPORT)
endif()
if(WAV_SUPPORT)
  add_definitions(-DWAV_SUPPORT)
endif()
if(MMCMP_SUPPORT)
  add_definitions(-DMMCMP_SUPPORT)
endif()
if(NOT EXPORT_CXX)
  add_definitions(-DNO_CXX_EXPORTS)
else()
  set(HEADERS_CXX
      src/libmodplug/it_defs.h
      src/libmodplug/sndfile.h
      src/libmodplug/stdafx.h)
endif()

include (CheckFunctionExists)
include (CheckIncludeFile)
include (CheckCCompilerFlag)
include (CheckCSourceCompiles)
include (TestBigEndian)

TEST_BIG_ENDIAN(WORDS_BIGENDIAN)
if(WORDS_BIGENDIAN)
  add_definitions(-DWORDS_BIGENDIAN=1)
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
  add_definitions(-Wall)
  # check symbol visibility attributes
  set(OLD_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS}")
  if(NOT WIN32 AND NOT CYGWIN)
    set(CMAKE_REQUIRED_FLAGS "${OLD_REQUIRED_FLAGS} -Werror")
    check_c_source_compiles("int foo(void) __attribute__((visibility(\"default\")));
                             int main(void) {return 0;}" HAVE_VISIBILITY_DEFAULT)
    if(HAVE_VISIBILITY_DEFAULT)
      check_c_compiler_flag(-fvisibility=hidden HAVE_VISIBILITY_HIDDEN)
    endif()
  endif()
  set(CMAKE_REQUIRED_FLAGS "-Wl,--no-undefined")
  check_c_compiler_flag("" HAVE_NO_UNDEFINED)
  set(CMAKE_REQUIRED_FLAGS "${OLD_REQUIRED_FLAGS}")
endif()

include_directories(AFTER
  src
  src/libmodplug
  ${PROJECT_BINARY_DIR}
)

if(UNIX AND NOT APPLE)
  find_library(MATH_LIB m)
  if(MATH_LIB)
    set(CMAKE_REQUIRED_LIBRARIES m)
  endif()
endif()

if (WIN32)
  add_definitions(-D_USE_MATH_DEFINES)
  add_definitions(-DNOMINMAX)
endif()

check_include_file("stdint.h" HAVE_STDINT)
if (HAVE_STDINT)
  add_definitions(-DHAVE_STDINT_H)
endif()

check_function_exists("sinf" HAVE_SINF)
if(HAVE_SINF)
  add_definitions(-DHAVE_SINF)
endif()

if (BUILD_SHARED_LIBS)
  set (LIB_TYPE SHARED)
  if (WIN32 OR CYGWIN)
    add_definitions(-DDLL_EXPORT)
  elseif (HAVE_VISIBILITY_HIDDEN)
    add_definitions(-fvisibility=hidden)
    add_definitions("-DSYM_VISIBILITY")
  endif()
else ()
  set(LIB_TYPE STATIC)
  add_definitions(-DMODPLUG_STATIC)
endif()

add_library(modplug ${LIB_TYPE}
  src/libmodplug/it_defs.h
  src/libmodplug/sndfile.h
  src/libmodplug/stdafx.h

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
  src/load_mdl.cpp
  src/load_med.cpp
  src/load_mid.cpp
  src/load_mod.cpp
  src/load_mt2.cpp
  src/load_mtm.cpp
  src/load_okt.cpp
  src/load_pat.cpp
  src/load_pat.h
  src/load_psm.cpp
  src/load_ptm.cpp
  src/load_s3m.cpp
  src/load_stm.cpp
  src/load_ult.cpp
  src/load_umx.cpp
  src/load_wav.cpp
  src/load_xm.cpp
  src/mmcmp.cpp
  src/modplug.cpp
  src/modplug.h
  src/snd_dsp.cpp
  src/snd_flt.cpp
  src/snd_fx.cpp
  src/sndfile.cpp
  src/sndmix.cpp
  src/tables.h
)

if (BUILD_SHARED_LIBS)
  if(APPLE)
    target_link_libraries(modplug -Wl,-undefined,error)
    target_link_libraries(modplug -Wl,-compatibility_version,2.0.0)
    target_link_libraries(modplug -Wl,-current_version,2.0.0)
  else()
    if(HAVE_NO_UNDEFINED)
      target_link_libraries(modplug -Wl,--no-undefined)
    endif()
    set_target_properties(modplug PROPERTIES
                                  VERSION 1.0.0 SOVERSION 1)
  endif()
  if(MATH_LIB)
    target_link_libraries(modplug m)
  endif()
endif()

# install the library:
install(TARGETS modplug DESTINATION lib)

# install the headers:
install(FILES
  src/modplug.h
  ${HEADERS_CXX}

  DESTINATION
  include/libmodplug
)

if (NOT WIN32)
  set(prefix "${CMAKE_INSTALL_PREFIX}")
  set(exec_prefix "${CMAKE_INSTALL_PREFIX}")
  set(libdir "${CMAKE_INSTALL_PREFIX}/lib")
  set(includedir "${CMAKE_INSTALL_PREFIX}/include")
  configure_file(libmodplug.pc.in libmodplug.pc)

  # install pkg-config file:
  install(FILES "${PROJECT_BINARY_DIR}/libmodplug.pc"
    DESTINATION lib/pkgconfig
  )
endif (NOT WIN32)
