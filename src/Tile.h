#ifndef EVOLUTION_TILE_H
#define EVOLUTION_TILE_H

#include <SDL.h>


class Tile {
private:
    Tile(SDL_Color color);

public:
    static Tile GRASS;
    static Tile STONE;
    static Tile SAND;
    static Tile WATER;

    SDL_Texture *texture;
    const SDL_Color color;

};

#endif //EVOLUTION_TILE_H
