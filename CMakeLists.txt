cmake_minimum_required(VERSION 3.22)

project(WTIde
    LANGUAGES C CXX
    VERSION 0.2
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RELEASE_POSTFIX "-release")
set(CMAKE_DEBUG_POSTFIX "-debug")
set(CMAKE_RELWITHDEBINFO_POSTFIX "-reldbg")

option(BUILD_SHARED_LIBS "Build shared libraries (.dll/.so) instead of static ones (.lib/.a)" ON)

# Set all compiler flags 
include(cmake/compiler.cmake)

if (EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(BUILD_SHARED_LIBS OFF)
endif()

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
        $<$<NOT:$<BOOL:EMSCRIPTEN>>:nfd>
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
        "SHELL:--preload-file ../resources"
        # "SHELL:--preload-file shaders"
        # "SHELL:--preload-file textures"
        # "SHELL:--no-heap-copy"
        # "SHELL:--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
    )
endif ()

configure_file(zep.cfg zep.cfg COPYONLY)