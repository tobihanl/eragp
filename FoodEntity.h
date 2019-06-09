
#ifndef ERAGP_MAIMUC_EVO_2019_FOODENTITY_H
#define ERAGP_MAIMUC_EVO_2019_FOODENTITY_H

#include "Entity.h"

class FoodEntity : public Entity {
private:

public:
    FoodEntity(int x, int y);
    void render() override;
    void tick() override;
};


#endif //ERAGP_MAIMUC_EVO_2019_FOODENTITY_H
