#ifndef EVOLUTION_TILE_H
#define EVOLUTION_TILE_H

#include <SDL.h>


class Tile {
private:
    Tile(int id, SDL_Color color);

    friend bool operator==(const Tile &lhs, const Tile &rhs);

    friend bool operator!=(const Tile &lhs, const Tile &rhs);
public:
    static Tile INVALID;
    static Tile GRASS;
    static Tile WATER;
    static Tile STONE;
    static Tile SAND;

    SDL_Texture *texture;
    const int id;
    const SDL_Color color;
};

#endif //EVOLUTION_TILE_H
