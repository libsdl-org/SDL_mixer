include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)
pkg_check_modules(PC_OPUS QUIET opus)

find_library(Opus_LIBRARY
    NAMES opus
    HINTS ${PC_OPUS_LIBDIR}
)

find_path(Opus_INCLUDE_PATH
    NAMES opus.h
    PATH_SUFFIXES opus
    HINTS ${PC_OPUS_INCLUDEDIR}
)
if(EXISTS "${Opus_INCLUDE_PATH}/opus")
    list(APPEND Opus_INCLUDE_PATH "${Opus_INCLUDE_PATH}/opus")
endif()

if(PC_OPUS_FOUND)
    get_flags_from_pkg_config("${Opus_LIBRARY}" "PC_OPUS" "_opus")
endif()

set(Opus_COMPILE_OPTIONS "${_opus_compile_options}" CACHE STRING "Extra compile options of opus")

set(Opus_LINK_LIBRARIES "${_opus_link_libraries}" CACHE STRING "Extra link libraries of opus")

set(Opus_LINK_OPTIONS "${_opus_link_options}" CACHE STRING "Extra link flags of opus")

set(Opus_LINK_DIRECTORIES "${_opus_link_directories}" CACHE PATH "Extra link directories of opus")

find_package(Ogg ${required})

find_package_handle_standard_args(Opus
    REQUIRED_VARS Opus_LIBRARY Opus_INCLUDE_PATH Ogg_FOUND
)

if(Opus_FOUND)
    set(Opus_dirs ${Opus_INCLUDE_PATH})
    if(NOT TARGET Opus::opus)
        add_library(Opus::opus UNKNOWN IMPORTED)
        set_target_properties(Opus::opus PROPERTIES
            IMPORTED_LOCATION "${Opus_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${Opus_dirs}"
            INTERFACE_COMPILE_OPTIONS "${Opus_COMPILE_OPTIONS}:$<TARGET_PROPERTY:Ogg::ogg,INTERFACE_INCLUDE_DIRECTORIES>"
            INTERFACE_LINK_LIBRARIES "${Opus_LINK_LIBRARIES}"
            INTERFACE_LINK_OPTIONS "${Opus_LINK_OPTIONS}"
            INTERFACE_LINK_DIRECTORIES "${Opus_LINK_DIRECTORIES}"
        )
    endif()
endif()

set(Opus_INCLUDE_DIRS ${Opus_INCLUDE_PATH})
