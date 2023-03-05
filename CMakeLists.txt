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
    add_definitions(-D__EMSCRIPTEN__)
    add_compile_options(-fwasm-exceptions)
    add_link_options(-fwasm-exceptions)
endif()

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

add_compile_definitions(IMGUI_USER_CONFIG="${CMAKE_SOURCE_DIR}/src/imgui/my_imconfig.h")
set(OPENGL_TARGET $<IF:$<BOOL:${EMSCRIPTEN}>,GL,OpenGL::GL>)

set(TARGET_NAME ${CMAKE_PROJECT_NAME})
add_executable(${TARGET_NAME})

add_subdirectory(lib)
add_subdirectory(src)

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(${TARGET_NAME} PRIVATE
    SDL2::SDL2
    ${OPENGL_TARGET}
    ImGui
    ImGui-Notify
    ICTE
    Zep::Zep
    WTStar
    clip
    $<IF:$<BOOL:${EMSCRIPTEN}>,,nfd>
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
    # Documentation for Emscripten options:
    # https://github.com/emscripten-core/emscripten/blob/main/src/settings.js

    target_link_options(${TARGET_NAME} PUBLIC
        "SHELL:-sUSE_SDL=2"
        "SHELL:-sUSE_WEBGL2=1"
        # "SHELL:-sUSE_GLFW=3"
        "SHELL:-sFULL_ES2=1"
        # "SHELL:-sUSE_WEBGPU=1"
        "SHELL:-sWASM=1"

        "SHELL:-sALLOW_MEMORY_GROWTH=1"
        # "SHELL:--no-heap-copy"
        
        "SHELL:-sASSERTIONS=1"
        # "SHELL:-sSAFE_HEAP=1"
        # "SHELL:-sDEMANGLE_SUPPORT=1"
        # "SHELL:-sDISABLE_EXCEPTION_CATCHING=0"
        # "SHELL:-sEXIT_RUNTIME=1"

        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/resources@resources"
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/resources/config@."
        # "SHELL:--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
    )
else ()
    # file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/resources/config/zep.cfg" DESTINATION ${CMAKE_BINARY_DIR})
    file(GLOB CONFIGS "${CMAKE_CURRENT_SOURCE_DIR}/resources/config/*")
    file(COPY ${CONFIGS} DESTINATION ${CMAKE_BINARY_DIR})
endif ()