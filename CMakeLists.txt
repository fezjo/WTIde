cmake_minimum_required(VERSION 3.22)

# set(CMAKE_TOOLCHAIN_FILE
#     ${CMAKE_CURRENT_SOURCE_DIR}/lib/vcpkg/scripts/buildsystems/vcpkg.cmake
#     CACHE STRING "Vcpkg toolchain file"
# )

project(WTIde LANGUAGES C CXX VERSION 0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_definitions(-g -O1)   
add_definitions(-DIMGUI_USER_CONFIG="${CMAKE_SOURCE_DIR}/src/imgui/my_imconfig.h")
add_definitions(-DZEP_USE_SDL)
add_definitions(-DZEP_FEATURE_CPP_FILE_SYSTEM)

set(SANITIZE_FLAGS
    -fsanitize=address
    -fsanitize-address-use-after-scope
    -fno-omit-frame-pointer
    -fno-optimize-sibling-calls
    -fsanitize=leak
    # -fsanitize=memory
    -fsanitize=undefined
    -fsanitize=bounds
)

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
        # ${SANITIZE_FLAGS}
)
target_link_options(
    ${TARGET_NAME} PRIVATE
        # ${SANITIZE_FLAGS}
)

configure_file(zep.cfg zep.cfg COPYONLY)