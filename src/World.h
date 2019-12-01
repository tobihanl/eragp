#ifndef ERAGP_MAIMUC_EVO_2019_WORLD_H
#define ERAGP_MAIMUC_EVO_2019_WORLD_H

#include <algorithm>
#include <vector>
#include <mpi.h>
#include "FoodEntity.h"
#include "LivingEntity.h"
#include "Tile.h"

#define TILE_SIZE 8
#define NUMBER_OF_MAIMUC_NODES 10
#define WORLD_PADDING (7 * TILE_SIZE)    // must be a multiple of TILE_SIZE!

#define VIEW_RANGE_SQUARED 25600 //160*160
#define VIEW_RANGE 160

#define ENEMY_MATE_SQUARED_DIFFERENCE_THRESHOLD 0.0016

#define MAX_FOOD_INTERVAL 1000000 //Can be much bigger because it is equally distributed

#define MSGS_PER_NEIGHBOR 3

#define MPI_TAG_LIVING_ENTITY 42
#define MPI_TAG_FOOD_ENTITY 50
#define MPI_TAG_REMOVED_FOOD_ENTITY 51

//================================== Structs ==================================
struct MPISendEntity {
    int rank;
    Entity *entity;
};

struct Point {
    int x = 0;
    int y = 0;
};

struct Rect {
    struct Point p;
    int w = 0;
    int h = 0;
};

struct PaddingRect {
    int rank = 0;
    struct Rect rect;
};

struct NearestLiving {
    LivingEntity *mate;
    LivingEntity *enemy;
};

typedef struct Rect WorldDim;

//=================================== Class ===================================
class World {
private:
    static int MPI_Rank;
    static int MPI_Nodes;

    static int x;
    static int y;
    static int width;
    static int height;

    static int ticksPerFoodInterval; //ticks per interval
    static int foodPerFoodInterval; //food to distribute over every interval (without the food spawned every tick)
    static int intervalTicksLeft; //remaining ticks in the current interval
    static int intervalFoodLeft; //remaining food to be distributed over the current interval (without the food spawned every tick)
    static int foodEveryTick; //amount of food to spawn every tick (if node spawnRate is bigger than 1)
    static int ticksToSkip; //ticks to skip until next food (not considering foodEveryTick) is spawned
    static int minTicksToSkip;
    static int maxTicksToSkip;

    static bool isSetup;

    //TODO change list implementation and handle shared data
    static std::vector<FoodEntity *> food; //Currently saved by copy, because they should only be here, so looping and accessing attributes (e.g. findNearest) is more cache efficient
    static std::vector<LivingEntity *> living;

    static std::vector<FoodEntity *> removeFood;
    static std::vector<LivingEntity *> removeLiving;
    static std::vector<FoodEntity *> addFood;
    static std::vector<LivingEntity *> addLiving;

    // MPI Sending storage
    static std::vector<MPISendEntity> livingEntitiesToMoveToNeighbors;
    static std::vector<MPISendEntity> foodToSendToNeighbors;
    static std::vector<MPISendEntity> removedFoodToSendToNeighbors;

    static WorldDim *worlds;
    static std::vector<PaddingRect> paddingRects;
    static std::vector<int> paddingRanks;

    static std::vector<Tile *> terrain;

    World() = default;

    ~World() = default;

public:
    static int overallWidth;
    static int overallHeight;

    static SDL_Texture *background;
    static SDL_Texture *entities;
    static SDL_Texture *rankTexture;

    /**
     * Initialize the world, which is part of the overall world and set
     * it up.
     *
     * @param   newOverallWidth     Width of the overall world
     * @param   newOverallHeight    Height of the overall world
     * @param   maimuc              Indicates, whether the program is executed
     *                              on MaiMUC or not
     */
    static void setup(int newOverallWidth, int newOverallHeight, bool maimuc, float foodRate, float zoom);

    static void finalize();

    static int getMPIRank() { return MPI_Rank; }

    static int getMPINodes() { return MPI_Nodes; }

    static WorldDim getWorldDim() { return getWorldDimOf(MPI_Rank); }

    static WorldDim getWorldDimOf(int rank) { return worlds[rank]; }

    static SDL_Texture *renderTerrain();

    static void render();

    static void tick();

    //non-surviving functions: always base thinking input on last tick, bot some kind of half-tick
    static FoodEntity *findNearestFood(int px, int py, bool surviving);

    static NearestLiving findNearestLiving(LivingEntity *le, bool surviving);

    static LivingEntity *findNearestLivingToPoint(int px, int py);

    static bool addLivingEntity(LivingEntity *e, bool received);

    static bool addFoodEntity(FoodEntity *e, bool received);

    static bool removeLivingEntity(LivingEntity *e);

    static bool removeFoodEntity(FoodEntity *e, bool received);

    static bool toRemoveFood(FoodEntity *e) {
        return std::find(removeFood.begin(), removeFood.end(), e) != removeFood.end();
    }

    static bool toRemoveLiving(LivingEntity *e) {
        return std::find(removeLiving.begin(), removeLiving.end(), e) != removeLiving.end();
    }

    static Tile *tileAt(int px, int py);

    static std::vector<PaddingRect> *getPaddingRects() { return &paddingRects; }

    static int getAmountOfLivings() { return living.size(); }

    static int getAmountOfFood() { return food.size(); }

private:
    static void calcWorldDimensions(WorldDim *dims, int rankStart, int rankEnd, int px, int py, int w, int h);

    static void calcPaddingRects();

    static void generateTerrain(float zoom);

    static bool toAddLiving(LivingEntity *e) {
        return std::find(addLiving.begin(), addLiving.end(), e) != addLiving.end();
    }

    static bool toAddFood(FoodEntity *e) {
        return std::find(addFood.begin(), addFood.end(), e) != addFood.end();
    }

    static size_t rankAt(int px, int py);

    /**
     * @return      Ranks having a padding on the given coordinates
     *
     * @attention   Only works for (x,y) coordinates on THIS node!
     */
    static std::vector<size_t> *paddingRanksAt(int px, int py);

    static void *sendEntities(const std::vector<MPISendEntity> &entityVec, int rank, int tag, MPI_Request *request);

    static void receiveEntities(int rank, int tag);

    static long gcd(long a, long b) {
        if (a == 0) return b;
        if (b == 0) return a;
        if (a < b) return gcd(a, b % a);
        return gcd(b, a % b);
    }

    static bool pointInRect(const Point &p, const Rect &r) {
        return r.p.x <= p.x && p.x < r.p.x + r.w && r.p.y <= p.y && p.y < r.p.y + r.h;
    }

    static Rect calcIntersection(const Rect &rect1, const Rect &rect2) {
        int x1 = std::max(rect1.p.x, rect2.p.x);
        int y1 = std::max(rect1.p.y, rect2.p.y);
        int x2 = std::min(rect1.p.x + rect1.w, rect2.p.x + rect2.w);
        int y2 = std::min(rect1.p.y + rect1.h, rect2.p.y + rect2.h);

        return {{x1, y1}, x2 - x1, y2 - y1};
    }
};

#endif //ERAGP_MAIMUC_EVO_2019_WORLD_H
