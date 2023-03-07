include(FindPackageHandleStandardArgs)

find_library(OpusFile_LIBRARY
    NAMES opusfile
)

set(OpusFile_COMPILE_OPTIONS "" CACHE STRING "Extra compile options of opusfile")

set(OpusFile_LINK_LIBRARIES "" CACHE STRING "Extra link libraries of opusfile")

set(OpusFile_LINK_OPTIONS "" CACHE STRING "Extra link flags of opusfile")

find_path(OpusFile_INCLUDE_PATH
    NAMES opusfile.h
    PATH_SUFFIXES opus
)

find_package_handle_standard_args(OpusFile
    REQUIRED_VARS OpusFile_LIBRARY OpusFile_INCLUDE_PATH
)

if (OpusFile_FOUND)
    set(OpusFile_dirs ${OpusFile_INCLUDE_PATH})
    if(EXISTS "${OpusFile_INCLUDE_PATH}/opus")
        list(APPEND OpusFile_dirs "${OpusFile_INCLUDE_PATH}/opus")
    endif()
    if (NOT TARGET OpusFile::opusfile)
        add_library(OpusFile::opusfile UNKNOWN IMPORTED)
        set_target_properties(OpusFile::opusfile PROPERTIES
            IMPORTED_LOCATION "${OpusFile_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OpusFile_dirs}"
            INTERFACE_COMPILE_OPTIONS "${OpusFile_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${OpusFile_LINK_LIBRARIES}"
            INTERFACE_LINK_OPTIONS "${OpusFile_LINK_OPTIONS}"
        )
    endif()
endif()
