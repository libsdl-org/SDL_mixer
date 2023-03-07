include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)
pkg_check_modules(PC_TREMOR QUIET vorbisidec)

find_library(tremor_LIBRARY
    NAMES vorbisidec libvorbisidec
    HINTS ${PC_TREMOR_LIBDIR}
)

find_path(tremor_INCLUDE_PATH
    NAMES tremor/ivorbisfile.h
    HINTS ${PC_TREMOR_INCLUDEDIR}
)

if(PC_TREMOR_FOUND)
    get_flags_from_pkg_config("${tremor_LIBRARY}" "PC_TREMOR" "_tremor")
endif()

set(tremor_COMPILE_OPTIONS "${_tremor_compile_options}" CACHE STRING "Extra compile options of vorbis")

set(tremor_LINK_LIBRARIES "${_tremor_link_libraries}" CACHE STRING "Extra link libraries of vorbis")

set(tremor_LINK_OPTIONS "${_tremor_link_options}" CACHE STRING "Extra link flags of vorbis")

set(tremor_LINK_DIRECTORIES "${_tremor_link_directories}" CACHE PATH "Extra link directories of vorbis")

find_package_handle_standard_args(tremor
    REQUIRED_VARS tremor_LIBRARY tremor_INCLUDE_PATH
)

if (tremor_FOUND)
    if (NOT TARGET tremor::tremor)
        add_library(tremor::tremor UNKNOWN IMPORTED)
        set_target_properties(tremor::tremor PROPERTIES
            IMPORTED_LOCATION "${tremor_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${tremor_INCLUDE_PATH}"
            INTERFACE_COMPILE_OPTIONS "${tremor_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${tremor_LINK_LIBRARIES}"
            INTERFACE_LINK_OPTIONS "${tremor_LINK_OPTIONS}"
            INTERFACE_LINK_DIRECTORIES "${tremor_LINK_DIRECTORIES}"
        )
    endif()
endif()
