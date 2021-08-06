add_executable(Editor
        ${PROJECT_SOURCE_DIR}/imgui/imgui.cpp
        ${PROJECT_SOURCE_DIR}/imgui/imgui_demo.cpp
        ${PROJECT_SOURCE_DIR}/imgui/imgui_draw.cpp
        ${PROJECT_SOURCE_DIR}/imgui/imgui_tables.cpp
        ${PROJECT_SOURCE_DIR}/imgui/imgui_widgets.cpp
        ${PROJECT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl.cpp
        ${PROJECT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp
        main.cpp
        Application.cpp
        Object.cpp
        )
# FIXME: This is copied from target_lagom, because the PROJECT_ variables don't work exactly how we want outside that project.
target_include_directories(Editor SYSTEM PRIVATE
        # This is pretty much solely for AK
        ${PROJECT_SOURCE_DIR}/Tappy/serenity/
        ${PROJECT_SOURCE_DIR}/Tappy/serenity/Userland/Libraries
        ${PROJECT_SOURCE_DIR}/imgui

        ${PROJECT_SOURCE_DIR}/Tappy

        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
        # FIXME: This is needed because imgui thought it would be funny to do #include <SDL.h>
        /usr/include/SDL2
        )
target_link_libraries(Editor PRIVATE LagomCore LagomGfx)

target_link_libraries(Editor PRIVATE Terraria SDL2 GLEW GL)
