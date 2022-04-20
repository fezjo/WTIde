set(IMGUI_PATH ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
file(GLOB IMGUI_SOURCES
    ${IMGUI_PATH}/*.cpp
    ${IMGUI_PATH}/backends/imgui_impl_sdl.cpp
    ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_PATH}/misc/cpp/*.cpp
)
message(${IMGUI_SOURCES})
add_library("ImGui" STATIC ${IMGUI_SOURCES})
target_link_libraries(
    ImGui
    PRIVATE
        SDL2::SDL2
        OpenGL::OpenGL
)
target_include_directories("ImGui"
    PUBLIC
        ${IMGUI_PATH}
        ${IMGUI_PATH}/backends
        ${IMGUI_PATH}/misc/cpp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/zep/src)
set(APP_ROOT ${CMAKE_SOURCE_DIR})
configure_file(${CMAKE_SOURCE_DIR}/cmake/config_app.h.cmake ${CMAKE_SOURCE_DIR}/config_app.h)

set(CLIP_PATH ${CMAKE_CURRENT_SOURCE_DIR}/clip)
add_subdirectory(${CLIP_PATH})
target_include_directories(clip PUBLIC ${CLIP_PATH})

set(TINYDIR_PATH ${CMAKE_CURRENT_SOURCE_DIR}/tinydir)
add_library(tinydir STATIC ${TINYDIR_PATH}/tinydir.h ${TINYDIR_PATH}/tinydir.cpp)
target_include_directories(tinydir PUBLIC ${TINYDIR_PATH})