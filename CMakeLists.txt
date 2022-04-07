cmake_minimum_required(VERSION 3.22)

# set(CMAKE_TOOLCHAIN_FILE
#     ${CMAKE_CURRENT_SOURCE_DIR}/lib/vcpkg/scripts/buildsystems/vcpkg.cmake
#     CACHE STRING "Vcpkg toolchain file"
# )

project(WTEdu LANGUAGES C CXX VERSION 0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} -g)
set(TARGET_NAME ${CMAKE_PROJECT_NAME})

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

add_executable(${TARGET_NAME} "")

add_subdirectory(lib)

add_subdirectory(src)

target_include_directories(
    ${TARGET_NAME}
    PRIVATE
        ${CMAKE_BINARY_DIR}
        lib/zep/include
)
target_link_libraries(
    ${TARGET_NAME}
    PRIVATE
        SDL2::SDL2
        OpenGL::OpenGL
        ImGui
        Zep::Zep
        tinydir
        stdc++fs
)