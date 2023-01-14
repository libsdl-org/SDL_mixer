include(FindPackageHandleStandardArgs)

find_library(mpg123_LIBRARY
    NAMES mpg123
)

find_path(mpg123_INCLUDE_PATH
    NAMES mpg123.h
)

set(mpg123_COMPILE_OPTIONS "" CACHE STRING "Extra compile options of mpg123")

set(mpg123_LINK_LIBRARIES "" CACHE STRING "Extra link libraries of mpg123")

set(mpg123_LINK_FLAGS "" CACHE STRING "Extra link flags of mpg123")

find_package_handle_standard_args(mpg123
    REQUIRED_VARS mpg123_LIBRARY mpg123_INCLUDE_PATH
)

if(mpg123_FOUND)
    if(NOT TARGET MPG123::libmpg123)
        add_library(MPG123::libmpg123 UNKNOWN IMPORTED)
        set_target_properties(MPG123::libmpg123 PROPERTIES
            IMPORTED_LOCATION "${mpg123_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${mpg123_INCLUDE_PATH}"
            INTERFACE_COMPILE_OPTIONS "${mpg123_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${mpg123_LINK_LIBRARIES}"
            INTERFACE_LINK_FLAGS "${mpg123_LINK_FLAGS}"
        )
    endif()
endif()
