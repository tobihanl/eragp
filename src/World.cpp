#include <algorithm>
#include <cstdlib>
#include <cassert>
#include "mpi.h"
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
std::vector<MPISendEntity> World::foodToSendToNeighbors = std::vector<MPISendEntity>();
std::vector<MPISendEntity> World::removedFoodToSendToNeighbors = std::vector<MPISendEntity>();

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
    int heightWithPadding = World::height + (2 * WORLD_PADDING);
    int widthWithPadding = World::width + (2 * WORLD_PADDING);

    terrain.reserve((heightWithPadding / TILE_SIZE) * (widthWithPadding / TILE_SIZE));

    int xOffset = World::x - WORLD_PADDING;
    int yOffset = World::y - WORLD_PADDING;
    if (xOffset < 0) xOffset += World::overallWidth;
    if (yOffset < 0) yOffset += World::overallHeight;

    for (int y = 0; y < heightWithPadding / TILE_SIZE; y++) {
        for (int x = 0; x < widthWithPadding / TILE_SIZE; x++) {
            int pointX = (x * TILE_SIZE) + xOffset;
            int pointY = (y * TILE_SIZE) + yOffset;

            // Overlap?
            if (pointX >= World::overallWidth) pointX -= World::overallWidth;
            if (pointY >= World::overallHeight) pointY -= World::overallHeight;

            // Noise
            float val = SimplexNoise::noise((float) pointX / (36.f * TILE_SIZE),
                                            (float) pointY / (36.f * TILE_SIZE));

            if (val < -0.4) {
                terrain[y * (widthWithPadding / TILE_SIZE) + x] = &Tile::WATER;
            } else if (val < -0.2) {
                terrain[y * (widthWithPadding / TILE_SIZE) + x] = &Tile::SAND;
            } else if (val < 0.7) {
                terrain[y * (widthWithPadding / TILE_SIZE) + x] = &Tile::GRASS;
            } else {
                terrain[y * (widthWithPadding / TILE_SIZE) + x] = &Tile::STONE;
            }
        }
    }
}

void World::renderTerrain() {
    int heightWithPadding = World::height + (2 * WORLD_PADDING);
    int widthWithPadding = World::width + (2 * WORLD_PADDING);

    // Pre-render terrain for faster rendering
    World::background = Renderer::createTexture(widthWithPadding, heightWithPadding, SDL_TEXTUREACCESS_TARGET);
    Renderer::setTarget(World::background);
    Renderer::clear();

    // Copy textures to background
    for (int y = 0; y < heightWithPadding / TILE_SIZE; y++) {
        for (int x = 0; x < widthWithPadding / TILE_SIZE; x++) {
            Renderer::copy(terrain[y * (widthWithPadding / TILE_SIZE) + x]->texture,
                           x * TILE_SIZE,
                           y * TILE_SIZE);
        }
    }

    // Change render target back to default
    Renderer::present();
    Renderer::setTarget(nullptr);
}

void World::render() {
    // "TILE_SIZE - (WORLD_PADDING % TILE_SIZE)": shifting, because in padding area a tile mustn't fit perfectly
    // into it. This shift is the overlap from the padding area.
    if (World::background == nullptr) renderTerrain();
    Renderer::copy(World::background,
                   -WORLD_PADDING - (TILE_SIZE - (WORLD_PADDING % TILE_SIZE)),
                   -WORLD_PADDING - (TILE_SIZE - (WORLD_PADDING % TILE_SIZE)));

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
        // Before moving: Is entity on THIS node?
        bool beforeOnThisNode = !(e->x < x || e->x >= x + width || e->y < y || e->y >= y + height);

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
        if (e->x >= x && e->x < x + width && e->y >= y && e->y < y + height) {
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
    MPI_Request reqs[numOfNeighbors() * MSGS_PER_NEIGHBOR];
    MPI_Status stats[numOfNeighbors() * MSGS_PER_NEIGHBOR];
    void *buffers[numOfNeighbors() * MSGS_PER_NEIGHBOR];

    //############################# SEND ENTITIES #############################
    for (int i = 0; i < numOfNeighbors(); i++) {
        buffers[MSGS_PER_NEIGHBOR * i] = sendEntities(livingEntitiesToMoveToNeighbors, neighbors[i],
                                                      MPI_TAG_LIVING_ENTITY,
                                                      &reqs[MSGS_PER_NEIGHBOR * i]);
        buffers[MSGS_PER_NEIGHBOR * i + 1] = sendEntities(foodToSendToNeighbors, neighbors[i],
                                                          MPI_TAG_FOOD_ENTITY,
                                                          &reqs[MSGS_PER_NEIGHBOR * i + 1]);
        buffers[MSGS_PER_NEIGHBOR * i + 2] = sendEntities(removedFoodToSendToNeighbors, neighbors[i],
                                                          MPI_TAG_REMOVED_FOOD_ENTITY,
                                                          &reqs[MSGS_PER_NEIGHBOR * i + 2]);
    }

    // Clear sending storage
    livingEntitiesToMoveToNeighbors.clear();
    foodToSendToNeighbors.clear();
    removedFoodToSendToNeighbors.clear();

    //############################ RECEIVE ENTITIES ###########################
    for (int i = 0; i < numOfNeighbors(); i++) {
        receiveEntities(neighbors[i], MPI_TAG_LIVING_ENTITY);
        receiveEntities(neighbors[i], MPI_TAG_FOOD_ENTITY);
        receiveEntities(neighbors[i], MPI_TAG_REMOVED_FOOD_ENTITY);
    }

    // Wait for all sends to complete and afterwards free buffers
    MPI_Waitall(numOfNeighbors() * MSGS_PER_NEIGHBOR, reqs, stats);
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
                addLivingEntity((LivingEntity *) e, false);
                break;

            case MPI_TAG_FOOD_ENTITY:
                e = new FoodEntity(buffer);
                addFoodEntity((FoodEntity *) e);
                break;

            case MPI_TAG_REMOVED_FOOD_ENTITY:
                e = new FoodEntity(buffer);
                for (const auto &f : food) {
                    if (e->getId() == f->getId() && !toRemoveFood(f))
                        removeFoodEntity(f);
                }
                delete e;
                break;

            default:
                break;
        }
    }

    free(start);
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

void World::addLivingEntity(LivingEntity *e, bool spawned) {
    if (toAddLiving(e)) return;

    // Entity sent to this node?
    if (!spawned) {
        addLiving.push_back(e);
        return;
    }

    // If on THIS node: add entity and send to neighbors with padding area
    if (e->x >= x && e->x < x + width && e->y >= y && e->y < y + height) {
        addLiving.push_back(e);
        auto *ranks = paddingRanksAt(e->x, e->y);
        for (int neighbor : *ranks)
            livingEntitiesToMoveToNeighbors.push_back({neighbor, e});
        delete ranks;
    }
}

void World::addFoodEntity(FoodEntity *e) {
    if (toAddFood(e)) return;
    addFood.push_back(e);

    // If on THIS node: entity in padding of some neighbors?
    if (e->x >= x && e->x < x + width && e->y >= y && e->y < y + height) {
        auto *ranks = paddingRanksAt(e->x, e->y);
        for (int neighbor : *ranks)
            foodToSendToNeighbors.push_back({neighbor, e});
        delete ranks;
    }
}

void World::removeLivingEntity(LivingEntity *e) {
    if (!toRemoveLiving(e))
        removeLiving.push_back(e);
}

void World::removeFoodEntity(FoodEntity *e) {
    assert(!toRemoveFood(e) && "Tried to remove same FoodEntity multiple times");
    removeFood.push_back(e);

    // If on THIS node: entity in padding of some neighbors?
    if (e->x >= x && e->x < x + width && e->y >= y && e->y < y + height) {
        auto *ranks = paddingRanksAt(e->x, e->y);
        for (int neighbor : *ranks)
            removedFoodToSendToNeighbors.push_back({neighbor, e});
        delete ranks;
    }
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
        if (i == MPI_Rank) continue; // Node doesn't need to know that it's its own neighbor!
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

int World::numOfNeighbors() {
    return neighbors.size();
}

size_t World::rankAt(int x, int y) {
    x = (x + overallWidth) % overallWidth; //TODO could cause overflow for large worlds. Use long instead?
    y = (y + overallHeight) % overallHeight;

    for (size_t i = 0; i < worlds.size(); i++) {
        if (worlds[i].x <= x && x < worlds[i].x + worlds[i].w && worlds[i].y <= y && y < worlds[i].y + worlds[i].h)
            return i;
    }
    return -1; // In case there's no matching world
}

/**
 * @return      Ranks having a padding on the given coordinates
 *
 * @attention   Only works for (x,y) coordinates on THIS node!
 */
std::vector<size_t> *World::paddingRanksAt(int x, int y) {
    assert(x >= World::x && x < World::x + World::width && y >= World::y && y < World::y + World::height &&
           "Coordinates NOT on THIS node");

    int shiftedX, shiftedY;
    auto *ranks = new std::vector<size_t>();
    for (int neighbor : neighbors) {
        WorldDim dim = worlds[neighbor];
        shiftedX = x;
        shiftedY = y;

        // Shift X coordinate
        if (dim.x == 0 && x + WORLD_PADDING >= overallWidth)                // Overlap?
            shiftedX = (x + WORLD_PADDING) % overallWidth;
        else if (dim.x + dim.w == overallWidth && x - WORLD_PADDING < 0)    // Overlap?
            shiftedX = (x - WORLD_PADDING + overallWidth) % overallWidth;
        else if (x < dim.x)
            shiftedX = (x + WORLD_PADDING) % overallWidth;
        else if (x >= dim.x + dim.w)
            shiftedX = (x - WORLD_PADDING + overallWidth) % overallWidth;

        // Shift Y coordinate
        if (dim.y == 0 && y + WORLD_PADDING >= overallHeight)               // Overlap?
            shiftedY = (y + WORLD_PADDING) % overallHeight;
        else if (dim.y + dim.h == overallHeight && y - WORLD_PADDING < 0)   // Overlap?
            shiftedY = (y - WORLD_PADDING + overallHeight) % overallHeight;
        else if (y < dim.y)
            shiftedY = (y + WORLD_PADDING) % overallHeight;
        else if (y >= dim.y + dim.h)
            shiftedY = (y - WORLD_PADDING + overallHeight) % overallHeight;

        // Point shifted into the world?
        if (shiftedX >= dim.x && shiftedX < dim.x + dim.w && shiftedY >= dim.y && shiftedY < dim.y + dim.h)
            ranks->push_back(neighbor);
    }

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

Tile *World::tileAt(int x, int y) {
    if (x < World::x - WORLD_PADDING || x >= World::width + WORLD_PADDING ||
        y < World::y - WORLD_PADDING || y >= World::height + WORLD_PADDING) {
        return &Tile::INVALID;
    } else {
        x = x - World::x + WORLD_PADDING;
        y = y - World::y + WORLD_PADDING;

        int heightWithPadding = World::height + (2 * WORLD_PADDING);
        int widthWithPadding = World::width + (2 * WORLD_PADDING);

        // Coordinates at border, where a tile doesn't have enough space?
        if (x + (WORLD_PADDING % TILE_SIZE) >= widthWithPadding || y + (WORLD_PADDING % TILE_SIZE) >= heightWithPadding)
            return &Tile::INVALID;

        // TODO: Fix bug that causes a segmentation fault in the line below!
        //return terrain[(y / TILE_SIZE) * (widthWithPadding / TILE_SIZE) + (x / TILE_SIZE)];
        return &Tile::INVALID;
    }
}

//TODO cleanup for destroyed entities
