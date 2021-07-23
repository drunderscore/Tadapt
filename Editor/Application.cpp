/*
 * Copyright (c) 2021, James Puleo <james@jame.xyz>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include <AK/MemoryStream.h>
#include <AK/String.h>
#include <Editor/Application.h>
#include <GL/glew.h>
#include <imgui/imgui.h>
#include <LibCore/File.h>
#include <LibGfx/PNGLoader.h>
#include <LibTerraria/World.h>
#include <LibTerraria/Model.h>

Application::Application(RefPtr<Terraria::World> world)
        : m_current_world(world)
{
    load_all_tile_texture_sheets();
}

void Application::process_event(SDL_Event* event)
{
    if (event->type == SDL_MOUSEMOTION)
    {
        m_hovered_visual_tile_x = ((event->motion.x - 1) | (m_tile_visual_size_x - 1)) + 1;
        m_hovered_visual_tile_y = ((event->motion.y - 1) | (m_tile_visual_size_y - 1)) + 1;
    }
    else if (event->type == SDL_MOUSEBUTTONDOWN)
    {
        if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGui::IsAnyItemHovered())
        {
            if (event->button.button == SDL_BUTTON_LEFT)
            {
                set_selected_tile((m_hovered_visual_tile_x / m_tile_visual_size_x) + m_offset_x - 1,
                                  (m_hovered_visual_tile_y / m_tile_visual_size_y) + m_offset_y - 1);
            }
        }
    }
    else if (event->type == SDL_MOUSEWHEEL)
    {
        if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGui::IsAnyItemHovered())
        {
            if ((SDL_GetModState() & KMOD_CTRL) != 0)
            {
                // FIXME: Once we can handle non powers of two here, allow more refined zooming
                // The problem is that our hover/selection box isn't being properly aligned, probably because
                // the math is not meant for non powers of two.
                if (event->wheel.y > 0)
                {
                    m_tile_visual_size_x += m_tile_visual_size_x;
                    m_tile_visual_size_y += m_tile_visual_size_y;
                }
                else
                {
                    m_tile_visual_size_x -= m_tile_visual_size_x / 2;
                    m_tile_visual_size_y -= m_tile_visual_size_y / 2;
                }
            }
            else
            {
                constexpr int offset_move_scale = 4;
                if ((SDL_GetModState() & KMOD_SHIFT) != 0)
                    m_offset_x -= event->wheel.y * offset_move_scale;
                else
                    m_offset_y -= event->wheel.y * offset_move_scale;
            }
        }
    }
}

void Application::draw()
{
    draw_main_menu_bar();

    if (m_current_world)
    {
        draw_tile_map();
        draw_selection_window();
    }
}

void Application::set_selected_tile(int x, int y)
{
    m_selected_tile_x = x;
    m_selected_tile_y = y;

    auto& tile = m_current_world->tile_map()->at(x, y);
    m_selected_has_red_wire = tile.has_red_wire();
    m_selected_has_blue_wire = tile.has_blue_wire();
    m_selected_has_green_wire = tile.has_green_wire();
    m_selected_has_yellow_wire = tile.has_yellow_wire();
    m_selected_has_actuator = tile.has_actuator();
    m_selected_is_actuated = tile.is_actuated();

    if (tile.block().has_value())
    {
        if (tile.block()->frame_x().has_value())
            m_selected_frame_x = *tile.block()->frame_x();

        if (tile.block()->frame_y().has_value())
            m_selected_frame_y = *tile.block()->frame_y();
    }
}

void Application::draw_main_menu_bar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open"))
            {
                // TODO: Implement this once loading after main works? See main.cpp for bug explanation.
                VERIFY_NOT_REACHED();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            ImGui::DragInt("Offset X", &m_offset_x);
            ImGui::DragInt("Offset Y", &m_offset_y);
            ImGui::Separator();
            ImGui::DragInt("Tile X Visual Size", &m_tile_visual_size_x);
            ImGui::DragInt("Tile Y Visual Size", &m_tile_visual_size_y);
            // TODO: Customizable wire alpha

            ImGui::EndMenu();
        }

        ImGui::Text("Selected Tile: %d, %d", m_selected_tile_x, m_selected_tile_y);

        ImGui::EndMainMenuBar();
    }
}

void Application::draw_tile_map()
{
    auto& io = ImGui::GetIO();
    auto* draw_list = ImGui::GetBackgroundDrawList();

    // +1 to be sure we don't have an empty edge on other resolutions/tile visual sizes
    auto tiles_to_draw_x = (static_cast<int>(io.DisplaySize.x) / m_tile_visual_size_x) + 1;
    auto tiles_to_draw_y = (static_cast<int>(io.DisplaySize.y) / m_tile_visual_size_y) + 1;

    for (int x = 0; x < tiles_to_draw_x; x++)
    {
        auto real_x = x + m_offset_x;
        if (real_x >= m_current_world->m_max_tiles_x)
            break;

        for (int y = 0; y < tiles_to_draw_y; y++)
        {
            auto real_y = y + m_offset_y;
            if (real_y >= m_current_world->m_max_tiles_y)
                break;

            auto& tile = m_current_world->tile_map()->at(real_x, real_y);
            if (tile.block().has_value())
            {
                auto& tex = *m_tile_textures.get(static_cast<int>(tile.block()->id()));
                short frame_x = 0;
                short frame_y = 0;

                if (tile.block()->frame_x().has_value())
                    frame_x = *tile.block()->frame_x();

                if (tile.block()->frame_y().has_value())
                    frame_y = *tile.block()->frame_y();

                auto color = tile.is_actuated() ? 0x5fffffff : 0xffffffff;

                draw_list->AddImage(reinterpret_cast<void*>(tex.gl_texture_id),
                                    ImVec2(x * m_tile_visual_size_x, y * m_tile_visual_size_y),
                                    ImVec2((x + 1.0f) * m_tile_visual_size_x, (y + 1.0f) * m_tile_visual_size_y),
                                    ImVec2((float) frame_x / (float) tex.width,
                                           (float) frame_y / (float) tex.height),
                                    ImVec2(((float) frame_x + 16.0f) / (float) tex.width,
                                           ((float) frame_y + 16.0f) / (float) tex.height), color);
            }

            if (tile.has_red_wire())
            {
                auto& top = m_current_world->tile_map()->at(real_x, real_y - 1);
                auto& bottom = m_current_world->tile_map()->at(real_x, real_y + 1);
                auto& left = m_current_world->tile_map()->at(real_x - 1, real_y);
                auto& right = m_current_world->tile_map()->at(real_x + 1, real_y);

                auto frames = Terraria::Tile::frames_for_wire(top.has_red_wire(), bottom.has_red_wire(),
                                                              left.has_red_wire(), right.has_red_wire());

                draw_list->AddImage(reinterpret_cast<void*>(m_red_wire_texture.gl_texture_id),
                                    ImVec2(x * m_tile_visual_size_x, y * m_tile_visual_size_y),
                                    ImVec2((x + 1.0f) * m_tile_visual_size_x, (y + 1.0f) * m_tile_visual_size_y),
                                    ImVec2((float) frames.x / (float) m_red_wire_texture.width,
                                           (float) frames.y / (float) m_red_wire_texture.height),
                                    ImVec2(((float) frames.x + 16.0f) / (float) m_red_wire_texture.width,
                                           ((float) frames.y + 16.0f) / (float) m_red_wire_texture.height), 0x7fffffff);
            }

            if (tile.has_blue_wire())
            {
                auto& top = m_current_world->tile_map()->at(real_x, real_y - 1);
                auto& bottom = m_current_world->tile_map()->at(real_x, real_y + 1);
                auto& left = m_current_world->tile_map()->at(real_x - 1, real_y);
                auto& right = m_current_world->tile_map()->at(real_x + 1, real_y);

                auto frames = Terraria::Tile::frames_for_wire(top.has_blue_wire(), bottom.has_blue_wire(),
                                                              left.has_blue_wire(), right.has_blue_wire());

                draw_list->AddImage(reinterpret_cast<void*>(m_blue_wire_texture.gl_texture_id),
                                    ImVec2(x * m_tile_visual_size_x, y * m_tile_visual_size_y),
                                    ImVec2((x + 1.0f) * m_tile_visual_size_x, (y + 1.0f) * m_tile_visual_size_y),
                                    ImVec2((float) frames.x / (float) m_blue_wire_texture.width,
                                           (float) frames.y / (float) m_blue_wire_texture.height),
                                    ImVec2(((float) frames.x + 16.0f) / (float) m_blue_wire_texture.width,
                                           ((float) frames.y + 16.0f) / (float) m_blue_wire_texture.height),
                                    0x7fffffff);
            }

            if (tile.has_green_wire())
            {
                auto& top = m_current_world->tile_map()->at(real_x, real_y - 1);
                auto& bottom = m_current_world->tile_map()->at(real_x, real_y + 1);
                auto& left = m_current_world->tile_map()->at(real_x - 1, real_y);
                auto& right = m_current_world->tile_map()->at(real_x + 1, real_y);

                auto frames = Terraria::Tile::frames_for_wire(top.has_green_wire(), bottom.has_green_wire(),
                                                              left.has_green_wire(), right.has_green_wire());

                draw_list->AddImage(reinterpret_cast<void*>(m_green_wire_texture.gl_texture_id),
                                    ImVec2(x * m_tile_visual_size_x, y * m_tile_visual_size_y),
                                    ImVec2((x + 1.0f) * m_tile_visual_size_x, (y + 1.0f) * m_tile_visual_size_y),
                                    ImVec2((float) frames.x / (float) m_green_wire_texture.width,
                                           (float) frames.y / (float) m_green_wire_texture.height),
                                    ImVec2(((float) frames.x + 16.0f) / (float) m_green_wire_texture.width,
                                           ((float) frames.y + 16.0f) / (float) m_green_wire_texture.height),
                                    0x7fffffff);
            }

            if (tile.has_yellow_wire())
            {
                auto& top = m_current_world->tile_map()->at(real_x, real_y - 1);
                auto& bottom = m_current_world->tile_map()->at(real_x, real_y + 1);
                auto& left = m_current_world->tile_map()->at(real_x - 1, real_y);
                auto& right = m_current_world->tile_map()->at(real_x + 1, real_y);

                auto frames = Terraria::Tile::frames_for_wire(top.has_yellow_wire(), bottom.has_yellow_wire(),
                                                              left.has_yellow_wire(), right.has_yellow_wire());

                draw_list->AddImage(reinterpret_cast<void*>(m_yellow_wire_texture.gl_texture_id),
                                    ImVec2(x * m_tile_visual_size_x, y * m_tile_visual_size_y),
                                    ImVec2((x + 1.0f) * m_tile_visual_size_x, (y + 1.0f) * m_tile_visual_size_y),
                                    ImVec2((float) frames.x / (float) m_yellow_wire_texture.width,
                                           (float) frames.y / (float) m_yellow_wire_texture.height),
                                    ImVec2(((float) frames.x + 16.0f) / (float) m_yellow_wire_texture.width,
                                           ((float) frames.y + 16.0f) / (float) m_yellow_wire_texture.height),
                                    0x7fffffff);
            }

            if (tile.has_actuator())
            {
                draw_list->AddImage(reinterpret_cast<void*>(m_actuator_texture.gl_texture_id),
                                    ImVec2(x * m_tile_visual_size_x, y * m_tile_visual_size_y),
                                    ImVec2((x + 1.0f) * m_tile_visual_size_x, (y + 1.0f) * m_tile_visual_size_y),
                                    ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 0x7fffffff);
            }

            if (real_x == m_selected_tile_x && real_y == m_selected_tile_y)
            {
                draw_list->AddRect(ImVec2(x * m_tile_visual_size_x, y * m_tile_visual_size_y),
                                   ImVec2((x + 1.0f) * m_tile_visual_size_x, (y + 1.0f) * m_tile_visual_size_y),
                                   0xff00ffff);
            }
        }
    }

    draw_list->AddRect(ImVec2(static_cast<float>(m_hovered_visual_tile_x) - static_cast<float>(m_tile_visual_size_x),
                              static_cast<float>(m_hovered_visual_tile_y) - static_cast<float>(m_tile_visual_size_y)),
                       ImVec2(static_cast<float>(m_hovered_visual_tile_x),
                              static_cast<float>(m_hovered_visual_tile_y)), 0xffff00ff);
}

void Application::draw_selection_window()
{
    if (ImGui::Begin("Selection", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        auto& tile = m_current_world->tile_map()->at(m_selected_tile_x, m_selected_tile_y);
        if (ImGui::BeginChild("Selection Image", ImVec2(64, 64), true, ImGuiWindowFlags_NoScrollbar))
        {
            if (tile.block().has_value())
            {
                short frame_x = 0;
                short frame_y = 0;

                if (tile.block()->frame_x().has_value())
                    frame_x = *tile.block()->frame_x();

                if (tile.block()->frame_y().has_value())
                    frame_y = *tile.block()->frame_y();

                auto tex = *m_tile_textures.get(static_cast<u16>(tile.block()->id()));

                ImGui::Image(reinterpret_cast<void*>(tex.gl_texture_id), ImVec2(64, 64),
                             ImVec2((float) frame_x / (float) tex.width,
                                    (float) frame_y / (float) tex.height),
                             ImVec2(((float) frame_x + 16.0f) / (float) tex.width,
                                    ((float) frame_y + 16.0f) / (float) tex.height));
                ImGui::Separator();
            }
        }

        ImGui::EndChild();

        auto preview_string = tile.block().has_value() ?
                              String::formatted("{}",
                                                Terraria::s_tiles[static_cast<int>(tile.block()->id())].internal_name)
                                                       : String::empty();

        if (ImGui::BeginCombo("Blocks", preview_string.characters()))
        {
            if (ImGui::Selectable("None"))
                tile.block() = {};

            for (auto& tex_for_blocks_combo : m_tile_textures)
            {
                ImGui::Image(reinterpret_cast<void*>(tex_for_blocks_combo.value.gl_texture_id), ImVec2(16, 16),
                             ImVec2(0.0f, 0.0f),
                             ImVec2(16.0f / (float) tex_for_blocks_combo.value.width,
                                    16.0f / (float) tex_for_blocks_combo.value.height));
                ImGui::SameLine();
                // FIXME: string allocation each time?? wtf, can't we just get the char* with a null term?
                if (ImGui::Selectable(String::formatted("{}",
                                                        Terraria::s_tiles[tex_for_blocks_combo.key].internal_name).characters()))
                {
                    tile.block() = static_cast<Terraria::Tile::Block::Id>(tex_for_blocks_combo.key);
                }
            }

            ImGui::EndCombo();
        }

        if (tile.block().has_value() && Terraria::s_tiles[static_cast<int>(tile.block()->id())].frame_important)
        {
            ImGui::SetNextItemWidth(75.0f);
            if (ImGui::InputInt("Frame X", &m_selected_frame_x, 18, 18) && tile.block().has_value())
                tile.block()->frame_x() = m_selected_frame_x;

            ImGui::SameLine();

            ImGui::SetNextItemWidth(75.0f);
            if (ImGui::InputInt("Frame Y", &m_selected_frame_y, 18, 18) && tile.block().has_value())
                tile.block()->frame_y() = m_selected_frame_y;

            ImGui::Separator();
        }

        if (ImGui::Checkbox("Red Wire", &m_selected_has_red_wire))
            tile.set_red_wire(m_selected_has_red_wire);

        if (ImGui::Checkbox("Green Wire", &m_selected_has_green_wire))
            tile.set_green_wire(m_selected_has_green_wire);

        if (ImGui::Checkbox("Blue Wire", &m_selected_has_blue_wire))
            tile.set_blue_wire(m_selected_has_blue_wire);

        if (ImGui::Checkbox("Yellow Wire", &m_selected_has_yellow_wire))
            tile.set_yellow_wire(m_selected_has_yellow_wire);

        if (ImGui::Checkbox("Actuator", &m_selected_has_actuator))
            tile.set_has_actuator(m_selected_has_actuator);

        if (ImGui::Checkbox("Actuated", &m_selected_is_actuated))
            tile.set_is_actuated(m_selected_is_actuated);
    }

    ImGui::End();
}

Application::Texture Application::load_texture(const RefPtr<Gfx::Bitmap> bitmap)
{
    Vector<Gfx::RGBA32> pixels;
    pixels.resize(bitmap->width() * bitmap->height());

    for (auto x = 0; x < bitmap->width(); x++)
    {
        for (auto y = 0; y < bitmap->height(); y++)
        {
            // The backing value of this color object is called RGBA32, and yet it's format is actually ARGB. what the fuck???
            auto col = bitmap->get_pixel(x, y);
            pixels[x + (bitmap->width() * y)] =
                    (((col.red() << 24) | col.green() << 16) | col.blue() << 8) | col.alpha();
        }
    }

    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap->width(), bitmap->height(), 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
                 pixels.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return {texture, bitmap->width(), bitmap->height()};
}

void Application::load_all_tile_texture_sheets()
{
    constexpr StringView content_directory = "Content";
    if (!Core::File::exists(content_directory) || !Core::File::is_directory(content_directory))
    {
        // TODO: Write a better error message and instructions on how to get the content.
        warnln("The Content directory could not be found.");
        warnln("It is _not_ distributed with Tadapt, as it's contents are owned be Re-Logic.");
        VERIFY_NOT_REACHED();
    }

    // TODO: Lazy tile texture loading?
    outln("Loading tile texture sheets");
    for (u16 i = 0; i < NumericLimits<u16>::max(); i++)
    {
        auto image = Gfx::load_png(String::formatted("Content/images/Tiles_{}.png", i));
        if (!image)
            break;

        m_tile_textures.set(i, load_texture(image));
    }

    outln("Loaded {} tile texture sheets", m_tile_textures.size());

    m_red_wire_texture = load_texture(Gfx::load_png("Content/images/Wires.png"));
    m_blue_wire_texture = load_texture(Gfx::load_png("Content/images/Wires2.png"));
    m_green_wire_texture = load_texture(Gfx::load_png("Content/images/Wires3.png"));
    m_yellow_wire_texture = load_texture(Gfx::load_png("Content/images/Wires4.png"));
    m_actuator_texture = load_texture(Gfx::load_png("Content/images/Actuator.png"));
}