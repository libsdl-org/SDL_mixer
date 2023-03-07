include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)
pkg_check_modules(PC_XMP QUIET libxmp)

find_library(libxmp_LIBRARY
    NAMES xmp
    HINTS ${PC_XMP_LIBDIR}
)

find_path(libxmp_INCLUDE_PATH
    NAMES xmp.h
    HINTS ${PC_XMP_INCLUDEDIR}
)

if(PC_XMP_FOUND)
    get_flags_from_pkg_config("${libxmp_LIBRARY}" "PC_XMP" "_libxmp")
endif()

set(libxmp_COMPILE_OPTIONS "${_libxmp_compile_options}" CACHE STRING "Extra compile options of libxmp")

set(libxmp_LINK_LIBRARIES "${_libxmp_link_libraries}" CACHE STRING "Extra link libraries of libxmp")

set(libxmp_LINK_OPTIONS "${_libxmp_link_options}" CACHE STRING "Extra link flags of libxmp")

set(libxmp_LINK_DIRECTORIES "${_libxmp_link_directories}" CACHE PATH "Extra link flags of libxmp")

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
            INTERFACE_LINK_OPTIONS "${libxmp_LINK_OPTIONS}"
            INTERFACE_LINK_DIRECTORIES "${libxmp_LINK_DIRECTORIES}"
        )
    endif()
endif()
