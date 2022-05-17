include(FindPackageHandleStandardArgs)

find_library(vorbisfile_LIBRARY
    NAMES vorbisfile
)

set(vorbisfile_COMPILE_OPTIONS "" CACHE STRING "Extra compile options of vorbisfile")

set(vorbisfile_LINK_LIBRARIES "" CACHE STRING "Extra link libraries of vorbisfile")

set(vorbisfile_LINK_FLAGS "" CACHE STRING "Extra link flags of vorbisfile")

find_path(vorbisfile_INCLUDE_PATH
    NAMES vorbis/vorbisfile.h
)

find_package_handle_standard_args(vorbisfile
    REQUIRED_VARS vorbisfile_LIBRARY vorbisfile_INCLUDE_PATH
)

if (vorbisfile_FOUND)
    if (NOT TARGET vorbisfile::vorbisfile)
        add_library(vorbisfile::vorbisfile UNKNOWN IMPORTED)
        set_target_properties(vorbisfile::vorbisfile PROPERTIES
            IMPORTED_LOCATION "${vorbisfile_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${vorbisfile_INCLUDE_PATH}"
            INTERFACE_COMPILE_OPTIONS "${vorbisfile_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${vorbisfile_LINK_LIBRARIES}"
            INTERFACE_LINK_FLAGS "${vorbisfile_LINK_FLAGS}"
        )
    endif()
endif()
