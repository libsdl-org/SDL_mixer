include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)
pkg_check_modules(PC_XMPLITE QUIET libxmp-lite)

find_library(libxmp_lite_LIBRARY
    NAMES xmp-lite libxmp-lite
    HINTS ${PC_XMPLITE_LIBDIR}
)

find_path(libxmp_lite_INCLUDE_PATH
    NAMES xmp.h
    PATH_SUFFIXES libxmp-lite
    HINTS ${PC_XMPLITE_INCLUDEDIR}
)

if(PC_XMPLITE_FOUND)
    get_flags_from_pkg_config("${libxmp_lite_LIBRARY}" "PC_XMPLITE" "_libxmp_lite")
endif()

set(libxmp_lite_COMPILE_OPTIONS "${_libxmp_lite_compile_options}" CACHE STRING "Extra compile options of libxmp_lite")

set(libxmp_lite_LINK_LIBRARIES "${_libxmp_lite_link_libraries}" CACHE STRING "Extra link libraries of libxmp_lite")

set(libxmp_lite_LINK_OPTIONS "${_libxmp_lite_link_options}" CACHE STRING "Extra link flags of libxmp_lite")

set(libxmp_lite_LINK_DIRECTORIES "${_libxmp_lite_link_directories}" CACHE PATH "Extra link directories of libxmp_lite")

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
            INTERFACE_LINK_OPTIONS "${libxmp_lite_LINK_OPTIONS}"
            INTERFACE_LINK_DIRECTORIES "${libxmp_lite_LINK_DIRECTORIES}"
        )
    endif()
endif()
