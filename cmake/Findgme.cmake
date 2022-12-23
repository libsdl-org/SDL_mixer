include(FindPackageHandleStandardArgs)

find_library(gme_LIBRARY
    NAMES gme
)

set(gme_COMPILE_OPTIONS "" CACHE STRING "Extra compile options of gme")

set(gme_LINK_LIBRARIES "" CACHE STRING "Extra link libraries of gme")

set(gme_LINK_FLAGS "" CACHE STRING "Extra link flags of gme")

find_path(gme_INCLUDE_PATH
    NAMES gme/gme.h
)

find_package_handle_standard_args(gme
    REQUIRED_VARS gme_LIBRARY gme_INCLUDE_PATH
)

if(gme_FOUND)
    set(gme_dirs ${gme_INCLUDE_PATH})
    if(EXISTS "${gme_INCLUDE_PATH}/gme")
        list(APPEND gme_dirs "${gme_INCLUDE_PATH}/gme")
    endif()
    if (NOT TARGET gme::gme)
        add_library(gme::gme UNKNOWN IMPORTED)
        set_target_properties(gme::gme PROPERTIES
            IMPORTED_LOCATION "${gme_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${gme_dirs}"
            INTERFACE_COMPILE_OPTIONS "${gme_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${gme_LINK_LIBRARIES}"
            INTERFACE_LINK_FLAGS "${gme_LINK_FLAGS}"
        )
    endif()
endif()
