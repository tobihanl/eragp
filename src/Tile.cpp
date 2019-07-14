#include "Tile.h"
#include "Renderer.h"

//TODO better fitting colors
Tile Tile::GRASS = Tile({0, 255, 0, 255});
Tile Tile::STONE = Tile({127, 127, 127, 255});
Tile Tile::SAND = Tile({255, 255, 0, 255});
Tile Tile::WATER = Tile({0, 0, 255, 255});

Tile::Tile(SDL_Color c) : color(c) {

}