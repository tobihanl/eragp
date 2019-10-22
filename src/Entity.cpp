#include "Entity.h"
#include <cmath>
#include "Renderer.h"
#include "World.h"

int Entity::idCounter = 0;

Entity::Entity(int startX, int startY, const SDL_Color &color, int radius) :
        x(startX),
        y(startY),
        texture(Renderer::renderDot(radius, color)),
        id((idCounter++ * World::getMPINodes()) + World::getMPIRank()) {

}

Entity::Entity(int id, int x, int y, const SDL_Color &color, int radius) : Entity(x, y, color, radius) {
    this->id = id;
}

Entity::Entity(int i, int ix, int iy, const SDL_Color &color, float size) :
        texture(Renderer::renderDot((int) ((1.0f + size) * TILE_SIZE / 2), color)),
        id(i),
        x(ix),
        y(iy) {

}

Entity::~Entity() {
    Renderer::cleanup(texture);
}

int Entity::getId() {
    return id;
}

bool operator==(const Entity &lhs, const Entity &rhs) {
    return lhs.id == rhs.id;
}

bool operator!=(const Entity &lhs, const Entity &rhs) {
    return !(lhs == rhs);
}

int Entity::getSquaredDistance(int px, int py) {
    return (x - px) * (x - px) + (y - py) * (y - py);
}

float Entity::getDistance(int px, int py) {
    return std::sqrt((x - px) * (x - px) + (y - py) * (y - py));
}

