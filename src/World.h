
#ifndef ERAGP_MAIMUC_EVO_2019_WORLD_H
#define ERAGP_MAIMUC_EVO_2019_WORLD_H

#include <vector>
#include "FoodEntity.h"
#include "LivingEntity.h"

#define WORLD_WIDTH 960
#define WORLD_HEIGHT 720
#define TILE_SIZE 8


class World {
private:
    //TODO change list implementation and handle shared data
    static std::vector<FoodEntity*> food; //Currently saved by copy, because they should only be here, so looping and accessing attributes (e.g. findNearest) is more cache efficient
    static std::vector<LivingEntity*> living;

    static std::vector<FoodEntity*> removeFood;
    static std::vector<LivingEntity*> removeLiving;
    static std::vector<FoodEntity*> addFood;
    static std::vector<LivingEntity*> addLiving;
public:
    static void render();
    static void tick();

    static FoodEntity *findNearestFood(int x, int y);
    static LivingEntity *findNearestLiving(int x, int y);
    static void addLivingEntity(LivingEntity *e);
    static void addFoodEntity(FoodEntity *e);
    static void removeLivingEntity(LivingEntity *e);
    static void removeFoodEntity(FoodEntity *e);

private:
    static bool toRemoveLiving(LivingEntity *e);

    static bool toAddLiving(LivingEntity *e);
    static bool toRemoveFood(FoodEntity *e);

    static bool toAddFood(FoodEntity *e);
};


#endif //ERAGP_MAIMUC_EVO_2019_WORLD_H
