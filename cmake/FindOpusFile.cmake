include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)
pkg_check_modules(PC_OPUSFILE QUIET opusfile)

find_library(OpusFile_LIBRARY
    NAMES opusfile
    HINTS ${PC_OPUSFILE_LIBDIR}
)

find_path(OpusFile_INCLUDE_PATH
    NAMES opusfile.h
    PATH_SUFFIXES opus
    HINTS ${PC_OPUSFILE_INCLUDEDIR}
)

if(PC_OPUSFILE_FOUND)
    get_flags_from_pkg_config("${OpusFile_LIBRARY}" "PC_OPUSFILE" "_opusfile")
endif()

set(OpusFile_COMPILE_OPTIONS "${_opusfile_compile_options}" CACHE STRING "Extra compile options of opusfile")

set(OpusFile_LINK_LIBRARIES "${_opusfile_link_libraries}" CACHE STRING "Extra link libraries of opusfile")

set(OpusFile_LINK_OPTIONS "${_opusfile_link_options}" CACHE STRING "Extra link flags of opusfile")

set(OpusFile_LINK_DIRECTORIES "${_opusfile_link_directories}" CACHE PATH "Extra link directories of opusfile")

find_package_handle_standard_args(OpusFile
    REQUIRED_VARS OpusFile_LIBRARY OpusFile_INCLUDE_PATH
)

if (OpusFile_FOUND)
    set(OpusFile_dirs ${OpusFile_INCLUDE_PATH})
    if(EXISTS "${OpusFile_INCLUDE_PATH}/opus")
        list(APPEND OpusFile_dirs "${OpusFile_INCLUDE_PATH}/opus")
    endif()
    if (NOT TARGET OpusFile::opusfile)
        add_library(OpusFile::opusfile UNKNOWN IMPORTED)
        set_target_properties(OpusFile::opusfile PROPERTIES
            IMPORTED_LOCATION "${OpusFile_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OpusFile_dirs}"
            INTERFACE_COMPILE_OPTIONS "${OpusFile_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${OpusFile_LINK_LIBRARIES}"
            INTERFACE_LINK_OPTIONS "${OpusFile_LINK_OPTIONS}"
            INTERFACE_LINK_DIRECTORIES "${OpusFile_LINK_DIRECTORIES}"
        )
    endif()
endif()
