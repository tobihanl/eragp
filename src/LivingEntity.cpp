
#include <cmath>
#include "LivingEntity.h"

LivingEntity::LivingEntity(int startX, int startY) : Entity(startX, startY) {}

void LivingEntity::render() {
    //TODO
}

void LivingEntity::tick() {
    //TODO
}

double LivingEntity::distanceToPoint(int x, int y) {
    return sqrt(pow(this->x - x, 2) + pow(this->y - y, 2));
}
