include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)
pkg_check_modules(PC_FLUIDSYNTH QUIET fluidsynth)

find_library(FluidSynth_LIBRARY
    NAMES fluidsynth fluidsynth-3 libfluidsynth
    HINTS ${PC_FLUIDSYNTH_LIBDIR}
)

find_path(FluidSynth_INCLUDE_PATH
    NAMES fluidsynth.h
    HINTS ${PC_FLUIDSYNTH_INCLUDEDIR}
)

if(PC_FLUIDSYNTH_FOUND)
    get_flags_from_pkg_config("${FluidSynth_LIBRARY}" "PC_FLUIDSYNTH" "_fluidsynth")
endif()

set(FluidSynth_COMPILE_OPTIONS "${_fluidsynth_compile_options}" CACHE STRING "Extra compile options of FluidSynth")

set(FluidSynth_LINK_LIBRARIES "${_fluidsynth_link_libraries}" CACHE STRING "Extra link libraries of FluidSynth")

set(FluidSynth_LINK_OPTIONS "${_fluidsynth_link_options}" CACHE STRING "Extra link flags of FluidSynth")

set(FluidSynth_LINK_DIRECTORIES "${_fluidsynth_link_directories}" CACHE PATH "Extra link directories of FluidSynth")

set(FLUIDSYNTH_VERSION "FLUIDSYNTH_VERSION-NOTFOUND")
if(EXISTS "${FluidSynth_INCLUDE_PATH}/fluidsynth/version.h")
    file(READ "${FluidSynth_INCLUDE_PATH}/fluidsynth/version.h" _fluidsynth_version_h)
    set(_fluidsynth_version_regex "#define[ \t]+FLUIDSYNTH_VERSION[ \t]+\"([1-9]+\\.[0-9]+\\.[0-9]+)\"")
    string(REGEX MATCH "${_fluidsynth_version_regex}" _re_fluidsynth_version "${_fluidsynth_version_h}")
    if(_re_fluidsynth_version)
        set(FLUIDSYNTH_VERSION "${CMAKE_MATCH_1}")
    endif()
endif()

find_package_handle_standard_args(FluidSynth
    REQUIRED_VARS FluidSynth_LIBRARY FluidSynth_INCLUDE_PATH
    VERSION_VAR FLUIDSYNTH_VERSION
)

if(FluidSynth_FOUND)
    if(NOT TARGET FluidSynth::libfluidsynth)
        add_library(FluidSynth::libfluidsynth UNKNOWN IMPORTED)
        set_target_properties(FluidSynth::libfluidsynth PROPERTIES
            IMPORTED_LOCATION "${FluidSynth_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${FluidSynth_INCLUDE_PATH}"
            INTERFACE_COMPILE_OPTIONS "${FluidSynth_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${FluidSynth_LINK_LIBRARIES}"
            INTERFACE_LINK_OPTIONS "${FluidSynth_LINK_OPTIONS}"
            INTERFACE_LINK_DIRECTORIES "${FluidSynth_LINK_DIRECTORIES}"
        )
    endif()
endif()
