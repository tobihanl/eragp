#include <algorithm>
#include <cstdlib>
#include <cassert>
#include "mpi.h"
#include "World.h"
#include "Renderer.h"
#include "SimplexNoise/SimplexNoise.h"
#include "Rng.h"

int *splitRect(int num, int width, int height);

// TODO [VERY IMPORTANT!] Implement checking if entity already exists in a vector to prevent duplicates!

std::vector<FoodEntity *> World::food = std::vector<FoodEntity *>();
std::vector<LivingEntity *> World::living = std::vector<LivingEntity *>();

std::vector<LivingEntity *> World::removeLiving = std::vector<LivingEntity *>();
std::vector<FoodEntity *> World::removeFood = std::vector<FoodEntity *>();
std::vector<LivingEntity *> World::addLiving = std::vector<LivingEntity *>();
std::vector<FoodEntity *> World::addFood = std::vector<FoodEntity *>();

std::vector<MPISendEntity> World::livingEntitiesToMoveToNeighbors = std::vector<MPISendEntity>();
std::vector<MPISendEntity> World::foodToSendToNeighbors = std::vector<MPISendEntity>();
std::vector<MPISendEntity> World::removedFoodToSendToNeighbors = std::vector<MPISendEntity>();

std::vector<WorldDim> World::worlds = std::vector<WorldDim>();
std::vector<PaddingRect> World::paddingRects = std::vector<PaddingRect>();
std::vector<int> World::paddingRanks = std::vector<int>();

// Init static attributes
int World::overallWidth = 0;
int World::overallHeight = 0;

int World::MPI_Rank = 0;
int World::MPI_Nodes = 0;

int World::x = 0;
int World::y = 0;
int World::width = 0;
int World::height = 0;

int World::ticksPerFoodInterval = 0;
int World::foodPerFoodInterval = 0;
int World::intervalTicksLeft = 0;
int World::intervalFoodLeft = 0;
int World::foodEveryTick = 0;
int World::ticksToSkip = 0;
int World::minTicksToSkip = 0;
int World::maxTicksToSkip = 0;

bool World::isSetup = false;

SDL_Texture *World::background = nullptr;

std::vector<Tile *> World::terrain = std::vector<Tile *>();

/**
 * Initialize the world, which is part of the overall world and set
 * it up.
 *
 * @param   newOverallWidth     Width of the overall world
 * @param   newOverallHeight    Height of the overall world
 * @param   maimuc              Indicates, whether the program is executed
 *                              on MaiMUC or not
 */
void World::setup(int newOverallWidth, int newOverallHeight, bool maimuc, float foodRate) {
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

        overallWidth = 800 * 2;
        overallHeight = 600 * 5;
    } else {
        overallWidth = newOverallWidth;
        overallHeight = newOverallHeight;

        // Get dimensions for all worlds
        for (int i = 0; i < MPI_Nodes; i++)
            worlds.push_back(calcWorldDimensions(i, MPI_Nodes));
    }

    // Set dimension for this world
    x = worlds[MPI_Rank].p.x;
    y = worlds[MPI_Rank].p.y;
    width = worlds[MPI_Rank].w;
    height = worlds[MPI_Rank].h;

    generateTerrain();

    // Calculate padding area stuff
    calcPaddingRects();
    for (const auto &r : paddingRects)
        if (std::find(paddingRanks.begin(), paddingRanks.end(), r.rank) == paddingRanks.end())
            paddingRanks.push_back(r.rank);

    foodRate *= (float) (width * height) / (2000.f * TILE_SIZE * TILE_SIZE); //spawnRate of Node
    //convert spawnRate of node to fraction
    foodEveryTick = (int) foodRate;
    foodRate -= (float) foodEveryTick; //only consider food that needs to be distributed
    long greatestCommonDivisor = gcd((long) round(foodRate * MAX_FOOD_INTERVAL), MAX_FOOD_INTERVAL);
    ticksPerFoodInterval = MAX_FOOD_INTERVAL / greatestCommonDivisor;
    foodPerFoodInterval = (int) ((long) round(foodRate * MAX_FOOD_INTERVAL) / greatestCommonDivisor);

    minTicksToSkip = (int) floor((float) (ticksPerFoodInterval - foodPerFoodInterval) / (float) foodPerFoodInterval);
    maxTicksToSkip = (int) ceil((float) (ticksPerFoodInterval - foodPerFoodInterval) / (float) foodPerFoodInterval);

    // Setup done
    isSetup = true;
}

void World::finalize() {
    for (const auto &e : food) delete e;
    for (const auto &e : living) delete e;

    food.clear();
    living.clear();
}

void World::generateTerrain() {
    int heightWithPadding = height + (2 * WORLD_PADDING) + (y % TILE_SIZE) + (TILE_SIZE - (y + height) % TILE_SIZE);
    int widthWithPadding = width + (2 * WORLD_PADDING) + (x % TILE_SIZE) + (TILE_SIZE - (x + width) % TILE_SIZE);

    int xOffset = x - WORLD_PADDING - (x % TILE_SIZE);
    int yOffset = y - WORLD_PADDING - (y % TILE_SIZE);

    for (int py = 0; py < heightWithPadding / TILE_SIZE; py++) {
        for (int px = 0; px < widthWithPadding / TILE_SIZE; px++) {
            // Noise
            float val = SimplexNoise::noise((float) ((px * TILE_SIZE) + xOffset) / (36.f * TILE_SIZE),
                                            (float) ((py * TILE_SIZE) + yOffset) / (36.f * TILE_SIZE));

            if (val < -0.4)
                terrain.push_back(&Tile::WATER);
            else if (val < -0.2)
                terrain.push_back(&Tile::SAND);
            else if (val < 0.7)
                terrain.push_back(&Tile::GRASS);
            else
                terrain.push_back(&Tile::STONE);
        }
    }
}

void World::renderTerrain() {
    int heightWithPadding = height + (2 * WORLD_PADDING) + (y % TILE_SIZE) + (TILE_SIZE - (y + height) % TILE_SIZE);
    int widthWithPadding = width + (2 * WORLD_PADDING) + (x % TILE_SIZE) + (TILE_SIZE - (x + width) % TILE_SIZE);

    // Pre-render terrain for faster rendering
    background = Renderer::createTexture(widthWithPadding, heightWithPadding, SDL_TEXTUREACCESS_TARGET);
    Renderer::setTarget(background);
    Renderer::clear();

    // Copy textures to background
    for (int py = 0; py < heightWithPadding / TILE_SIZE; py++) {
        for (int px = 0; px < widthWithPadding / TILE_SIZE; px++) {
            Renderer::copy(terrain[py * (widthWithPadding / TILE_SIZE) + px]->texture,
                           px * TILE_SIZE,
                           py * TILE_SIZE);
        }
    }

    // Change render target back to default
    Renderer::present();
    Renderer::setTarget(nullptr);
}

void World::render() {
    if (World::background == nullptr) renderTerrain();
    Renderer::copy(World::background, -(WORLD_PADDING + (x % TILE_SIZE)), -(WORLD_PADDING + (y % TILE_SIZE)));

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
    if (intervalTicksLeft == 0) {
        assert(intervalFoodLeft == 0 && "Not enough food spawned!");
        intervalFoodLeft = foodPerFoodInterval;
        intervalTicksLeft = ticksPerFoodInterval;
        ticksToSkip = 0;
    }
    for (int i = 0; i < foodEveryTick; i++) {
        addFoodEntity(new FoodEntity(getRandomIntBetween(0, World::width) + World::x,
                                     getRandomIntBetween(0, World::height) + World::y, 8 * 60), false);
    }
    intervalTicksLeft--;
    if (ticksToSkip == 0 && intervalFoodLeft > 0) {
        intervalFoodLeft--;
        addFoodEntity(new FoodEntity(getRandomIntBetween(0, World::width) + World::x,
                                     getRandomIntBetween(0, World::height) + World::y, 8 * 60), false);
        if (intervalTicksLeft != 0)
            ticksToSkip = ((float) intervalFoodLeft / (float) intervalTicksLeft <
                           (float) foodPerFoodInterval / (float) ticksPerFoodInterval) ? maxTicksToSkip
                                                                                       : minTicksToSkip;
    } else {
        ticksToSkip--;
    }


    for (const auto &e : living) {
        // Before moving: Is entity on THIS node?
        bool beforeOnThisNode = pointInRect({e->x, e->y}, {{x, y}, width, height});

        e->tick();

        // Entity dead?
        if (toRemoveLiving(e)) continue;

        // TODO: Think about better solution (i.e. update entity accordingly)!
        // Was entity in padding area? -> Get's overwritten by other node!
        if (!beforeOnThisNode) {
            removeLivingEntity(e);
            continue;
        }

        // After moving: Entity on THIS node?
        if (pointInRect({e->x, e->y}, {{x, y}, width, height})) {
            // Send entity if needed
            auto *ranks = paddingRanksAt(e->x, e->y);
            for (int neighbor : *ranks)
                livingEntitiesToMoveToNeighbors.push_back({neighbor, e});
            delete ranks;
        } else {
            // Send entity to other node
            livingEntitiesToMoveToNeighbors.push_back({static_cast<int>(rankAt(e->x, e->y)), e});
            removeLivingEntity(e);

            /* TODO: Re-enable! (Right now, a node NOT being a neighbor gets data sent with the code below)
            // Send entity to nodes having a padding at this position (excluding THIS node!)
            auto *ranks = paddingRanksAt(e->x, e->y); // TODO: Not working as this only works for coordinates on THIS node!
            for (int neighbor : *ranks) {
                if (neighbor != MPI_Rank) livingEntitiesToMoveToNeighbors.push_back({neighbor, e});
            }
            delete ranks;
            */
        }
    }

    //=============================================================================
    //                            BEGIN MPI SEND/RECEIVE
    //=============================================================================
    MPI_Request reqs[paddingRanks.size() * MSGS_PER_NEIGHBOR];
    MPI_Status stats[paddingRanks.size() * MSGS_PER_NEIGHBOR];
    void *buffers[paddingRanks.size() * MSGS_PER_NEIGHBOR];

    //############################# SEND ENTITIES #############################
    for (int i = 0; i < (int) paddingRanks.size(); i++) {
        buffers[MSGS_PER_NEIGHBOR * i] = sendEntities(livingEntitiesToMoveToNeighbors, paddingRanks[i],
                                                      MPI_TAG_LIVING_ENTITY,
                                                      &reqs[MSGS_PER_NEIGHBOR * i]);
        buffers[MSGS_PER_NEIGHBOR * i + 1] = sendEntities(foodToSendToNeighbors, paddingRanks[i],
                                                          MPI_TAG_FOOD_ENTITY,
                                                          &reqs[MSGS_PER_NEIGHBOR * i + 1]);
        buffers[MSGS_PER_NEIGHBOR * i + 2] = sendEntities(removedFoodToSendToNeighbors, paddingRanks[i],
                                                          MPI_TAG_REMOVED_FOOD_ENTITY,
                                                          &reqs[MSGS_PER_NEIGHBOR * i + 2]);
    }

    // Clear sending storage
    livingEntitiesToMoveToNeighbors.clear();
    foodToSendToNeighbors.clear();
    removedFoodToSendToNeighbors.clear();

    //############################ RECEIVE ENTITIES ###########################
    for (int paddingOfRank : paddingRanks) {
        receiveEntities(paddingOfRank, MPI_TAG_LIVING_ENTITY);
        receiveEntities(paddingOfRank, MPI_TAG_FOOD_ENTITY);
        receiveEntities(paddingOfRank, MPI_TAG_REMOVED_FOOD_ENTITY);
    }

    // Wait for all sends to complete and afterwards free buffers
    MPI_Waitall((int) paddingRanks.size() * MSGS_PER_NEIGHBOR, reqs, stats);
    for (void *e : buffers)
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

void *World::sendEntities(const std::vector<MPISendEntity> &entities, int rank, int tag, MPI_Request *request) {
    int totalSize = 0;
    for (const auto &e : entities)
        if (e.rank == rank) totalSize += e.entity->serializedSize();

    void *buffer = malloc(totalSize);
    void *start = buffer;

    for (const auto &e : entities)
        if (e.rank == rank)
            e.entity->serialize(buffer);

    MPI_Isend(start, totalSize, MPI_BYTE, rank, tag, MPI_COMM_WORLD, request);
    return start;
}

void World::receiveEntities(int rank, int tag) {
    int receivedBytes;

    // MPI Probe to get information about the message (i.e. length)
    MPI_Status probeStat;
    MPI_Probe(rank, tag, MPI_COMM_WORLD, &probeStat);
    MPI_Get_count(&probeStat, MPI_BYTE, &receivedBytes);

    void *buffer = malloc(receivedBytes);
    void *start = buffer;

    MPI_Status stat;
    MPI_Recv(buffer, receivedBytes, MPI_BYTE, rank, tag, MPI_COMM_WORLD, &stat);

    while (buffer < (char *) start + receivedBytes) {
        Entity *e;
        switch (tag) {
            case MPI_TAG_LIVING_ENTITY:
                e = new LivingEntity(buffer);
                addLivingEntity((LivingEntity *) e, true);
                break;

            case MPI_TAG_FOOD_ENTITY:
                e = new FoodEntity(buffer);
                addFoodEntity((FoodEntity *) e, true);
                break;

            case MPI_TAG_REMOVED_FOOD_ENTITY:
                e = new FoodEntity(buffer);
                for (const auto &f : food) {
                    if (e->getId() == f->getId() && !toRemoveFood(f))
                        removeFoodEntity(f, true);
                }
                delete e;
                break;

            default:
                break;
        }
    }

    free(start);
}

FoodEntity *World::findNearestFood(int px, int py) {
    if (food.empty()) return nullptr;
    FoodEntity *f = nullptr;
    int dist = 0;
    for (const auto &e : food) {
        int tempDist = e->getSquaredDistance(px, py);
        if (tempDist <= VIEW_RANGE_SQUARED && (!f || tempDist < dist)) {
            f = e;
            dist = tempDist;
        }
    }
    return f;
}

FoodEntity *World::findNearestSurvivingFood(int px, int py) {
    FoodEntity *f = nullptr;
    int dist = 0;
    for (const auto &e : food) {
        if (toRemoveFood(e)) continue;
        int tempDist = e->getSquaredDistance(px, py);
        if (tempDist <= VIEW_RANGE_SQUARED && (!f || tempDist < dist)) {
            f = e;
            dist = tempDist;
        }
    }
    return f;
}

/**
 * @param id    ID of the LivingEntity, which will be excluded for the
 *              search of the nearest LivingEntity
 */
LivingEntity *World::findNearestLiving(int px, int py, int id) {
    LivingEntity *n = nullptr;
    int dist = 0;
    for (const auto &e : living) {
        if (e->getId() == id) continue;
        int tempDist = e->getSquaredDistance(px, py);
        if (tempDist <= VIEW_RANGE_SQUARED && (!n || tempDist < dist)) {
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
        if (*e == *le || le->difference(*e) < 0.04 || !e->visibleOn(tileAt(le->x, le->y))) continue;
        int tempDist = e->getSquaredDistance(le->x, le->y);
        if (tempDist <= VIEW_RANGE_SQUARED && (!n || tempDist < dist)) {
            n = e;
            dist = tempDist;
        }
    }
    return n;
}

LivingEntity *World::findNearestSurvivingEnemy(LivingEntity *le) {
    if (living.empty() || living.size() == 1) return nullptr;
    LivingEntity *n = nullptr;
    int dist = 0;
    for (const auto &e : living) {
        if (toRemoveLiving(e) || *e == *le || le->difference(*e) < 0.04 || !e->visibleOn(tileAt(le->x, le->y)))
            continue;
        int tempDist = e->getSquaredDistance(le->x, le->y);
        if (tempDist <= VIEW_RANGE_SQUARED && (!n || tempDist < dist)) {
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
        if (tempDist <= VIEW_RANGE_SQUARED && (!n || tempDist < dist)) {
            n = e;
            dist = tempDist;
        }
    }
    return n;
}

LivingEntity *World::findNearestSurvivingMate(LivingEntity *le) {
    if (living.empty() || living.size() == 1) return nullptr;
    LivingEntity *n = nullptr;
    int dist = 0;
    for (const auto &e : living) {
        if (toRemoveLiving(e) || *e == *le || le->difference(*e) >= 0.04) continue;
        int tempDist = e->getSquaredDistance(le->x, le->y);
        if (tempDist <= VIEW_RANGE_SQUARED && (!n || tempDist < dist)) {
            n = e;
            dist = tempDist;
        }
    }
    return n;
}

bool World::addLivingEntity(LivingEntity *e, bool received) {
    if (toAddLiving(e)) return false;

    // Received via MPI? -> Always add!
    if (received) {
        addLiving.push_back(e);
        return true;
    }

    // Only add and broadcast to other nodes when laying on THIS node
    if (pointInRect({e->x, e->y}, {{x, y}, width, height})) {
        addLiving.push_back(e);

        auto *ranks = paddingRanksAt(e->x, e->y);
        for (int neighbor : *ranks)
            livingEntitiesToMoveToNeighbors.push_back({neighbor, e});
        delete ranks;

        return true;
    }

    return false;
}

bool World::addFoodEntity(FoodEntity *e, bool received) {
    if (toAddFood(e)) return false;

    // Received via MPI? -> Always add!
    if (received) {
        addFood.push_back(e);
        return true;
    }

    // Only add and broadcast to other nodes when laying on THIS node
    if (pointInRect({e->x, e->y}, {{x, y}, width, height})) {
        addFood.push_back(e);

        auto *ranks = paddingRanksAt(e->x, e->y);
        for (int neighbor : *ranks)
            foodToSendToNeighbors.push_back({neighbor, e});
        delete ranks;

        return true;
    }

    return false;
}

bool World::removeLivingEntity(LivingEntity *e) {
    if (!toRemoveLiving(e)) {
        removeLiving.push_back(e);
        return true;
    }

    return false;
}

bool World::removeFoodEntity(FoodEntity *e, bool received) {
    assert(!toRemoveFood(e) && "Tried to remove same FoodEntity multiple times");

    // Received via MPI? -> Always remove!
    if (received) {
        removeFood.push_back(e);
        return true;
    }

    // Only remove and broadcast to other nodes when laying on THIS node
    if (pointInRect({e->x, e->y}, {{x, y}, width, height})) {
        removeFood.push_back(e);

        auto *ranks = paddingRanksAt(e->x, e->y);
        for (int neighbor : *ranks)
            removedFoodToSendToNeighbors.push_back({neighbor, e});
        delete ranks;

        return true;
    }

    return false;
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
        dim.p.x += dim.w;

        // Height overlap? -> Last row
        if ((dim.p.y + dim.h) > overallHeight) {
            overlap = (dim.p.y + dim.h) - overallHeight;
            dim.h -= overlap;
            dim.w += (dim.w * overlap) / dim.h;
        }

        // Width overlap?
        if ((dim.p.x + dim.w) >= overallWidth) {
            if (i == rank) {
                overlap = (dim.p.x + dim.w) - overallWidth;
                if (overlap == dim.w) {
                    dim.p.x = 0;
                    dim.p.y += dim.h;
                } else {
                    dim.w -= overlap;
                }
            } else {
                dim.p.x = -dim.w;
                dim.p.y += dim.h;
            }
        }

        // Last rectangle?
        if ((i + 1) == num)
            dim.w = overallWidth - dim.p.x;
    }

    return dim;
}

void World::calcPaddingRects() {
    WorldDim world = getWorldDim();
    for (size_t i = 0; i < worlds.size(); i++) {
        if (i == MPI_Rank) continue;

        WorldDim otherWorld = worlds[i];
        otherWorld.p.x -= WORLD_PADDING;
        otherWorld.p.y -= WORLD_PADDING;
        otherWorld.w += 2 * WORLD_PADDING;
        otherWorld.h += 2 * WORLD_PADDING;

        Rect intersection = calcIntersection(world, otherWorld);
        if (intersection.w > 0 && intersection.h > 0)
            paddingRects.push_back({(int) i, intersection});
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

size_t World::rankAt(int px, int py) {
    //TODO could cause overflow for large worlds. Use long instead?
    Point p = {(px + overallWidth) % overallWidth, (py + overallHeight) % overallHeight};
    for (size_t i = 0; i < worlds.size(); i++) {
        if (pointInRect(p, worlds[i]))
            return i;
    }
    return -1; // In case there's no matching world
}

/**
 * @return      Ranks having a padding on the given coordinates
 *
 * @attention   Only works for (x,y) coordinates on THIS node!
 */
std::vector<size_t> *World::paddingRanksAt(int px, int py) {
    assert(px >= x && px < x + width && py >= y && py < y + height && "Coordinates NOT on THIS node");

    auto *ranks = new std::vector<size_t>();
    for (const auto &p : paddingRects)
        if (pointInRect({px, py}, p.rect))
            ranks->push_back(p.rank);

    return ranks;
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

Tile *World::tileAt(int px, int py) {
    if (px < x - WORLD_PADDING || px >= x + width + WORLD_PADDING ||
        py < y - WORLD_PADDING || py >= y + height + WORLD_PADDING)
        return &Tile::INVALID;

    int xOffset = x - WORLD_PADDING - (x % TILE_SIZE);
    int yOffset = y - WORLD_PADDING - (y % TILE_SIZE);
    int widthWithPadding = width + (2 * WORLD_PADDING) + (x % TILE_SIZE) + (TILE_SIZE - (x + width) % TILE_SIZE);

    return terrain[((py - yOffset) / TILE_SIZE) * (widthWithPadding / TILE_SIZE) + ((px - xOffset) / TILE_SIZE)];
}

//TODO cleanup for destroyed entities

long World::gcd(long a, long b) {
    if (a == 0) return b;
    if (b == 0) return a;
    if (a < b) return gcd(a, b % a);
    return gcd(b, a % b);
}

Rect World::calcIntersection(const Rect &rect1, const Rect &rect2) {
    int x1 = std::max(rect1.p.x, rect2.p.x);
    int y1 = std::max(rect1.p.y, rect2.p.y);
    int x2 = std::min(rect1.p.x + rect1.w, rect2.p.x + rect2.w);
    int y2 = std::min(rect1.p.y + rect1.h, rect2.p.y + rect2.h);

    return {{x1, y1}, x2 - x1, y2 - y1};
}

bool World::pointInRect(const Point &p, const Rect &r) {
    return r.p.x <= p.x && p.x < r.p.x + r.w && r.p.y <= p.y && p.y < r.p.y + r.h;
}

std::vector<PaddingRect> *World::getPaddingRects() {
    return &paddingRects;
}
