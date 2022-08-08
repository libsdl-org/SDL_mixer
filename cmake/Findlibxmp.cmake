include(FindPackageHandleStandardArgs)

find_library(libxmp_LIBRARY
    NAMES xmp
)

find_path(libxmp_INCLUDE_PATH
    NAMES xmp.h
)

set(libxmp_COMPILE_OPTIONS "" CACHE STRING "Extra compile options of libxmp")

set(libxmp_LINK_LIBRARIES "" CACHE STRING "Extra link libraries of libxmp")

set(libxmp_LINK_FLAGS "" CACHE STRING "Extra link flags of libxmp")

find_package_handle_standard_args(libxmp
    REQUIRED_VARS libxmp_LIBRARY libxmp_INCLUDE_PATH
)

if(libxmp_FOUND)
    if(NOT TARGET libxmp::libxmp)
        add_library(libxmp::libxmp UNKNOWN IMPORTED)
        set_target_properties(libxmp::libxmp PROPERTIES
            IMPORTED_LOCATION "${libxmp_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${libxmp_INCLUDE_PATH}"
            INTERFACE_COMPILE_OPTIONS "${libxmp_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${libxmp_LINK_LIBRARIES}"
            INTERFACE_LINK_FLAGS "${libxmp_LINK_FLAGS}"
        )
    endif()
endif()
