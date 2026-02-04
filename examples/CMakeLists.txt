#
# CMake script for building the SDL examples
#

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")

include(CheckIncludeFile)
include(CheckStructHasMember)
include(CMakePushCheckState)

set(SDLMIXER_EXAMPLE_EXECUTABLES)

if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(example_bin_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    if(NOT IS_ABSOLUTE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
        set(example_bin_dir "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    endif()
else()
    set(example_bin_dir "${CMAKE_CURRENT_BINARY_DIR}")
endif()
if(NOT CMAKE_VERSION VERSION_LESS 3.20)
    get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    set(example_bin_dir "${example_bin_dir}$<$<BOOL:${is_multi_config}>:/$<CONFIG>>")
endif()

file(GLOB RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.mp3 ${CMAKE_CURRENT_SOURCE_DIR}/*.wav)

set(RESOURCE_FILE_NAMES)
set(RESOURCE_FILES_BINDIR)
foreach(resource_file IN LISTS RESOURCE_FILES)
    get_filename_component(res_file_name ${resource_file} NAME)
    list(APPEND RESOURCE_FILE_NAMES "${res_file_name}")
    set(resource_file_bindir "${example_bin_dir}/${res_file_name}")
    add_custom_command(OUTPUT "${resource_file_bindir}"
        COMMAND "${CMAKE_COMMAND}" -E copy "${resource_file}" "${resource_file_bindir}"
        DEPENDS "${resource_file}"
    )
    list(APPEND RESOURCE_FILES_BINDIR "${resource_file_bindir}")
endforeach()
add_custom_target(copy-sdlmixer-example-resources
    DEPENDS "${RESOURCE_FILES_BINDIR}"
)

set(SDLMIXER_EXAMPLE_EXECUTABLES)

macro(add_sdlmixer_example_executable TARGET)
    cmake_parse_arguments(AST "BUILD_DEPENDENT" "" "SOURCES;DATAFILES" ${ARGN})
    if(AST_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown argument(s): ${AST_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT AST_SOURCES)
        message(FATAL_ERROR "add_sdlmixer_example_executable needs at least one source")
    endif()
    if(ANDROID)
        add_library(${TARGET} SHARED ${AST_SOURCES} ${AST_DATAFILES})
    else()
        add_executable(${TARGET} ${AST_SOURCES} ${AST_DATAFILES})
    endif()

    target_compile_definitions(${TARGET}
        PRIVATE
            $<TARGET_PROPERTY:${sdl3_mixer_target_name},COMPILE_DEFINITIONS>
    )
    sdl_add_warning_options(${TARGET} WARNING_AS_ERROR ${SDLMIXER_WERROR})

    target_link_libraries(${TARGET} PRIVATE SDL3_mixer::SDL3_mixer SDL3::SDL3)

    list(APPEND SDLMIXER_EXAMPLE_EXECUTABLES ${TARGET})
    if(AST_DATAFILES)
        if(PSP OR PS2)
            add_custom_command(TARGET ${TARGET} POST_BUILD
                COMMAND ${CMAKE_COMMAND} ARGS -E make_directory $<TARGET_FILE_DIR:${TARGET}>/sdl-${TARGET}
                COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${AST_DATAFILES} $<TARGET_FILE_DIR:${TARGET}>/sdl-${TARGET}
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            )
        elseif(NOT APPLE AND NOT N3DS)
            add_dependencies(${TARGET} copy-sdlmixer-example-resources)
        endif()
        if(EMSCRIPTEN)
            foreach(res IN LISTS AST_DATAFILES)
                get_filename_component(res_name "${res}" NAME)
                target_link_options(${TARGET} PRIVATE "SHELL:--embed-file ${res}@${res_name}")
            endforeach()
        endif()
        set_property(TARGET ${TARGET} APPEND PROPERTY ADDITIONAL_CLEAN_FILES "$<TARGET_FILE_DIR:${TARGET}>/$<JOIN:${AST_DATAFILES},$<SEMICOLON>$<TARGET_FILE_DIR:${TARGET}>/>")
    endif()

    if(APPLE)
        # Set Apple App ID / Bundle ID.  This is needed to launch apps on some Apple
        # platforms (iOS, for example).
        set_target_properties(${TARGET} PROPERTIES
          RESOURCES "${AST_DATAFILES}"
          MACOSX_BUNDLE TRUE
          MACOSX_BUNDLE_GUI_IDENTIFIER "org.libsdl.${TARGET}"
          MACOSX_BUNDLE_BUNDLE_VERSION "${SDL3_VERSION}"
          MACOSX_BUNDLE_SHORT_VERSION_STRING "${SDL3_VERSION}"
        )
        set_property(SOURCE ${AST_DATAFILES} PROPERTY MACOSX_PACKAGE_LOCATION "Resources")
    elseif(WINDOWS)
        # CET support was added in VS 16.7
        if(MSVC_VERSION GREATER 1926 AND CMAKE_GENERATOR_PLATFORM MATCHES "Win32|x64")
            set_property(TARGET ${TARGET} APPEND_STRING PROPERTY LINK_FLAGS " -CETCOMPAT")
        endif()
    elseif(EMSCRIPTEN)
        set_property(TARGET ${TARGET} PROPERTY SUFFIX ".html")
        target_link_options(${TARGET} PRIVATE -sALLOW_MEMORY_GROWTH=1)
    elseif(N3DS)
        set(ROMFS_DIR "${CMAKE_CURRENT_BINARY_DIR}/romfs/${TARGET}")
        file(MAKE_DIRECTORY "${ROMFS_DIR}")
        file(COPY ${AST_DATAFILES} DESTINATION "${ROMFS_DIR}")
        ctr_generate_smdh("${TARGET}.smdh"
            NAME "SDL-${TARGET}"
            DESCRIPTION "SDL3 example application"
            AUTHOR "SDL3 Contributors"
            ICON "${CMAKE_CURRENT_SOURCE_DIR}/../test/n3ds/logo48x48.png"
        )
        ctr_create_3dsx(
            ${TARGET}
            ROMFS "${ROMFS_DIR}"
            SMDH "${TARGET}.smdh"
        )
    elseif(NGAGE)
        string(MD5 TARGET_MD5 "${TARGET}")
        string(SUBSTRING "${TARGET_MD5}" 0 8 TARGET_MD5_8)
        target_link_options(${TARGET} PRIVATE "SHELL:-s UID3=0x${TARGET_MD5_8}")
    endif()
endmacro()

add_sdlmixer_example_executable(basics-load-and-play SOURCES basics/01-load-and-play/load-and-play.c DATAFILES ${CMAKE_CURRENT_SOURCE_DIR}/music.mp3)
add_sdlmixer_example_executable(basics-play-with-options SOURCES basics/02-play-with-options/play-with-options.c DATAFILES ${CMAKE_CURRENT_SOURCE_DIR}/music.mp3)
add_sdlmixer_example_executable(basics-play-multiple-sounds SOURCES basics/03-play-multiple-sounds/play-multiple-sounds.c DATAFILES ${CMAKE_CURRENT_SOURCE_DIR}/music.mp3 ${CMAKE_CURRENT_SOURCE_DIR}/sword.wav)
add_sdlmixer_example_executable(basics-metadata SOURCES basics/04-metadata/metadata.c DATAFILES ${CMAKE_CURRENT_SOURCE_DIR}/music.mp3)
add_sdlmixer_example_executable(basics-sinewave SOURCES basics/05-sinewave/sinewave.c)
add_sdlmixer_example_executable(basics-seeking SOURCES basics/06-seeking/seeking.c DATAFILES ${CMAKE_CURRENT_SOURCE_DIR}/music.mp3)

# When you add an example, remember to add the Visual Studio project as well:
# - Add a new example in examples/
# - Run python VisualC/examples/generate.py
# - Take note of the newly generated .vcxproj files
# - Modify the .vcxproj files if necessary (adding content such as PNG or WAV files)
# - Open VisualC/SDL.sln in Visual Studio or JetBrains Rider
# - Locate the appropriate folder in the Solution Explorer
# - Add the newly generated projects: Right click -> Add -> Existing project...
# - Test if they work
# - Save the SDL.sln solution

if(PSP)
    # Build EBOOT files if building for PSP
    foreach(APP ${SDLMIXER_EXAMPLE_EXECUTABLES})
        create_pbp_file(
            TARGET          ${APP}
            TITLE           SDL-${APP}
            ICON_PATH       NULL
            BACKGROUND_PATH NULL
            PREVIEW_PATH    NULL
            OUTPUT_DIR      $<TARGET_FILE_DIR:${APP}>/sdl-${APP}
        )
    endforeach()
endif()

if(RISCOS)
    set(SDL_EXAMPLE_EXECUTABLES_AIF)
    foreach(APP ${SDLMIXER_EXAMPLE_EXECUTABLES})
        set_property(TARGET ${APP} APPEND_STRING PROPERTY LINK_FLAGS " -static")
        add_custom_command(
            OUTPUT ${APP},ff8
            COMMAND elf2aif ${APP} ${APP},ff8
            DEPENDS ${APP}
        )
        add_custom_target(${APP}-aif ALL DEPENDS ${APP},ff8)
        list(APPEND SDL_EXAMPLE_EXECUTABLES_AIF ${CMAKE_CURRENT_BINARY_DIR}/${APP},ff8)
    endforeach()
endif()

if(SDLMIXER_EXAMPLES_INSTALL)
    if(RISCOS)
        install(
            FILES ${SDL_EXAMPLE_EXECUTABLES_AIF}
            DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/installed-examples/SDL3_mixer
        )
    else()
        install(
            TARGETS ${SDLMIXER_EXAMPLE_EXECUTABLES}
            DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/installed-examples/SDL3_mixer
        )
    endif()
    if(MSVC)
        foreach(example IN LISTS SDLMIXER_EXAMPLE_EXECUTABLES)
            SDL_install_pdb(${example} "${CMAKE_INSTALL_LIBEXECDIR}/installed-examples/SDL3_mixer")
        endforeach()
    endif()
    install(
        FILES ${RESOURCE_FILES}
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/installed-examples/SDL3_mixer
    )
endif()
