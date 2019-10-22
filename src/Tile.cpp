#include "Tile.h"

//TODO better fitting colors
Tile Tile::INVALID = Tile(-1, {0, 0, 0, 0});
Tile Tile::GRASS = Tile(0, {0, 255, 0, 255});
Tile Tile::WATER = Tile(1, {0, 0, 255, 255});
Tile Tile::STONE = Tile(2, {127, 127, 127, 255});
Tile Tile::SAND = Tile(3, {255, 255, 0, 255});

Tile::Tile(int i, SDL_Color c) : id(i), color(c), texture(nullptr) {

}

bool operator==(const Tile &lhs, const Tile &rhs) {
    return lhs.id == rhs.id;
}

bool operator!=(const Tile &lhs, const Tile &rhs) {
    return lhs.id != rhs.id;
}
