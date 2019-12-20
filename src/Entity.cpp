#include "Entity.h"
#include "World.h"

int Entity::idCounter = 0;

Entity::Entity(int startX, int startY, const Color &color, int radius, int energy) :
        x(startX),
        y(startY),
        radius(radius),
        color(color),
        energy(energy),
        id((idCounter++ * World::getMPINodes()) + World::getMPIRank()) {

}

Entity::Entity(int id, int x, int y, const Color &color, int radius, int energy) : Entity(x, y, color, radius, energy) {
    this->id = id;
}

Entity::Entity(int i, int ix, int iy, const Color &color, float size, int energy) :
        radius((int) ((1.0f + size) * TILE_SIZE / 2)), // WARNING: Cast position is important for right radius calc!
        color(color),
        energy(energy),
        id(i),
        x(ix),
        y(iy) {

}
