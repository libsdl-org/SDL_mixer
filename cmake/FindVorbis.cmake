include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)
pkg_check_modules(PC_VORBIS QUIET vorbisfile)

find_library(Vorbis_vorbisfile_LIBRARY
    NAMES vorbisfile
    HINTS ${PC_VORBIS_LIBDIR}
)

find_path(Vorbis_vorbisfile_INCLUDE_PATH
    NAMES vorbis/vorbisfile.h
    HINTS ${PC_VORBIS_INCLUDEDIR}
)

if(PC_VORBIS_FOUND)
    get_flags_from_pkg_config("${Vorbis_vorbisfile_LIBRARY}" "PC_VORBIS" "_vorbisfile")
endif()

set(Vorbis_vorbisfile_COMPILE_OPTIONS "${_vorbisfile_compile_options}" CACHE STRING "Extra compile options of vorbisfile")

set(Vorbis_vorbisfile_LINK_LIBRARIES "${_vorbisfile_link_libraries}" CACHE STRING "Extra link libraries of vorbisfile")

set(Vorbis_vorbisfile_LINK_OPTIONS "${_vorbisfile_link_options}" CACHE STRING "Extra link flags of vorbisfile")

set(Vorbis_vorbisfile_LINK_DIRECTORIES "${_vorbisfile_link_directories}" CACHE PATH "Extra link directories of vorbisfile")

find_package_handle_standard_args(Vorbis
    REQUIRED_VARS Vorbis_vorbisfile_LIBRARY Vorbis_vorbisfile_INCLUDE_PATH
)

if (Vorbis_FOUND)
    if (NOT TARGET Vorbis::vorbisfile)
        add_library(Vorbis::vorbisfile UNKNOWN IMPORTED)
        set_target_properties(Vorbis::vorbisfile PROPERTIES
            IMPORTED_LOCATION "${Vorbis_vorbisfile_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${Vorbis_vorbisfile_INCLUDE_PATH}"
            INTERFACE_COMPILE_OPTIONS "${Vorbis_vorbisfile_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${Vorbis_vorbisfile_LINK_LIBRARIES}"
            INTERFACE_LINK_OPTIONS "${Vorbis_vorbisfile_LINK_OPTIONS}"
            INTERFACE_LINK_DIRECTORIES "${Vorbis_vorbisfile_LINK_DIRECTORIES}"
        )
    endif()
endif()
