include(FindPackageHandleStandardArgs)

find_library(opusfile_LIBRARY
    NAMES opusfile
)

set(opusfile_COMPILE_OPTIONS "" CACHE STRING "Extra compile options of opusfile")

set(opusfile_LINK_LIBRARIES "" CACHE STRING "Extra link libraries of opusfile")

set(opusfile_LINK_FLAGS "" CACHE STRING "Extra link flags of opusfile")

find_path(opusfile_INCLUDE_PATH
    NAMES opusfile.h
    PATH_SUFFIXES opus
)

find_package_handle_standard_args(opusfile
    REQUIRED_VARS opusfile_LIBRARY opusfile_INCLUDE_PATH
)

if (opusfile_FOUND)
    set(opusfile_dirs ${opusfile_INCLUDE_PATH})
    if(EXISTS "${opusfile_INCLUDE_PATH}/opus")
        list(APPEND opusfile_dirs "${opusfile_INCLUDE_PATH}/opus")
    endif()
    if (NOT TARGET opusfile::opusfile)
        add_library(opusfile::opusfile UNKNOWN IMPORTED)
        set_target_properties(opusfile::opusfile PROPERTIES
            IMPORTED_LOCATION "${opusfile_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${opusfile_dirs}"
            INTERFACE_COMPILE_OPTIONS "${opusfile_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${opusfile_LINK_LIBRARIES}"
            INTERFACE_LINK_FLAGS "${opusfile_LINK_FLAGS}"
        )
    endif()
endif()
