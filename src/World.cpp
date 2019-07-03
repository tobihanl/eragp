#include "World.h"
#include <algorithm>
#include <stdlib.h>

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
    addFoodEntity(new FoodEntity(rand() % WORLD_WIDTH, rand() % WORLD_HEIGHT, 4 * 60));
    for(const auto &e : living) {
        e->tick();
    }
}

FoodEntity* World::findNearestFood(int x, int y) {
    if(food.size() == 0) return nullptr;
    FoodEntity *f = food[0];
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

void World::removeLivingEntity(LivingEntity *e) {
    living.erase(std::remove(living.begin(), living.end(), e), living.end());
}

void World::removeFoodEntity(FoodEntity *e) {
    food.erase(std::remove(food.begin(), food.end(), e), food.end());
}
//TODO cleanup for destroyed entities