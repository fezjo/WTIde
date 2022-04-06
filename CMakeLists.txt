cmake_minimum_required(VERSION 3.22)

# set(CMAKE_TOOLCHAIN_FILE
#     ${CMAKE_CURRENT_SOURCE_DIR}/lib/vcpkg/scripts/buildsystems/vcpkg.cmake
#     CACHE STRING "Vcpkg toolchain file"
# )

project(WTEdu LANGUAGES C CXX VERSION 0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} -g)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

set(IMGUI_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui)
file(GLOB IMGUI_SOURCES
    ${IMGUI_PATH}/*.cpp
    ${IMGUI_PATH}/backends/imgui_impl_sdl.cpp
    ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp
)
add_library("ImGui" STATIC ${IMGUI_SOURCES})
target_link_libraries(
    ImGui
    PRIVATE
        SDL2::SDL2
        OpenGL::OpenGL
)
target_include_directories("ImGui" PUBLIC ${IMGUI_PATH} ${IMGUI_PATH}/backends)

add_definitions(-DZEP_FEATURE_CPP_FILE_SYSTEM)
add_subdirectory(lib/zep/src)
set(APP_ROOT ${CMAKE_CURRENT_LIST_DIR})
configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/config_app.h.cmake ${CMAKE_BINARY_DIR}/config_app.h)

add_subdirectory(src)


file(GLOB SOURCE_FILES
    src/*.cpp
    src/*.h
)
add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(
    ${CMAKE_PROJECT_NAME}
    PRIVATE
        ${CMAKE_BINARY_DIR}
        lib/zep/include
)
target_link_libraries(
    ${CMAKE_PROJECT_NAME}
    PRIVATE
        SDL2::SDL2
        OpenGL::OpenGL
        ImGui
        Zep::Zep
        stdc++fs
)
