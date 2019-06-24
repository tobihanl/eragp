
#include "World.h"

World::World() = default;

void World::render() {
    //TODO render terrain
    for (auto &f : food) {
        f.render();
    }
    for (auto &e : living) {
        e.render();
    }
}

void World::tick() {
    for (auto &f : food) {
        f.tick();
    }
    for (auto &e : living) {
        e.tick();
    }
}

/*
 * Returns itself if no other entities exist
 */
LivingEntity World::getNearestEntity(LivingEntity entity) {
    LivingEntity nearestEntity = living.front();
    if (nearestEntity == entity && living.size() >= 2) {
        nearestEntity = living[1];
    }
    double minDistance = entity.distanceToPoint(nearestEntity.x, nearestEntity.y);

    for (auto &e : living) {
        double distance = entity.distanceToPoint(e.x, e.y);
        if (distance < minDistance && e != entity) {
            nearestEntity = e;
            minDistance = distance;
        }
    }
    return nearestEntity;
}
