# CMake script for building SDL_mixer tests

set(CMAKE_POSITION_INDEPENDENT_CODE OFF)

set(RESOURCE_FILES
)

function(add_sdl_mixer_test_executable TARGET)
    add_executable(${TARGET} ${ARGN})
    if("c_std_99" IN_LIST CMAKE_C_COMPILE_FEATURES)
      target_compile_features(${TARGET} PRIVATE c_std_99)
    endif()
    target_compile_definitions(${TARGET}
        PRIVATE
            $<TARGET_PROPERTY:${sdl3_mixer_target_name},COMPILE_DEFINITIONS>
    )
    sdl_add_warning_options(${TARGET} WARNING_AS_ERROR ${SDLMIXER_WERROR})
    target_link_libraries(${TARGET} PRIVATE SDL3_mixer::SDL3_mixer SDL3::SDL3)

    if(SDLMIXER_TESTS_INSTALL)
        install(
            TARGETS ${TARGET}
            DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}/installed-tests/${PROJECT_NAME}"
        )
        if(MSVC)
            SDL_install_pdb("${TARGET}" "${CMAKE_INSTALL_LIBEXECDIR}/installed-tests/${PROJECT_NAME}")
        endif()
    endif()
endfunction()

add_sdl_mixer_test_executable(testaudiodecoder testaudiodecoder.c)
add_sdl_mixer_test_executable(testmixer testmixer.c)
add_sdl_mixer_test_executable(testspacialization testspatialization.c)

if(SDLMIXER_TESTS_INSTALL)
    install(
        FILES ${RESOURCE_FILES}
        DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}/installed-tests/${PROJECT_NAME}"
    )
endif()
