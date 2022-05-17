include(FindPackageHandleStandardArgs)

find_library(FLAC_LIBRARY
    NAMES FLAC
)

find_path(FLAC_INCLUDE_PATH
    NAMES FLAC/all.h
)

set(FLAC_COMPILE_OPTIONS "" CACHE STRING "Extra compile options of FLAC")

set(FLAC_LINK_LIBRARIES "" CACHE STRING "Extra link libraries of FLAC")

set(FLAC_LINK_FLAGS "" CACHE STRING "Extra link flags of FLAC")

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
            INTERFACE_LINK_FLAGS "${FLAC_LINK_FLAGS}"
        )
    endif()
endif()
