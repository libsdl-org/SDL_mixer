include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)
pkg_check_modules(PC_OGG QUIET ogg)

find_library(Ogg_LIBRARY
    NAMES ogg
    HINTS ${PC_OGG_LIBDIR}
)

find_path(Ogg_INCLUDE_PATH
    NAMES ogg/ogg.h
    HINTS ${PC_OGG_INCLUDEDIR}
)

if(PC_OGG_FOUND)
    get_flags_from_pkg_config("${Ogg_LIBRARY}" "PC_OGG" "_ogg")
endif()

set(Ogg_COMPILE_OPTIONS "${_ogg_compile_options}" CACHE STRING "Extra compile options of ogg")

set(Ogg_LINK_LIBRARIES "${_ogg_link_libraries}" CACHE STRING "Extra link libraries of ogg")

set(Ogg_LINK_OPTIONS "${_ogg_link_options}" CACHE STRING "Extra link flags of ogg")

set(Ogg_LINK_DIRECTORIES "${_ogg_link_directories}" CACHE PATH "Extra link directories of ogg")

find_package(Ogg)

find_package_handle_standard_args(Ogg
    REQUIRED_VARS Ogg_LIBRARY Ogg_INCLUDE_PATH Ogg_FOUND
)

if(Ogg_FOUND)
    set(Ogg_dirs ${Ogg_INCLUDE_PATH})
    if(EXISTS "${Ogg_INCLUDE_PATH}/opus")
        list(APPEND Ogg_dirs "${Ogg_INCLUDE_PATH}/opus")
    endif()
    if (NOT TARGET Ogg::Ogg)
        add_library(Ogg::Ogg UNKNOWN IMPORTED)
        set_target_properties(Ogg::Ogg PROPERTIES
            IMPORTED_LOCATION "${Ogg_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${Ogg_dirs}"
            INTERFACE_COMPILE_OPTIONS "${Ogg_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${Ogg_LINK_LIBRARIES}"
            INTERFACE_LINK_OPTIONS "${Ogg_LINK_OPTIONS}"
            INTERFACE_LINK_DIRECTORIES "${Ogg_LINK_DIRECTORIES}"
        )
    endif()
endif()
