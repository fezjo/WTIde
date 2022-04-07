set(IMGUI_PATH ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
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
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/zep/src)
set(APP_ROOT ${CMAKE_SOURCE_DIR})
configure_file(${CMAKE_SOURCE_DIR}/cmake/config_app.h.cmake ${CMAKE_SOURCE_DIR}/config_app.h)

set(TINYDIR_PATH ${CMAKE_CURRENT_SOURCE_DIR}/tinydir)
add_library(tinydir STATIC ${TINYDIR_PATH}/tinydir.h)
target_include_directories("ImGui" PUBLIC ${TINYDIR_PATH})
set_target_properties(tinydir PROPERTIES LINKER_LANGUAGE C)