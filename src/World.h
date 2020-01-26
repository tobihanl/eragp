#ifndef ERAGP_MAIMUC_EVO_2019_WORLD_H
#define ERAGP_MAIMUC_EVO_2019_WORLD_H

#include <algorithm>
#include <list>
#include <vector>
#include <list>
#include <mpi.h>
#include "FoodEntity.h"
#include "LivingEntity.h"
#include "Tile.h"
#include "Structs.h"
#include "Constants.h"

//================================== Structs ==================================
struct MPISendEntity {
    int rank;
    bool minimal;
    Entity *entity;
};

struct NearestLiving {
    LivingEntity *mate;
    LivingEntity *enemy;
};

struct Food2Add {
    int tick;
    FoodEntity *entity;
};

//=================================== Class ===================================
class World {
private:
    static int MPI_Rank;
    static int MPI_Nodes;

    static int x;
    static int y;
    static int width;
    static int height;

    static int numThreads;

    static int ticksPerFoodInterval; //ticks per interval
    static int foodPerFoodInterval; //food to distribute over every interval (without the food spawned every tick)
    static int intervalTicksLeft; //remaining ticks in the current interval
    static int intervalFoodLeft; //remaining food to be distributed over the current interval (without the food spawned every tick)
    static int foodEveryTick; //amount of food to spawn every tick (if node spawnRate is bigger than 1)
    static int ticksToSkip; //ticks to skip until next food (not considering foodEveryTick) is spawned
    static int minTicksToSkip;
    static int maxTicksToSkip;

    static bool isSetup;

    static std::list<Food2Add> addFoodBuffer;

    static std::vector<FoodEntity *> removeFood;
    static std::vector<FoodEntity *> addFood;
    static std::vector<LivingEntity *> addLiving;

    static std::list<LivingEntity *> **livingBuckets;
    static std::list<FoodEntity *> **foodBuckets;

    // MPI Sending storage
    static std::vector<MPISendEntity> livingEntitiesToMoveToNeighbors;
    static std::vector<MPISendEntity> foodToSendToNeighbors;
    static std::vector<MPISendEntity> removedFoodToSendToNeighbors;

    static WorldDim *worlds;
    static std::vector<PaddingRect> paddingRects;
    static std::vector<int> paddingRanks;

    static int remainingTicksInFoodBuffer;
    static LFSR random;

    static int xChunks;
    static int yChunks;

    World() = default;

    ~World() = default;

public:
    static int overallWidth;
    static int overallHeight;
    static int dangerZone; //if distance to border is <= dangerZone, the danger neuron will have value (dist / dangerZone)

    static std::vector<Tile *> terrain;

    static std::vector<FoodEntity *> food; //Currently saved by copy, because they should only be here, so looping and accessing attributes (e.g. findNearest) is more cache efficient
    static std::vector<LivingEntity *> living;
    static std::vector<LivingEntity *> livingsInPadding;
    /**
     * Initialize the world, which is part of the overall world and set
     * it up.
     *
     * @param   newOverallWidth     Width of the overall world
     * @param   newOverallHeight    Height of the overall world
     * @param   maimuc              Indicates, whether the program is executed
     *                              on MaiMUC or not
     */
    static void setup(int newOverallWidth, int newOverallHeight, bool maimuc, float foodRate, float zoom, uint32_t seed,
                      int numThreads);

    static void finalize();

    static int getMPIRank() { return MPI_Rank; }

    static int getMPINodes() { return MPI_Nodes; }

    static WorldDim getWorldDim() { return getWorldDimOf(MPI_Rank); }

    static WorldDim getWorldDimOf(int rank) { return worlds[rank]; }

    static void tick();

    //non-surviving functions: always base thinking input on last tick, bot some kind of half-tick
    static FoodEntity *findNearestFood(int px, int py, bool surviving);

    static NearestLiving findNearestLiving(LivingEntity *le, bool surviving);

    static LivingEntity *findNearestLivingToPoint(int px, int py);

    static bool addLivingEntity(LivingEntity *e, bool send);

    static bool addFoodEntity(FoodEntity *e, bool received);

    static bool removeFoodEntity(FoodEntity *e, bool received);

    static bool toRemoveFood(FoodEntity *e) {
        return std::find(removeFood.begin(), removeFood.end(), e) != removeFood.end();
    }

    static size_t rankAt(int px, int py);

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

    /**
     * @return      Ranks having a padding on the given coordinates
     *
     * @attention   Only works for (x,y) coordinates on THIS node!
     */
    static std::vector<size_t> *paddingRanksAt(int px, int py);

    static void *sendEntities(const std::vector<MPISendEntity> &entityVec, int rank, int tag, MPI_Request *request);

    static void receiveEntities(int rank, int tag);

    static void fillFoodBuffer();

    template<typename T>
    static T getEntityBucket(const Point &p, const T *buckets) {
        int ix = (p.x + WORLD_PADDING - x) / CHUNK_SIZE, iy = (p.y + WORLD_PADDING - y) / CHUNK_SIZE;
        return (ix < 0 || iy < 0 || ix >= xChunks || iy >= yChunks) ? nullptr : &buckets[ix][iy];
    }

    template<typename T>
    static void searchBucketsForNearestEntity(Point entityPos, T searchBucketFunc);

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
