#include <mpi.h>
#include <algorithm>
#include <cstdlib>
#include <cassert>
#include "World.h"
#include "Renderer.h"
#include "SimplexNoise/SimplexNoise.h"

int *splitRect(int num, int width, int height);

// TODO [VERY IMPORTANT!] Implement checking if entity already exists in a vector to prevent duplicates!

std::vector<FoodEntity *> World::food = std::vector<FoodEntity *>();
std::vector<LivingEntity *> World::living = std::vector<LivingEntity *>();

std::vector<LivingEntity *> World::removeLiving = std::vector<LivingEntity *>();
std::vector<FoodEntity *> World::removeFood = std::vector<FoodEntity *>();
std::vector<LivingEntity *> World::addLiving = std::vector<LivingEntity *>();
std::vector<FoodEntity *> World::addFood = std::vector<FoodEntity *>();

std::vector<MPISendEntity> World::livingEntitiesToMoveToNeighbors = std::vector<MPISendEntity>();
std::vector<WorldDim> World::worlds = std::vector<WorldDim>();
std::vector<int> World::neighbors = std::vector<int>();

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

SDL_Texture *World::background = nullptr;

std::vector<Tile *> World::terrain = std::vector<Tile *>();

/**
 * Initialize the world, which is part of the overall world and set
 * it up.
 *
 * @param   overallWidth    Width of the overall world
 * @param   overallHeight   Height of the overall world
 * @param   maimuc          Indicates, whether the program is executed on
 *                          MaiMUC or not
 */
void World::setup(int overallWidth, int overallHeight, bool maimuc) {
    if (isSetup)
        return;

    // Get MPI Rank and number of nodes
    MPI_Comm_rank(MPI_COMM_WORLD, &MPI_Rank);
    MPI_Comm_size(MPI_COMM_WORLD, &MPI_Nodes);

    // MaiMUC specific configuration?
    if (maimuc) {
        if (MPI_Nodes != NUMBER_OF_MAIMUC_NODES) {
            std::cerr << "Program started on MaiMUC without running on 10 nodes!" << std::endl;
            abort();
        }

        // Get dimensions for all worlds
        for (int i = 0; i < NUMBER_OF_MAIMUC_NODES; i++)
            worlds.push_back({
                                     ((i % 2) == 0) ? 0 : 800,
                                     (i / 2) * 600,
                                     800,
                                     600
                             });

        World::overallWidth = 800 * 2;
        World::overallHeight = 600 * 5;
    } else {
        World::overallWidth = overallWidth;
        World::overallHeight = overallHeight;

        // Get dimensions for all worlds
        for (int i = 0; i < MPI_Nodes; i++)
            worlds.push_back(calcWorldDimensions(i, MPI_Nodes));
    }

    // Set dimension for this world
    x = worlds[MPI_Rank].x;
    y = worlds[MPI_Rank].y;
    width = worlds[MPI_Rank].w;
    height = worlds[MPI_Rank].h;

    generateTerrain();
    calcNeighbors();
    isSetup = true;
}

void World::generateTerrain() {
    terrain.reserve((World::height / TILE_SIZE) * (World::width / TILE_SIZE));

    float xOffset = (float) World::x / TILE_SIZE;
    float yOffset = (float) World::y / TILE_SIZE;

    for (int y = 0; y < World::height / TILE_SIZE; y++) {
        for (int x = 0; x < World::width / TILE_SIZE; x++) {
            float val = SimplexNoise::noise(((float) x + xOffset) / 36.f, ((float) y + yOffset) / 36.f);

            if (val < -0.4) {
                terrain[y * (World::width / TILE_SIZE) + x] = &Tile::WATER;
            } else if (val < -0.2) {
                terrain[y * (World::width / TILE_SIZE) + x] = &Tile::SAND;
            } else if (val < 0.7) {
                terrain[y * (World::width / TILE_SIZE) + x] = &Tile::GRASS;
            } else {
                terrain[y * (World::width / TILE_SIZE) + x] = &Tile::STONE;
            }
        }
    }
}

void World::renderTerrain() {
    // Pre-render terrain for faster rendering
    World::background = Renderer::createTexture(World::width, World::height, SDL_TEXTUREACCESS_TARGET);
    Renderer::setTarget(World::background);
    Renderer::clear();

    // Copy textures to background
    for (int y = 0; y < World::height / TILE_SIZE; y++)
        for (int x = 0; x < World::width / TILE_SIZE; x++)
            Renderer::copy(terrain[y * (World::width / TILE_SIZE) + x]->texture, x * TILE_SIZE, y * TILE_SIZE);

    // Change render target back to default
    Renderer::present();
    Renderer::setTarget(nullptr);
}

void World::render() {
    if (World::background == nullptr) renderTerrain();
    Renderer::copy(World::background, 0, 0);

    for (const auto &f : food) {
        f->render();
    }
    for (const auto &e : living) {
        e->render();
    }

    // Show the rank of the node in the upper left of the window
    SDL_Texture *t = Renderer::renderFont(std::to_string(MPI_Rank), 25, {255, 255, 255, 255}, "font.ttf");
    Renderer::copy(t, 10, 10);
}

void World::tick() {
    //TODO the same amount of food is spawned, regardless of the size of the node
    addFoodEntity(new FoodEntity((rand() % World::width) + World::x, (rand() % World::height) + World::y, 8 * 60));
    for (const auto &e : living) {
        //TODO change or remove with padding (because one entity can be on multiple nodes)
        assert(e->x >= x && e->x < x + width && e->y >= y && e->y < y + height && "Coordinates don't match node.");
        e->tick();

        // Send to OTHER node?
        if (e->x >= x + width || e->x < x || e->y >= y + height || e->y < y) {
            int rank = World::getRankAt(e->x, e->y);
            if (rank != World::getMPIRank()) World::moveToNeighbor(e, rank);
        }
    }

    //=============================================================================
    //                            BEGIN MPI SEND/RECEIVE
    //=============================================================================
    MPI_Request reqs[numOfNeighbors()];
    MPI_Status stats[numOfNeighbors()];
    void *bufferStarts[numOfNeighbors()];

    for (int i = 0; i < numOfNeighbors(); i++) {
        int totalSize = 0;
        for (const auto &e : livingEntitiesToMoveToNeighbors)
            if (e.rank == neighbors[i]) totalSize += e.entity->serializedSize();

        void *buffer = malloc(totalSize);
        bufferStarts[i] = buffer;

        for (const auto &e : livingEntitiesToMoveToNeighbors) {
            if (e.rank == neighbors[i]) {
                e.entity->serialize(buffer);
                // Method moveToNeighbor() guarantees that e.entity is of type LivingEntity*!!!
                removeLivingEntity((LivingEntity *) e.entity);
            }
        }

        MPI_Isend(bufferStarts[i], totalSize, MPI_BYTE, neighbors[i], MPI_TAG_LIVING_ENTITY, MPI_COMM_WORLD, &reqs[i]);
    }

    livingEntitiesToMoveToNeighbors.clear();

    for (int i = 0; i < numOfNeighbors(); i++) {
        // MPI Probe to get information about the message (i.e. length)
        int receivedBytes;
        MPI_Status probeStat;
        MPI_Probe(neighbors[i], MPI_TAG_LIVING_ENTITY, MPI_COMM_WORLD, &probeStat);
        MPI_Get_count(&probeStat, MPI_BYTE, &receivedBytes);

        void *buffer = malloc(receivedBytes);
        void *start = buffer;

        MPI_Status stat;
        MPI_Recv(buffer, receivedBytes, MPI_BYTE, neighbors[i], MPI_TAG_LIVING_ENTITY, MPI_COMM_WORLD, &stat);

        while (buffer < (char *) start + receivedBytes) {
            auto *e = new LivingEntity(buffer);
            addLivingEntity(e);
        }
        free(start);
    }

    // Wait for all sends and afterwards free buffers
    MPI_Waitall(numOfNeighbors(), reqs, stats);
    for (void *e : bufferStarts)
        free(e);
    //=============================================================================
    //                             END MPI SEND/RECEIVE
    //=============================================================================

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

LivingEntity *World::findNearestLiving(LivingEntity *le) {
    if (living.empty() || living.size() == 1) return nullptr;
    LivingEntity *n = living[0];
    if (*n == *le) n = living[1];
    int dist = n->getSquaredDistance(le->x, le->y);
    for (const auto &e : living) {
        if (*e == *le) continue;
        int tempDist = e->getSquaredDistance(le->x, le->y);
        if (tempDist < dist) {
            n = e;
            dist = tempDist;
        }
    }
    return n;
}

/**
 * @param id    ID of the LivingEntity, which will be excluded for the
 *              search of the nearest LivingEntity
 */
LivingEntity *World::findNearestLiving(int x, int y, int id) {
    LivingEntity *n = nullptr;
    int dist = 0;
    for (const auto &e : living) {
        if (n && n->getId() == id) continue;
        int tempDist = e->getSquaredDistance(x, y);
        if (!n || tempDist < dist) {
            n = e;
            dist = tempDist;
        }
    }
    return n;
}

LivingEntity *World::findNearestEnemy(LivingEntity *le) {
    if (living.empty() || living.size() == 1) return nullptr;
    LivingEntity *n = nullptr;
    int dist = 0;
    for (const auto &e : living) {
        if (*e == *le || le->difference(*e) < 0.04) continue;
        int tempDist = e->getSquaredDistance(le->x, le->y);
        if (!n || tempDist < dist) {
            n = e;
            dist = tempDist;
        }
    }
    return n;
}

LivingEntity *World::findNearestMate(LivingEntity *le) {
    if (living.empty() || living.size() == 1) return nullptr;
    LivingEntity *n = nullptr;
    int dist = 0;
    for (const auto &e : living) {
        if (*e == *le || le->difference(*e) >= 0.04) continue;
        int tempDist = e->getSquaredDistance(le->x, le->y);
        if (!n || tempDist < dist) {
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

void World::calcNeighbors() {
    int start1, end1, start2, end2;
    for (size_t i = 0; i < worlds.size(); i++) {
        auto dim = worlds[i];

        // Upper or lower line (of THIS world)?
        if ((dim.y + dim.h) % overallHeight == y ||
            (y + height) % overallHeight == dim.y) {
            // 1st line
            start1 = x;
            end1 = x + width;

            // 2nd line
            start2 = dim.x;
            end2 = dim.x + dim.w;

            // Do the lines touch each other?
            if ((start1 <= start2 && start2 <= end1) ||
                (start1 <= end2 && end2 <= end1) ||
                (start2 <= start1 && end1 <= end2) ||
                (start1 == 0 && end2 == overallWidth) ||
                (end1 == overallWidth && start2 == 0)) {
                neighbors.push_back(i);
            }
        } else if ((x + width) % overallWidth == dim.x ||
                   (dim.x + dim.w) % overallWidth == x) { // Right or left line (of THIS world)?
            // 1st line
            start1 = y;
            end1 = y + height;

            // 2nd line
            start2 = dim.y;
            end2 = dim.y + dim.h;

            // Do the lines touch each other?
            if ((start1 <= start2 && start2 <= end1) ||
                (start1 <= end2 && end2 <= end1) ||
                (start2 <= start1 && end1 <= end2) ||
                (start1 == 0 && end2 == overallWidth) ||
                (end1 == overallWidth && start2 == 0)) {
                neighbors.push_back(i);
            }
        }
    }
}

WorldDim World::getWorldDim() {
    return getWorldDimOf(MPI_Rank);
}

WorldDim World::getWorldDimOf(int rank) {
    return worlds[rank];
}

int World::getMPIRank() {
    return MPI_Rank;
}

int World::getMPINodes() {
    return MPI_Nodes;
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

void World::moveToNeighbor(LivingEntity *e, int rank) {
    assert(std::find(neighbors.begin(), neighbors.end(), rank) != neighbors.end() && "Given rank not a neighbor!");
    livingEntitiesToMoveToNeighbors.push_back({rank, e});
}

int World::numOfNeighbors() {
    return neighbors.size();
}

size_t World::getRankAt(int x, int y) {
    x = (x + overallWidth) % overallWidth; //TODO could cause overflow for large worlds. Use long instead?
    y = (y + overallHeight) % overallHeight;

    for (size_t i = 0; i < worlds.size(); i++) {
        if (worlds[i].x <= x && x < worlds[i].x + worlds[i].w && worlds[i].y <= y && y < worlds[i].y + worlds[i].h)
            return i;
    }
    return -1; // In case there's no matching world
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

Tile *World::tileAt(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) return &Tile::INVALID;
    return terrain[(y / TILE_SIZE) * (width / TILE_SIZE) + (x / TILE_SIZE)];
}

//TODO cleanup for destroyed entities
