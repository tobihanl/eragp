#ifndef ERAGP_MAIMUC_EVO_2019_WORLD_H
#define ERAGP_MAIMUC_EVO_2019_WORLD_H

#include <vector>
#include "mpi.h"
#include "FoodEntity.h"
#include "LivingEntity.h"
#include "Tile.h"

#define TILE_SIZE 8
#define NUMBER_OF_MAIMUC_NODES 10
#define WORLD_PADDING (7 * TILE_SIZE)    // must be a multiple of TILE_SIZE!

#define VIEW_RANGE_SQUARED 25600 //160*160
#define VIEW_RANGE 160

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

    static void setup(int newOverallWidth, int newOverallHeight, bool maimuc, float foodRate);

    static void finalize();

    static int getMPIRank();

    static int getMPINodes();

    static WorldDim getWorldDim();

    static WorldDim getWorldDimOf(int rank);

    static SDL_Texture *renderTerrain();

    static void render();

    static void tick();

    static FoodEntity *findNearestFood(int px, int py);

    static FoodEntity *findNearestSurvivingFood(int px, int py);

    static LivingEntity *findNearestLiving(int px, int py, int id);

    static LivingEntity *findNearestEnemy(LivingEntity *le);

    //non-surviving functions: always base thinking input on last tick, bot some kind of half-tick
    static LivingEntity *findNearestSurvivingEnemy(LivingEntity *le);

    static LivingEntity *findNearestMate(LivingEntity *le);

    static LivingEntity *findNearestSurvivingMate(LivingEntity *le);

    static bool addLivingEntity(LivingEntity *e, bool received);

    static bool addFoodEntity(FoodEntity *e, bool received);

    static bool removeLivingEntity(LivingEntity *e);

    static bool removeFoodEntity(FoodEntity *e, bool received);

    static bool toRemoveFood(FoodEntity *e);

    static bool toRemoveLiving(LivingEntity *e);

    static Tile *tileAt(int px, int py);

    static std::vector<PaddingRect> *getPaddingRects();

private:
    static void calcWorldDimensions(WorldDim *dims, int rankStart, int rankEnd, int px, int py, int w, int h);

    static void calcPaddingRects();

    static void generateTerrain();

    static bool toAddLiving(LivingEntity *e);

    static bool toAddFood(FoodEntity *e);

    static size_t rankAt(int px, int py);

    static std::vector<size_t> *paddingRanksAt(int px, int py);

    static void *sendEntities(const std::vector<MPISendEntity> &entities, int rank, int tag, MPI_Request *request);

    static void receiveEntities(int rank, int tag);

    static long gcd(long a, long b);

    static bool pointInRect(const Point &p, const Rect &r);

    static Rect calcIntersection(const Rect &rect1, const Rect &rect2);
};

#endif //ERAGP_MAIMUC_EVO_2019_WORLD_H
