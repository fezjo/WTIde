cmake_minimum_required(VERSION 3.22)

# set(CMAKE_TOOLCHAIN_FILE
#     ${CMAKE_CURRENT_SOURCE_DIR}/lib/vcpkg/scripts/buildsystems/vcpkg.cmake
#     CACHE STRING "Vcpkg toolchain file"
# )

project(WTEdu LANGUAGES C CXX VERSION 0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_definitions(-g)   
add_definitions(-DIMGUI_USER_CONFIG="${CMAKE_SOURCE_DIR}/src/my_imconfig.h")
add_definitions(-DZEP_USE_SDL)
add_definitions(-DZEP_FEATURE_CPP_FILE_SYSTEM)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

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
        Zep::Zep
        WTStar
        clip
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