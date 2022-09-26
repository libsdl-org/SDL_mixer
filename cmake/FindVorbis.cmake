include(FindPackageHandleStandardArgs)

find_library(Vorbis_vorbisfile_LIBRARY
    NAMES vorbisfile
)

set(Vorbis_vorbisfile_COMPILE_OPTIONS "" CACHE STRING "Extra compile options of vorbisfile")

set(Vorbis_vorbisfile_LINK_LIBRARIES "" CACHE STRING "Extra link libraries of vorbisfile")

set(Vorbis_vorbisfile_LINK_FLAGS "" CACHE STRING "Extra link flags of vorbisfile")

find_path(Vorbis_vorbisfile_INCLUDE_PATH
    NAMES vorbis/vorbisfile.h
)

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
            INTERFACE_LINK_FLAGS "${Vorbis_vorbisfile_LINK_FLAGS}"
        )
    endif()
endif()
