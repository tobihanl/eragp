
#ifndef ERAGP_MAIMUC_EVO_2019_WORLD_H
#define ERAGP_MAIMUC_EVO_2019_WORLD_H

#include <vector>
#include "FoodEntity.h"
#include "LivingEntity.h"

class World {
public:
    static int overallWidth;
    static int overallHeight;

private:
    int MPI_Rank;
    int MPI_Nodes;

    int x;
    int y;
    int width;
    int height;

    //TODO change list implementation and handle shared data
    std::vector<FoodEntity> food;
    std::vector<LivingEntity> living;
public:
    World();

    void render();

    void tick();

    LivingEntity getNearestEntity(LivingEntity entity);
};

#endif //ERAGP_MAIMUC_EVO_2019_WORLD_H
