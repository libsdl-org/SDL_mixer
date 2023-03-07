include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)
pkg_check_modules(PC_GME QUIET libgme)

find_library(gme_LIBRARY
    NAMES gme
    HINTS ${PC_GME_LIBDIR}
)

find_path(gme_INCLUDE_PATH
    NAMES gme/gme.h
    HINTS ${PC_GME_INCLUDEDIR}
)

if(PC_GME_FOUND)
    get_flags_from_pkg_config("${gme_LIBRARY}" "PC_GME" "_gme")
endif()

set(gme_COMPILE_OPTIONS "${_gme_compile_options}" CACHE STRING "Extra compile options of gme")

set(gme_LINK_LIBRARIES "${_gme_link_libraries}" CACHE STRING "Extra link libraries of gme")

set(gme_LINK_OPTIONS "${_gme_link_options}" CACHE STRING "Extra link flags of gme")

set(gme_LINK_DIRECTORIES "${_gme_link_directories}" CACHE PATH "Extra link directories of gme")

find_package_handle_standard_args(gme
    REQUIRED_VARS gme_LIBRARY gme_INCLUDE_PATH
)

if(gme_FOUND)
    set(gme_dirs ${gme_INCLUDE_PATH})
    if(EXISTS "${gme_INCLUDE_PATH}/gme")
        list(APPEND gme_dirs "${gme_INCLUDE_PATH}/gme")
    endif()
    if(NOT TARGET gme::gme)
        add_library(gme::gme UNKNOWN IMPORTED)
        set_target_properties(gme::gme PROPERTIES
            IMPORTED_LOCATION "${gme_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${gme_dirs}"
            INTERFACE_COMPILE_OPTIONS "${gme_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${gme_LINK_LIBRARIES}"
            INTERFACE_LINK_OPTIONS "${gme_LINK_OPTIONS}"
            INTERFACE_LINK_DIRECTORIES "${gme_LINK_DIRECTORIES}"
        )
    endif()
endif()
