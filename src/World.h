#ifndef ERAGP_MAIMUC_EVO_2019_WORLD_H
#define ERAGP_MAIMUC_EVO_2019_WORLD_H

#include <vector>
#include "mpi.h"
#include "FoodEntity.h"
#include "LivingEntity.h"
#include "Tile.h"

#define TILE_SIZE 8
#define NUMBER_OF_MAIMUC_NODES 10
#define WORLD_PADDING 50

#define MSGS_PER_NEIGHBOR 3

#define MPI_TAG_LIVING_ENTITY 42
#define MPI_TAG_FOOD_ENTITY 50
#define MPI_TAG_REMOVED_FOOD_ENTITY 51

//================================== Structs ==================================
struct WorldDim {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
};

struct MPISendEntity {
    int rank;
    Entity *entity;
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

    static bool isSetup;

    static SDL_Texture *background;

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

    static std::vector<WorldDim> worlds;
    static std::vector<int> neighbors;

    static std::vector<Tile *> terrain;

    World() = default;

    ~World() = default;

public:
    static int overallWidth;
    static int overallHeight;

    static void setup(int overallWidth, int overallHeight, bool maimuc);

    static int getMPIRank();

    static int getMPINodes();

    static WorldDim getWorldDim();

    static WorldDim getWorldDimOf(int rank);

    static void render();

    static void tick();

    static FoodEntity *findNearestFood(int x, int y);

    static FoodEntity *findNearestSurvivingFood(int x, int y);

    static LivingEntity *findNearestLiving(LivingEntity *le);

    static LivingEntity *findNearestLiving(int x, int y, int id);

    static LivingEntity *findNearestEnemy(LivingEntity *le);

    static LivingEntity *findNearestMate(LivingEntity *le);

    static void addLivingEntity(LivingEntity *e, bool spawned);

    static void addFoodEntity(FoodEntity *e);

    static void removeLivingEntity(LivingEntity *e);

    static void removeFoodEntity(FoodEntity *e);

    static bool toRemoveFood(FoodEntity *e);

    static Tile *tileAt(int x, int y);

private:
    static WorldDim calcWorldDimensions(int rank, int num);

    static void generateTerrain();

    static void renderTerrain();

    static bool toRemoveLiving(LivingEntity *e);

    static bool toAddLiving(LivingEntity *e);

    static bool toAddFood(FoodEntity *e);

    static size_t rankAt(int x, int y);

    static std::vector<size_t> *paddingRanksAt(int x, int y);

    static void *sendEntities(const std::vector<MPISendEntity> &entities, int rank, int tag, MPI_Request *request);

    static void receiveEntities(int rank, int tag);

    static void calcNeighbors();

    static int numOfNeighbors();
};

#endif //ERAGP_MAIMUC_EVO_2019_WORLD_H
