#include <cstdlib>
#include <cassert>
#include "SimplexNoise/SimplexNoise.h"
#include "World.h"
#include "Rng.h"
#include "Log.h"

#ifdef RENDER

#include "Renderer.h"

#endif

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

WorldDim *World::worlds = nullptr;
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

std::vector<Tile *> World::terrain = std::vector<Tile *>();

void World::setup(int newOverallWidth, int newOverallHeight, bool maimuc, float foodRate, float zoom) {
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
        worlds = new WorldDim[NUMBER_OF_MAIMUC_NODES];
        for (int i = 0; i < NUMBER_OF_MAIMUC_NODES; i++)
            worlds[i] = {((i % 2) == 0) ? 0 : 800, (i / 2) * 600, 800, 600};

        overallWidth = 800 * 2;
        overallHeight = 600 * 5;
    } else {
        overallWidth = newOverallWidth;
        overallHeight = newOverallHeight;

        // Get dimensions for all worlds
        worlds = new WorldDim[MPI_Nodes];
        calcWorldDimensions(worlds, 0, MPI_Nodes - 1, 0, 0, overallWidth, overallHeight);
    }

    // Set dimension for this world
    x = worlds[MPI_Rank].p.x;
    y = worlds[MPI_Rank].p.y;
    width = worlds[MPI_Rank].w;
    height = worlds[MPI_Rank].h;

    generateTerrain(zoom);

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

    delete[] worlds;
}

void World::generateTerrain(float zoom) {
    int heightWithPadding = height + (2 * WORLD_PADDING) + (y % TILE_SIZE) + (TILE_SIZE - (y + height) % TILE_SIZE);
    int widthWithPadding = width + (2 * WORLD_PADDING) + (x % TILE_SIZE) + (TILE_SIZE - (x + width) % TILE_SIZE);

    int xOffset = x - WORLD_PADDING - (x % TILE_SIZE);
    int yOffset = y - WORLD_PADDING - (y % TILE_SIZE);

    for (int py = 0; py < heightWithPadding / TILE_SIZE; py++) {
        for (int px = 0; px < widthWithPadding / TILE_SIZE; px++) {
            // Noise
            float val = SimplexNoise::noise((float) ((px * TILE_SIZE) + xOffset) / (36.f * zoom * TILE_SIZE),
                                            (float) ((py * TILE_SIZE) + yOffset) / (36.f * zoom * TILE_SIZE));

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

#ifdef RENDER
            // Render entity as it will be deleted before world will be rendered!
            // TODO: Improve?
            Renderer::renderEntity(e->getRenderData());
#endif

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
    int mpiTime = Log::currentTime();
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

    Log::data.mpi = Log::endTime(mpiTime);
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

void *World::sendEntities(const std::vector<MPISendEntity> &entityVec, int rank, int tag, MPI_Request *request) {
    int totalSize = 0;
    for (const auto &e : entityVec)
        if (e.rank == rank) totalSize += e.entity->serializedSize();

    void *buffer = malloc(totalSize);
    void *start = buffer;

    for (const auto &e : entityVec)
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

FoodEntity *World::findNearestFood(int px, int py, bool surviving) {
    FoodEntity *f = nullptr;
    int dist = VIEW_RANGE_SQUARED + 1;
    for (const auto &e : food) {
        if (surviving && toRemoveFood(e)) continue;
        int tempDist = e->getSquaredDistance(px, py);
        if (tempDist <= VIEW_RANGE_SQUARED && tempDist < dist) {
            f = e;
            dist = tempDist;
        }
    }
    return f;
}

NearestLiving World::findNearestLiving(LivingEntity *le, bool surviving) {
    NearestLiving nearest{nullptr, nullptr};
    int distEnemy = VIEW_RANGE_SQUARED + 1;
    int distMate = VIEW_RANGE_SQUARED + 1;

    for (const auto &e : living) {
        if (*e == *le || (surviving && toRemoveLiving(e))) continue;

        bool isEnemy = le->squaredDifference(*e) >= ENEMY_MATE_SQUARED_DIFFERENCE_THRESHOLD;
        if (isEnemy && !e->visibleOn(tileAt(le->x, le->y))) continue;

        int dist = e->getSquaredDistance(le->x, le->y);
        if (dist <= VIEW_RANGE_SQUARED) {
            if (isEnemy && dist < distEnemy) {
                nearest.enemy = e;
                distEnemy = dist;
            } else if (dist < distMate) {
                nearest.mate = e;
                distMate = dist;
            }
        }
    }
    return nearest;
}

LivingEntity *World::findNearestLivingToPoint(int px, int py) {
    LivingEntity *n = nullptr;
    int dist = VIEW_RANGE_SQUARED + 1;
    for (const auto &e : living) {
        int tempDist = e->getSquaredDistance(px, py);
        if (tempDist <= VIEW_RANGE_SQUARED && tempDist < dist) {
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

void World::calcWorldDimensions(WorldDim *dims, int rankStart, int rankEnd, int px, int py, int w, int h) {
    int n = rankEnd - rankStart + 1;

    // No split?
    if (n < 1) return;

    // Only split into 1?
    if (n == 1) {
        dims[0] = {{px, py}, w, h};
        return;
    }

    // Only split into 3?
    if (n == 3) {
        if (w > h) {
            int newW = w / 3;
            dims[rankStart] = {{px, py}, newW, h};
            dims[rankStart + 1] = {{px + newW, py}, newW, h};
            dims[rankStart + 2] = {{px + 2 * newW, py}, newW + (w % 3), h};
        } else {
            int newH = h / 3;
            dims[rankStart] = {{px, py}, w, newH};
            dims[rankStart + 1] = {{px, py + newH}, w, newH};
            dims[rankStart + 2] = {{px, py + 2 * newH}, w, newH + (h % 3)};
        }
        return;
    }

    int middle = (rankStart + rankEnd) / 2;
    if (w > h) {
        int newW = w / 2;
        if (n == 2) {
            dims[rankStart] = {{px, py}, newW, h};
            dims[rankStart + 1] = {{px + newW, py}, newW + (w % 2), h};
        } else {
            calcWorldDimensions(dims, rankStart, middle, px, py, newW, h);
            calcWorldDimensions(dims, middle + 1, rankEnd, px + newW, py, newW + (w % 2), h);
        }
    } else {
        int newH = h / 2;
        if (n == 2) {
            dims[rankStart] = {{px, py}, w, newH};
            dims[rankStart + 1] = {{px, py + newH}, w, newH + (h % 2)};
        } else {
            calcWorldDimensions(dims, rankStart, middle, px, py, w, newH);
            calcWorldDimensions(dims, middle + 1, rankEnd, px, py + newH, w, newH + (h % 2));
        }
    }
}

void World::calcPaddingRects() {
    WorldDim world = getWorldDim();
    for (size_t i = 0; i < MPI_Nodes; i++) {
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

size_t World::rankAt(int px, int py) {
    //TODO could cause overflow for large worlds. Use long instead?
    Point p = {(px + overallWidth) % overallWidth, (py + overallHeight) % overallHeight};
    for (size_t i = 0; i < MPI_Nodes; i++) {
        if (pointInRect(p, worlds[i]))
            return i;
    }
    return -1; // In case there's no matching world
}

std::vector<size_t> *World::paddingRanksAt(int px, int py) {
    assert(px >= x && px < x + width && py >= y && py < y + height && "Coordinates NOT on THIS node");

    auto *ranks = new std::vector<size_t>();
    for (const auto &p : paddingRects)
        if (pointInRect({px, py}, p.rect))
            ranks->push_back(p.rank);

    return ranks;
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
