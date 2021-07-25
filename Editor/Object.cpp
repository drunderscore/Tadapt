/*
 * Copyright (c) 2021, James Puleo <james@jame.xyz>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include <Editor/Object.h>

Vector<Object> Object::s_all_objects;

Object::Object(String name, u8 width, u8 height, Vector<Terraria::Tile> tiles, Optional<int> style_offset_x,
               Optional<int> style_offset_y, bool individual_styling)
        : m_name(move(name)),
          m_width(width),
          m_height(height),
          m_tiles(move(tiles)),
          m_style_offset_x(move(style_offset_x)),
          m_style_offset_y(move(style_offset_y)),
          m_individual_styling(individual_styling)
{
    s_all_objects.append(*this);
}

Object::Object(String name, u8 width, u8 height, Terraria::Tile::Block::Id id, Optional<int> style_offset_x,
               Optional<int> style_offset_y,
               bool individual_styling)
        : m_name(move(name)),
          m_width(width),
          m_height(height),
          m_style_offset_x(move(style_offset_x)),
          m_style_offset_y(move(style_offset_y)),
          m_individual_styling(individual_styling)
{
    for (auto y = 0; y < m_height; y++)
    {
        for (auto x = 0; x < m_width; x++)
        {
            Terraria::Tile tile(Terraria::Tile::Block(id, 18 * x, 18 * y));
            m_tiles.append(move(tile));
        }
    }

    s_all_objects.append(*this);
}

static Object life_crystal("Life Crystal", 2, 2, Terraria::Tile::Block::Id::Heart);
static Object work_bench("Work Bench", 2, 1, Terraria::Tile::Block::Id::WorkBenches, 18 * 2);
static Object pot("Pot", 2, 2, Terraria::Tile::Block::Id::Pots, 18 * 2, 18 * 2);
static Object bed("Bed", 4, 2, Terraria::Tile::Block::Id::Beds, 18 * 4, 18 * 2, true);
static Object campfire("Campfire", 3, 2, Terraria::Tile::Block::Id::Campfire, 18 * 3);
static Object anvil("Anvil", 2, 1, Terraria::Tile::Block::Id::Anvils, 18 * 2);
static Object furnace("Furnace", 3, 2, Terraria::Tile::Block::Id::Furnaces);