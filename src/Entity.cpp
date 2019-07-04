#include "Entity.h"
#include <cmath>
#include "Renderer.h"

Entity::Entity(int startX, int startY, const SDL_Color &color, int radius) : x(startX), y(startY), texture(Renderer::renderDot(radius, color)) {}

int Entity::getSquaredDistance(int x, int y) {
    return (this->x - x) * (this->x - x) + (this->y - y) * (this->y - y);
}

float Entity::getDistance(int x, int y) {
    return std::sqrt((this->x - x) * (this->x - x) + (this->y - y) * (this->y - y));
}