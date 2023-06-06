include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)
pkg_check_modules(PC_FLAC QUIET flac)

find_library(FLAC_LIBRARY
    NAMES FLAC
    HINTS ${PC_FLAC_LIBDIR}
)

find_path(FLAC_INCLUDE_PATH
    NAMES FLAC/all.h
    HINTS ${PC_FLAC_INCLUDEDIR}
)

if(PC_FLAC_FOUND)
    get_flags_from_pkg_config("${FLAC_LIBRARY}" "PC_FLAC" "_flac")
endif()

set(FLAC_COMPILE_OPTIONS "${_flac_compile_options}" CACHE STRING "Extra compile options of FLAC")

set(FLAC_LINK_LIBRARIES "${_flac_link_libraries}" CACHE STRING "Extra link libraries of FLAC")

set(FLAC_LINK_OPTIONS "${_flac_link_options}" CACHE STRING "Extra link flags of FLAC")

set(FLAC_LINK_DIRECTORIES "${_flac_link_directories}" CACHE PATH "Extra link directories of FLAC")

find_package_handle_standard_args(FLAC
    REQUIRED_VARS FLAC_LIBRARY FLAC_INCLUDE_PATH
)

if(FLAC_FOUND)
    if(NOT TARGET FLAC::FLAC)
        add_library(FLAC::FLAC UNKNOWN IMPORTED)
        set_target_properties(FLAC::FLAC PROPERTIES
            IMPORTED_LOCATION "${FLAC_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${FLAC_INCLUDE_PATH}"
            INTERFACE_COMPILE_OPTIONS "${FLAC_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${FLAC_LINK_LIBRARIES}"
            INTERFACE_LINK_OPTIONS "${FLAC_LINK_OPTIONS}"
            INTERFACE_LINK_DIRECTORIES "${FLAC_LINK_DIRECTORIES}"
        )
    endif()
endif()
