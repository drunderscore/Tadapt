/*
 * Copyright (c) 2021, James Puleo <james@jame.xyz>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include <AK/Format.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <Editor/Application.h>
#include <LibTerraria/World.h>
#include <AK/MemoryStream.h>
#include <LibCore/File.h>
#include <LibCore/ArgsParser.h>

constexpr bool show_metrics_window = false;
Application* s_application;

int main(int argc, char** argv)
{
    Core::ArgsParser args_parser;

    String world_path;

    args_parser.add_positional_argument(world_path, "Path to the world file", "world");

    if (!args_parser.parse(argc, argv))
        return 1;

    auto file_or_error = Core::File::open(world_path, Core::IODevice::OpenMode::ReadOnly);

    if (file_or_error.is_error())
    {
        warnln("Failed to open world file: {}", file_or_error.error());
        return 2;
    }

    auto file_bytes = file_or_error.value()->read_all();
    auto bytes_stream = InputMemoryStream(file_bytes);

    auto world_or_error = Terraria::World::try_load_world(bytes_stream);

    if (world_or_error.is_error())
    {
        warnln("Failed to load world file: {}", world_or_error.error());
        return 3;
    }

    auto world = world_or_error.release_value();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) < 0)
    {
        warnln("Failed to initialize SDL: {}", SDL_GetError());
        return 1;
    }

#if defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    constexpr auto window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    auto* window = SDL_CreateWindow("Tadapt", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);

    auto gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    auto glew_init_return = glewInit();
    if (glew_init_return != GLEW_OK)
    {
        warnln("Failed to initialize GLEW: {}", glewGetErrorString(glew_init_return));
        return 2;
    }

    s_application = new Application(world);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool exit_requested;
    SDL_Event event;

    while (!exit_requested)
    {
        while (SDL_PollEvent(&event) != 0)
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            s_application->process_event(&event);

            if (event.type == SDL_QUIT)
            {
                exit_requested = true;
                break;
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (show_metrics_window)
            ImGui::ShowMetricsWindow();

        s_application->draw();

        ImGui::Render();
        glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // TODO: cleanup

    return 0;
}