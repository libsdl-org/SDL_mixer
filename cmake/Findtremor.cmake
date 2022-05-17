include(FindPackageHandleStandardArgs)

find_library(tremor_LIBRARY
    NAMES vorbisidec libvorbisidec
)

find_path(tremor_INCLUDE_PATH
    NAMES tremor/ivorbisfile.h
)

set(tremor_COMPILE_OPTIONS "" CACHE STRING "Extra compile options of vorbis")

set(tremor_LINK_LIBRARIES "" CACHE STRING "Extra link libraries of vorbis")

set(tremor_LINK_FLAGS "" CACHE STRING "Extra link flags of vorbis")

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
            INTERFACE_LINK_FLAGS "${tremor_LINK_FLAGS}"
        )
    endif()
endif()
