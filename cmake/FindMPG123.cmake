include(FindPackageHandleStandardArgs)

find_library(MPG123_LIBRARY
    NAMES mpg123
)

find_path(MPG123_INCLUDE_PATH
    NAMES mpg123.h
)

set(MPG123_COMPILE_OPTIONS "" CACHE STRING "Extra compile options of mpg123")

set(MPG123_LINK_LIBRARIES "" CACHE STRING "Extra link libraries of mpg123")

set(MPG123_LINK_FLAGS "" CACHE STRING "Extra link flags of mpg123")

find_package_handle_standard_args(MPG123
    REQUIRED_VARS MPG123_LIBRARY MPG123_INCLUDE_PATH
)

if (MPG123_FOUND)
    if (NOT TARGET MPG123::mpg123)
        add_library(MPG123::mpg123 UNKNOWN IMPORTED)
        set_target_properties(MPG123::mpg123 PROPERTIES
            IMPORTED_LOCATION "${MPG123_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${MPG123_INCLUDE_PATH}"
            INTERFACE_COMPILE_OPTIONS "${MPG123_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${MPG123_LINK_LIBRARIES}"
            INTERFACE_LINK_FLAGS "${MPG123_LINK_FLAGS}"
        )
    endif()
endif()
