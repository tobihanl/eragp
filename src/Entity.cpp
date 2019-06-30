#include "Entity.h"
#include <cmath>

Entity::Entity(int startX, int startY) : x(startX), y(startY) {}

int Entity::getSquaredDistance(int x, int y) {
    return (this->x - x) * (this->x - x) + (this->y - y) * (this->y - y);
}

float Entity::getDistance(int x, int y) {
    return std::sqrt((this->x - x) * (this->x - x) + (this->y - y) * (this->y - y));
}