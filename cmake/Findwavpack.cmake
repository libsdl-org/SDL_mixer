include(FindPackageHandleStandardArgs)

if(WIN32)
    set(wavpack_find_names wavpack libwavpack wavpackdll)
else()
    set(wavpack_find_names wavpack)
endif()
find_library(wavpack_LIBRARY
    NAMES ${wavpack_find_names}
)

find_path(wavpack_INCLUDE_PATH
    NAMES wavpack/wavpack.h
)

set(wavpack_COMPILE_OPTIONS "" CACHE STRING "Extra compile options of wavpack")

set(wavpack_LINK_LIBRARIES "" CACHE STRING "Extra link libraries of wavpack")

set(wavpack_LINK_OPTIONS "" CACHE STRING "Extra link flags of wavpack")

find_package_handle_standard_args(wavpack
    REQUIRED_VARS wavpack_LIBRARY wavpack_INCLUDE_PATH
)

if (wavpack_FOUND)
    if (NOT TARGET WavPack::WavPack)
        add_library(WavPack::WavPack UNKNOWN IMPORTED)
        set_target_properties(WavPack::WavPack PROPERTIES
            IMPORTED_LOCATION "${wavpack_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${wavpack_INCLUDE_PATH}"
            INTERFACE_COMPILE_OPTIONS "${wavpack_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${wavpack_LINK_LIBRARIES}"
            INTERFACE_LINK_OPTIONS "${wavpack_LINK_OPTIONS}"
        )
    endif()
endif()
