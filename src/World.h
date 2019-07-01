
#ifndef ERAGP_MAIMUC_EVO_2019_WORLD_H
#define ERAGP_MAIMUC_EVO_2019_WORLD_H

#include <vector>
#include "FoodEntity.h"
#include "LivingEntity.h"

#define TILE_SIZE 8

class World {
private:
    //TODO change list implementation and handle shared data
    static std::vector<FoodEntity*> food; //Currently saved by copy, because they should only be here, so looping and accessing attributes (e.g. findNearest) is more cache efficient
    static std::vector<LivingEntity*> living;
public:
    static void render();
    static void tick();

    static FoodEntity *findNearestFood(int x, int y);
    static LivingEntity *findNearestLiving(int x, int y);
    static void addLivingEntity(LivingEntity *e);
    static void addFoodEntity(FoodEntity *e);
};


#endif //ERAGP_MAIMUC_EVO_2019_WORLD_H
