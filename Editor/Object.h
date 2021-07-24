/*
 * Copyright (c) 2021, James Puleo <james@jame.xyz>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <AK/Types.h>
#include <AK/String.h>
#include <AK/Vector.h>
#include <initializer_list>
#include <LibTerraria/Tile.h>

class Object
{
public:
    Object(String name, u8 width, u8 height, Vector<Terraria::Tile> tiles, Optional<int> style_offset_x = {},
           Optional<int> style_offset_y = {});

    static const Vector<Object>& all_objects()
    { return s_all_objects; }

    const String& name() const
    { return m_name; }

    u8 width() const
    { return m_width; }

    u8 height() const
    { return m_height; }

    const Optional<int>& style_offset() const
    { return m_style_offset; }

    const Vector<Terraria::Tile>& tiles() const
    { return m_tiles; }

    ALWAYS_INLINE constexpr size_t index_for_position(u8 x, u8 y) const
    {
        return x + (m_width * y);
    }

private:
    static Vector<Object> s_all_objects;

    String m_name;
    u8 m_width;
    u8 m_height;
    Optional<int> m_style_offset_x;
    Optional<int> m_style_offset_y;
    Vector<Terraria::Tile> m_tiles;
};