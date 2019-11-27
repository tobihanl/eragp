#include "Tile.h"

Tile Tile::INVALID = Tile(-1, {0, 0, 0, 0});
Tile Tile::GRASS = Tile(0, {110, 169, 67, 255});
Tile Tile::WATER = Tile(1, {47, 120, 254});
Tile Tile::STONE = Tile(2, {147, 147, 147, 255});
Tile Tile::SAND = Tile(3, {218, 210, 159, 255});
