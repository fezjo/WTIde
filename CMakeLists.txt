cmake_minimum_required(VERSION 3.22)

project(WTIde
    LANGUAGES C CXX
    VERSION 0.2
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message("emscripten::::" ${EMSCRIPTEN_ROOT_PATH})
set(EMSCRIPTEN on)
if (DEFINED EMSCRIPTEN)
    # set ld and compiler flags -s DISABLE_EXCEPTION_CATCHING=1
    # set(EMSCRIPTEN_COMPILER_FLAGS "-s WASM=1 -s USE_SDL=2 -s USE_WEBGL2=1 -s USE_GLFW=3 -s FULL_ES3=1 -s USE_WEBGPU=1 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=0 -s ASSERTIONS=1")
    # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EMSCRIPTEN_COMPILER_FLAGS}")
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMSCRIPTEN_COMPILER_FLAGS}")
    set(CMAKE_TOOLCHAIN_FILE ${EMSCRIPTEN_ROOT_PATH}/cmake/Modules/Platform/Emscripten.cmake)
endif ()

set(CMAKE_RELEASE_POSTFIX "-release")
set(CMAKE_DEBUG_POSTFIX "-debug")
set(CMAKE_RELWITHDEBINFO_POSTFIX "-reldbg")

# Set all compiler flags 
include(cmake/compiler.cmake)

# message("EMSCRIPTEN SET1")
# message("file first" ${CMAKE_TOOLCHAIN_FILE})
# if (EMSCRIPTEN)
#     message("Building for Emscripten")
#     set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/emscripten.cmake)
# endif()
# message(${CMAKE_TOOLCHAIN_FILE})

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
        -Wall -Wextra -Wpedantic
        -Wshadow -Wformat=2 -Wfloat-equal -Wconversion
        # -Wlogical-op -Wduplicated-cond
        -Wcast-qual -Wcast-align
        -Wno-unused-parameter -Wno-shadow
        # -Weverything -Wno-c++98-compat-pedantic
)
if (EMSCRIPTEN)
    add_definitions(-D__EMSCRIPTEN__)
    target_link_options(${TARGET_NAME} PUBLIC
        "SHELL:-s USE_SDL=2"
        "SHELL:-s USE_WEBGL2=1"
        # "SHELL:-s USE_GLFW=3"
        "SHELL:-s FULL_ES3=1"
        # "SHELL:-s USE_WEBGPU=1"
        "SHELL:-s WASM=1"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-s ASSERTIONS=1"
        # "SHELL:-s EXIT_RUNTIME=1"
        # "SHELL:--preload-file assets"
        # "SHELL:--preload-file shaders"
        # "SHELL:--preload-file textures"
        # "SHELL:--no-heap-copy"
        # "SHELL:--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
    )
endif ()

configure_file(zep.cfg zep.cfg COPYONLY)