include(FindPackageHandleStandardArgs)

find_library(SndFile_LIBRARY
    NAMES sndfile sndfile-1
)

find_path(SndFile_INCLUDE_PATH
    NAMES sndfile.h
)

set(SndFile_COMPILE_OPTIONS "" CACHE STRING "Extra compile options of libsndfile")

set(SndFile_LINK_LIBRARIES "" CACHE STRING "Extra link libraries of libsndfile")

set(SndFile_LINK_FLAGS "" CACHE STRING "Extra link flags of libsndfile")

find_package_handle_standard_args(SndFile
    REQUIRED_VARS SndFile_LIBRARY SndFile_INCLUDE_PATH
)

if(SndFile_FOUND)
    if(NOT TARGET SndFile::sndfile)
        add_library(SndFile::sndfile UNKNOWN IMPORTED)
        set_target_properties(SndFile::sndfile PROPERTIES
            IMPORTED_LOCATION "${SndFile_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SndFile_INCLUDE_PATH}"
            INTERFACE_COMPILE_OPTIONS "${SndFile_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${SndFile_LINK_LIBRARIES}"
            INTERFACE_LINK_FLAGS "${SndFile_LINK_FLAGS}"
        )
    endif()
endif()
