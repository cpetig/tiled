/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "tilelayer.h"
#include "tile.h"

using namespace Tiled;

TileLayer::TileLayer(const QString &name, int x, int y, int width, int height,
                     Map *map):
    Layer(name, x, y, width, height, map),
    mMaxTileHeight(0),
    mTiles(width * height)
{
}

Tile* TileLayer::tileAt(int x, int y) const
{
    return mTiles.at(x + y * mWidth);
}

void TileLayer::setTile(int x, int y, Tile *tile)
{
    if (tile && tile->height() > mMaxTileHeight)
        mMaxTileHeight = tile->height();

    mTiles[x + y * mWidth] = tile;
}
