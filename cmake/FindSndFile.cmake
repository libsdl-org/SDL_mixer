include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)
pkg_check_modules(PC_SNDFILE QUIET sndfile)

find_library(SndFile_LIBRARY
    NAMES sndfile sndfile-1
    HINTS ${PC_SNDFILE_LIBDIR}
)

find_path(SndFile_INCLUDE_PATH
    NAMES sndfile.h
    HINTS ${PC_SNDFILE_INCLUDEDIR}
)

if(PC_SNDFILE_FOUND)
    get_flags_from_pkg_config("${SndFile_LIBRARY}" "PC_SNDFILE" "_sndfile")
endif()

set(SndFile_COMPILE_OPTIONS "${_sndfile_compile_options}" CACHE STRING "Extra compile options of libsndfile")

set(SndFile_LINK_LIBRARIES "${_sndfile_link_libraries}" CACHE STRING "Extra link libraries of libsndfile")

set(SndFile_LINK_OPTIONS "${_sndfile_link_options}" CACHE STRING "Extra link flags of libsndfile")

set(SndFile_LINK_DIRECTORIES "${_sndfile_link_directories}" CACHE PATH "Extra link directories of libsndfile")

find_package_handle_standard_args(SndFile
    REQUIRED_VARS SndFile_LIBRARY SndFile_INCLUDE_PATH
)

if(SndFile_FOUND)
    if(NOT TARGET SndFile::sndfile)
        add_library(SndFile::sndfile UNKNOWN IMPORTED)
        set_target_properties(SndFile::sndfile PROPERTIES
            IMPORTED_LOCATION "${SndFile_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SndFile_INCLUDE_PATH}"
            INTERFACE_COMPILE_OPTIONS "${SndFile_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${SndFile_LINK_LIBRARIES}"
            INTERFACE_LINK_OPTIONS "${SndFile_LINK_OPTIONS}"
            INTERFACE_LINK_DIRECTORIES "${SndFile_LINK_DIRECTORIES}"
        )
    endif()
endif()
