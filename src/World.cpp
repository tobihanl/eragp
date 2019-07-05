#include "World.h"
#include <algorithm>
#include <stdlib.h>

std::vector<LivingEntity *> World::living = std::vector<LivingEntity *>();
std::vector<FoodEntity *> World::food = std::vector<FoodEntity *>();

std::vector<LivingEntity *> World::removeLiving = std::vector<LivingEntity *>();
std::vector<FoodEntity *> World::removeFood = std::vector<FoodEntity *>();
std::vector<LivingEntity *> World::addLiving = std::vector<LivingEntity *>();
std::vector<FoodEntity *> World::addFood = std::vector<FoodEntity *>();

void World::render() {
    //TODO render terrain
    for (const auto &f : food) {
        f->render();
    }
    for (const auto &e : living) {
        e->render();
    }
}

bool World::toRemoveLiving(LivingEntity *e){
    return std::find(removeLiving.begin(), removeLiving.end(), e) != removeLiving.end();
}
bool World::toRemoveFood(FoodEntity *e){
    return std::find(removeFood.begin(), removeFood.end(), e) != removeFood.end();
}
void World::tick() {
    addFoodEntity(new FoodEntity(rand() % WORLD_WIDTH, rand() % WORLD_HEIGHT, 4 * 60));
    for (const auto &e : living) {
        e->tick();
    }
    living.erase(std::remove_if(living.begin(), living.end(), toRemoveLiving), living.end());
    living.insert(living.end(), addLiving.begin(), addLiving.end());
    food.erase(std::remove_if(food.begin(), food.end(), toRemoveFood), food.end());
    food.insert(food.end(), addFood.begin(), addFood.end());

    for(int i = 0; i < removeLiving.size(); i++) {
        delete removeLiving[i];
    }
    for(int i = 0; i < removeFood.size(); i++) {
        delete removeFood[i];
    }
    removeLiving.clear();
    removeFood.clear();
    addLiving.clear();
    addFood.clear();

}

FoodEntity* World::findNearestFood(int x, int y) {
    if (food.size() == 0) return nullptr;
    FoodEntity *f = food[0];
    int dist = f->getSquaredDistance(x, y);
    for (const auto &e : food) {
        int tempDist = e->getSquaredDistance(x, y);
        if (tempDist < dist) {
            f = e;
            dist = tempDist;
        }
    }
    return f;
}

LivingEntity *World::findNearestLiving(int x, int y) {
    if (living.size() == 0) return nullptr;
    LivingEntity *n = living[0];
    int dist = n->getSquaredDistance(x, y);
    for (const auto &e : living) {
        int tempDist = e->getSquaredDistance(x, y);
        if (tempDist < dist) {
            n = e;
            dist = tempDist;
        }
    }
    return n;
}

void World::addLivingEntity(LivingEntity *e) {
    addLiving.push_back(e);
}

void World::addFoodEntity(FoodEntity *e) {
    addFood.push_back(e);
}

void World::removeLivingEntity(LivingEntity *e) {
    removeLiving.push_back(e);
}

void World::removeFoodEntity(FoodEntity *e) {
    removeFood.push_back(e);
}

//TODO cleanup for destroyed entities