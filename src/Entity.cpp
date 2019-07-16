#include "Entity.h"
#include <cmath>
#include "Renderer.h"

int Entity::currentId = 0;

Entity::Entity(int startX, int startY, const SDL_Color &color, int radius) : x(startX), y(startY), texture(Renderer::renderDot(radius, color)), id(currentId++) {}

Entity::~Entity() {
    Renderer::cleanup(texture);
}

bool Entity::operator==(const Entity &other) const {
    return this->id == other.id;
}

bool Entity::operator!=(const Entity &other) const {
    return !(*this == other);
}

int Entity::getSquaredDistance(int x, int y) {
    return (this->x - x) * (this->x - x) + (this->y - y) * (this->y - y);
}

float Entity::getDistance(int x, int y) {
    return std::sqrt((this->x - x) * (this->x - x) + (this->y - y) * (this->y - y));
}