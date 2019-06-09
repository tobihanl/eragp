
#ifndef ERAGP_MAIMUC_EVO_2019_WORLD_H
#define ERAGP_MAIMUC_EVO_2019_WORLD_H

#include <vector>
#include "FoodEntity.h"
#include "LivingEntity.h"

class World {
private:
    //TODO change list implementation and handle shared data
    std::vector<FoodEntity> food;
    std::vector<LivingEntity> living;
public:
    World();

    void render();
    void tick();

};


#endif //ERAGP_MAIMUC_EVO_2019_WORLD_H
