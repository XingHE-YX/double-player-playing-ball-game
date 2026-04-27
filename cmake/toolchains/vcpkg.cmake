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

if(NOT EXISTS "${GAME_VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    if(NOT GAME_BOOTSTRAP_VCPKG)
        message(FATAL_ERROR
            "vcpkg was not found at ${GAME_VCPKG_ROOT} and GAME_BOOTSTRAP_VCPKG is OFF.\n"
            "Run CMake with -DGAME_BOOTSTRAP_VCPKG=ON or provide a populated GAME_VCPKG_ROOT."
        )
    endif()

    find_program(GIT_EXECUTABLE git REQUIRED)

    if(EXISTS "${GAME_VCPKG_ROOT}" AND NOT IS_DIRECTORY "${GAME_VCPKG_ROOT}")
        message(FATAL_ERROR "GAME_VCPKG_ROOT points to a file: ${GAME_VCPKG_ROOT}")
    endif()

    if(NOT EXISTS "${GAME_VCPKG_ROOT}/.git")
        message(STATUS "Cloning vcpkg into ${GAME_VCPKG_ROOT}")
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" clone "${GAME_VCPKG_REPOSITORY}" "${GAME_VCPKG_ROOT}"
            COMMAND_ERROR_IS_FATAL ANY
        )
    endif()

    if(GAME_VCPKG_GIT_REF)
        message(STATUS "Checking out vcpkg ref ${GAME_VCPKG_GIT_REF}")
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" -C "${GAME_VCPKG_ROOT}" checkout "${GAME_VCPKG_GIT_REF}"
            COMMAND_ERROR_IS_FATAL ANY
        )
    endif()

    if(WIN32)
        set(GAME_VCPKG_BOOTSTRAP_SCRIPT "${GAME_VCPKG_ROOT}/bootstrap-vcpkg.bat")
        execute_process(
            COMMAND cmd /c "${GAME_VCPKG_BOOTSTRAP_SCRIPT}" -disableMetrics
            COMMAND_ERROR_IS_FATAL ANY
        )
    else()
        set(GAME_VCPKG_BOOTSTRAP_SCRIPT "${GAME_VCPKG_ROOT}/bootstrap-vcpkg.sh")
        execute_process(
            COMMAND sh "${GAME_VCPKG_BOOTSTRAP_SCRIPT}" -disableMetrics
            COMMAND_ERROR_IS_FATAL ANY
        )
    endif()
endif()

set(ENV{VCPKG_ROOT} "${GAME_VCPKG_ROOT}")
set(VCPKG_ROOT "${GAME_VCPKG_ROOT}" CACHE PATH "vcpkg root used by this project" FORCE)
set(VCPKG_MANIFEST_DIR "${GAME_PROJECT_ROOT}" CACHE PATH "Manifest directory for the project-local vcpkg setup" FORCE)
set(VCPKG_INSTALLED_DIR "${GAME_VCPKG_INSTALLED_DIR}" CACHE PATH "Installed directory for the project-local vcpkg setup" FORCE)
set(VCPKG_MANIFEST_INSTALL ON CACHE BOOL "Install manifest dependencies during configure" FORCE)

include("${GAME_VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
