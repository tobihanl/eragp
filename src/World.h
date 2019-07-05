#ifndef ERAGP_MAIMUC_EVO_2019_WORLD_H
#define ERAGP_MAIMUC_EVO_2019_WORLD_H

#include <vector>
#include "FoodEntity.h"
#include "LivingEntity.h"

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
    static std::vector<FoodEntity> food;
    static std::vector<LivingEntity> living;

    World() = default;

    ~World() = default;

public:
    static void setup(int overallWidth, int overallHeight);

    static void render();

    static void tick();

    static WorldDim calcWorldDimensions(int rank, int num);

    static LivingEntity getNearestEntity(LivingEntity entity);
};

#endif //ERAGP_MAIMUC_EVO_2019_WORLD_H
