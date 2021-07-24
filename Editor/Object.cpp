/*
 * Copyright (c) 2021, James Puleo <james@jame.xyz>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include <Editor/Object.h>

Vector<Object> Object::s_all_objects;

Object::Object(String name, u8 width, u8 height, Vector<Terraria::Tile> tiles, Optional<int> style_offset_x,
               Optional<int> style_offset_y)
        : m_name(move(name)),
          m_width(width),
          m_height(height),
          m_tiles(move(tiles)),
          m_style_offset_x(move(style_offset_x)),
          m_style_offset_y(move(style_offset_y))
{
    s_all_objects.append(*this);
}

// @formatter:off
static Object life_crystal("Life Crystal", 2, 2,
{{
    Terraria::Tile(Terraria::Tile::Block(Terraria::Tile::Block::Id::Heart)),
    Terraria::Tile(Terraria::Tile::Block(Terraria::Tile::Block::Id::Heart, 18, 0)),
    Terraria::Tile(Terraria::Tile::Block(Terraria::Tile::Block::Id::Heart, 0, 18)),
    Terraria::Tile(Terraria::Tile::Block(Terraria::Tile::Block::Id::Heart, 18, 18))
}});

static Object work_bench("Work Bench", 2, 1,
{{
    Terraria::Tile(Terraria::Tile::Block(Terraria::Tile::Block::Id::WorkBenches, 0, 0)),
    Terraria::Tile(Terraria::Tile::Block(Terraria::Tile::Block::Id::WorkBenches, 18, 0)),
}}, 18 * 2);

static Object pot("Pot", 2, 2,
{{
    Terraria::Tile(Terraria::Tile::Block(Terraria::Tile::Block::Id::Pots, 0, 0)),
    Terraria::Tile(Terraria::Tile::Block(Terraria::Tile::Block::Id::Pots, 18, 0)),
    Terraria::Tile(Terraria::Tile::Block(Terraria::Tile::Block::Id::Pots, 0, 18)),
    Terraria::Tile(Terraria::Tile::Block(Terraria::Tile::Block::Id::Pots, 18, 18))
}}, 18 * 2, 18 * 2);
// @formatter:on