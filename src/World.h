#ifndef ERAGP_MAIMUC_EVO_2019_WORLD_H
#define ERAGP_MAIMUC_EVO_2019_WORLD_H

#include <vector>
#include "FoodEntity.h"
#include "LivingEntity.h"
#include "Tile.h"

#define TILE_SIZE 8


struct WorldDim {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
};

class World {
private:
    static int overallWidth;
    static int overallHeight;

    static int MPI_Rank;
    static int MPI_Nodes;

    static int x;
    static int y;
    static int width;
    static int height;

    static bool isSetup;

    //TODO change list implementation and handle shared data
    static std::vector<FoodEntity *> food; //Currently saved by copy, because they should only be here, so looping and accessing attributes (e.g. findNearest) is more cache efficient
    static std::vector<LivingEntity *> living;

    static std::vector<FoodEntity *> removeFood;
    static std::vector<LivingEntity *> removeLiving;
    static std::vector<FoodEntity *> addFood;
    static std::vector<LivingEntity *> addLiving;

    static std::vector<Tile *> terrain;

    World() = default;

    ~World() = default;

public:
    static void generateTerrain();

    static void setup(int overallWidth, int overallHeight, bool maimuc);

    static WorldDim calcWorldDimensions(int rank, int num);

    static WorldDim getWorldDim();

    static void render();
    static void tick();

    static FoodEntity *findNearestFood(int x, int y);

    static FoodEntity *findNearestSurvivingFood(int x, int y);

    static LivingEntity *findNearestLiving(LivingEntity *le);

    static LivingEntity *findNearestLiving(int x, int y, int id);

    static LivingEntity *findNearestEnemy(LivingEntity *le);

    static LivingEntity *findNearestMate(LivingEntity *le);

    static void addLivingEntity(LivingEntity *e);

    static void addFoodEntity(FoodEntity *e);

    static void removeLivingEntity(LivingEntity *e);

    static void removeFoodEntity(FoodEntity *e);

    static bool toRemoveFood(FoodEntity *e);

    static Tile *tileAt(int x, int y);

private:
    static bool toRemoveLiving(LivingEntity *e);

    static bool toAddLiving(LivingEntity *e);

    static bool toAddFood(FoodEntity *e);
};

#endif //ERAGP_MAIMUC_EVO_2019_WORLD_H
