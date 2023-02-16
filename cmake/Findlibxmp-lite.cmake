include(FindPackageHandleStandardArgs)

find_library(libxmp_lite_LIBRARY
    NAMES xmp-lite
)

find_path(libxmp_lite_INCLUDE_PATH
    NAMES xmp.h
    PATH_SUFFIXES libxmp-lite
)

set(libxmp_lite_COMPILE_OPTIONS "" CACHE STRING "Extra compile options of libxmp_lite")

set(libxmp_lite_LINK_LIBRARIES "" CACHE STRING "Extra link libraries of libxmp_lite")

set(libxmp_lite_LINK_FLAGS "" CACHE STRING "Extra link flags of libxmp_lite")

find_package_handle_standard_args(libxmp-lite
    REQUIRED_VARS libxmp_lite_LIBRARY libxmp_lite_INCLUDE_PATH
)

if(libxmp-lite_FOUND)
    if(NOT TARGET libxmp-lite::libxmp-lite)
        add_library(libxmp-lite::libxmp-lite UNKNOWN IMPORTED)
        set_target_properties(libxmp-lite::libxmp-lite PROPERTIES
            IMPORTED_LOCATION "${libxmp_lite_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${libxmp_lite_INCLUDE_PATH}"
            INTERFACE_COMPILE_OPTIONS "${libxmp_lite_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${libxmp_lite_LINK_LIBRARIES}"
            INTERFACE_LINK_FLAGS "${libxmp_lite_LINK_FLAGS}"
        )
    endif()
endif()
