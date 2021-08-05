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
#include <Editor/Object.h>

class Application
{
public:
    Application(RefPtr<Terraria::World> world);

    void process_event(SDL_Event*);

    void draw();

    void set_selected_tile(int x, int y);

private:
    enum class Tool
    {
        Select,
        PlaceObject,
        Paint
    };
    struct Texture
    {
        u32 gl_texture_id;
        int width;
        int height;
    };

    void paint_tile(u16 x, u16 y);

    static Texture load_texture(const RefPtr<Gfx::Bitmap>&);

    void draw_main_menu_bar();

    void draw_tile_map();

    void draw_tiles_combo_box(const StringView& preview, Function<void(Optional<int>)> on_select);

    void draw_tile_properties(Terraria::Tile&);

    void draw_selection_window();

    void draw_selected_chest_window();

    void draw_selected_sign_window();

    void load_all_tile_texture_sheets();

    void load_all_item_texture_sheets();

    void frame_region(i16 x, i16 y, i16 end_x, i16 end_y);

    void frame_implicit_tiles();

    Tool m_current_tool{};

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

    const Object* m_selected_object{};
    int m_selected_object_style_x{};
    int m_selected_object_style_y{};

    Terraria::Chest* m_selected_chest{};
    char m_selected_chest_name[20]{};
    i16 m_selected_chest_selected_item_stack{};

    Terraria::Tile m_tile_to_paint;
    bool m_paint_allow_drag{};

    Terraria::Sign* m_selected_sign{};
    char m_selected_sign_text[512]{};

    bool m_tile_properties_has_red_wire{};
    bool m_tile_properties_has_blue_wire{};
    bool m_tile_properties_has_green_wire{};
    bool m_tile_properties_has_yellow_wire{};
    bool m_tile_properties_has_actuator{};
    bool m_tile_properties_is_actuated{};
};