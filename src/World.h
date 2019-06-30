
#ifndef ERAGP_MAIMUC_EVO_2019_WORLD_H
#define ERAGP_MAIMUC_EVO_2019_WORLD_H

#include <vector>
#include "FoodEntity.h"
#include "LivingEntity.h"

class World {
private:
    //TODO change list implementation and handle shared data
    std::vector<FoodEntity> food; //Currently saved by copy, because they should only be here, so looping and accessing attributes (e.g. findNearest) is more cache efficient
    std::vector<LivingEntity> living;
public:
    World();

    void render();
    void tick();

    FoodEntity &findNearestFood(int x, int y);

};


#endif //ERAGP_MAIMUC_EVO_2019_WORLD_H
