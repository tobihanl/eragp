#include <mpi.h>
#include <cfloat>
#include "World.h"
#include <algorithm>
#include <cstdlib>
#include <cassert>
#include "Renderer.h"

int *splitRect(int num, int width, int height);

// TODO [VERY IMPORTANT!] Implement checking if entity already exists in a vector to prevent duplicates!

std::vector<LivingEntity *> World::living = std::vector<LivingEntity *>();
std::vector<FoodEntity *> World::food = std::vector<FoodEntity *>();

std::vector<LivingEntity *> World::removeLiving = std::vector<LivingEntity *>();
std::vector<FoodEntity *> World::removeFood = std::vector<FoodEntity *>();
std::vector<LivingEntity *> World::addLiving = std::vector<LivingEntity *>();
std::vector<FoodEntity *> World::addFood = std::vector<FoodEntity *>();

// Init static attributes
int World::overallWidth = 0;
int World::overallHeight = 0;

int World::MPI_Rank = 0;
int World::MPI_Nodes = 0;

int World::x = 0;
int World::y = 0;
int World::width = 0;
int World::height = 0;

bool World::isSetup = false;

std::vector<Tile *> World::terrain = std::vector<Tile *>();

/**
 * Initialize the world, which is part of the overall world and set
 * it up.
 *
 * @param overallWidth  Width of the overall world
 * @param overallHeight Height of the overall world
 * @param maimuc        Indicates, whether the program is executed on
 *                      MaiMUC or not
 */
void World::setup(int overallWidth, int overallHeight, bool maimuc) {
    if (isSetup)
        return;

    // Get MPI Rank and number of nodes
    MPI_Comm_rank(MPI_COMM_WORLD, &MPI_Rank);
    MPI_Comm_size(MPI_COMM_WORLD, &MPI_Nodes);

    // MaiMUC specific configuration?
    if (maimuc) {
        // MaiMUC consists of 10 nodes!
        if (MPI_Nodes != 10) {
            std::cerr << "Program started on MaiMUC without running on 10 nodes!" << std::endl;
            abort();
        }

        World::overallWidth = 960;
        World::overallHeight = 1600;

        // Set dimensions for this world on MaiMUC
        x = ((MPI_Rank % 2) == 0) ? 0 : 480;
        y = (MPI_Rank / 2) * 320;
        width = 480;
        height = 320;

    } else {
        World::overallWidth = overallWidth;
        World::overallHeight = overallHeight;

        // Get and set dimesions for this world
        WorldDim dim = calcWorldDimensions(MPI_Rank, MPI_Nodes);
        x = dim.x;
        y = dim.y;
        width = dim.w;
        height = dim.h;
    }

    generateTerrain();
    isSetup = true;
}

void World::generateTerrain() {
    terrain.reserve((World::height / TILE_SIZE) * (World::width / TILE_SIZE));

    for (int y = 0; y < World::height / TILE_SIZE; y++) {
        for (int x = 0; x < World::width / TILE_SIZE; x++) {
            if ((x / 8) % 2 == 0) {
                if ((y / 8) % 2 == 0) {
                    terrain[y * (World::width / TILE_SIZE) + x] = &Tile::GRASS;
                } else {
                    terrain[y * (World::width / TILE_SIZE) + x] = &Tile::SAND;
                }
            } else {
                if ((y / 8) % 2 == 0) {
                    terrain[y * (World::width / TILE_SIZE) + x] = &Tile::STONE;
                } else {
                    terrain[y * (World::width / TILE_SIZE) + x] = &Tile::WATER;
                }
            }
        }
    }
}

void World::render() {
    for (int y = 0; y < World::height / TILE_SIZE; y++) {
        for (int x = 0; x < World::width / TILE_SIZE; x++) {
            SDL_Texture *t = terrain[y * (World::width / TILE_SIZE) + x]->texture;
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
    addFoodEntity(new FoodEntity(rand() % World::width, rand() % World::height, 4 * 60));
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
        if (toRemoveFood(e)) continue;
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

/**
 * Calculate dimensions (x & y position, width, height) of a world laying
 * on the node with the given MPI rank.
 *
 * @param rank Rank of the node, which world should be calculated
 * @param num Number of nodes in the MPI_COMM_WORLD
 *
 * @return dimensions of the world on the node with the given MPI rank
 */
WorldDim World::calcWorldDimensions(int rank, int num) {
    WorldDim dim;

    // Get Width and Height of the world
    int *rect = splitRect(num, overallWidth, overallHeight);
    dim.w = rect[0];
    dim.h = rect[1];

    // Get Position of the world (and update width and height if needed)
    int overlap;
    for (int i = 1; i <= rank; i++) {
        dim.x += dim.w;

        // Height overlap? -> Last row
        if ((dim.y + dim.h) > overallHeight) {
            overlap = (dim.y + dim.h) - overallHeight;
            dim.h -= overlap;
            dim.w += (dim.w * overlap) / dim.h;
        }

        // Width overlap?
        if ((dim.x + dim.w) >= overallWidth) {
            if (i == rank) {
                overlap = (dim.x + dim.w) - overallWidth;
                if (overlap == dim.w) {
                    dim.x = 0;
                    dim.y += dim.h;
                } else {
                    dim.w -= overlap;
                }
            } else {
                dim.x = -dim.w;
                dim.y += dim.h;
            }
        }

        // Last rectangle?
        if ((i + 1) == num)
            dim.w = overallWidth - dim.x;
    }

    return dim;
}

WorldDim World::getWorldDim() {
    return {x, y, width, height};
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

/**
 * Splits a given rectangle (with width and height) into num smaller
 * rectangles that have around the same area as the given rectangle
 *
 * @param   num     Number of rectangles, the given rectangle has to be split
 * @param   width   The width of the given rectangle
 * @param   height  The height of the given rectangle
 *
 * @return  Array with the width (1st index) and height (2nd index) for the
 *          resulting rectangle
 */
int *splitRect(int num, int width, int height) {
    static int dim[2] = {width, height};

    // Doesn't the rectangle have to be split?
    if (num < 2)
        return dim;

    // Split the rectangle (into two halves) as long as there are fewer rectangles then wanted
    int rects = 1;
    while (rects < num) {
        rects *= 2;
        if (width > height)
            width /= 2;
        else
            height /= 2;
    }

    // Split into too many rectangles?
    if (rects > num) {
        if (width > height)
            height += (height / num) * (rects - num);
        else
            width += (width / num) * (rects - num);
    }

    // Return width and height of one rectangle (~ 1/num of the original)
    dim[0] = width;
    dim[1] = height;
    return dim;
}

//TODO cleanup for destroyed entities