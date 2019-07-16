#include <cfloat>
#include "World.h"
#include <algorithm>
#include <cstdlib>
#include <assert.h>
#include "Renderer.h"

// TODO [VERY IMPORTANT!] Implement checking if entity already exists in a vector to prevent duplicates!

std::vector<LivingEntity *> World::living = std::vector<LivingEntity *>();
std::vector<FoodEntity *> World::food = std::vector<FoodEntity *>();

std::vector<LivingEntity *> World::removeLiving = std::vector<LivingEntity *>();
std::vector<FoodEntity *> World::removeFood = std::vector<FoodEntity *>();
std::vector<LivingEntity *> World::addLiving = std::vector<LivingEntity *>();
std::vector<FoodEntity *> World::addFood = std::vector<FoodEntity *>();

const Tile* World::terrain[(WORLD_HEIGHT / TILE_SIZE) * (WORLD_WIDTH / TILE_SIZE)];

void World::generateTerrain() {
    for(int y = 0; y < WORLD_HEIGHT / TILE_SIZE; y++) {
        for(int x = 0; x < WORLD_WIDTH / TILE_SIZE; x++) {
            if((x / 8) % 2 == 0) {
                if((y / 8) % 2 == 0) {
                    terrain[y * (WORLD_WIDTH / TILE_SIZE) + x] = &Tile::GRASS;
                } else {
                    terrain[y * (WORLD_WIDTH / TILE_SIZE) + x] = &Tile::SAND;
                }
            } else {
                if ((y / 8) % 2 == 0) {
                    terrain[y * (WORLD_WIDTH / TILE_SIZE) + x] = &Tile::STONE;
                } else {
                    terrain[y * (WORLD_WIDTH / TILE_SIZE) + x] = &Tile::WATER;
                }
            }
        }
    }
}

void World::render() {
    for(int y = 0; y < WORLD_HEIGHT / TILE_SIZE; y++) {
        for(int x = 0; x < WORLD_WIDTH / TILE_SIZE; x++) {
            SDL_Texture* t = terrain[y * (WORLD_WIDTH / TILE_SIZE) + x]->texture;
            Renderer::copy(t, x * TILE_SIZE, y * TILE_SIZE);
        }
    }
    for (const auto &f : food) {
        f->render();
    }
    for (const auto &e : living) {
        e->render();
    }
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

    // Destroy entities
    for (const auto &e : removeLiving) delete e;
    for (const auto &e : removeFood) delete e;

    // Clear vectors without deallocating memory
    removeFood.clear();
    removeLiving.clear();
    addFood.clear();
    addLiving.clear();
}

FoodEntity *World::findNearestFood(int x, int y) {
    if (food.empty()) return nullptr;
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

FoodEntity *World::findNearestSurvivingFood(int x, int y) {
    FoodEntity *f = nullptr;
    int dist = 0;
    for (const auto &e : food) {
        if(toRemoveFood(e)) continue;
        int tempDist = e->getSquaredDistance(x, y);
        if (!f || tempDist < dist) {
            f = e;
            dist = tempDist;
        }
    }
    return f;
}

LivingEntity *World::findNearestLiving(int x, int y) {
    if (living.empty()) return nullptr;
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
    if (!toAddLiving(e))
        addLiving.push_back(e);
}

void World::addFoodEntity(FoodEntity *e) {
    if (!toAddFood(e))
        addFood.push_back(e);
}

void World::removeLivingEntity(LivingEntity *e) {
    if (!toRemoveLiving(e))
        removeLiving.push_back(e);
}

void World::removeFoodEntity(FoodEntity *e) {
    assert(!toRemoveFood(e) && "Tried to remove same FoodEntity multiple times");
    removeFood.push_back(e);
}

bool World::toRemoveLiving(LivingEntity *e) {
    return std::find(removeLiving.begin(), removeLiving.end(), e) != removeLiving.end();
}

bool World::toAddLiving(LivingEntity *e) {
    return std::find(addLiving.begin(), addLiving.end(), e) != addLiving.end();
}

bool World::toRemoveFood(FoodEntity *e) {
    return std::find(removeFood.begin(), removeFood.end(), e) != removeFood.end();
}

bool World::toAddFood(FoodEntity *e) {
    return std::find(addFood.begin(), addFood.end(), e) != addFood.end();
}

//TODO cleanup for destroyed entities