include(FindPackageHandleStandardArgs)

find_library(modplug_LIBRARY
    NAMES modplug
)

find_path(modplug_INCLUDE_PATH
    NAMES libmodplug/modplug.h
)

set(modplug_COMPILE_OPTIONS "" CACHE STRING "Extra compile options of modplug")

set(modplug_LINK_LIBRARIES "" CACHE STRING "Extra link libraries of modplug")

set(modplug_LINK_FLAGS "" CACHE STRING "Extra link flags of modplug")

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
            INTERFACE_LINK_FLAGS "${modplug_LINK_FLAGS}"
        )
    endif()
endif()
