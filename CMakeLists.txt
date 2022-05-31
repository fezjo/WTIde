cmake_minimum_required(VERSION 3.22)

project(WTIde
    LANGUAGES C CXX
    VERSION 0.2
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RELEASE_POSTFIX "-release")
set(CMAKE_DEBUG_POSTFIX "-debug")
set(CMAKE_RELWITHDEBINFO_POSTFIX "-reldbg")

# Set all compiler flags 
include(cmake/compiler.cmake)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

add_definitions(-DIMGUI_USER_CONFIG="${CMAKE_SOURCE_DIR}/src/imgui/my_imconfig.h")

set(TARGET_NAME ${CMAKE_PROJECT_NAME})
add_executable(${TARGET_NAME} "")

add_subdirectory(lib)
add_subdirectory(src)

target_include_directories(
    ${TARGET_NAME}
    PRIVATE
        ${CMAKE_BINARY_DIR}
)
target_link_libraries(
    ${TARGET_NAME}
    PRIVATE
        SDL2::SDL2
        OpenGL::OpenGL
        ImGui
        ImGui-Notify
        ICTE
        Zep::Zep
        WTStar
        clip
        nfd
        stdc++fs
)
target_compile_options(
    ${TARGET_NAME} PRIVATE
        -Wall -Wextra -pedantic
        -Wshadow -Wformat=2 -Wfloat-equal -Wconversion
        -Wlogical-op -Wshift-overflow=2 -Wduplicated-cond
        -Wcast-qual -Wcast-align
        -Wno-unused-parameter -Wno-shadow
)

configure_file(zep.cfg zep.cfg COPYONLY)