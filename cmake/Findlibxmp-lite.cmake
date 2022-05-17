find_library(libxmp_lite_LIBRARY
    NAMES xmp
)

find_path(libxmp_lite_INCLUDE_PATH
    NAMES xmp.h
)

set(libxmp_lite_COMPILE_OPTIONS "" CACHE STRING "Extra compile options of libxmp_lite")

set(libxmp_lite_LINK_LIBRARIES "" CACHE STRING "Extra link libraries of libxmp_lite")

set(libxmp_lite_LINK_FLAGS "" CACHE STRING "Extra link flags of libxmp_lite")

find_package_handle_standard_args(libxmp_lite
    REQUIRED_VARS libxmp_lite_LIBRARY libxmp_lite_INCLUDE_PATH
)

if(libxmp_lite_FOUND)
    if(NOT TARGET libxmp-lite::libxmp-lite)
        add_library(libxmp-lite::libxmp-lite UNKNOWN IMPORTED)
        set_target_properties(libxmp_lite::libxmp_lite-shared PROPERTIES
            IMPORTED_LOCATION "${libxmp_lite_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${libxmp_lite_INCLUDE_PATH}"
            INTERFACE_COMPILE_OPTIONS "${libxmp_lite_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${libxmp_lite_LINK_LIBRARIES}"
            INTERFACE_LINK_FLAGS "${libxmp_lite_LINK_FLAGS}"
        )
        endif()
    endif()
endif()
