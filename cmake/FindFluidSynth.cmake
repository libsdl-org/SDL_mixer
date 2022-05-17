include(FindPackageHandleStandardArgs)

find_library(FluidSynth_LIBRARY
    NAMES fluidsynth
)

find_path(FluidSynth_INCLUDE_PATH
    NAMES fluidsynth.h
)

set(FluidSynth_COMPILE_OPTIONS "" CACHE STRING "Extra compile options of FluidSynth")

set(FluidSynth_LINK_LIBRARIES "" CACHE STRING "Extra link libraries of FluidSynth")

set(FluidSynth_LINK_FLAGS "" CACHE STRING "Extra link flags of FluidSynth")

find_package_handle_standard_args(FluidSynth
    REQUIRED_VARS FluidSynth_LIBRARY FluidSynth_INCLUDE_PATH
)

if(FluidSynth_FOUND)
    if(NOT TARGET FluidSynth::FluidSynth)
        add_library(FluidSynth::FluidSynth UNKNOWN IMPORTED)
        set_target_properties(FluidSynth::FluidSynth PROPERTIES
            IMPORTED_LOCATION "${FluidSynth_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${FluidSynth_INCLUDE_PATH}"
            INTERFACE_COMPILE_OPTIONS "${FluidSynth_COMPILE_OPTIONS}"
            INTERFACE_LINK_LIBRARIES "${FluidSynth_LINK_LIBRARIES}"
            INTERFACE_LINK_FLAGS "${FluidSynth_LINK_FLAGS}"
        )
    endif()
endif()
