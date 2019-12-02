#include "Entity.h"
#include "World.h"

int Entity::idCounter = 0;

Entity::Entity(int startX, int startY, const Color &color, int radius) :
        x(startX),
        y(startY),
        radius(radius),
        color(color),
        id((idCounter++ * World::getMPINodes()) + World::getMPIRank()) {

}

Entity::Entity(int id, int x, int y, const Color &color, int radius) : Entity(x, y, color, radius) {
    this->id = id;
}

Entity::Entity(int i, int ix, int iy, const Color &color, float size) :
        radius(((int) (1.0f + size) * TILE_SIZE / 2)),
        color(color),
        id(i),
        x(ix),
        y(iy) {

}
