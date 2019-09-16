#include "Entity.h"
#include <cmath>
#include "Renderer.h"
#include "World.h"

int Entity::currentId = 0;

Entity::Entity(int startX, int startY, const SDL_Color &color, int radius) : x(startX), y(startY),
                                                                             texture(Renderer::renderDot(radius,
                                                                                                         color)),
                                                                             id(currentId++) {

}

Entity::Entity(int i, int ix, int iy, const SDL_Color &color, float size)
        : texture(Renderer::renderDot((int) ((1.0f + size) * TILE_SIZE / 2), color)), id(i), x(ix), y(iy) {}

Entity::~Entity() {
    Renderer::cleanup(texture);
}

bool operator==(const Entity &lhs, const Entity &rhs) {
    return lhs.id == rhs.id;
}

bool operator!=(const Entity &lhs, const Entity &rhs) {
    return !(lhs == rhs);
}

int Entity::getSquaredDistance(int x, int y) {
    return (this->x - x) * (this->x - x) + (this->y - y) * (this->y - y);
}

float Entity::getDistance(int x, int y) {
    return std::sqrt((this->x - x) * (this->x - x) + (this->y - y) * (this->y - y));
}