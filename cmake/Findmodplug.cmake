include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)
pkg_check_modules(PC_MODPLUG QUIET libmodplug)

find_library(modplug_LIBRARY
    NAMES modplug
)

find_path(modplug_INCLUDE_PATH
    NAMES libmodplug/modplug.h
)

if(PC_MODPLUG_FOUND)
    get_flags_from_pkg_config("${modplug_LIBRARY}" "PC_MODPLUG" "_modplug")
endif()

set(modplug_COMPILE_OPTIONS "${_modplug_compile_options}" CACHE STRING "Extra compile options of modplug")

set(modplug_LINK_LIBRARIES "${_modplug_link_libraries}" CACHE STRING "Extra link libraries of modplug")

set(modplug_LINK_OPTIONS "${_modplug_link_options}" CACHE STRING "Extra link flags of modplug")

set(modplug_LINK_DIRECTORIES "${_modplug_link_directories}" CACHE PATH "Extra link directories of modplug")

find_package_handle_standard_args(modplug
    REQUIRED_VARS modplug_LIBRARY modplug_INCLUDE_PATH
)

if (modplug_FOUND)
    if (NOT TARGET modplug::modplug)
        add_library(modplug::modplug UNKNOWN IMPORTED)
        set_target_properties(modplug::modplug PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
            IMPORTED_LOCATION "${modplug_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${modplug_INCLUDE_PATH}"
            INTERFACE_COMPILE_OPTIONS "${modplug_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${modplug_LINK_LIBRARIES}"
            INTERFACE_LINK_OPTIONS "${modplug_LINK_OPTIONS}"
            INTERFACE_LINK_DIRECTORIES "${modplug_LINK_DIRECTORIES}"
        )
    endif()
endif()
