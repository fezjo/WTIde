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

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    message(STATUS "ccache found, using it")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

# Set all compiler flags 
include(cmake/compiler.cmake)

if (EMSCRIPTEN)
    message(STATUS "Building for Emscripten")
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
        # common warnings
        -Wall -Wextra -Wpedantic -pedantic
        -Wformat=2 -Wextra-semi -Wredundant-decls -Wimplicit-fallthrough
        -Wnon-virtual-dtor -Wctor-dtor-privacy -Woverloaded-virtual -Wsuggest-override
        -Wfloat-equal -Wconversion -Wsign-conversion -Wcast-qual # -Wcast-align
        -Wzero-as-null-pointer-constant
        -Wno-unused-parameter

        # gcc warnings
        $<$<CXX_COMPILER_ID:GNU>:
            -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wuseless-cast
            -Wsuggest-final-types -Wsuggest-final-methods
            -Wnull-dereference -Wstrict-null-sentinel
        >
        # clang warnings
        $<$<CXX_COMPILER_ID:Clang>:
            -Wshadow
        >

        ## extreme warnings
        # -Weverything -Wno-c++98-compat-pedantic -Wold-style-cast
)

if (EMSCRIPTEN)
    # Documentation for Emscripten options:
    # https://github.com/emscripten-core/emscripten/blob/main/src/settings.js
    set(EMSCRIPTEN_HOME_DIR "/home/web_user")

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

        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/resources/config@${EMSCRIPTEN_HOME_DIR}"
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/resources/@${EMSCRIPTEN_HOME_DIR}/resources"
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/resources/sample_project@${EMSCRIPTEN_HOME_DIR}/project"
        "SHELL:--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/lib/emscripten/shell.html"
    )
else ()
    # file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/resources/config/zep.cfg" DESTINATION ${CMAKE_BINARY_DIR})
    file(GLOB CONFIGS "${CMAKE_CURRENT_SOURCE_DIR}/resources/config/*")
    file(COPY ${CONFIGS} DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/resources" DESTINATION "${CMAKE_BINARY_DIR}")
    file(GLOB SAMPLE_PROJECT "${CMAKE_CURRENT_SOURCE_DIR}/resources/sample_project/*")
    file(COPY ${SAMPLE_PROJECT} DESTINATION "${CMAKE_BINARY_DIR}/project")
endif ()