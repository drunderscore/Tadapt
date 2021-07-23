/*
 * Copyright (c) 2021, James Puleo <james@jame.xyz>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <AK/HashMap.h>
#include <LibTerraria/World.h>
#include <SDL2/SDL_events.h>
#include <LibGfx/Bitmap.h>

class Application
{
public:
    Application(RefPtr<Terraria::World> world);

    void process_event(SDL_Event*);

    void draw();

    void set_selected_tile(int x, int y);

private:
    struct Texture
    {
        u32 gl_texture_id;
        int width;
        int height;
    };

    static Texture load_texture(const RefPtr<Gfx::Bitmap>&);

    void draw_main_menu_bar();

    void draw_tile_map();

    void draw_selection_window();

    void draw_selected_chest_window();

    void load_all_tile_texture_sheets();

    void load_all_item_texture_sheets();

    void frame_implicit_tiles();

    HashMap<u16, Texture> m_tile_textures;
    HashMap<u16, Texture> m_item_textures;
    RefPtr<Terraria::World> m_current_world;
    Texture m_red_wire_texture;
    Texture m_blue_wire_texture;
    Texture m_green_wire_texture;
    Texture m_yellow_wire_texture;
    Texture m_actuator_texture;

    int m_offset_x{};
    int m_offset_y{};
    int m_hovered_visual_tile_x{};
    int m_hovered_visual_tile_y{};
    int m_selected_tile_x{};
    int m_selected_tile_y{};
    int m_tile_visual_size_x{16};
    int m_tile_visual_size_y{16};

    int m_selected_frame_x{};
    int m_selected_frame_y{};

    Terraria::Chest* m_selected_chest{};
    char m_selected_chest_name[20]{};
    i16 m_selected_chest_selected_item_stack{};
    u8 m_selected_chest_selected_item_prefix{};

    bool m_selected_has_red_wire{};
    bool m_selected_has_blue_wire{};
    bool m_selected_has_green_wire{};
    bool m_selected_has_yellow_wire{};
    bool m_selected_has_actuator{};
    bool m_selected_is_actuated{};
};