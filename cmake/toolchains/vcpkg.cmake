include_guard(GLOBAL)

set(GAME_PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
cmake_path(NORMAL_PATH GAME_PROJECT_ROOT)

set(GAME_BOOTSTRAP_VCPKG ON CACHE BOOL "Automatically clone and bootstrap vcpkg when it is missing")
set(GAME_VCPKG_ROOT "${GAME_PROJECT_ROOT}/.vcpkg" CACHE PATH "Path to the project-local vcpkg checkout")
set(GAME_VCPKG_INSTALLED_DIR "${GAME_VCPKG_ROOT}/vcpkg_installed" CACHE PATH "Path to the project-local vcpkg installed tree")
set(GAME_VCPKG_REPOSITORY "https://github.com/microsoft/vcpkg.git" CACHE STRING "Git repository used to bootstrap vcpkg")
set(GAME_VCPKG_GIT_REF "c3867e714dd3a51c272826eea77267876517ed99" CACHE STRING "vcpkg git ref used as the bootstrap baseline")

cmake_path(ABSOLUTE_PATH GAME_VCPKG_ROOT BASE_DIRECTORY "${GAME_PROJECT_ROOT}" NORMALIZE OUTPUT_VARIABLE GAME_VCPKG_ROOT_ABS)
cmake_path(ABSOLUTE_PATH GAME_VCPKG_INSTALLED_DIR BASE_DIRECTORY "${GAME_PROJECT_ROOT}" NORMALIZE OUTPUT_VARIABLE GAME_VCPKG_INSTALLED_DIR_ABS)

set(GAME_VCPKG_ROOT "${GAME_VCPKG_ROOT_ABS}" CACHE PATH "Path to the project-local vcpkg checkout" FORCE)
set(GAME_VCPKG_INSTALLED_DIR "${GAME_VCPKG_INSTALLED_DIR_ABS}" CACHE PATH "Path to the project-local vcpkg installed tree" FORCE)

function(game_run_git)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${GAME_VCPKG_ROOT}" ${ARGN}
        RESULT_VARIABLE game_git_result
        OUTPUT_VARIABLE game_git_stdout
        ERROR_VARIABLE game_git_stderr
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )

    set(GAME_GIT_RESULT "${game_git_result}" PARENT_SCOPE)
    set(GAME_GIT_STDOUT "${game_git_stdout}" PARENT_SCOPE)
    set(GAME_GIT_STDERR "${game_git_stderr}" PARENT_SCOPE)
endfunction()

if(APPLE)
    if(DEFINED ENV{PKG_CONFIG} AND EXISTS "$ENV{PKG_CONFIG}")
        set(GAME_PKG_CONFIG_EXECUTABLE "$ENV{PKG_CONFIG}")
    else()
        find_program(
            GAME_PKG_CONFIG_EXECUTABLE
            NAMES pkg-config
            HINTS /opt/homebrew/bin /usr/local/bin
        )
    endif()

    if(NOT GAME_PKG_CONFIG_EXECUTABLE)
        message(FATAL_ERROR
            "pkg-config is required before configuring this project with the local vcpkg toolchain on macOS.\n"
            "Install it first with Homebrew:\n"
            "  brew install pkg-config"
        )
    endif()

    set(ENV{PKG_CONFIG} "${GAME_PKG_CONFIG_EXECUTABLE}")
endif()

if(NOT GAME_BOOTSTRAP_VCPKG)
    if(NOT EXISTS "${GAME_VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        message(FATAL_ERROR
            "vcpkg was not found at ${GAME_VCPKG_ROOT} and GAME_BOOTSTRAP_VCPKG is OFF.\n"
            "Run CMake with -DGAME_BOOTSTRAP_VCPKG=ON or provide a populated GAME_VCPKG_ROOT."
        )
    endif()
else()
    find_program(GIT_EXECUTABLE git REQUIRED)

    if(EXISTS "${GAME_VCPKG_ROOT}" AND NOT IS_DIRECTORY "${GAME_VCPKG_ROOT}")
        message(FATAL_ERROR "GAME_VCPKG_ROOT points to a file: ${GAME_VCPKG_ROOT}")
    endif()

    if(NOT EXISTS "${GAME_VCPKG_ROOT}")
        file(MAKE_DIRECTORY "${GAME_VCPKG_ROOT}")
    endif()

    if(NOT EXISTS "${GAME_VCPKG_ROOT}/.git")
        file(GLOB GAME_VCPKG_ROOT_CONTENTS LIST_DIRECTORIES true "${GAME_VCPKG_ROOT}/*")
        if(GAME_VCPKG_ROOT_CONTENTS)
            message(FATAL_ERROR
                "GAME_VCPKG_ROOT exists but is not a git checkout: ${GAME_VCPKG_ROOT}\n"
                "Remove that directory and rerun CMake so the local vcpkg clone can be recreated."
            )
        endif()

        message(STATUS "Cloning vcpkg into ${GAME_VCPKG_ROOT}")
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" clone "${GAME_VCPKG_REPOSITORY}" "${GAME_VCPKG_ROOT}"
            COMMAND_ERROR_IS_FATAL ANY
        )
    endif()

    game_run_git(rev-parse --is-inside-work-tree)
    if(NOT GAME_GIT_RESULT EQUAL 0)
        message(FATAL_ERROR
            "Failed to use ${GAME_VCPKG_ROOT} as a git repository.\n"
            "${GAME_GIT_STDERR}"
        )
    endif()

    if(GAME_VCPKG_GIT_REF)
        game_run_git(rev-parse --verify "${GAME_VCPKG_GIT_REF}^{commit}")
        if(NOT GAME_GIT_RESULT EQUAL 0)
            message(STATUS "Fetching vcpkg ref ${GAME_VCPKG_GIT_REF}")
            execute_process(
                COMMAND "${GIT_EXECUTABLE}" -C "${GAME_VCPKG_ROOT}" fetch --depth 1 origin "${GAME_VCPKG_GIT_REF}"
                RESULT_VARIABLE GAME_VCPKG_FETCH_RESULT
                OUTPUT_VARIABLE GAME_VCPKG_FETCH_STDOUT
                ERROR_VARIABLE GAME_VCPKG_FETCH_STDERR
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_STRIP_TRAILING_WHITESPACE
            )
            if(NOT GAME_VCPKG_FETCH_RESULT EQUAL 0)
                message(STATUS "Direct ref fetch failed; fetching remote refs from origin")
                execute_process(
                    COMMAND "${GIT_EXECUTABLE}" -C "${GAME_VCPKG_ROOT}" fetch --tags origin
                    COMMAND_ERROR_IS_FATAL ANY
                )
            endif()
            game_run_git(rev-parse --verify "${GAME_VCPKG_GIT_REF}^{commit}")
            if(NOT GAME_GIT_RESULT EQUAL 0)
                message(FATAL_ERROR
                    "Fetched vcpkg ref ${GAME_VCPKG_GIT_REF}, but the commit is still unavailable.\n"
                    "${GAME_GIT_STDERR}"
                )
            endif()
        endif()
        set(GAME_VCPKG_TARGET_COMMIT "${GAME_GIT_STDOUT}")

        game_run_git(rev-parse --verify HEAD)
        set(GAME_VCPKG_HEAD_REF "${GAME_GIT_STDOUT}")
        if(NOT "${GAME_VCPKG_HEAD_REF}" STREQUAL "${GAME_VCPKG_TARGET_COMMIT}")
            message(STATUS "Checking out vcpkg ref ${GAME_VCPKG_GIT_REF}")
            execute_process(
                COMMAND "${GIT_EXECUTABLE}" -C "${GAME_VCPKG_ROOT}" checkout --force "${GAME_VCPKG_GIT_REF}"
                COMMAND_ERROR_IS_FATAL ANY
            )
        endif()
    endif()

    if(NOT EXISTS "${GAME_VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        message(FATAL_ERROR
            "The local vcpkg checkout at ${GAME_VCPKG_ROOT} is incomplete after checkout.\n"
            "Remove that directory and rerun CMake."
        )
    endif()

    if(WIN32)
        set(GAME_VCPKG_BOOTSTRAP_SCRIPT "${GAME_VCPKG_ROOT}/bootstrap-vcpkg.bat")
        set(GAME_VCPKG_EXECUTABLE "${GAME_VCPKG_ROOT}/vcpkg.exe")
        if(NOT EXISTS "${GAME_VCPKG_EXECUTABLE}")
            message(STATUS "Bootstrapping vcpkg executable")
            execute_process(
                COMMAND cmd /c "${GAME_VCPKG_BOOTSTRAP_SCRIPT}" -disableMetrics
                COMMAND_ERROR_IS_FATAL ANY
            )
        endif()
    else()
        set(GAME_VCPKG_BOOTSTRAP_SCRIPT "${GAME_VCPKG_ROOT}/bootstrap-vcpkg.sh")
        set(GAME_VCPKG_EXECUTABLE "${GAME_VCPKG_ROOT}/vcpkg")
        if(NOT EXISTS "${GAME_VCPKG_EXECUTABLE}")
            message(STATUS "Bootstrapping vcpkg executable")
            execute_process(
                COMMAND sh "${GAME_VCPKG_BOOTSTRAP_SCRIPT}" -disableMetrics
                COMMAND_ERROR_IS_FATAL ANY
            )
        endif()
    endif()
endif()

set(ENV{VCPKG_ROOT} "${GAME_VCPKG_ROOT}")
set(VCPKG_ROOT "${GAME_VCPKG_ROOT}" CACHE PATH "vcpkg root used by this project" FORCE)
set(VCPKG_MANIFEST_DIR "${GAME_PROJECT_ROOT}" CACHE PATH "Manifest directory for the project-local vcpkg setup" FORCE)
set(VCPKG_INSTALLED_DIR "${GAME_VCPKG_INSTALLED_DIR}" CACHE PATH "Installed directory for the project-local vcpkg setup" FORCE)
set(VCPKG_MANIFEST_INSTALL ON CACHE BOOL "Install manifest dependencies during configure" FORCE)

include("${GAME_VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
