cmake_minimum_required(VERSION 3.23)

project(DualBallShooter VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt is resolved through the project-local vcpkg toolchain when presets are used.
find_package(Qt6 REQUIRED COMPONENTS Quick Qml QuickControls2)

qt_policy(SET QTP0001 NEW)
qt_standard_project_setup()

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE GAME_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/includes/*.hpp"
)

file(GLOB_RECURSE GAME_QML_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" CONFIGURE_DEPENDS
    "qml/*.qml"
)

if(NOT GAME_SOURCES)
    message(FATAL_ERROR "No .cpp files were found under ${CMAKE_CURRENT_SOURCE_DIR}/src.")
endif()

qt_add_executable(${PROJECT_NAME}
    ${GAME_SOURCES}
    ${GAME_HEADERS}
)

qt_add_qml_module(${PROJECT_NAME}
    URI GameApp
    VERSION 1.0
    QML_FILES
        ${GAME_QML_FILES}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/includes"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Quick
        Qt6::Qml
        Qt6::QuickControls2
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GAME_SOURCES} ${GAME_HEADERS} ${GAME_QML_FILES})
