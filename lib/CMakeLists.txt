set(IMGUI_PATH ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
file(GLOB IMGUI_SOURCES
    ${IMGUI_PATH}/*.cpp
    ${IMGUI_PATH}/backends/imgui_impl_sdl.cpp
    ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_PATH}/misc/cpp/*.cpp
)
add_library("ImGui" STATIC ${IMGUI_SOURCES})
target_link_libraries(
    ImGui
    PRIVATE
        SDL2::SDL2
        OpenGL::OpenGL
)
target_include_directories("ImGui"
    SYSTEM PUBLIC
        ${IMGUI_PATH}
        ${IMGUI_PATH}/backends
        ${IMGUI_PATH}/misc/cpp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/zep/src)
set(APP_ROOT ${CMAKE_SOURCE_DIR})
target_include_directories(Zep SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/zep/include)
configure_file(${CMAKE_SOURCE_DIR}/cmake/config_app.h.cmake config_app.h COPYONLY)

set(CLIP_PATH ${CMAKE_CURRENT_SOURCE_DIR}/clip)
add_subdirectory(${CLIP_PATH})
target_include_directories(clip SYSTEM PUBLIC ${CLIP_PATH})

set(WTSTAR_PATH ${CMAKE_CURRENT_SOURCE_DIR}/wtstar)
file(GLOB WTSTAR_SOURCES ${WTSTAR_PATH}/src/*.c)
list(REMOVE_ITEM WTSTAR_SOURCES 
    ${WTSTAR_PATH}/src/parser_utils.c 
    ${WTSTAR_PATH}/src/web_interface.c)
add_library("WTStar" STATIC ${WTSTAR_SOURCES}) # TODO SHARED
target_include_directories(WTStar PUBLIC ${WTSTAR_PATH}/src)