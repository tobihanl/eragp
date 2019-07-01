#include "World.h"

std::vector<LivingEntity*> World::living = std::vector<LivingEntity*>();
std::vector<FoodEntity*> World::food = std::vector<FoodEntity*>();

void World::render() {
    //TODO render terrain
    for(const auto &f : food) {
        f->render();
    }
    for(const auto &e : living) {
        e->render();
    }
}

void World::tick() {
    for(const auto &f : food) {
        f->tick();
    }
    for(const auto &e : living) {
        e->tick();
    }
}

FoodEntity* World::findNearestFood(int x, int y) {
    if(living.size() == 0) return nullptr;
    FoodEntity* f = food[0];
    int dist = f->getSquaredDistance(x, y);
    for(const auto &e : food) {
        int tempDist = e->getSquaredDistance(x, y);
        if(tempDist < dist) {
            f = e;
            dist = tempDist;
        }
    }
    return f;
}

LivingEntity* World::findNearestLiving(int x, int y) {
    if(living.size() == 0) return nullptr;
    LivingEntity* n = living[0];
    int dist = n->getSquaredDistance(x, y);
    for(const auto &e : living) {
        int tempDist = e->getSquaredDistance(x, y);
        if(tempDist < dist) {
            n = e;
            dist = tempDist;
        }
    }
    return n;
}

void World::addLivingEntity(LivingEntity *e) {
    living.push_back(e);
}

void World::addFoodEntity(FoodEntity *e) {
    food.push_back(e);
}

//TODO cleanup for destroyed entities