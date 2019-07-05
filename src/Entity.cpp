
#include "Entity.h"

Entity::Entity(int startX, int startY) : x(startX), y(startY), id(currentId++) {}

bool Entity::operator==(const Entity &other) const {
    return this->id == other.id;
}

bool Entity::operator!=(const Entity &other) const {
    return !(*this == other);
}

int Entity::currentId;

