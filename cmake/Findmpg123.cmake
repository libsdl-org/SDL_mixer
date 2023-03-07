include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)
pkg_check_modules(PC_MPG123 QUIET libmpg123)

find_library(mpg123_LIBRARY
    NAMES mpg123
    HINTS ${PC_MPG123_LIBDIR}
)

find_path(mpg123_INCLUDE_PATH
    NAMES mpg123.h
    HINTS ${PC_MPG123_INCLUDEDIR}
)

if(PC_MPG123_FOUND)
    get_flags_from_pkg_config("${mpg123_LIBRARY}" "PC_MPG123" "_mpg123")
endif()

set(mpg123_COMPILE_OPTIONS "${_mpg123_compile_options}" CACHE STRING "Extra compile options of mpg123")

set(mpg123_LINK_LIBRARIES "${_mpg123_link_libraries}" CACHE STRING "Extra link libraries of mpg123")

set(mpg123_LINK_OPTIONS "${_mpg123_link_options}" CACHE STRING "Extra link flags of mpg123")

set(mpg123_LINK_DIRECTORIES "${_mpg123_link_directories}" CACHE PATH "Extra link directories of mpg123")

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
            INTERFACE_LINK_OPTIONS "${mpg123_LINK_OPTIONS}"
            INTERFACE_LINK_DIRECTORIES "${mpg123_LINK_DIRECTORIES}"
        )
    endif()
endif()
