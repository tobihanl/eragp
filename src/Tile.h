#ifndef EVOLUTION_TILE_H
#define EVOLUTION_TILE_H

#include "Structs.h"
#include "Constants.h"

class Tile {
private:
    Tile(int id, Color color) : id(id), color(color) {}

    friend bool operator==(const Tile &lhs, const Tile &rhs) {
        return lhs.id == rhs.id;
    }

    friend bool operator!=(const Tile &lhs, const Tile &rhs) {
        return lhs.id != rhs.id;
    }
public:
    static Tile INVALID;
    static Tile GRASS;
    static Tile WATER;
    static Tile STONE;
    static Tile SAND;

    const int id;
    const Color color;
};

#endif //EVOLUTION_TILE_H
